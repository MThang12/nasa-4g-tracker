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
#ifndef ABOOT_TINY_H
#define ABOOT_TINY_H

#include <sys/types.h>
#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>

#ifdef  __cplusplus
extern "C" {
#endif

#if defined _WIN32 || defined __CYGWIN__ || defined __MINGW32__
#if defined(BUILD_DLL)
#define ABOOT_API __declspec(dllexport)
#elif defined(USE_DLL)
#define ABOOT_API __declspec(dllimport)
#else
#define ABOOT_API
#endif
#else
#define ABOOT_API __attribute__ ((visibility("default")))
#endif

/**
 * Callback events to upper layer
 */
typedef enum {
  ABOOT_TINY_EVENT_INIT,
  ABOOT_TINY_EVENT_START,
  ABOOT_TINY_EVENT_DOWNLOAD,
  ABOOT_TINY_EVENT_STOP,
  ABOOT_TINY_EVENT_EXIT,
  ABOOT_TINY_EVENT_LOG,
  ABOOT_TINY_EVENT_PROGRESS,
  ABOOT_TINY_EVENT_STATUS,
} aboot_tiny_event_t;

/**
 * Aboot device running status
 */
#define ABOOT_TINY_STATUS_CONNECTING  "CONNECTING"
#define ABOOT_TINY_STATUS_RUNNING     "RUNNING"
#define ABOOT_TINY_STATUS_FAILED      "FAILED"
#define ABOOT_TINY_STATUS_SUCCEEDED   "SUCCEEDED"

/**
 * Callback message to upper layer
 */
typedef struct {
  aboot_tiny_event_t event;
  int error;
  union {
    const char *message;
    int progress;
    const char *status;
  } u;
} aboot_tiny_message_t;

/**
 * Callback function pointer type
 *
 * @param json callback info
 * @param ctx user data pointer
 */
typedef void (*aboot_tiny_callback_t)(const aboot_tiny_message_t *msg, void *ctx);

/**
 * Register aboot tiny call back.
 *
 * @param cb callback function pointer
 * @param ctx user data pointer
 */
void ABOOT_API aboot_tiny_register_cb(aboot_tiny_callback_t cb, void *ctx);

/**
 * Initialize aboot tiny engine.
 *
 * @return 0 on success, otherwise failed
 */
int ABOOT_API aboot_tiny_init(void);

/**
 * Start aboot tiny engine.
 *
 * @param dev_path device path to open
 * @param baud_rate baud rate to communicate with
 *
 * @return 0 on success, otherwise failed
 */
int ABOOT_API aboot_tiny_start(const char *dev_path, int baud_rate);

/**
 * Start download from file.
 *
 * @param file firmware file pointer
 * @param size firmware size
 * @param reboot if reboot after completed
 *
 * @return 0 on success, otherwise failed
 */
int ABOOT_API aboot_tiny_download_file(FILE *file, size_t size, bool reboot);

/**
 * Start download from data.
 *
 * @param data firmware data pointer
 * @param size firmware size
 * @param reboot if reboot after completed
 *
 * @return 0 on success, otherwise failed
 */
int ABOOT_API aboot_tiny_download_data(const void *data, size_t size, bool reboot);

/**
 * Stop aboot tiny engine.
 *
 * @return 0 on success, otherwise failed
 */
int ABOOT_API aboot_tiny_stop(void);

/**
 * Cleanup aboot tiny engine.
 */
int ABOOT_API aboot_tiny_exit(void);

#ifdef  __cplusplus
}
#endif

#endif /* ABOOT_TINY_H */
