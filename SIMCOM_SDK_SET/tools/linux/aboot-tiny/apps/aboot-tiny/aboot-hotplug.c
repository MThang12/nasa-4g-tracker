#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>

#include "libusb.h"
#include "aboot-hotplug.h"
#include "at-fallback.h"
#include "aboot-device.h"

#include "sys/log.h"
#define LOG_MODULE "Hotplug"
#define LOG_LEVEL LOG_LEVEL_INFO
/*---------------------------------------------------------------------------*/
typedef struct {
  uint16_t vendorId;
  uint16_t productId;
} usb_device_id_t;
/*---------------------------------------------------------------------------*/
static tty_device_info_t aboot_device_info;
static tty_device_info_t atcmd_device_info;
static bool hotplug_inited;
/*---------------------------------------------------------------------------*/
static const usb_device_id_t aboot_device_ids[] = {
  { 0x2ecc, 0x3017 }, /* crane arom */
  { 0x2ecc, 0x3004 }, /* crane(boot2), cranem/craneg arom */
};
static const usb_device_id_t atcmd_device_ids[] = {
  { 0x2ecc, 0x3010 }, /* ASR default device */
  { 0x2c7c, 0x6002 }, /* Quectel */
  { 0x1e0e, 0x9011 }, /* Simcomm */
};
/*---------------------------------------------------------------------------*/
static bool
is_aboot_device(uint16_t vendorId, uint16_t productId)
{
  int i;
  for(i = 0; i < sizeof(aboot_device_ids) / sizeof(aboot_device_ids[0]); i++) {
    if(aboot_device_ids[i].vendorId == vendorId
       && aboot_device_ids[i].productId == productId) {
      return true;
    }
  }

  return false;
}
/*---------------------------------------------------------------------------*/
static bool
is_atcmd_device(uint16_t vendorId, uint16_t productId)
{
  int i;
  for(i = 0; i < sizeof(atcmd_device_ids) / sizeof(atcmd_device_ids[0]); i++) {
    if(atcmd_device_ids[i].vendorId == vendorId
       && atcmd_device_ids[i].productId == productId) {
      return true;
    }
  }

  return false;
}
/*---------------------------------------------------------------------------*/
static int
read_sysfs_string(const char *sysfs_name, const char *sysfs_node, char *buf, int bufsize)
{
  char path[80];
  int fd, n;

  snprintf(path, sizeof(path),
           "/sys/bus/usb/devices/%s/%s", sysfs_name, sysfs_node);
  path[sizeof(path) - 1] = '\0';

  fd = open(path, O_RDONLY);
  if(fd < 0) {
    return -1;
  }

  n = read(fd, buf, bufsize - 1);
  close(fd);

  if(n < 0) {
    return -1;
  }

  buf[n] = '\0';

  return n;
}
/*---------------------------------------------------------------------------*/
static int
get_location(libusb_device *dev, char *location)
{
  const int USB_TOPOLOGY_MAXLEN = 7;
  uint8_t numbers[USB_TOPOLOGY_MAXLEN];
  uint8_t bus = libusb_get_bus_number(dev);
  int ret = libusb_get_port_numbers(dev, numbers, USB_TOPOLOGY_MAXLEN);

  if(ret <= 0) {
    LOG_ERR("error locating usb device: %s\n", libusb_strerror(ret));
    return -1;
  }
  sprintf(location, "%d-", bus);
  for(int n = 0; n < ret; ++n) {
    sprintf(location + strlen(location), "%d%s", numbers[n], n + 1 < ret ? "." : "");
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
find_tty_device(const char *sysfs_name, char *description, char *dev_name)
{
  char base[PATH_MAX], subbase[PATH_MAX];
  DIR *dir, *subdir;
  struct dirent *de, *subde;
  char buf[128];
  bool is_composite_dev = false;

  strcpy(base, "/sys/bus/usb/devices/");
  strcat(base, sysfs_name);

  /*std::this_thread::sleep_for(std::chrono::milliseconds(100)); */

  if(read_sysfs_string(sysfs_name, "product", buf, sizeof(buf)) > 0) {
    buf[strlen(buf) - 1] = '\0';            /* trim last newline char */
    strcpy(description, buf);
    if(strstr(buf, "Composite")) {
      is_composite_dev = true;
    } else if(read_sysfs_string(sysfs_name, "bDeviceClass", buf, sizeof(buf)) > 0) {
      buf[strlen(buf) - 1] = '\0';          /* trim last newline char */
      if(!strcmp(buf, "ef")) {
        if(read_sysfs_string(sysfs_name, "bDeviceSubClass", buf, sizeof(buf)) > 0) {
          buf[strlen(buf) - 1] = '\0';      /* trim last newline char */
          if(!strcmp(buf, "02")) {
            is_composite_dev = true;
          }
        }
      }
    }
  } else {
    strcpy(description, "No description");
  }

  dir = opendir(base);
  if(dir == 0) {
    return -1;
  }

  while((de = readdir(dir))) {
    if(strncmp(de->d_name, sysfs_name, strlen(sysfs_name))) {
      continue;
    }

    strcpy(subbase, base);
    strcat(subbase, "/");
    strcat(subbase, de->d_name);
    subdir = opendir(subbase);
    if(subdir == 0) {
      continue;
    }

    if(is_composite_dev) {
      char sub_sysfs_name[128];
      strcpy(sub_sysfs_name, sysfs_name);
      strcat(sub_sysfs_name, "/");
      strcat(sub_sysfs_name, de->d_name);
      if(read_sysfs_string(sub_sysfs_name, "interface", buf, sizeof(buf)) > 0) {
        buf[strlen(buf) - 1] = '\0';         /* trim last newline char */
        if(!strstr(buf, "AT")) {
          closedir(subdir);
          continue;
        }
        strcpy(description, buf);
      }
    }
    while((subde = readdir(subdir))) {
      if(strncmp(subde->d_name, "tty", 3)) {
        continue;
      }

      if(strlen(subde->d_name) > 3) {
        strcat(subbase, "/");
        strcat(subbase, subde->d_name);
        closedir(subdir);
        subdir = opendir(subbase);
        continue;
      } else if(strlen(subde->d_name) == 3) {
        strcat(subbase, "/");
        strcat(subbase, subde->d_name);
        closedir(subdir);
        subdir = opendir(subbase);
        while((subde = readdir(subdir))) {
          if(strcmp(subde->d_name, ".") && strcmp(subde->d_name, "..")) {
            break;
          }
        }
        strcpy(dev_name, "/dev/");
        strcat(dev_name, subde->d_name);
        closedir(subdir);
        closedir(dir);
        return 0;
      }
    }
    closedir(subdir);
  }
  closedir(dir);

  return -1;
}
/*---------------------------------------------------------------------------*/
static int LIBUSB_CALL
hotplug_callback(libusb_context *ctx, libusb_device *dev, libusb_hotplug_event event, void *user_data)
{
  struct libusb_device_descriptor desc;
  tty_device_info_t *device;
  int rc;
  bool is_atcmd;

  (void)ctx;
  (void)user_data;

  rc = libusb_get_device_descriptor(dev, &desc);
  if(LIBUSB_SUCCESS != rc) {
    LOG_ERR("Error getting device descriptor\n");
    return 0;
  }

  if(!is_atcmd_device(desc.idVendor, desc.idProduct)) {
    if(!is_aboot_device(desc.idVendor, desc.idProduct)) {
      return 0;
    } else {
      is_atcmd = false;
      device = &aboot_device_info;
    }
  } else {
    is_atcmd = true;
    device = &atcmd_device_info;
  }

  if(LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED == event) {
    char location[32];

    if(device->dev_id) {
      return 0;
    }

    device->vendorId = desc.idVendor;
    device->productId = desc.idProduct;
    device->highSpeed = libusb_get_device_speed(dev) >= LIBUSB_SPEED_HIGH ? true : false;
    if(get_location(dev, location)) {
      return 0;
    }
    if(find_tty_device(location, device->description, device->path)) {
      return 0;
    }
    if(read_sysfs_string(location, "manufacturer", device->manufacturer, sizeof(device->manufacturer)) > 0) {
      device->manufacturer[strlen(device->manufacturer) - 1] = '\0';           /* trim last newline char */
    } else {
      strcpy(device->manufacturer, "Unknown manufacturer");
    }
    device->dev_id = dev;
    LOG_INFO("ID %04x:%04x %s %s (%s) - ONLINE\n", device->vendorId, device->productId,
             device->manufacturer, device->description, device->path);
    if(is_atcmd) {
      atcmd_device_online(device);
    } else {
      aboot_device_online(device);
    }
  } else if(LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT == event) {
    if(device->dev_id == dev) {
      LOG_INFO("ID %04x:%04x %s %s (%s) - OFFLINE\n", device->vendorId, device->productId,
               device->manufacturer, device->description, device->path);
      if(is_atcmd) {
        atcmd_device_offline(device);
      } else {
        aboot_device_offline(device);
      }
      device->dev_id = NULL;
    }
  }

  return 0;
}
/*---------------------------------------------------------------------------*/
int
aboot_hotplug_init(void)
{
  int rc, i;

  memset(&aboot_device_info, 0, sizeof(aboot_device_info));
  memset(&atcmd_device_info, 0, sizeof(atcmd_device_info));

  rc = libusb_init(NULL);
  if(rc < 0) {
    LOG_ERR("failed to initialise libusb: %s\n", libusb_error_name(rc));
    return rc;
  }

  if(!libusb_has_capability(LIBUSB_CAP_HAS_HOTPLUG)) {
    LOG_ERR("Hotplug capabilites are not supported on this platform\n");
    libusb_exit(NULL);
    return -1;
  }

  libusb_hotplug_event events = LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED | LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT;
  libusb_hotplug_flag flag = LIBUSB_HOTPLUG_ENUMERATE;

  for(i = 0; i < sizeof(aboot_device_ids) / sizeof(aboot_device_ids[0]); i++) {
    rc = libusb_hotplug_register_callback(
      NULL,
      events,
      flag,
      aboot_device_ids[i].vendorId,
      aboot_device_ids[i].productId,
      LIBUSB_HOTPLUG_MATCH_ANY,
      hotplug_callback,
      NULL,
      NULL);
    if(LIBUSB_SUCCESS != rc) {
      LOG_ERR("Error registering callback 0\n");
      libusb_exit(NULL);
      return rc;
    }
  }

  for(i = 0; i < sizeof(atcmd_device_ids) / sizeof(atcmd_device_ids[0]); i++) {
    rc = libusb_hotplug_register_callback(
      NULL,
      events,
      flag,
      atcmd_device_ids[i].vendorId,
      atcmd_device_ids[i].productId,
      LIBUSB_HOTPLUG_MATCH_ANY,
      hotplug_callback,
      NULL,
      NULL);
    if(LIBUSB_SUCCESS != rc) {
      LOG_ERR("Error registering callback 0\n");
      libusb_exit(NULL);
      return rc;
    }
  }

  hotplug_inited = true;

  return 0;
}
/*---------------------------------------------------------------------------*/
void
aboot_hotplug_handle_events(void)
{
  struct timeval tv;

  if(hotplug_inited) {
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    libusb_handle_events_timeout_completed(NULL, &tv, NULL);
  }
}
/*---------------------------------------------------------------------------*/
void
aboot_hotplug_exit(void)
{
  hotplug_inited = false;
  libusb_exit(NULL);
}
/*---------------------------------------------------------------------------*/
