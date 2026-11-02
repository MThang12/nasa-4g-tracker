/*
 * DiagSaver application
 *
 * Marvell Diag log saver on Linux - 1.0.0.0
 *
 * Copyright (C) 1995-2014 Marvell.Co.,LTD
 */
#include <sys/ioctl.h>
#include <sys/types.h>
#include <dirent.h>
#include <pthread.h>
#include <ctype.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <linux/usbdevice_fs.h>
#include <linux/usbdevice_fs.h>
#include <linux/version.h>
#include <stdio.h>  
#include <stdlib.h> 
#include <string.h>   
#include <unistd.h>   
#include <fcntl.h>   
#include <errno.h>   
#include <termios.h> 
#include <time.h>  
#include <signal.h>
#include <sys/time.h> 
#include <sys/stat.h>

#define	_FILE_SIZE		200  //2MB


#define USB_DIR_BASE "/sys/bus/usb/devices/"
#define PATH_SIZE 1024
#define NETLINK_BUFFLEN (12 * 1024)
#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))

#define  MAX_PORTNAME_LEN 20

char diagport[MAX_PORTNAME_LEN];    /* diag port */
//const char *idVendor;
// char *idProduct;
const char *gbcdDevice;

char filename[256] = {0};
FILE *fp = NULL;
unsigned int g_uiFileIndex = 0x0;

struct CSDLFileHeader
{
	long    dwHeaderVersion;//0x0
	long    dwDataFormat;//0x1
	long    dwAPVersion;
	long    dwCPVersion;
	long    dwSequenceNum;//ÎÄ¼þÐòºÅ£¬´Ó0¿ªÊ¼µÝÔö
	long    dwTime;//Total seconds from 1970.1.1 0:0:0
	long    dwCheckSum;//0x0
};

static int readfile(char *path, char *content, size_t size)
{
    int ret;
    FILE *f;
    f = fopen(path, "r");
    if (f == NULL)
        return -1;

    ret = fread(content, 1, size, f);
    fclose(f);
    return ret;
}
static int refreshhzc_port_name(char *device_path, char* port_name, const char* bcddevice)
{
    struct dirent *dent;
    DIR *usbdir;
    int ret = -1;

    //printf("refreshhzc_port_name found path:%s port:%s bcdDevice:%s\n",
        //device_path,port_name,bcddevice);

    usbdir = opendir(device_path);
    if (usbdir == NULL) {
        return ret;
    }

    while ((dent = readdir(usbdir)) != NULL)
    {
        if (strcmp(dent->d_name, ".") == 0
            || strcmp(dent->d_name, "..") == 0)
            continue;

        //printf("d_name:%s\n",dent->d_name);

        if(strncmp(dent->d_name,"tty",3)== 0)
        {
            sprintf(port_name,"/dev/%s",dent->d_name);
            printf("usb d_name:%s\n",dent->d_name);
            ret = 0;
        }

    }
    closedir(usbdir);
    return ret;
}

static int refresh_device_name(char *device_path)
{
    struct dirent *dent;
    DIR *usbdir;
    char path[PATH_SIZE];
    char bInterfaceNumber[8];
    unsigned char bInterfaceNumber_i = 0;
    int ret = -1;


    usbdir = opendir(device_path);
    if (usbdir == NULL) {
        return ret;
    }

    memset(path, 0, PATH_SIZE);
    memset(diagport,0, MAX_PORTNAME_LEN);


    while ((dent = readdir(usbdir)) != NULL) {

        memset(bInterfaceNumber, 0, sizeof(bInterfaceNumber));

        strcpy(path, device_path);
        strcat(path, "/");
        strcat(path, dent->d_name);
        strcat(path, "/bInterfaceNumber");

        if (readfile(path, bInterfaceNumber, 4) <= 0)
            continue;

        bInterfaceNumber_i = atoi(bInterfaceNumber);

        //printf("bInterfaceNumber path=%s:%d,%s \n",path,bInterfaceNumber_i,bInterfaceNumber);

//PATH = /sys/bus/usb/devices/1-1.1/1-1.1:1.1
        if(bInterfaceNumber_i == 2)
        {
            strcpy(path, device_path);
            strcat(path, "/");
            strcat(path, dent->d_name);
            printf("path = %s\n",path);
            ret = refreshhzc_port_name(path,diagport,gbcdDevice);
            printf("diagport = %s\n",diagport);
            break;
        }
    }

    closedir(usbdir);

    return ret;
}

