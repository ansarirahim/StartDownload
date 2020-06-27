#include <iostream>
#include <iostream>
#include "SimpleGPIO_SPI.h"
#include "SPI_SS_Def.H"
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
//#include <spidev.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
using std::cout;
using std::endl;
bool SectorNoHavingNData[128];
bool SectorNoHavingData[128];
static uint8_t mode;
static uint8_t bits = 8;
static uint32_t speed = 10000000;                  //need to be speeded up when working
static uint16_t delay = 5;
int time_enable=0;
#define CLK_PER_SEC_ACTUAL (1000000)
//(99000)

void msleep(int ms)
{
	struct timespec ts;

	ts.tv_sec = ms / 1000;
	ts.tv_nsec = (ms % 1000) * 1000000;

	nanosleep(&ts, NULL);
}

#define FLASH_CERS 0xC7
#define FLASH_SE   0xD8
//definitions for AT25512 device
#define WRITE_CYCLE_TIME (5000)                     //AT25512 write cycle time in us
#define WRSR (0x01)                              //AT25512 write status register
#define WRITE (0x02)                           //AT25512 write data to memory array
#define READ (0x03)                              //AT25512 read data from memory array
#define WRDI (0x04)                              //AT25512 reset write enable latch
#define RDSR (0x05)                              //AT25512 read status register
#define WREN (0x06)                              //AT25512 set write enable latch
static void pabort(const char *s)
{
   perror(s);
   abort();
}
typedef unsigned char tByte;
//tByte TempTxData128[128];
tByte Rxd256[256];
tByte Rxd[2048];
const char *DIRECTION_SET_GPIO48="echo high > /sys/class/gpio/gpio48/direction";
const char *DIRECTION_SET_GPIO50="echo high > /sys/class/gpio/gpio50/direction";
const char *DIRECTION_SET_GPIO51=	"echo high > /sys/class/gpio/gpio51/direction";
const char *DIRECTION_SET_GPIO60=	"echo high > /sys/class/gpio/gpio60/direction";
const char *DIRECTION_SET_GPIO115=	"echo high > /sys/class/gpio/gpio115/direction";
const char *DIRECTION_SET_GPIO49="echo high > /sys/class/gpio/gpio49/direction";//qhc


const char *Export_GPIO48="echo 48 > /sys/class/gpio/export";
const char *Export_GPIO50=" echo 50 > /sys/class/gpio/export";
const char *Export_GPIO51="echo 51 > /sys/class/gpio/export";
const char *Export_GPIO60="echo 60 > /sys/class/gpio/export";


const char *DIRECTION_CLEAR_GPIO48="echo low > /sys/class/gpio/gpio48/direction";
const char *DIRECTION_CLEAR_GPIO50="echo low > /sys/class/gpio/gpio50/direction";
const char *DIRECTION_CLEAR_GPIO51=	"echo low > /sys/class/gpio/gpio51/direction";
const char *DIRECTION_CLEAR_GPIO60=	"echo low > /sys/class/gpio/gpio60/direction";
const char *DIRECTION_CLEAR_GPIO115=	"echo low > /sys/class/gpio/gpio115/direction";
const char *DIRECTION_CLEAR_GPIO49="echo low > /sys/class/gpio/gpio49/direction";//qhc
////////////////////////////////////////////////////

const char *Clear_GPIO48="echo 0 > /sys/class/gpio/gpio48/value";
const char *Clear_GPIO50="echo 0 > /sys/class/gpio/gpio50/value";
const char *Clear_GPIO51="echo 0 > /sys/class/gpio/gpio51/value";
const char *Clear_GPIO60="echo 0 > /sys/class/gpio/gpio60/value";

