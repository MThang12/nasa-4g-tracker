/**
 * @section LICENSE
 * Copyright (C) 2020, ASR microelectronics, All rights reserved.
 *
 * @section DESCRIPTION
 *
 * The time class represents a moment of time.
 */
#ifndef AT_FALLBACK_H
#define AT_FALLBACK_H

#ifdef  __cplusplus
extern "C" {
#endif

#include "aboot-hotplug.h"

void atcmd_device_online(tty_device_info_t *device);
void atcmd_device_offline(tty_device_info_t *device);

#ifdef  __cplusplus
}
#endif

#endif /* AT_FALLBACK_H */