int find_device_node(void)
{
    struct dirent *dent;
    DIR *usbdir;
    char path[PATH_SIZE], path2[PATH_SIZE];
    char idvendor[64];
    char idproduct[64];
    char bcddevice[64];
    int ret = -1;



    usbdir = opendir(USB_DIR_BASE);
    if (usbdir == NULL) {
        printf("opendir -- %s error\n", USB_DIR_BASE);

        return ret;
    }

    memset(path, 0, PATH_SIZE);
    memset(path2, 0, PATH_SIZE);

    while ((dent = readdir(usbdir)) != NULL)
    {
        if (strcmp(dent->d_name, ".") == 0 || strcmp(dent->d_name, "..") == 0)
        {
            continue;
        }

        memset(idvendor, 0, sizeof(idvendor));
        memset(idproduct, 0, sizeof(idproduct));
        memset(bcddevice, 0, sizeof(idproduct));
        strcpy(path, USB_DIR_BASE);
        strcat(path, dent->d_name);

        //printf("find_matched_device -- path=%s\n",path);

        strcpy(path2, path);
        strcat(path2, "/idVendor");

        if (readfile(path2, idvendor, 4) <= 0)
            continue;

        strcpy(path2, path);
        strcat(path2, "/idProduct");

        if (readfile(path2, idproduct, 4) <= 0)
            continue;

        strcpy(path2, path);
        strcat(path2, "/bcdDevice");

        if (readfile(path2, bcddevice, 4) <= 0)
            continue;

        bcddevice[4]='\0'; //sometimes there is an empty character at the end of the string of bcdDevice value.

        //printf("find devices: idVendor=%s,idProduct=%s, bcdDevice=%s.\n",
        //        (char*)idvendor, (char*)idproduct, (char*)bcddevice);

        if((strncmp(idvendor,"1e0e",4) == 0) && (strncmp(idproduct,"9011",4) == 0))
        {
            printf("find devices: idVendor=%s,idProduct=%s, bcdDevice=%s.\n",
                (char*)idvendor, (char*)idproduct, (char*)bcddevice);
            printf("devices find ok!\n");
            ret = refresh_device_name(path);
            break;
        }
    }

    closedir(usbdir);

    return ret;
}


int openport(char *pszDeviceName)   
 {
	 int fd = open( pszDeviceName, O_RDWR|O_SYNC|O_NOCTTY|O_NDELAY ); 
	 if (-1 == fd) 
	 {    
		  perror("Can't Open Diag device!");
		  return -1;  
	 } 
	 else 
	 {
		 ////tprintf (_T("\nOpened Diag DeviceName : %s\n"),pszDeviceName);
	  	return fd;
	 }
 }   
    
