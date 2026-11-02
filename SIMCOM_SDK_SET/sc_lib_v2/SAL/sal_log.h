#ifndef __SAL_LOG_H__
#define __SAL_LOG_H__


#define SAL_DBG_ERROR
#define SAL_DBG_INFO

#ifdef SIMCOM_DEBUG
#define SAL_DBG_TRACE
#endif

int sal_printf(const char *file, int line, const char *format, ...);

#ifdef SIMCOM_DEBUG
#define sal_log(...) sal_printf("",__LINE__,##__VA_ARGS__)
#else
#define sal_log(...)
#endif

#ifdef SAL_DBG_ERROR
#define sal_log_error(...) sal_printf("",__LINE__,"ERROR -> " __VA_ARGS__)
#else
#define sal_log_error(...)
#endif

#ifdef SAL_DBG_INFO
#define sal_log_info(...)  sal_printf("",__LINE__,"INFO -> " __VA_ARGS__)
#else
#define sal_log_info(...)
#endif

#ifdef SAL_DBG_TRACE
#define sal_log_trace(...) sal_printf(__FUNCTION__,__LINE__,"TRACE -> " __VA_ARGS__)
#else
#define sal_log_trace(...)
#endif



#endif /* __SAL_LOG_H__ */
