#ifndef ALOG_H
#define ALOG_H

#ifdef  __cplusplus
extern "C" {
#endif

void alog_init(void);
int alog(const char *format, ...);

#ifdef  __cplusplus
}
#endif

#endif