int setport(int fd, int baud,int databits,int stopbits,int parity)
{
	 int baudrate;
	 struct   termios   newtio;   
	 switch(baud)
	 {
		 case 300:
			  baudrate=B300;
			  break;
		 case 600:
			  baudrate=B600;
			  break;
		 case 1200:
			  baudrate=B1200;
			  break;
		 case 2400:
			  baudrate=B2400;
			  break;
		 case 4800:
			  baudrate=B4800;
			  break;
		 case 9600:
			  baudrate=B9600;
			  break;
		 case 19200:
			  baudrate=B19200;
			  break;
		 case 38400:
			  baudrate=B38400;
			  break;
		 default :
			  baudrate=B9600;  
			  break;
	 }
	 tcgetattr(fd,&newtio);     
	 bzero(&newtio,sizeof(newtio));   
	   //setting   c_cflag 
	 newtio.c_cflag   &=~CSIZE;     
	 switch (databits) 
	 {   
		 case 7:  
		  	newtio.c_cflag |= CS7; 
		  	break;
		 case 8:     
			  newtio.c_cflag |= CS8; 
			  break;   
		 default:    
			  newtio.c_cflag |= CS8;
			  break;    
	 }
	 switch (parity) //ÉèÖÃÐ£Ñé
	 {   
		 case 'n':
		 case 'N':    
			  newtio.c_cflag &= ~PARENB;   /* Clear parity enable */
			  newtio.c_iflag &= ~INPCK;     /* Enable parity checking */ 
			  break;  
		 case 'o':   
		 case 'O':     
			  newtio.c_cflag |= (PARODD | PARENB);
			  newtio.c_iflag |= INPCK;             /* Disnable parity checking */ 
			  break;  
		 case 'e':  
		 case 'E':   
			newtio.c_cflag |= PARENB;     /* Enable parity */    
			newtio.c_cflag &= ~PARODD;      
			newtio.c_iflag |= INPCK;       /* Disnable parity checking */
			break;
		 case 'S': 
		 case 's':  /*as no parity*/   
			newtio.c_cflag &= ~PARENB;
			newtio.c_cflag &= ~CSTOPB;
			break;  
		 default:   
			  newtio.c_cflag &= ~PARENB;   /* Clear parity enable */
			  newtio.c_iflag &= ~INPCK;     /* Enable parity checking */ 
			  break;   
	 } 
	 switch (stopbits)//ÉèÖÃÍ£Ö¹Î»
	 {   
		 case 1:    
			  newtio.c_cflag &= ~CSTOPB;  //1
			  break;  
		 case 2:    
			  newtio.c_cflag |= CSTOPB;  //2
			  break;
		 default:  
			  newtio.c_cflag &= ~CSTOPB;  
			  break;  
	 } 
	 newtio.c_cc[VTIME] = 0;    
	 newtio.c_cc[VMIN] = 0; 
	 newtio.c_cflag   |=   (CLOCAL|CREAD);
	 //newtio.c_oflag|=OPOST; 
	 newtio.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); //Input, RawData mode
	 newtio.c_oflag &= ~OPOST; //Output
	 newtio.c_iflag   &=~(IXON|IXOFF|IXANY);  
	 cfsetispeed(&newtio,baudrate);   
	 cfsetospeed(&newtio,baudrate);   
	 tcflush(fd,   TCIFLUSH); 
	 if (tcsetattr(fd,TCSANOW,&newtio) != 0)   
	 { 
		  perror("SetupSerial 3");  
		  return -1;  
	 }  
	 return 0;
}

int writeport(int fd,char *buf,int len)
{
	int i;
	write(fd,buf,len);
	printf("Send CMD to UE: ");
	for(i=0; i<16; i++)
		printf("0x%2X ", buf[i]);
	printf("\n");
}

void clearport(int fd) 
{
 	tcflush(fd,TCIOFLUSH);
}

int readport(int fd,char *buf,int maxwaittime)
{
	 int len=0;
	 int rc;
	 int i;
	 
	 struct timeval tv;
	 fd_set readfd;
	 tv.tv_sec=maxwaittime/1000;    //SECOND
	 tv.tv_usec=maxwaittime%1000*1000;  //USECOND
	 FD_ZERO(&readfd);
	 FD_SET(fd,&readfd);
	 rc=select(fd+1,&readfd,NULL,NULL,&tv);
	 if(rc>0)
	 {
		rc=read(fd,buf,4096);
		printf("recv:%d\n",rc);
		return rc;
	 }
	 else
	 {
		return -1;
	 }
}
int savelog(char *buf, int len)
{
        int i;
        time_t ltime;
        struct tm *currtime;
        char openmode[4] = {0};
        struct stat fileinfo;
        struct CSDLFileHeader SDLHeader;
        char *pHeader = (char *)&SDLHeader;
        // init the SDL file header
        SDLHeader.dwHeaderVersion = 0x0;
        SDLHeader.dwDataFormat = 0x1;//0x1
        SDLHeader.dwAPVersion = 0x0;
        SDLHeader.dwCPVersion = 0x0;
        SDLHeader.dwTime = 0x0;//Total seconds from 1970.1.1 0:0:0
        SDLHeader.dwCheckSum = 0x0;//0x0
        if ( 0 == strlen(filename)  )
        {
                time(&ltime);
                SDLHeader.dwTime = (long)ltime;
                SDLHeader.dwSequenceNum = g_uiFileIndex;//?ļ???ţ???0??ʼ????
                currtime = localtime(&ltime);
                sprintf(filename, "%d_%d_%d_%d_%d_%d.sdl", (currtime->tm_year+1900), (currtime->tm_mon+1), currtime->tm_mday, currtime->tm_hour, currtime->tm_min, currtime->tm_sec);
                sprintf(openmode, "wb");
                fp = fopen(filename, openmode);
                // Write the file header.
                for(i=0; i<28; i++)
                        fputc(pHeader[i], fp);
        }
        else
        {
                stat(filename, &fileinfo);
                if( fileinfo.st_size > _FILE_SIZE*1024*1024-len )
                {
                        fclose(fp);
                        g_uiFileIndex++;        // Increse the file index
                        time(&ltime);
                        SDLHeader.dwTime = (long)ltime;
                        SDLHeader.dwSequenceNum = g_uiFileIndex;//?ļ???ţ???0??ʼ????
                        currtime = localtime(&ltime);
                        sprintf(filename, "%d_%d_%d_%d_%d_%d.sdl", currtime->tm_year+1900, currtime->tm_mon+1, currtime->tm_mday, currtime->tm_hour, currtime->tm_min, currtime->tm_sec);
                        sprintf(openmode, "wb");
                        fp = fopen(filename, openmode);
                }
                else
                {
                        sprintf(openmode, "ab");
                }
        }
        
        // Write the data to current file.
        for(i=0; i<len; i++)
                fputc(buf[i], fp);
	
}

