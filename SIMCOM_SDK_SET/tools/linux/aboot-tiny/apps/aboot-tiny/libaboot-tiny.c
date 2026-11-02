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

#include <pthread.h>
#include <stdbool.h>

#include "contiki.h"
#include "aboot-tiny.h"
#include "aboot-callback.h"
#include "aboot-download.h"
#include "preamble.h"

#include "sys/log.h"
#define LOG_MODULE "AbootTiny"
#define LOG_LEVEL LOG_LEVEL_INFO

/*---------------------------------------------------------------------------*/
extern void *aboot_tiny_main(void *data);
/*---------------------------------------------------------------------------*/
static pthread_t aboot_tiny_thread;
static bool aboot_tiny_running;
/*---------------------------------------------------------------------------*/
bool
aboot_tiny_is_running(void)
{
  return aboot_tiny_running;
}
/*---------------------------------------------------------------------------*/
void
aboot_tiny_register_cb(aboot_tiny_callback_t cb, void *ctx)
{
  aboot_callback_register(cb, ctx);
}
/*---------------------------------------------------------------------------*/
int
aboot_tiny_init(void)
{
  pthread_attr_t attr;
  int rc;

  /* Initialize and set thread attribute */
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  pthread_attr_setstacksize(&attr, 32 * 1024);

  aboot_tiny_running = true;

  rc = pthread_create(&aboot_tiny_thread, &attr, aboot_tiny_main, NULL);
  if(rc) {
    LOG_ERR("ERROR; return code from pthread_create() is %d\n", rc);
  }

  /* Free attribute */
  pthread_attr_destroy(&attr);

  return rc;
}
/*---------------------------------------------------------------------------*/
int
aboot_tiny_start(const char *dev_path, int baud_rate)
{
  if(preamble_start(dev_path, baud_rate) < 0) {
    return -1;
  } else {
    return 0;
  }
}
/*---------------------------------------------------------------------------*/
int
aboot_tiny_download_file(FILE *file, size_t size, bool reboot)
{
  aboot_download_start_file(file, size, reboot);
  return 0;
}
/*---------------------------------------------------------------------------*/
int
aboot_tiny_download_data(const void *data, size_t size, bool reboot)
{
  aboot_download_start_data(data, size, reboot);
  return 0;
}
/*---------------------------------------------------------------------------*/
int
aboot_tiny_stop(void)
{
  preamble_stop();
  return 0;
}
/*---------------------------------------------------------------------------*/
int
aboot_tiny_exit(void)
{
  int rc;

  /* tell aboot_tiny_thread to stop gracefully */
  aboot_tiny_running = false;

  rc = pthread_join(aboot_tiny_thread, NULL);
  if(rc) {
    LOG_ERR("ERROR; return code from pthread_join() is %d\n", rc);
    return rc;
  }

  aboot_callback_exit(0, NULL);

  return 0;
}
/*---------------------------------------------------------------------------*/
