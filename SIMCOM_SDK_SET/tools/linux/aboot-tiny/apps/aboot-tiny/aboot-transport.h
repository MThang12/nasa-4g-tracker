/**
 * @file
 * @author  Jinhua Huang <jinhuahuang@asrmicro.com>
 * @version 1.0
 *
 * @section LICENSE
 * Copyright (C) 2020, ASR microelectronics, All rights reserved.
 *
 * @section DESCRIPTION
 *
 * The time class represents a moment of time.
 */
#ifndef ABOOT_TRANSPORT_H
#define ABOOT_TRANSPORT_H

#include <stdint.h>
#include <stddef.h>

/*
 * Aboot device type
 */
typedef enum {
  ABOOT_DEVICE_TYPE_SMUX,
  ABOOT_DEVICE_TYPE_ETHOS,
} aboot_device_type_t;

void aboot_transport_init(aboot_device_type_t dev_type);
void aboot_transport_send_cmd(const uint8_t *cmd, size_t size);
void aboot_transport_set_data_size(size_t size);
void aboot_transport_send_data(const uint8_t *data, size_t size);
aboot_device_type_t aboot_transport_get_device_type(void);
void aboot_transport_exit(void);

#ifdef  __cplusplus
}
#endif

#endif /* ABOOT_TRANSPORT_H */
