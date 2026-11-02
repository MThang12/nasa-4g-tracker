#include <stdint.h>
#include <stdlib.h>

#include "contiki.h"
#include "dev/ethos.h"
#include "net/netstack.h"
#include "net/routing/routing.h"
#include "net/ipv6/simple-udp.h"
#include "aboot-callback.h"
#include "aboot-transport.h"
#include "udp-transport.h"

#include "sys/log.h"
#define LOG_MODULE "Udp"
#define LOG_LEVEL LOG_LEVEL_INFO

#define min(a, b)    ((a) < (b) ? (a) : (b))

#define SPARSE_HEADER_MAGIC 0xed26ff3a

#define kClientPort         5553
#define kServerPort         5554
#define kProtocolVersion    1
#define kMinPacketSize      512
#define kHeaderSize         4
#define kHostMaxPacketSize  2048
#define kResponseTimeoutMs  500

/*---------------------------------------------------------------------------*/
enum udp_process_event {
  UDP_SEND_COMMAND,
  UDP_RECV_RESPONSE,
};
/*---------------------------------------------------------------------------*/
typedef enum {
  kIdError = 0x00,
  kIdDeviceQuery = 0x01,
  kIdInitialization = 0x02,
  kIdFastboot = 0x03,
  kIdMax = 0x04
} Id;

typedef enum {
  kFlagNone = 0x00,
  kFlagContinuation = 0x01
} Flag;

typedef enum {
  kIndexId    = 0,
  kIndexFlags = 1,
  kIndexSeqH  = 2,
  kIndexSeqL  = 3
} Index;

/* Packet header handling. */
typedef struct {
  uint8_t bytes[kHeaderSize];
} Header;
/*---------------------------------------------------------------------------*/
static uint16_t sequence;
static struct simple_udp_connection udp_conn;
static uint8_t udp_buffer[kHostMaxPacketSize];
static uint16_t udp_size;
static udp_transport_rx_cb_t udp_command_cb;
static udp_transport_rx_cb_t udp_data_cb;
static bool is_sparse_file;
static bool is_data_mode;
static uint16_t max_data_length;
static uint8_t *send_buffer;
static size_t send_total_size;
static size_t send_offset_read;
static size_t send_offset_write;
PROCESS(udp_transport_process, "UDP transport process");
/*---------------------------------------------------------------------------*/
static void
header_set(Header *header, uint8_t id, uint16_t sequence, Flag flag)
{
  header->bytes[kIndexId] = id;
  header->bytes[kIndexFlags] = flag;
  header->bytes[kIndexSeqH] = sequence >> 8;
  header->bytes[kIndexSeqL] = sequence;
}
/*---------------------------------------------------------------------------*/
static uint8_t
header_id(Header *header)
{
  return header->bytes[kIndexId];
}
/*---------------------------------------------------------------------------*/
/*
   static uint8_t
   header_flag(Header *header)
   {
     return header->bytes[kIndexFlags];
   }
 */
