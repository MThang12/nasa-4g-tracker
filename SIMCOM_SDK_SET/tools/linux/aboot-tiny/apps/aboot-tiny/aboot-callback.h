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
#ifndef ABOOT_CALLBACK_H
#define ABOOT_CALLBACK_H

#include "aboot-tiny.h"

#ifdef  __cplusplus
extern "C" {
#endif

void aboot_callback_register(aboot_tiny_callback_t cb, void *ctx);
void aboot_callback_log(const char *message);
void aboot_callback_init(int error, const char *message);
void aboot_callback_start(int error, const char *message);
void aboot_callback_download(int error, const char *message);
void aboot_callback_stop(int error, const char *message);
void aboot_callback_progress(int progress);
void aboot_callback_status(int error, const char *status);
void aboot_callback_exit(int error, const char *message);

#ifdef  __cplusplus
}
#endif

#endif /* ABOOT_CALLBACK_H */
