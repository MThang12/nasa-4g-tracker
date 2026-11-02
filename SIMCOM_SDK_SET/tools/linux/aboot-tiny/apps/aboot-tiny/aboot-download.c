#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "contiki.h"
#include "aboot-download.h"
#include "aboot-callback.h"
#include "aboot-transport.h"

#include "sys/log.h"
#define LOG_MODULE "Download"
#define LOG_LEVEL LOG_LEVEL_INFO

/*---------------------------------------------------------------------------*/
#define MAX_DOWNLOAD_BUF_SIZE 1024
/*---------------------------------------------------------------------------*/
const char cmd_getvar[] = "getvar:";
const char cmd_download[] = "download:";
const char cmd_call[] = "call";
const char cmd_nop[] = "nop";
const char cmd_reboot[] = "reboot";
const char cmd_complete[] = "complete";
const char crane_version_bootrom[] = "2019.01.15";
/*---------------------------------------------------------------------------*/
static FILE *firmware_file;
static const void *firmware_data;
static size_t firmware_size;
static off_t firmwar_start;
static off_t firmwar_end;
static bool reboot_after_completed;
static bool is_crane_bootrom;
static bool is_firmware_from_file;
PROCESS(aboot_download_process, "Aboot download process");
/*---------------------------------------------------------------------------*/
int
aboot_download_init(void)
{
  firmware_file = NULL;
  firmware_size = 0;
  firmwar_start = 0;
  firmwar_end = 0;

  process_start(&aboot_download_process, NULL);
  return 0;
}
/*---------------------------------------------------------------------------*/
int
aboot_download_start_file(FILE *file, size_t size, bool reboot)
{
  firmware_file = file;
  firmware_size = size;
  firmwar_start = ftell(file);
  firmwar_end = firmwar_start + size;
  reboot_after_completed = reboot;
  is_firmware_from_file = true;

  process_poll(&aboot_download_process);
  aboot_callback_download(0, NULL);

  return 0;
}
/*---------------------------------------------------------------------------*/
int
aboot_download_start_data(const void *data, size_t size, bool reboot)
{
  firmware_data = data;
  firmware_size = size;
  firmwar_start = 0;
  firmwar_end = firmwar_start + size;
  reboot_after_completed = reboot;
  is_firmware_from_file = false;

  process_poll(&aboot_download_process);
  aboot_callback_download(0, NULL);

  return 0;
}
/*---------------------------------------------------------------------------*/
int
aboot_download_exit(void)
{
  process_exit(&aboot_download_process);
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
download_read_line(char *line)
{
  off_t current_pos;

  while(1) {
    if(is_firmware_from_file) {
      current_pos = ftell(firmware_file);
    } else {
      current_pos = firmwar_start;
    }
    if(current_pos < firmwar_end) {
      if(is_firmware_from_file) {
        if(!fgets(line, ABOOT_COMMAND_SZ, firmware_file)) {
          return -1;
        }
      } else {
        const uint8_t *firmware_current = firmware_data + firmwar_start;
        const uint8_t *p = memchr(firmware_current, '\n', ABOOT_COMMAND_SZ);
        if(!p) {
          return -1;
        }
        int len = p - firmware_current + 1;
        memcpy(line, firmware_current, len);
        line[len] = '\0';
        firmwar_start += len;
      }
      if(line[0] == '\n') {
        continue;
      } else {
        line[strlen(line) - 1] = '\0'; /* replace '\n' to '\0' */
        LOG_INFO("line(cmd_line) = %s\n",line);
        return 1;
      }
    } else {
      return 0;
    }
  }
}
/*---------------------------------------------------------------------------*/
static int
download_read_cmd_response(char *cmd_line, char *response)
{
  int rc = download_read_line(cmd_line);
  if(rc < 0) {
    return -1;
  } else if(rc == 0) {
    return 0;
  }

  rc = download_read_line(response);
  if(rc < 0) {
    return -1;
  } else if(rc == 0) {
    return 0;
  }

  return 1;
}
/*---------------------------------------------------------------------------*/
static int
download_handle_getvar(const char *variable, char *expect, const char *response)
{
  if(memcmp(expect, response, 4)) {
    return -1;
  }
  expect += 4;
  response += 4;

  if(!strcmp(variable, "max-download-size")) {
    uint32_t expect_size = (uint32_t)strtoul(expect, NULL, 16);
    uint32_t response_size = (uint32_t)strtoul(response, NULL, 16);
    if(expect_size > response_size) {
      return -1;
    }
  } else if(!strcmp(variable, "version-bootrom")) {
    if(strcmp(expect, response)) {
      return -1;
    }
    if(strcmp(crane_version_bootrom, response)) {
      is_crane_bootrom = false;
    } else {
      is_crane_bootrom = true;
    }
  } else {
    char *token;
    while((token = strsep(&expect, "|"))) {
      if(!strcmp(token, response)) {
        break;
      }
    }
    if(!token) {
      return -1;
    }
  }

  return 0;
}
/*---------------------------------------------------------------------------*/
static int
download_handle_download(uint32_t size, char *expect, const char *response)
{
  if(memcmp(expect, response, 4)) {
    return -1;
  }
  expect += 4;
  response += 4;

  uint32_t expect_size = (uint32_t)strtoul(expect, NULL, 16);
  uint32_t response_size = (uint32_t)strtoul(response, NULL, 16);
  if(size != response_size || size != expect_size) {
    return -1;
  }

  return 0;
}
/*---------------------------------------------------------------------------*/
static int
download_do_download_data(uint32_t size)
{
  uint8_t buf[MAX_DOWNLOAD_BUF_SIZE];
  size_t len;

  aboot_transport_set_data_size(size);

  while(size) {
    len = size > MAX_DOWNLOAD_BUF_SIZE ? MAX_DOWNLOAD_BUF_SIZE : size;
    if(is_firmware_from_file) {
      if(len != fread(buf, 1, len, firmware_file)) {
        return -1;
      }
    } else {
      if(firmwar_start + len > firmware_size) {
        return -1;
      }
      memcpy(buf, firmware_data + firmwar_start, len);
      firmwar_start += len;
    }
    aboot_transport_send_data(buf, len);
    size -= len;
  }

  return 0;
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(aboot_download_process, ev, data)
{
  int num;
  static char cmd_line[ABOOT_COMMAND_SZ];
  static char response[ABOOT_RESPONSE_SZ];
  static bool oob;

  PROCESS_BEGIN();

  while(1) {
    PROCESS_YIELD();

    if(ev == PROCESS_EVENT_POLL) {
      /* start download */
      oob = false;
      if(download_read_line(cmd_line) <= 0) {
        LOG_INFO("download_read_line<=0");
        aboot_callback_status(-1, ABOOT_TINY_STATUS_FAILED);
        continue;
      }
      char magic[9];
      uint32_t size;
      num = sscanf(cmd_line, "%08s%08x", magic, &size);
      LOG_INFO("cmd_line = %s\n",cmd_line);
      LOG_INFO("size = %d\n",size);
      LOG_INFO("fsize_d = %d\n",firmware_size);
      if(num != 2 || size != firmware_size) {
        LOG_INFO("num = %d, error!", num);
        aboot_callback_status(-1, ABOOT_TINY_STATUS_FAILED);
        continue;
      }
      if(!strcmp(magic, "!CRANE!")) {
        LOG_INFO("magic[] = %s\n",magic);
        LOG_INFO("Found a crane firmware with size %u\n", (uint32_t)size);
      }
      if(download_read_cmd_response(cmd_line, response) > 0) {
        LOG_INFO("download_read_cmd_response>0");
        aboot_transport_send_cmd((uint8_t *)cmd_line, strlen(cmd_line));
      }
    } else if(ev == ABOOT_RESPONSE_EVENT) {
      if(strlen(cmd_line) > strlen(cmd_getvar) &&
         !memcmp(cmd_line, cmd_getvar, strlen(cmd_getvar))) {
        char variable[ABOOT_COMMAND_SZ];
        strcpy(variable, cmd_line + strlen(cmd_getvar));
        if(download_handle_getvar(variable, response, (const char *)data) < 0) {
          aboot_callback_status(-1, ABOOT_TINY_STATUS_FAILED);
          continue;
        }
      } else if(strlen(cmd_line) > strlen(cmd_download) &&
                !memcmp(cmd_line, cmd_download, strlen(cmd_download))) {
        uint32_t size = (uint32_t)strtoul(cmd_line + strlen(cmd_download), NULL, 16);
        if(download_handle_download(size, response, (const char *)data) < 0) {
          aboot_callback_status(-1, ABOOT_TINY_STATUS_FAILED);
          continue;
        }
        if(download_do_download_data(size) < 0) {
          aboot_callback_status(-1, ABOOT_TINY_STATUS_FAILED);
          continue;
        }
        /* forgery a command/response */
        strcpy(cmd_line, response);
        if(download_read_line(response) <= 0) {
          aboot_callback_status(-1, ABOOT_TINY_STATUS_FAILED);
        }
        continue;
      } else {
        if(strcmp(response, (const char *)data)) {
          aboot_callback_status(-1, ABOOT_TINY_STATUS_FAILED);
          continue;
        }
      }
      int rc = 0;
      if(!oob) {
        rc = download_read_cmd_response(cmd_line, response);
      } else {
        if(reboot_after_completed) {
          reboot_after_completed = false;
        } else {
          aboot_callback_status(0, ABOOT_TINY_STATUS_SUCCEEDED);
          continue;
        }
      }
      if(rc > 0) {
        if(!strcmp(cmd_line, cmd_call) && is_crane_bootrom &&
           aboot_transport_get_device_type() == ABOOT_DEVICE_TYPE_SMUX) {
          strcpy(cmd_line, cmd_nop);
        }
        aboot_transport_send_cmd((uint8_t *)cmd_line, strlen(cmd_line));
      } else if(rc < 0) {
        aboot_callback_status(-1, ABOOT_TINY_STATUS_FAILED);
      } else {
        oob = true;
        if(reboot_after_completed) {
          strcpy(cmd_line, cmd_reboot);
        } else {
          strcpy(cmd_line, cmd_complete);
        }
        strcpy(response, "OKAY");
        aboot_transport_send_cmd((uint8_t *)cmd_line, strlen(cmd_line));
      }
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
