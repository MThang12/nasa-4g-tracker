#ifndef __PUB_CACHE_H__
#define __PUB_CACHE_H__

#include "sal_os.h"


#define CACHE_STOPED  (0)
#define CACHE_WORKING (1)
#define CACHE_DELETED (2)

#define CACHE_ASYNC   (0)  // 0 << 0 (if cache empty/full, read/write will return 0 immediately.)
#define CACHE_SYNC_R  (1)  // 1 << 0 (if cache empty, will case read blocked.)
#define CACHE_SYNC_W  (2)  // 1 << 1 (if cache full, will case write blocked.)
#define CACHE_SYNC_RW (3)  // CACHE_SYNC_R | CACHE_SYNC_W (read/write blocked)

#define CACHE_TYPE_BUFFER (0)
#define CACHE_TYPE_LIST   (1)

typedef void *pub_cache_p;

typedef struct pub_listcache_data
{
    char *data;
    int size;
    void *data_info;
    struct pub_listcache_data *next;
} pub_listcache_data_t;


/**
 * @brief Create a cache to save data. If use list type. The data will save as package.
 *
 * @param max_size Max size of the cache.
 * @param type User buffer or list to save data.
 * @param sync_mode Write block or read block or both or bot not.
 * @return When success, Return the handle of the cache. Otherwise, NULL will return.
 */
pub_cache_p pub_cache_create(unsigned int max_size, int type, int sync_mode);

int pub_cache_delete(pub_cache_p *handle);

/**
 * @brief Save data to cache. When use list type, It can set the extern infomation to each pacakge. Also if data size lagger than then free size of the cache. All data will be save to cache, and then the free size of cache will be negative.
 *
 * @param handle The handle of the cache.
 * @param data The data to save to cache.
 * @param size The size of the data.
 * @param info The extern infomation by list type.
 * @param info_size The size of the infomation.
 * @return When success return the size actual saved. Otherwise, errcode will return.
 */
int pub_cache_write_ex(pub_cache_p handle, const char *data, unsigned int size, void *info, int info_size, int wait_ms);

/**
 * @brief Save data to cache same as pub_cache_write_ex.
 *
 * @param handle The handle of the cache.
 * @param data The data to save to cache.
 * @param size The size of the data.
 * @return When success return the size actual saved. Otherwise, errcode will return.
 */
#define pub_cache_write(handle,data,size) pub_cache_write_ex(handle,data,size,NULL,0,SAL_OS_TIMEOUT_SUSPEND)

/**
 * @brief Read data from the cache. When use list type. If the buffer big enough, one read just get one package data. If the buffer too smaller, the remained data of a package can be read at next time.
 *
 * @param handle The handle of the cache.
 * @param buffer The buffer to save the read data.
 * @param size The size of the buffer.
 * @param info The buffer to save the extern infomation by list type.
 * @param info_size The size of the infomation buffer.
 * @return When success return the size actual read. Otherwise, errcode will return.
 */
int pub_cache_read_ex(pub_cache_p handle, char *buffer, unsigned int size, void *info, int info_size, int wait_ms);

/**
 * @brief Read data from the cache same as pub_cache_read_ex.
 *
 * @param handle The handle of the cache.
 * @param buffer The buffer to save the read data.
 * @param size The size of the buffer.
 * @return When success return the size actual read. Otherwise, errcode will return.
 */
#define pub_cache_read(handle,buffer,size) pub_cache_read_ex(handle,buffer,size,NULL,0,SAL_OS_TIMEOUT_SUSPEND)

int pub_cache_set_status(pub_cache_p handle, int status);

bool pub_cache_is_working(pub_cache_p handle);

/**
 * @brief Get the free size of the cache. It may be negative of package mode.
 *
 * @param handle The handle of the cache.
 * @return Retrun the free size of the cache.
 */
int pub_cache_get_free_size(pub_cache_p handle);

unsigned int pub_cache_get_data_size(pub_cache_p handle);


#endif /* __PUB_CACHE_H__ */
