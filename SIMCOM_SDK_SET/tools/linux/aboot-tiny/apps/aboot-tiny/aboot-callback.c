#include "aboot-callback.h"

/*---------------------------------------------------------------------------*/
static aboot_tiny_callback_t aboot_callback_func;
static void *aboot_callback_ctx;
/*---------------------------------------------------------------------------*/
void
aboot_callback_register(aboot_tiny_callback_t cb, void *ctx)
{
  aboot_callback_func = cb;
  aboot_callback_ctx = ctx;
}
/*---------------------------------------------------------------------------*/
void
aboot_callback_log(const char *message)
{
  aboot_tiny_message_t m;

  m.event = ABOOT_TINY_EVENT_LOG;
  m.error = 0;
  m.u.message = message;

  if(aboot_callback_func) {
    aboot_callback_func(&m, aboot_callback_ctx);
  }
}
/*---------------------------------------------------------------------------*/
void
aboot_callback_init(int error, const char *message)
{
  aboot_tiny_message_t m;

  m.event = ABOOT_TINY_EVENT_INIT;
  m.error = error;
  m.u.message = message;

  if(aboot_callback_func) {
    aboot_callback_func(&m, aboot_callback_ctx);
  }
}
/*---------------------------------------------------------------------------*/
void
aboot_callback_start(int error, const char *message)
{
  aboot_tiny_message_t m;

  m.event = ABOOT_TINY_EVENT_START;
  m.error = error;
  m.u.message = message;

  if(aboot_callback_func) {
    aboot_callback_func(&m, aboot_callback_ctx);
  }
}
/*---------------------------------------------------------------------------*/
void
aboot_callback_download(int error, const char *message)
{
  aboot_tiny_message_t m;

  m.event = ABOOT_TINY_EVENT_DOWNLOAD;
  m.error = error;
  m.u.message = message;

  if(aboot_callback_func) {
    aboot_callback_func(&m, aboot_callback_ctx);
  }
}
/*---------------------------------------------------------------------------*/
void
aboot_callback_stop(int error, const char *message)
{
  aboot_tiny_message_t m;

  m.event = ABOOT_TINY_EVENT_STOP;
  m.error = error;
  m.u.message = message;

  if(aboot_callback_func) {
    aboot_callback_func(&m, aboot_callback_ctx);
  }
}
/*---------------------------------------------------------------------------*/
void
aboot_callback_progress(int progress)
{
  aboot_tiny_message_t m;

  m.event = ABOOT_TINY_EVENT_PROGRESS;
  m.error = 0;
  m.u.progress = progress;

  if(aboot_callback_func) {
    aboot_callback_func(&m, aboot_callback_ctx);
  }
}
/*---------------------------------------------------------------------------*/
void
aboot_callback_status(int error, const char *status)
{
  aboot_tiny_message_t m;

  m.event = ABOOT_TINY_EVENT_STATUS;
  m.error = error;
  m.u.status = status;

  if(aboot_callback_func) {
    aboot_callback_func(&m, aboot_callback_ctx);
  }
}
/*---------------------------------------------------------------------------*/
void
aboot_callback_exit(int error, const char *message)
{
  aboot_tiny_message_t m;

  m.event = ABOOT_TINY_EVENT_EXIT;
  m.error = error;
  m.u.message = message;

  if(aboot_callback_func) {
    aboot_callback_func(&m, aboot_callback_ctx);
  }
}
/*---------------------------------------------------------------------------*/
