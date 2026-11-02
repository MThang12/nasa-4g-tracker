#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <dirent.h>
#include <ctype.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <linux/usbdevice_fs.h>
#include <linux/version.h>      
#include <fcntl.h>   
#include <errno.h>   
#include <termios.h> 
#include <time.h>  
#include <signal.h>

#include "aboot-tiny.h"


#define	_FILE_SIZE		200  //2MB


#define USB_DIR_BASE "/sys/bus/usb/devices/"
#define PATH_SIZE 1024
#define NETLINK_BUFFLEN (12 * 1024)
#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))

#define  MAX_PORTNAME_LEN 20
/*---------------------------------------------------------------------------*/
static FILE *firmware_file;
static void *firmware_data;
static size_t firmware_size;
static const char *device_name;
static int baud_rate = 115200;
static bool reboot_after_completed = true;
static bool from_memory;
static pthread_mutex_t aboot_mutex;
static pthread_cond_t aboot_cond;
static volatile bool aboot_stopped;
static int rc = 0;
/*---------------------------------------------------------------------------*/
static void aboot_tiny_callback(const aboot_tiny_message_t *msg, void *ctx)
{
  (void)ctx;

  switch(msg->event) {
  case ABOOT_TINY_EVENT_INIT:
    if(msg->u.message) {
      printf("ABOOT_TINY_EVENT_INIT:  \n");
      printf("%s\n", msg->u.message);
    }
    if(!msg->error && !aboot_tiny_start(device_name, baud_rate)) {
      break;
    }
    pthread_mutex_lock(&aboot_mutex);
    aboot_stopped = true;
    pthread_cond_signal(&aboot_cond);
    pthread_mutex_unlock(&aboot_mutex);
    break;

  case ABOOT_TINY_EVENT_START:
    if(msg->u.message) {
      printf("ABOOT_TINY_EVENT_START:  \n");
      printf("%s\n", msg->u.message);
    }
    if(!msg->error) {
      if(from_memory) {
        if(!aboot_tiny_download_data(firmware_data, firmware_size, reboot_after_completed)) {
          break;
        }
      } else {
        if(!aboot_tiny_download_file(firmware_file, firmware_size, reboot_after_completed)) {
          break;
        }
      }
    }
    aboot_tiny_stop();
    break;

  case ABOOT_TINY_EVENT_DOWNLOAD:
    if(msg->u.message) {
      printf("ABOOT_TINY_EVENT_DOWNLOAD:  \n");
      printf("%s\n", msg->u.message);
    }
    if(!msg->error) {
      break;
    }
    aboot_tiny_stop();
    break;

  case ABOOT_TINY_EVENT_STOP:
    if(msg->u.message) {
      printf("ABOOT_TINY_EVENT_STOP:  \n");
      printf("%s\n", msg->u.message);
    }
    pthread_mutex_lock(&aboot_mutex);
    aboot_stopped = true;
    pthread_cond_signal(&aboot_cond);
    pthread_mutex_unlock(&aboot_mutex);
    break;

  case ABOOT_TINY_EVENT_EXIT:
  case ABOOT_TINY_EVENT_LOG:
    if(msg->u.message) {
      printf("%s", msg->u.message);
    }
    break;

  case ABOOT_TINY_EVENT_PROGRESS:
    printf("ABOOT_TINY_EVENT_PROGRESS:  \n");
    printf("PROGRESS: %d\n", msg->u.progress);
    break;

  case ABOOT_TINY_EVENT_STATUS:
    printf("ABOOT_TINY_EVENT_STATUS:  \n");
    printf("STATUS: %s\n", msg->u.status);
    printf("ERROR: %d\n", msg->error);
    if(msg->error || !strcmp(msg->u.status, ABOOT_TINY_STATUS_FAILED)) {
      rc = -1;
      aboot_tiny_stop();
    } else if(!strcmp(msg->u.status, ABOOT_TINY_STATUS_SUCCEEDED)) {
      rc = 0;
      aboot_tiny_stop();
    }
    break;

  default:
    printf("Unknown event: %d\n", msg->event);
    break;
  }
}


/*---------------------------------------------------------------------------*/
extern int find_device_node(void);
extern char diagport[20];

int main(int argc, char *argv[])
{
  off_t base = 0;
  size_t size = 0;
  int ret = -1;
	int delay_count = 0;
  char *atcmd = "echo -e \"at+bootldr=1\\\\r\\\\n\"";
  char cmd[256];

  if((getopt(argc, argv , "m")) != -1)
  {
      from_memory = true;
  }

  do
	{
		ret = find_device_node();
		
		printf("ret= %d \n",ret);

		if(ret == 0)
		{
			printf("find device\n");
			memset(cmd,0,sizeof(cmd));
			snprintf(cmd,sizeof(cmd),"%s > %s",atcmd,diagport);
			printf("atcmd= %s \n",cmd);		
			system(cmd);
		}

		usleep(500000);
		delay_count++;
	}
	while (ret != 2 && delay_count <= 40);

  if(delay_count > 40)
	{
		printf("Device Force download port does not exist!\n");
		return 0;	
	}
	else
	{
		delay_count = 0;
	}

  const char *filename = argv[optind];
  struct stat st;

  if(stat(filename, &st) == -1) { //update packege status
    perror("stat");
    exit(EXIT_FAILURE);
  }

  if(size == 0) {
    size = st.st_size;
  } else {
    assert(base + size <= st.st_size);
  }

  firmware_file = fopen(filename, "rb");
  if(!firmware_file) {
    perror("fopen");
  }
  fseek(firmware_file, base, SEEK_SET);//cong file kaitou pianyi
  firmware_size = size;
  if(from_memory) {
    firmware_data = malloc(size);
    if(!firmware_data) {
      fprintf(stderr, "Out of memory\n");
      return -1;
    }
    if(fread(firmware_data, 1, size, firmware_file) != size) {
      fprintf(stderr, "Can not read enough data from file\n");
      return -1;
    }
  }

  /* Initialize mutex and condition variable objects */
  pthread_mutex_init(&aboot_mutex, NULL);
  pthread_cond_init(&aboot_cond, NULL);

  /*
   * Lock mutex and wait for signal.  Note that the pthread_cond_wait
   * routine will automatically and atomically unlock mutex while it waits.
   * Also, note that if aboot_stopped is true before this routine is run by
   * the waiting thread, the loop will be skipped to prevent pthread_cond_wait
   * from never returning.
   */
  pthread_mutex_lock(&aboot_mutex);
  aboot_stopped = false;
  aboot_tiny_register_cb(aboot_tiny_callback, NULL);
  if(aboot_tiny_init()) {
    rc = EXIT_FAILURE;
    goto cleanup;
  }
  while(!aboot_stopped) {
    pthread_cond_wait(&aboot_cond, &aboot_mutex);
  }

cleanup:
  fclose(firmware_file);
  if(firmware_data) {
    free(firmware_data);
  }
  pthread_mutex_unlock(&aboot_mutex);
  pthread_mutex_destroy(&aboot_mutex);
  pthread_cond_destroy(&aboot_cond);
  if(aboot_tiny_exit()) {
    rc = EXIT_FAILURE;
  }

  return rc;
}
/*---------------------------------------------------------------------------*/
