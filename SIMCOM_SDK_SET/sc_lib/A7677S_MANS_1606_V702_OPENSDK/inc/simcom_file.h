#ifndef __SIMCOM_FILE_H__
#define __SIMCOM_FILE_H__

#include <stdint.h>
#include "stdio.h"

/*Determine the location to correspond to different file systems*/
/*Seek from start of file*/
#define FS_SEEK_BEGIN	0

/*Seek from current position*/
#define FS_SEEK_CURREN	1

/*Seek from end of file*/
#define FS_SEEK_END		2

#ifndef MAX_FULL_NAME_LENGTH
#define MAX_FULL_NAME_LENGTH 255
#endif
struct dirent_t
{
    unsigned char type;
    long size;
    char name[MAX_FULL_NAME_LENGTH];
};

typedef enum{
    DIR_C_TO_SIMDIR = 0,
    DIR_SIMDIR_TO_C,
}FILE_SEITH_DIR_E;

typedef struct
{
    void *disk;
} SCFILE;

typedef struct
{
    void *dir;
} SCDIR;

typedef enum{
    TYPE_REG        = 0,/*regular file*/
    TYPE_DIR        = 1,/*sub_directory*/
}file_type;



/*
 * Name:        sAPI_fopen
 *
 * Description: Create a file that doesn't exist or open an existing file.
 *
 * Parameters:
 *  [IN] *fname     The file name containing the path.
 *  [IN] *mode      Open mode(eg: rb wb ab rb+ wb+ ab+).
 *
 * Returns:
 *  *fd     File handle
 *  NUlL   Creat or open file failed
 *
 * Notes:
 */
SCFILE *sAPI_fopen(const char *fname, const char *mode);

/*
 * Name:        sAPI_fformat
 *
 * Description: Format and mount the SD card.
 *
 * Parameters:
 *  [IN] *vol    Drive letter to be formatted. "D: "is supported..
 *
 * Returns:
 *  ret     0      Format success
 *          other  Format failed
 *
 * Notes:
 */
int sAPI_fformat(char *vol);

/*
 * Name:        sAPI_fclose
 *
 * Description: Close a file.
 *
 * Parameters:
 *  [IN] *fp     File handle.
 *
 * Returns:
 *  ret     0      close file successed
 *          other  close file failed
 *
 * Notes:
 */
int sAPI_fclose(SCFILE *fp);

/*
 * Name:        sAPI_fread
 *
 * Description: Read data from file.
 *
 * Parameters:
 *  [OUT] *buffer     A buffer that stores the read data.
 *  [IN] size      In bytes,size of each element to read.
 *  [IN] num       Number of elements.
 *  [IN] *fp       File handle.
 *
 * Returns:
 *  Length of data successfully read
 *
 * Notes: size*num -- The length of the data to be read
 */
int sAPI_fread(void *buffer, size_t size, size_t num, SCFILE *fp);

/*
 * Name:        sAPI_fwrite
 *
 * Description: write data to file.
 *
 * Parameters:
 *  [OUT] *buffer     Pointer to the data to be written to the file.
 *  [IN] size      In bytes,size of each element to write.
 *  [IN] num       Number of elements.
 *  [IN] *fp       File handle.
 *
 * Returns:
 *  Length of data successfully written
 *
 * Notes: size*num -- The length of the data to be written
 */

int sAPI_fwrite(const void *buffer, size_t size, size_t num, SCFILE *fp);

/*
* Name:        sAPI_fseek
*
* Description: Move file pointer to specified location.
*
* Parameters:
*  [IN] *fp       File handle.
*  [IN] offset
*  [IN] whence    Where does the offset start in the file
*              FS_SEEK_BEGIN  0   Seek from start of file
*              FS_SEEK_CUR  1   Seek from current position
*              FS_SEEK_END  2   Seek from end of file
*
* Returns:
*     0      success
*     other  fail
*
* Notes: Offset can be positive or negative
*/
int sAPI_fseek(SCFILE *fp, long offset, int whence);

/*
* Name:        sAPI_ftell
*
* Description: Returns the position of the current file pointer.
*
* Parameters:
*  [IN] *fp       File handle.
*
* Returns:
*   Returns the position of the file pointer
*
* Notes: Offset can be positive or negative
*/
long sAPI_ftell(SCFILE *fp);

/*
* Name:        sAPI_frewind
*
* Description: Move file pointer to the beginning of the file
*
* Parameters:
*  [IN] *fp       File handle.
*
* Returns:
*
* Notes: There is no return value, so cannot do a security check
*/
int sAPI_frewind(SCFILE *fp);

/*
* Name:        sAPI_fsize
*
* Description: Gets the size of a file
*
* Parameters:
*  [IN] *fp       File handle.
*
* Returns:
*   the size of a file
*
* Notes:
*/
int sAPI_fsize(SCFILE *fp);

/*
* Name:        sAPI_fsync
*
* Description: Synchronize a file
*
* Parameters:
*  [IN] *fp       File handle.
*
* Returns:
*   0      success
*   other  fail
*
* Notes:
*/
int sAPI_fsync(SCFILE *fp);

/*
 * Name:        sAPI_mkdir
 *
 * Description: create a directory.
 *
 * Parameters:
 *  [IN] *path     The directory name containing the path.
 *  [IN] mode
 *
 * Returns:
 *  0      success
 *  other  fail
 *
 * Notes: parameter mode has no practical effect here
 */
int sAPI_mkdir(const char *path, unsigned int mode);

/*
 * Name:        sAPI_opendir
 *
 * Description: open a directory.
 *
 * Parameters:
 *  [IN] *path     The directory name containing the path.
 *
 * Returns:
 *  *fd     directory handle
 *  NULL    failed
 *
 * Notes:
 */
