#include <stdlib.h>
#include <string.h>

#include "contiki.h"

#include "aboot-transport.h"
#include "udp-transport.h"
#include "aboot-callback.h"
#include "aboot-download.h"
#include "smux.h"

#include "sys/log.h"
#define LOG_MODULE "Transport"
#define LOG_LEVEL LOG_LEVEL_INFO

/*---------------------------------------------------------------------------*/
static aboot_device_type_t aboot_device_type;
/*---------------------------------------------------------------------------*/
static void
aboot_transport_rx_callback(const uint8_t *status, size_t size)
{
  char response[ABOOT_RESPONSE_SZ];

  memset(response, 0, ABOOT_RESPONSE_SZ);

  if(size < 4) {
    sprintf(response, "FAILstatus malformed (%d bytes)", (int)size);
    process_post_synch(&aboot_download_process, ABOOT_RESPONSE_EVENT, response);
    return;
  }
  if(size >= ABOOT_RESPONSE_SZ) {
    size = ABOOT_RESPONSE_SZ - 1;
  }

  if(!memcmp(status, "INFO", 4)) {
    strncpy(response, (char *)status + 4, size - 4);
    response[size - 4] = '\0';
    LOG_INFO("%s\n", response);
  } else if(!memcmp(status, "PROG", 4)) {
    strncpy(response, (const char *)status + 4, size - 4);
    int progress = atoi(response);
    if(progress <= 100) {
      aboot_callback_progress(progress);
    }
  } else if(!memcmp(status, "DATA", 4) || !memcmp(status, "OKAY", 4) || !memcmp(status, "FAIL", 4)) {
    strncpy(response, (const char *)status, size);
    process_post_synch(&aboot_download_process, ABOOT_RESPONSE_EVENT, response);
  } else {
    sprintf(response, "FAILunknown response status recieved");
    process_post_synch(&aboot_download_process, ABOOT_RESPONSE_EVENT, response);
  }
}
/*---------------------------------------------------------------------------*/
void
aboot_transport_init(aboot_device_type_t dev_type)
{
  aboot_device_type = dev_type;

  switch(aboot_device_type) {
  case ABOOT_DEVICE_TYPE_SMUX:
    smux_init();
    smux_register_cmd_callback(aboot_transport_rx_callback);
    break;

  case ABOOT_DEVICE_TYPE_ETHOS:
    udp_transport_init();
    udp_transport_register_cmd_callback(aboot_transport_rx_callback);
    break;

  default:
    break;
  }
}
/*---------------------------------------------------------------------------*/
void
aboot_transport_send_cmd(const uint8_t *cmd, size_t size)
{
  switch(aboot_device_type) {
  case ABOOT_DEVICE_TYPE_SMUX:
    smux_write_aboot_cmd(cmd, size);
    break;

  case ABOOT_DEVICE_TYPE_ETHOS:
    udp_transport_write_aboot_cmd(cmd, size);
    break;

  default:
    break;
  }
}
/*---------------------------------------------------------------------------*/
void
aboot_transport_set_data_size(size_t size)
{
  switch(aboot_device_type) {
  case ABOOT_DEVICE_TYPE_SMUX:
    smux_set_aboot_data_size(size);
    break;

  case ABOOT_DEVICE_TYPE_ETHOS:
    udp_transport_set_aboot_data_size(size);
    break;

  default:
    break;
  }
}
/*---------------------------------------------------------------------------*/
void
aboot_transport_send_data(const uint8_t *data, size_t size)
{
  switch(aboot_device_type) {
  case ABOOT_DEVICE_TYPE_SMUX:
    smux_write_aboot_data(data, size);
    break;

  case ABOOT_DEVICE_TYPE_ETHOS:
    udp_transport_write_aboot_data(data, size);
    break;

  default:
    break;
  }
}
/*---------------------------------------------------------------------------*/
aboot_device_type_t
aboot_transport_get_device_type(void)
{
  return aboot_device_type;
}
/*---------------------------------------------------------------------------*/
void
aboot_transport_exit(void)
{
  switch(aboot_device_type) {
  case ABOOT_DEVICE_TYPE_SMUX:
    smux_exit();
    break;

  case ABOOT_DEVICE_TYPE_ETHOS:
    udp_transport_exit();
    break;

  default:
    break;
  }
}
/*---------------------------------------------------------------------------*/
