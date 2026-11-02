/**
 * @section LICENSE
 * Copyright (C) 2020, ASR microelectronics, All rights reserved.
 *
 * @section DESCRIPTION
 *
 * The time class represents a moment of time.
 */
#ifndef ABOOT_HOTPLUG_H
#define ABOOT_HOTPLUG_H

#ifdef  __cplusplus
extern "C" {
#endif

typedef struct {
  uint16_t vendorId;
  uint16_t productId;
  bool highSpeed;
  char path[32];
  char description[64];
  char manufacturer[64];
  void *dev_id;
} tty_device_info_t;

int aboot_hotplug_init(void);
void aboot_hotplug_handle_events(void);
void aboot_hotplug_exit(void);

#ifdef  __cplusplus
}
#endif

#endif /* ABOOT_HOTPLUG_H */
