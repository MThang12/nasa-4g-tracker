#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "alog.h"
#include "aboot-callback.h"

/*---------------------------------------------------------------------------*/
static int alog_index;
static char alog_buffer[2048];
/*---------------------------------------------------------------------------*/
void
alog_init(void)
{
  alog_index = 0;
}
/*---------------------------------------------------------------------------*/
int
alog(const char *format, ...)
{
  char msg[2048];
  va_list arg;
  int done;

  va_start(arg, format);

  done = vsnprintf(alog_buffer + alog_index, sizeof(alog_buffer) - alog_index - 1, format, arg);
  alog_index += done;

  while(alog_index) {
    char *p = strchr(alog_buffer, '\n');
    if(!p) {
      break;
    }
    int len = p - alog_buffer + 1;
    strncpy(msg, alog_buffer, len);
    msg[len] = '\0';
    aboot_callback_log(msg);
    alog_index -= len;
    if(alog_index) {
      memmove(alog_buffer, p + 1, alog_index);
    }
  }

  va_end(arg);

  return done;
}
/*---------------------------------------------------------------------------*/