void ctrl_c_process(int signo)
{
        if(fp>0)
	    fclose(fp);
	printf("\nExiting DiagSaver...\n");
	exit(1);
}

int main()   
{   
	 int   fd,rc,i,ret,readlen;   	
	 FILE * fp;
	 char buffer[80];
	 
	 readlen = 0;

	 unsigned char rbuf[8192] = {0}; 
	 unsigned char ACATReady[16]={0x10, 0x00, 0x00, 0x00, 0x00, 0x04, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};	// ACAT Ready command
	 unsigned char GetAPDBVer[16]={0x10, 0x00, 0x00, 0x00, 0x80, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};	// Get AP DB Ver command
	 unsigned char GetCPDBVer[16]={0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};	// Get CP DB Ver command
	//char dev[256];	 
	char *dev = diagport;    //Set the ZTE Diag device path

	// Capture the Ctrl+C
	struct sigaction act;
	act.sa_handler = ctrl_c_process;
	act.sa_flags = 0;

	if( sigaction(SIGINT, &act, NULL) < 0 )
	{
		printf("Install Signal Action Error: %s \n", strerror(errno));
		return 0;
	}

	//printf("Please enter the device path to capture the log. [Example: /dev/ttyACM1]:\n");
	//scanf("%s", dev);


step1:
	while(find_device_node() < 0)
	{
		printf("Device does not exist!\n");
		sleep(5);
		//return 0;		
	}



	//Open Diag device
	 fd  =  openport(dev);
	 
	 if(fd>0)
	 {	
		printf("Opened the device %s\n", dev);
		if ( strstr( dev, "MDiagUSB") == NULL )
		{
			ret=setport(fd,115200,8,1,'n');  //Set serial port
			if(ret<0)
			{
			  printf("Can't Set Serial Port!\n");
			  close(fd); 
			  //return 0;
			  goto step1;
			}
			else
			{
				printf("Set Serial Port Done.\n");
			}
		}
	 }
	 else
	 {
		  printf("Can't Open Serial Port!\n");
		  //return 0;
		  goto step1;
	 }
	 
	//Send the ACAT Ready to UE
	 writeport(fd,ACATReady,16);
	 usleep(200);
	 writeport(fd,GetAPDBVer,16);
	 usleep(300);
	 writeport(fd,GetCPDBVer,16);
	 
	printf("Sent CMD, receiving data. Press Ctrl+C to exit.\n");
	 while(1)
	{
		rc=readport(fd,rbuf,500);   //Read data, timeout is 500 ms
		if(rc>0)
		{		   
			//for(i=0;i<rc;i++)
			//	printf("%02x ",rbuf[i]);
			//printf("\n");
			savelog(rbuf, rc);
		}
		else   //no log recv
		{
			//writeport(fd,wbuf,16);
		   	printf("recv none, data length: %d Bytes\n", rc);
            if(find_device_node() < 0)   //if device is not exist
            {
			    close(fd);    //close fd
			    goto step1;   
            }
		}
	 }
	 
	 if(fd>0)
	 {   
	     close(fd);
	 }   
}   