/*---------------------------------------------------------------------------*/
static int
header_matches(Header *header, const uint8_t *response)
{
  /* Sequence numbers must be the same to match, but the response ID can either be the same */
  /* or an error response which is always accepted. */
  return header->bytes[kIndexSeqH] == response[kIndexSeqH] &&
         header->bytes[kIndexSeqL] == response[kIndexSeqL] &&
         (header->bytes[kIndexId] == response[kIndexId] ||
          response[kIndexId] == kIdError);
}
/*---------------------------------------------------------------------------*/
/* Extracts a big-endian uint16_t from a byte array. */
static uint16_t
extract_uint16(const uint8_t *bytes)
{
  return ((uint16_t)(bytes[0]) << 8) | bytes[1];
}
/*---------------------------------------------------------------------------*/
static void
udp_rx_callback(struct simple_udp_connection *c,
                const uip_ipaddr_t *sender_addr,
                uint16_t sender_port,
                const uip_ipaddr_t *receiver_addr,
                uint16_t receiver_port,
                const uint8_t *data,
                uint16_t datalen)
{
  if(datalen < kHeaderSize) {
    LOG_ERR("protocol error: incomplete header\n");
    if(header_id((Header *)data) < kIdFastboot) {
      aboot_callback_start(-1, NULL);
    } else {
      aboot_callback_status(-1, ABOOT_TINY_STATUS_FAILED);
    }
  }

  if(header_matches((Header *)udp_buffer, data)) {
    memcpy(udp_buffer, data, datalen);
    udp_size = datalen;
    process_post_synch(&udp_transport_process, UDP_RECV_RESPONSE, NULL);
  }
}
/*---------------------------------------------------------------------------*/
static void
ethos_net_callback(const uint8_t *data, size_t len)
{
  int hdr_size = sizeof(struct uip_eth_hdr);

  if(len > hdr_size && len <= UIP_BUFSIZE) {
    memcpy(uip_buf, data + hdr_size, len - hdr_size);
    uip_len = len - hdr_size;
    tcpip_input();
  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_transport_process, ev, data)
{
  static struct etimer periodic_timer;
  uip_ipaddr_t dest_ipaddr;
  Header *header = (Header *)udp_buffer;

  PROCESS_BEGIN();

  /* Initialize UDP connection */
  simple_udp_register(&udp_conn, kClientPort, NULL,
                      kServerPort, udp_rx_callback);

  etimer_set(&periodic_timer, kResponseTimeoutMs);

  while(1) {
    PROCESS_YIELD();

    if(etimer_expired(&periodic_timer) || ev == UDP_SEND_COMMAND) {
      if(NETSTACK_ROUTING.node_is_reachable() && NETSTACK_ROUTING.get_root_ipaddr(&dest_ipaddr)) {
        simple_udp_sendto(&udp_conn, udp_buffer, udp_size, &dest_ipaddr);
      }
      etimer_restart(&periodic_timer);
    } else if(ev == UDP_RECV_RESPONSE) {
      etimer_stop(&periodic_timer);
      if(header_id(header) == kIdError) {
        aboot_callback_status(-1, ABOOT_TINY_STATUS_FAILED);
        continue;
      } else if(header_id(header) == kIdDeviceQuery) {
        if(udp_size - kHeaderSize < 2) {
          LOG_ERR("invalid query response from target\n");
          aboot_callback_start(-1, NULL);
          continue;
        }
        /* The first two bytes contain the next expected sequence number. */
        sequence = extract_uint16(udp_buffer + kHeaderSize);
        header_set(header, kIdInitialization, sequence++, kFlagNone);
        /* Now send the initialization packet with our version and maximum packet size. */
        uint8_t init_data[] = { kProtocolVersion >> 8, kProtocolVersion & 0xFF,
                                kHostMaxPacketSize >> 8, kHostMaxPacketSize & 0xFF };
        memcpy(udp_buffer + kHeaderSize, init_data, sizeof(init_data));
        udp_size = kHeaderSize + sizeof(init_data);
        process_post(PROCESS_CURRENT(), UDP_SEND_COMMAND, NULL);
      } else if(header_id(header) == kIdInitialization) {
        if(udp_size - kHeaderSize < 4) {
          LOG_ERR("invalid initialization response from target\n");
          aboot_callback_start(-1, NULL);
          continue;
        }
        /* The first two data bytes contain the version, the second two bytes contain the target max */
        /* supported packet size, which must be at least 512 bytes. */
        uint16_t version = extract_uint16(udp_buffer + kHeaderSize);
        if(version < kProtocolVersion) {
          LOG_ERR("target reported invalid protocol version %d\n", version);
          aboot_callback_start(-1, NULL);
          continue;
        }
        uint16_t packet_size = extract_uint16(udp_buffer + kHeaderSize + 2);
        if(packet_size < kMinPacketSize) {
          LOG_ERR("target reported invalid packet size %d\n", packet_size);
          aboot_callback_start(-1, NULL);
          continue;
        }
        packet_size = min(kHostMaxPacketSize, packet_size);
        max_data_length = packet_size - kHeaderSize;
        aboot_callback_start(0, NULL);
      } else if(header_id(header) == kIdFastboot) {
        if(!is_data_mode) {
          if(udp_size == kHeaderSize) {
            header_set(header, kIdFastboot, sequence++, kFlagNone);
            udp_size = kHeaderSize;
            process_post(PROCESS_CURRENT(), UDP_SEND_COMMAND, NULL);
          } else if(udp_size < kHeaderSize) {
            LOG_ERR("received packe too small (%d)\n", udp_size);
            aboot_callback_status(-1, ABOOT_TINY_STATUS_FAILED);
          } else {
            size_t len = udp_size - kHeaderSize;
            uint8_t *response = udp_buffer + kHeaderSize;
            if(len >= 4) {
              if(udp_command_cb) {
                udp_command_cb(response, len);
              }
              if(!memcmp(response, "INFO", 4) || !memcmp(response, "PROG", 4)) {
                header_set(header, kIdFastboot, sequence++, kFlagNone);
                udp_size = kHeaderSize;
                process_post(PROCESS_CURRENT(), UDP_SEND_COMMAND, NULL);
              }
            } else {
              LOG_ERR("received packe too small (%d)\n", udp_size);
              aboot_callback_status(-1, ABOOT_TINY_STATUS_FAILED);
            }
          }
        } else {
          if(udp_size == kHeaderSize) {
            size_t remain = send_total_size - send_offset_read;
            if(remain == 0) {
              is_data_mode = false;
            }
            size_t len = max_data_length < remain ? max_data_length : remain;
            memcpy(udp_buffer + kHeaderSize, send_buffer + send_offset_read, len);
            send_offset_read += len;
            if(send_offset_read < send_total_size) {
              header_set(header, kIdFastboot, sequence++, kFlagContinuation);
            } else {
              header_set(header, kIdFastboot, sequence++, kFlagNone);
            }
            udp_size = kHeaderSize + len;
            process_post(PROCESS_CURRENT(), UDP_SEND_COMMAND, NULL);
          } else {
            LOG_ERR("unexpected response recieved when in data mode\n");
            aboot_callback_status(-1, ABOOT_TINY_STATUS_FAILED);
          }
        }
      }
    } else if(ev == PROCESS_EVENT_EXIT) {
      if(send_buffer) {
        free(send_buffer);
      }
      send_buffer = NULL;
      etimer_stop(&periodic_timer);
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
void
udp_transport_init(void)
{
  sequence = 0;
  udp_command_cb = NULL;
  udp_data_cb = NULL;
  max_data_length = 0;

  Header *header = (Header *)udp_buffer;
  header_set(header, kIdDeviceQuery, sequence++, kFlagNone);
  udp_size = kHeaderSize;

  ethos_init();
  ethos_register_cmd_callback(ethos_net_callback);
  ethos_register_data_callback(NULL);
  process_start(&udp_transport_process, NULL);
}
/*---------------------------------------------------------------------------*/
void
udp_transport_register_cmd_callback(udp_transport_rx_cb_t cb)
{
  udp_command_cb = cb;
}
/*---------------------------------------------------------------------------*/
void
udp_transport_register_data_callback(udp_transport_rx_cb_t cb)
{
  udp_data_cb = cb;
}
/*---------------------------------------------------------------------------*/
void
udp_transport_write_aboot_cmd(const uint8_t *data, size_t len)
{
  is_data_mode = false;
  Header *header = (Header *)udp_buffer;
  header_set(header, kIdFastboot, sequence++, kFlagNone);
  memcpy(udp_buffer + kHeaderSize, data, len);
  udp_size = kHeaderSize + len;
  process_post(&udp_transport_process, UDP_SEND_COMMAND, NULL);
}
/*---------------------------------------------------------------------------*/
void
udp_transport_set_aboot_data_size(size_t size)
{
  if(send_buffer) {
    free(send_buffer);
  }
  send_buffer = NULL;
  send_total_size = size;
  send_offset_write = 0;
  is_data_mode = true;
}
/*---------------------------------------------------------------------------*/
void
udp_transport_write_aboot_data(const uint8_t *data, size_t len)
{
  if(!send_offset_write) {
    if(*((const uint32_t *)data) == SPARSE_HEADER_MAGIC) {
      is_sparse_file = true;
      ethos_set_aboot_data_size(send_total_size);
    } else {
      is_sparse_file = false;
      send_buffer = (uint8_t *)malloc(send_total_size);
      if(!send_buffer) {
        LOG_ERR("out of memory\n");
        aboot_callback_status(-1, ABOOT_TINY_STATUS_FAILED);
        return;
      }
    }
  }

  if(send_offset_write + len > send_total_size) {
    LOG_ERR("too many data received\n");
    aboot_callback_status(-1, ABOOT_TINY_STATUS_FAILED);
    return;
  }

  if(is_sparse_file) {
    Header *header = (Header *)udp_buffer;
    memcpy(udp_buffer + kHeaderSize, data, len);
    send_offset_write += len;
    if(send_offset_read < send_total_size) {
      header_set(header, kIdFastboot, sequence++, kFlagContinuation);
    } else {
      header_set(header, kIdFastboot, sequence++, kFlagNone);
    }
    ethos_write_aboot_data(udp_buffer, kHeaderSize + len);
    if(send_offset_write == send_total_size) {
      is_data_mode = false;
      header_set(header, kIdFastboot, sequence++, kFlagNone);
      udp_size = kHeaderSize;
      process_post(&udp_transport_process, UDP_SEND_COMMAND, NULL);
    }
  } else {
    memcpy(send_buffer + send_offset_write, data, len);
    send_offset_write += len;
    if(send_offset_write == send_total_size) {
      Header *header = (Header *)udp_buffer;
      send_offset_read = 0;
      len = max_data_length < send_total_size ? max_data_length : send_total_size;
      memcpy(udp_buffer + kHeaderSize, send_buffer, len);
      send_offset_read += len;
      if(send_offset_read < send_total_size) {
        header_set(header, kIdFastboot, sequence++, kFlagContinuation);
      } else {
        header_set(header, kIdFastboot, sequence++, kFlagNone);
      }
      udp_size = kHeaderSize + len;
      process_post(&udp_transport_process, UDP_SEND_COMMAND, NULL);
    }
  }
}
/*---------------------------------------------------------------------------*/
void
udp_transport_exit(void)
{
  process_exit(&udp_transport_process);
  ethos_exit();
}
/*---------------------------------------------------------------------------*/