const char *Set_GPIO48="echo 1 > /sys/class/gpio/gpio48/value";
const char *Set_GPIO50="echo 1 > /sys/class/gpio/gpio50/value";
const char *Set_GPIO51="echo 1 > /sys/class/gpio/gpio51/value";
const char *Set_GPIO60="echo 1 > /sys/class/gpio/gpio60/value";
void timeval_print(struct timeval *tv)
{
    char buffer[30];
    time_t curtime;

    printf("\n%ld.%06ld", tv->tv_sec, tv->tv_usec);
    curtime = tv->tv_sec;
    strftime(buffer, 30, "%m-%d-%Y  %T", localtime(&curtime));
    printf(" = %s.%06ld\n", buffer, tv->tv_usec);
}
///multiple erase
void EraseFlashFull_Cs(int time_enable,int fd, tByte Css)///16,0,1000)
{

	   int ret;
	   int address;
	   char addresslow, addressmid,addresshigh;
//	   clock_t start, end;
///	   	   	    double duration;
	   	   //double clkpersec;
////if(time_enable)
///	   	   	    start = clock();
	   	    // diff
/*cout<<"\nStart="<<endl;
for(int i=0;i<1000;i++)
usleep(1000);
duration = (((double)( end - start )) / CLK_PER_SEC_ACTUAL)/1000;
////
                             cout << duration << " seconds" << endl;	   	    // do stuff
*/


	   ///struct timeval tvBegin, tvEnd, tvDiff;
	 ///  timeval_print(&tvBegin);
///	    addresshigh= Spi_address>>16;//
///	    		addressmid= Spi_address>>8;//
//	    		 addresslow =Spi_address;

	    		//

	    		        	////	 printf("\nAdress=0x%.2X%.2X%.2X, Adress=%ld", addresshigh,addressmid,addresslow,Spi_address );
	   char writeenable[1] = {WREN, };
	   char writecommand[1] = {FLASH_CERS,};//, addresshigh, addressmid,addresslow, };
	 //  char data[] = {0x5a,0x65,0x4e,0x73,0x59,0x73};//65 4E 73 59 73 00 00 01 8C 5B 1E
	   //setcharbuffer(TemCharBuffer,64,0);
	  // memsetbuffer(TempByteBuffer,0x55,256);



	   struct spi_ioc_transfer message[1]={0,};//2] = {0, };         //setup spi transfer data structure

	   message[0].tx_buf = (unsigned long)writeenable;      //send the write enable command
	   message[0].rx_buf = (unsigned long)NULL;
	   message[0].len = sizeof(writeenable);
	   message[0].cs_change = 1;                     //chip select needs to be released
/*
	   message[1].tx_buf = (unsigned long)writecommand;   //send the write command and address
	   message[1].rx_buf = (unsigned long)NULL;
	   message[1].len = sizeof(writecommand);
	   message[1].cs_change = 1;                     //keep holding chip select state
*/
gpio_set_value_spi(Css, LOW);////	                      //release the chip select line

	   ret = ioctl(fd, SPI_IOC_MESSAGE(1), &message);      //spi check if sent
gpio_set_value_spi(Css, HIGH);
	   if (ret < 1)
	      pabort("can't send spi message");
 message[0].tx_buf = (unsigned long)writecommand;   //send the write command and address
           message[0].rx_buf = (unsigned long)NULL;
           message[0].len = sizeof(writecommand);
           message[0].cs_change = 1;
gpio_set_value_spi(Css, LOW);
 ret = ioctl(fd, SPI_IOC_MESSAGE(1), &message);
gpio_set_value_spi(Css, HIGH);
if (ret < 1)
              pabort("can't send spi message");
	  ///////////////////// usleep(5000);                              //wait 5ms for write command to complete
	   uint8_t rxx[2];
	    		// printf("\nDone :Sector Erase");
usleep(100);
gpio_set_value_spi(Css, LOW);
	///////////////////////////////////////
	   do {
	   char readsr[1] = {RDSR,};// addresshigh, addressmid,addresslow ,};

		    		     rxx[2] = {0, };                        //create an array of data to be received

		    		    struct spi_ioc_transfer message[2] = {0, };         //setup spi transfer data structure

		    		    message[0].tx_buf = (unsigned long)readsr;
		    		    message[0].rx_buf = (unsigned long)NULL;
		    		    message[0].len = sizeof(readsr);
		    		    message[0].cs_change = 0;

		    		    message[1].tx_buf = (unsigned long)NULL;
		    		    message[1].rx_buf = (unsigned long)rxx;
		    		    message[1].len = sizeof(rxx);
		    		    message[1].cs_change = 1;
///gpio_set_value_spi(Css, LOW);
		    		    ret = ioctl(fd, SPI_IOC_MESSAGE(2), &message);      //spi check if sent
///gpio_set_value_spi(Css, HIGH);
		    		    if (ret < 1)
		    		       pabort("can't send spi message");
		    		   /* for (ret = 0; ret < sizeof(rxx); ret++) {         //prints returned array on the screen
		    		   	    		       if (!(ret % 32))
		    		   	    		          puts("");
		    		   	    		       printf("%.2X ", rxx[ret]);
		    		   	    		    }*/

		    		   usleep(10000);
if((rxx[0]==2)||(rxx[0]==0xff)){
cout<<"\nNACK\r\nFlash or It's Connection Bad"<<endl;
break;
}
	   }
	   while(rxx[0]&01!=0);
////\	cout<<"ACK\r\n"<<endl;///printf("\nErased");
//////	  end = clock();
gpio_set_value_spi(Css, HIGH);

		   	        // Compute the duration
      ///                     if(time_enable){
	///	   	        duration =( ((double)( end - start )) / CLK_PER_SEC_ACTUAL);

		   	  ///      cout << duration << "mili seconds" << endl;}

}
//Spi_Scan_Erase_Cs(2048,fd,P9_15);;
///////if(pCss==48)
int main(){
int ret,fd;
fd = open("/dev/spidev1.0", O_RDWR);            //open the spi device
                                   if (fd < 0)
                                      pabort("can't open device");

                                   ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);         //set the spi mode
                                   if (ret == -1)
                                      pabort("can't set spi mode");

                                   ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);         //get the spi mode (test)
                                   if (ret == -1)
                                      pabort("can't get spi mode");

                                   ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);   //set the spi bits per word
                                   if (ret == -1)
                                      pabort("can't set bits per word");

                                   ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);   //get the spi bits per word (test)
                                   if (ret == -1)
                                      pabort("can't get bits per word");

                                   ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);   //set the spi max speed
                                   if (ret == -1)
                                      pabort("can't set max speed hz");

                                   ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);   //get the spi max speed (test)
                                   if (ret == -1)
                                      pabort("can't get max speed hz");

                                   ///puts("");
                                   ///printf("The spi mode is set to: %d\n", mode);      //output successful settings to the terminal
                                   ///printf("Bits per word: %d\n", bits);
                                   speed=100000;
                                   ///printf("Max speed is set to: %d Hz (%d KHz) (%d MHz)\n", speed, speed/1000, speed/1000000);
EraseFlashFull_Cs(time_enable,fd,P9_12);/// pCss);///P9_15);
EraseFlashFull_Cs(time_enable,fd,P9_15);

close(fd);
cout<<"ACK"<<endl;
	   
return 0;
}