SCDIR *sAPI_opendir(const char *path);

/*
 * Name:        sAPI_closedir
 *
 * Description: Close a directory.
 *
 * Parameters:
 *  [IN] *dirp     directory handle.
 *
 * Returns:
 *  ret     0      successed
 *          other  failed
 *
 * Notes:
 */
int sAPI_closedir(SCDIR *dirp);

/*
 * Name:        sAPI_readdir
 *
 * Description: read file from a directory.
 *
 * Parameters:
 *  [IN] *dirp     directory handle.
 *
 * Returns:
 *  lfs_info*     File info structure
 *  NULL  failed
 *
 * Notes:
 */
struct dirent_t *sAPI_readdir(SCDIR *dirp);

/*
 * Name:        sAPI_seekdir
 *
 * Description: Sets the read location for the next call to readdir().
 *
 * Parameters:
 *  [IN] *dirp     directory handle.
 *  [IN] offset     directory handle.
 *
 * Returns:
 *     0      success
 *     other  fail
 *
 * Notes:
 */
int sAPI_seekdir(SCDIR *dirp, unsigned long offset);

/*
 * Name:        sAPI_telldir
 *
 * Description: The current location of the directory stream
 *
 * Parameters:
 *  [IN] *dirp     directory handle.
 *
 * Returns:
 *     current location
 *
 * Notes:
 */
int sAPI_telldir(SCDIR *dirp);
int sAPI_rewinddir(SCDIR *dirp);

#if defined(CUS_LYTBOX) || defined(CUS_TOPFLY)
/*
 * Name:        sAPI_removeDir
 *
 * Description: remove a directory with flag.
 *
 * Parameters:
 *  [IN] *fname     The directory name containing the path.
 *  [IN] *flag      NULL:delete the empty directory.
                    "rf":delete the directory whether it is empty or not.
                    others: do nothing and return error.
 *
 * Returns:
 *     0      success
 *     other  fail
 *
 * Notes:
 */
int sAPI_removeDir(const char *fname,char *flag);
#ifdef CUS_TOPFLY
int sAPI_getErrorCode(void);
#endif
#endif
/*
 * Name:        sAPI_remove
 *
 * Description: remove a file/directory.
 *
 * Parameters:
 *  [IN] *fname     The file/directory name containing the path.
 *
 * Returns:
 *     0      success
 *     other  fail
 *
 * Notes:
 */
int sAPI_remove(const char *fname);

/*
 * Name:        sAPI_rename
 *
 * Description: rename a file
 *
 * Parameters:
 *  [IN] *oldpath     The old name of the file.
 *  [IN] *newpath     The new name of the file.
 *
 * Returns:
 *     0      success
 *     other  fail
 *
 * Notes:
 */
int sAPI_rename(const char *oldpath, const char *newpath);

/*
 * Name:        sAPI_access
 *
 * Description: access a file.
 *
 * Parameters:
 *  [IN] *path     The file name containing the path.
 *  [IN] mode  (Have no practical effect)
 *
 * Returns:
 *     0      file exists
 *     other  file doesn't exist
 *
 * Notes:Used to determine whether a file or directory exists in lfs
 */
int sAPI_access(const char *path, int mode);

/*
 * Name:        sAPI_stat
 *
 * Description: Gets the information structure of a file/directory
 *
 * Parameters:
 *  [IN] *path   The file/directory name containing the path.
 *  [IN] *info   the information structure of a file/directory(type,name,size)
 *
 * Returns:
 *     0      file exists
 *     other  file doesn't exist
 *
 * Notes:if the Parameter 1 is a directory,size is 0
 */
int sAPI_stat(const char *path, struct dirent_t *info);
/*
 * Name:        sAPI_GetSize
 *
 * Description: Get file system space size
 *
 * Parameters:
 *  [IN] *disc   The directory name.("C:/" or "D:/)
 *
 * Returns:
 *  file system space size
 */
long long sAPI_GetSize(char *disc);
/*
 * Name:        sAPI_GetFreeSize
 *
 * Description: Get the remaining space of the file system
 *
 * Parameters:
 *  [IN] *disc   The directory name.("C:/" or "D:/)
 *
 * Returns:
 *  the remaining space of the file system
 */
long long sAPI_GetFreeSize(char *disc);
/*
 * Name:        sAPI_GetUsedSize
 *
 * Description: Get the space used by the file system
 *
 * Parameters:
 *  [IN] *disc   The directory name.("C:/" or "D:/)
 *
 * Returns:
 *  the space used by the file system
 */
long long sAPI_GetUsedSize(char *disc);
/*
 * Name:        sAPI_truncate
 *
 * Description:  truncate a file to a specified length with name
 *
 * Parameters:
 *  [IN] fname      File name.
 *  [IN] length    truncated to a size of precisely length bytes
 * Returns:
 *  On success, zero is returned.  On error, -1 is returned
 * Notes: the file must be open for writing
 */

int sAPI_truncate(const char *fname, size_t length);

/*
 * Name:        sAPI_ftruncate
 *
 * Description:  truncate a file to a specified length with handle
 *
 * Parameters:
 *  [IN] *fp       File handle.
 *  [IN] length    truncated to a size of precisely length bytes
 * Returns:
 *  On success, zero is returned.  On error, -1 is returned
 * Notes: the file must be open for writing
 */
int sAPI_ftruncate(SCFILE *fp, size_t length);
int sAPI_FsSwitchDir(char *file_name, FILE_SEITH_DIR_E direction);

#ifdef CUS_BL
SCFILE *sAPI_fopen_nvm(const char *fname, const char *mode);
#endif

#endif

