/**
 * @section LICENSE
 * Copyright (C) 2020, ASR microelectronics, All rights reserved.
 *
 * @section DESCRIPTION
 *
 * The time class represents a moment of time.
 */
#ifndef ABOOT_DEVICE_H
#define ABOOT_DEVICE_H

#ifdef  __cplusplus
extern "C" {
#endif

#include "hotplug.h"

void aboot_device_online(tty_device_info_t *device);
void aboot_device_offline(tty_device_info_t *device);

#ifdef  __cplusplus
}
#endif

#endif /* ABOOT_DEVICE_H */
