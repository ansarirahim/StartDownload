#include <sstream>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include "iosfwd"
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

#define CLK_PER_SEC_ACTUAL (99000)
static void AllFlashChipsSelect(PIN_VALUE x)
{
	gpio_set_value_spi(SS2, x);
	gpio_set_value_spi(SS0, x);
	gpio_set_value_spi(SS4, x);
	gpio_set_value_spi(SS6, x);
}
void msleep(int ms)
{
	struct timespec ts;

	ts.tv_sec = ms / 1000;
	ts.tv_nsec = (ms % 1000) * 1000000;

	nanosleep(&ts, NULL);
}
bool SearchForData(  uint8_t *buff,long size)
{
	for (long i=0;i<size;i++)
	{
		if(buff[i]<0xff)
			return true;
	}
	return false;
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
tByte FlashReadCmd[4];
tByte NVM16ReadCmd[3];
//tByte TempTxData128[128];
tByte Rxd256[256];
tByte Rxd[2048];
struct spi_ioc_transfer messageRead[2],message16Read[2],message16Read256[2];
struct spi_ioc_transfer messageWREN[1];// = {0, };
tByte WriteEnableCmd[1];
struct spi_ioc_transfer messageData[1];
struct spi_ioc_transfer messageWriteCmd[1] = {0, };
tByte WriteCmd[4];
static uint8_t mode;
static uint8_t bits = 8;
static uint32_t speed = 10000000;                  //need to be speeded up when working
static uint16_t delay = 5;
//changed to AT25512 write cycle time
unsigned int Spi_address=0;
tByte TempTxData128[128];
tByte TempTxNvmData128[128];
tByte Write16Cmd[3];
tByte TempByteBuffer[256];
char TemCharBuffer[256];
struct spi_ioc_transfer messageWrite16CmdFF[2] = {0, };
struct spi_ioc_transfer messageWrite16CmdData[2] = {0, };
tByte ReadFCS[4];
static void Init_16messageRead256()
{
	 	 	 	   message16Read256[0].tx_buf = (unsigned long) NVM16ReadCmd;
		    	   message16Read256[0].rx_buf = (unsigned long)NULL;
		    	   message16Read256[0].len = sizeof(NVM16ReadCmd);
		    	   message16Read256[0].cs_change = 0;//1;//0 working for P9_17

		    	   message16Read256[1].tx_buf = (unsigned long)NULL;
		    	   message16Read256[1].rx_buf = (unsigned long)Rxd256;
		    	   message16Read256[1].len = sizeof(Rxd256);
		    	   message16Read256[1].cs_change =0;// 1;

		    	   NVM16ReadCmd[0] =READ;
//		    	   (readcommand+1)= &(Spi_address>>16);//
//		    	  	    		  (readcommand+2) =&(Spi_address>>8);//
//		    	  	    		  (readcommand+3) =&(Spi_address);

}
///////////////////////////////
static void Init_messageRead()
{
	 	 	 	   messageRead[0].tx_buf = (unsigned long)FlashReadCmd;
		    	   messageRead[0].rx_buf = (unsigned long)NULL;
		    	   messageRead[0].len = sizeof(FlashReadCmd);
		    	   messageRead[0].cs_change =0;// 1;//0 working for P9_17

		    	   messageRead[1].tx_buf = (unsigned long)NULL;
		    	   messageRead[1].rx_buf = (unsigned long)Rxd256;///Rxd
		    	   messageRead[1].len = sizeof(Rxd256);
		    	   messageRead[1].cs_change = 0;//1;

		    	   FlashReadCmd[0] =READ;
//		    	   (readcommand+1)= &(Spi_address>>16);//
//		    	  	    		  (readcommand+2) =&(Spi_address>>8);//
//		    	  	    		  (readcommand+3) =&(Spi_address);

}

static void Init_16messageRead()
{
	 	 	 	   message16Read[0].tx_buf = (unsigned long) NVM16ReadCmd;
		    	   message16Read[0].rx_buf = (unsigned long)NULL;
		    	   message16Read[0].len = sizeof(NVM16ReadCmd);
		    	   message16Read[0].cs_change =0;// 1;//0 working for P9_17

		    	   message16Read[1].tx_buf = (unsigned long)NULL;
		    	   message16Read[1].rx_buf = (unsigned long)Rxd;
		    	   message16Read[1].len = sizeof(Rxd);
		    	   message16Read[1].cs_change = 0;//1;

		    	   NVM16ReadCmd[0] =READ;
//		    	   (readcommand+1)= &(Spi_address>>16);//
//		    	  	    		  (readcommand+2) =&(Spi_address>>8);//
//		    	  	    		  (readcommand+3) =&(Spi_address);

}
////////////////////////
static void Init_messageWREN(){

	        //setup spi transfer data structure

	messageWREN[0].tx_buf = (unsigned long)WriteEnableCmd;      //send the write enable command
	messageWREN[0].rx_buf = (unsigned long)NULL;
	messageWREN[0].len =1;// sizeof(WriteEnableCmd);
	messageWREN[0].cs_change =0;// 1;


}

static void Init_WriteCommand() {
	//     //setup spi transfer data structure

	messageWriteCmd[0].tx_buf = (unsigned long)WriteCmd;   //send the write command and address
	messageWriteCmd[0].rx_buf = (unsigned long)NULL;
	messageWriteCmd[0].len = sizeof(WriteCmd);
	messageWriteCmd[0].cs_change =0;// 1;
}


static void Init_WriteCommand16FF() {
	//     //setup spi transfer data structure
	  Write16Cmd[0]=WRITE;
	messageWrite16CmdFF[0].tx_buf = (unsigned long)Write16Cmd;   //send the write command and address
	messageWrite16CmdFF[0].rx_buf = (unsigned long)NULL;
	messageWrite16CmdFF[0].len = sizeof(Write16Cmd);
	messageWrite16CmdFF[0].cs_change =0;// 1;

	messageWrite16CmdFF[1].tx_buf = (unsigned long)TempTxData128;   //send the write command and address
	messageWrite16CmdFF[1].rx_buf = (unsigned long)NULL;
	messageWrite16CmdFF[1].len = sizeof(TempTxData128);
	messageWrite16CmdFF[1].cs_change = 0;//1;

}
static void Init_WriteCommand16Data() {
	//     //setup spi transfer data structure
	  Write16Cmd[0]=WRITE;
	messageWrite16CmdData[0].tx_buf = (unsigned long)Write16Cmd;   //send the write command and address
	messageWrite16CmdData[0].rx_buf = (unsigned long)NULL;
	messageWrite16CmdData[0].len = sizeof(Write16Cmd);
	messageWrite16CmdData[0].cs_change = 0;//1;

	messageWrite16CmdData[1].tx_buf = (unsigned long)TempTxNvmData128;   //send the write command and address
	messageWrite16CmdData[1].rx_buf = (unsigned long)NULL;
	messageWrite16CmdData[1].len = sizeof(TempTxNvmData128);
	messageWrite16CmdData[1].cs_change = 0;//1;

}
struct spi_ioc_transfer messageWrite16Cmd[1] = {0, };
static void Init_Write16Command() {
	//     //setup spi transfer data structure

	messageWrite16Cmd[0].tx_buf = (unsigned long)WriteCmd;   //send the write command and address
	messageWrite16Cmd[0].rx_buf = (unsigned long)NULL;
	messageWrite16Cmd[0].len = sizeof(Write16Cmd);
	messageWrite16Cmd[0].cs_change = 0;//1;
}
char ReadStatusRegCmd[1] = {RDSR,};
uint8_t StatusRegValues[2];
struct spi_ioc_transfer messageStatusReg[2];// = {0, };
static void Init_ReadStatusReg()
		{

	messageStatusReg[0].tx_buf = (unsigned long) ReadStatusRegCmd;
	messageStatusReg[0].rx_buf = (unsigned long)NULL;
	messageStatusReg[0].len = sizeof(ReadStatusRegCmd);
	messageStatusReg[0].cs_change =0;// 1;

	 messageStatusReg[1].tx_buf = (unsigned long)NULL;
	 messageStatusReg[1].rx_buf = (unsigned long)StatusRegValues;
	messageStatusReg[1].len = sizeof(StatusRegValues);
	messageStatusReg[1].cs_change = 0;//1;


		}
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
void initLCD_cd()
{

				system(Export_GPIO60);

			system(DIRECTION_SET_GPIO60);


			system(Set_GPIO60);//l595
			usleep(10000);
}

	int ReadFlashID_Cs(int fd,tByte Cs)
	{
	int memorystatus=0;

		//////////////////////
		 int ret;
		   int address;
		   char addresslow, addressmid,addresshigh;
 		   char readcommand[1] = {0x9f,};// addresshigh, addressmid,addresslow ,};

		    		    uint8_t rx[4] = {0, };                        //create an array of data to be received

		    		    struct spi_ioc_transfer message[2] = {0, };         //setup spi transfer data structure

		    		    message[0].tx_buf = (unsigned long)readcommand;
		    		    message[0].rx_buf = (unsigned long)NULL;
		    		    message[0].len = sizeof(readcommand);
		    		    message[0].cs_change =0;// 1;  //working commented to check other cs message[0].cs_change = 0;

		    		    message[1].tx_buf = (unsigned long)NULL;
		    		    message[1].rx_buf = (unsigned long)rx;
		    		    message[1].len = sizeof(rx);
		    		    message[1].cs_change = 0;//1;
		    		    //gpio_set_value_spi(Cs, LOW);
		    		    ret = ioctl(fd, SPI_IOC_MESSAGE(2), &message);      //spi check if sent
		    		    if (ret < 1)
		    		       pabort("can't send spi message");

		    		    for (unsigned int j = 0; j < sizeof(rx); j++) {         //prints returned array on the screen
		    		       if (!(j % 32))
		    		          puts("");
		    		       printf("%.2X ", rx[j]);
		    		    }
		    		    puts("");

		    		    puts("");
		    		    gpio_set_value_spi(Cs, HIGH);
		    		    switch(rx[0])
		    		    {
		    		    printf("\nManufactrer:");
		    		    case 0x20 :
		    		    	 printf("Macron");
		    		    	///\ TargetMemory.mf=macron_target;
		    		    	break;
		    		    }
		    		    switch(rx[1])
		    		   	    		    {
		    		    printf("\nMemory Type:");
		    		   	    		    case 0x80 :
		    		   	    		    			memorystatus=1;
		    		   	    		    ///\TargetMemory.mstatus=flash_target;


		    		   	    		     printf("Flash");
		    		   	    		    	break;
		    		   	    		    }
		    		    switch(rx[2])
		    		   	    		    {
		    		    printf("\nCapacity:");
		    		   	    		    case 0x14 :
		    		   	    		     printf("1-MByte\n");
		    		   	    		     memorystatus=11;
		    		   	    		///\  TargetMemory.PageSize=256;
		    		   	    		 ///\ TargetMemory.NoOfPages=1024*(1024/TargetMemory.PageSize);
		    		   	    		 ///\ TargetMemory.sectorsize=0x10000;
		    		   	    		    	break;
		    		   	    		    }
	if(rx[0]==0xff && rx[1]==0xff && rx[2]==0xff && rx[3]==0xff)
		{printf("\nTarget is not Connected");
		memorystatus=-2;
		}
	if(rx[0]==0x0 && rx[1]==0x0 && rx[2]==0x0 && rx[3]==0x0)
		{
		printf("\nTarget is NVM or unknown");
		memorystatus=2;
		}
	return memorystatus;
	}
void EraseFlashFull_Cs(int fd, tByte Css)///16,0,1000)
{

	   int ret;
	   int address;
	   char addresslow, addressmid,addresshigh;
	   clock_t start, end;
	   	   	    double duration;
	   	   //double clkpersec;
	   	   	    start = clock();
	   	    // diff
	   	    // do stuff


	   struct timeval tvBegin, tvEnd, tvDiff;
	  /// timeval_print(&tvBegin);
	    addresshigh= Spi_address>>16;//
	    		addressmid= Spi_address>>8;//
	    		 addresslow =Spi_address;

	    		//

	    		        	////	 printf("\nAdress=0x%.2X%.2X%.2X, Adress=%ld", addresshigh,addressmid,addresslow,Spi_address );
	   char writeenable[1] = {WREN, };
	   char writecommand[1] = {FLASH_CERS,};//, addresshigh, addressmid,addresslow, };
	 //  char data[] = {0x5a,0x65,0x4e,0x73,0x59,0x73};//65 4E 73 59 73 00 00 01 8C 5B 1E
	   //setcharbuffer(TemCharBuffer,64,0);
	  // memsetbuffer(TempByteBuffer,0x55,256);



	   struct spi_ioc_transfer message[2] = {0, };         //setup spi transfer data structure

	   message[0].tx_buf = (unsigned long)writeenable;      //send the write enable command
	   message[0].rx_buf = (unsigned long)NULL;
	   message[0].len = sizeof(writeenable);
	   message[0].cs_change =0;// 1;                     //chip select needs to be released

	   message[1].tx_buf = (unsigned long)writecommand;   //send the write command and address
	   message[1].rx_buf = (unsigned long)NULL;
	   message[1].len = sizeof(writecommand);
	   message[1].cs_change = 0;//1;                     //keep holding chip select state

//gpio_set_value_spi(Css, LOW);////	                      //release the chip select line

	   ret = ioctl(fd, SPI_IOC_MESSAGE(2), &message);      //spi check if sent
//gpio_set_value_spi(Css, HIGH);
	   if (ret < 1)
	      pabort("can't send spi message");

	  ///////////////////// usleep(5000);                              //wait 5ms for write command to complete
	   uint8_t rxx[2];
	    		// printf("\nDone :Sector Erase");
///gpio_set_value_spi(Css, LOW);
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
		    		    message[1].cs_change =0;// 1;
///gpio_set_value_spi(Css, LOW);
		    		    ret = ioctl(fd, SPI_IOC_MESSAGE(2), &message);      //spi check if sent
///gpio_set_value_spi(Css, HIGH);
		    		    if (ret < 1)
		    		       pabort("can't send spi message");
		    		    for (ret = 0; ret < sizeof(rxx); ret++) {         //prints returned array on the screen
		    		   	    		       if (!(ret % 32))
		    		   	    		          puts("");
		    		   	    		       printf("%.2X ", rxx[ret]);
		    		   	    		    }
		    		   usleep(10000);
	   }
	   while(rxx[0]&01!=0);
	printf("\nErased");
	  end = clock();
//gpio_set_value_spi(Css, HIGH);

		   	        // Compute the duration
		   	        duration = ((double)( end - start )) / CLK_PER_SEC_ACTUAL;

		   	        cout << duration << " seconds" << endl;
}
void Spi_Scan_Erase_Cs(unsigned int SizeofMemory,int fdd,tByte Css)
{
	time_t tcclem=time(NULL);
	int ret;
	for(ret=0;ret<128;ret++)
		SectorNoHavingData[ret]=false;
	ret=0;
	    		    	 std::cout << "\nScanning SS0 P_15 @48";
	    		    	 	    	//Scan24ReadAndEraseCS( fdd ,48,4096,0x10000);

	    		    	    Spi_address=0;

	    		    	printf("\nScanning..");

	    		    	///  for(unsigned  int page=0;page<nopages;)
	    		    		  for(unsigned  int page=0;page<SizeofMemory;)//512;)
	    		    	    {

	    		    	        		////


	    		    		  FlashReadCmd[1]= Spi_address>>16;//
	    		    		  FlashReadCmd[2] =Spi_address>>8;//
	    		    		  FlashReadCmd[3] =Spi_address;


	    		    		memset(Rxd256,0xff,0x100);
	    		    	  // spi_ioc_transfer messageRead[2] = {0, };         //setup spi transfer data structure


//	    		    	   gpio_set_value_spi(Css, LOW);

	    		    	   ret = ioctl(fdd, SPI_IOC_MESSAGE(2), &messageRead);      //spi check if sent

//	    		    	   gpio_set_value_spi(Css, HIGH);
	    		    	  // usleep(500);


	    		    	   if (ret < 1)
	    		    	      pabort("can't send spi message");

	    		    	   if(SearchForData(Rxd256,sizeof(Rxd256)))
	    		    	   		  {
	    		    		   printf("\nData @ Page No=%d =%X\n",page,page);

	    		    		SectorNoHavingData[ Spi_address/0x10000]=true;


	    		    	   		  Spi_address+=0x10000;

page=Spi_address>>8;


	    		    	   		  }
	    		    	   else
	    		    	   {
	    		    		   page++;//=8;
	    		    		   Spi_address=page<<8;
	    		    		//   Spi_address=page*sizeof(Rxd256);
	    		    		///   printf("\nold Adress=%lu =%X\n",Spi_address);

	    		    	   }

	    		    	   }

	    		    		  for(unsigned int i=0;i<16;i++)
	    		    			  {
	    		    			  if(SectorNoHavingData[i]==true)
	    		    			  {
	    		    				 /// printf( "\nSECOTR ADRESS=%lu =%X\n",(i*0x10000));

	    		    				  Spi_address=i*0x10000;
	    		    				 /// printf( "\nSpi_address=%lu =%X\n",Spi_address);
	    		    				  	    		    	///   		printf("\nAdress of Sector=%d =%X\n",Spi_address);
	    		    				  	    		    	   	//	AllFlashChipsSelect(LOW);
	    		    				  	    		    	   		ret = ioctl(fdd, SPI_IOC_MESSAGE(1), &messageWREN);
	    		    				  	    		    	   		if (ret < 1)
	    		    				  	    		    	   		pabort("can't send spi message");
	    		    				  	    		    	   	//	AllFlashChipsSelect(HIGH);
	    		    				  	    		    	   	 usleep(1);
	    		    				  	    		    	 	//AllFlashChipsSelect(LOW);
	    		    				  	    		    	   	WriteCmd[0]=FLASH_SE;
	    		    				  	    		    	   	WriteCmd[1]=Spi_address>>16;
	    		    				  	    		    	   	WriteCmd[2]=Spi_address>>8;
	    		    				  	    		    	   	WriteCmd[3]=Spi_address;
	    		    				  	    		    	   	ret = ioctl(fdd, SPI_IOC_MESSAGE(1), &messageWriteCmd);
	    		    				  	    		    	   	AllFlashChipsSelect(HIGH);
	    		    				  	    		    	   		    	   		if (ret < 1)
	    		    				  	    		    	   		    	   		pabort("can't send spi message");
	    		    				  	    		    	   		    while(1)
	    		    				  	    		    	   		    {

	    		    				  	    		    	   		     gpio_set_value_spi(Css, LOW);
	    		    				  	    		    	   		    	 		 ret = ioctl(fdd, SPI_IOC_MESSAGE(2), &messageStatusReg);
	    		    				  	    		    	   		    	 gpio_set_value_spi(Css, HIGH);
	    		    				  	    		    	   		    	   	if (ret < 1)
	    		    				  	    		    	   		    	   	   	pabort("can't send spi message");
	    		    				  	    		    	   		     for (ret = 0; ret < 2; ret++) {         //prints returned array on the screen
	    		    				  	    		    	     if (!(ret % 32))
	    		    				  	    		        puts("");
	    		    				  	    		   printf("%.2X ", StatusRegValues[ret]);
	    		    				  	    		      }
	    		    				  	    		 	if((StatusRegValues[0] & 01 )!=0)
	    		    				  	    		break;

	    		    				  	    		  usleep(5000);

	    		    				  	    		      }
	    		    				  	    		    	   		 msleep(800);




	    		    			  }
	    		    			  }
	    		    	  printf ("\nScanning and Erase time= %ld\n", time(NULL)-tcclem);
}
#include <iostream>
#include <string>
using namespace std;

typedef unsigned int tShort;
typedef unsigned char tByte;
enum MemoryType{faulty_target,nvm_target,flash_target};
enum Manufacturer{unknown_target,macron_target,sst_target};
#define NO_OF_FPAGES (512*1024)
#define NO_OF_NPAGES (0x20000)
class MemoryPage
{
   public:

      bool DataPresent;   // data is there or no
  char PageData[256];  // Data details.
     int PageNumber;   // Which PAge number
     // tByte MemoryAddressing;
//int pagesize;
};
/////////

int aclem,HowManyFpages;
int SizeofMemory[2];
#define FLASH_MEMORY 0

MemoryPage FMemorypages[NO_OF_FPAGES];
#define FPAGE_SIZE 256
const tByte ChipSS[]={SS0,SS1,SS2,SS3,SS4,SS5,SS6,SS7};
////tByte Rxd256[256];
class MemoryInf
{
   public:

     //

   unsigned int PageSize;
   unsigned long NoOfPages;
   unsigned long sectorsize;
   MemoryType mstatus;
   Manufacturer mf;


     // tByte MemoryAddressing;
};
MemoryInf TargetMemory;
unsigned int bytearray[16];
void InitPages(int m){
for (int i=0;i<m;i++)
	{
		FMemorypages[i].DataPresent=false;
		for(int j=0;j<256;j++)
		FMemorypages[i].PageData[j]=0xff;

	}}
string strg;
string strgff="FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF";
tByte  getNVM_CRC(tByte *CKBlock, uint32_t  Length, tByte Seed)
{
	tByte val,y,crc, bbit1;
	uint32_t  k;
	 crc = Seed;
	 for(k = 0; k < Length; k++)
	 {
		val = CKBlock[k];
	    crc = crc ^ ((val << 4));
		for(y = 0; y < 8; y++)
		{
			bbit1 = crc & 0x80;
			if(bbit1)
			{
				crc = crc ^ 74;
				bbit1 = 1;
			}
			else
				bbit1 = 0;
			crc = ((crc << 1) + bbit1);
		}
	    crc = crc ^ 82;
		crc = crc ^ (val >> 4);
	 }
	 return(crc);
}
tByte  PrintFPageHavingCRC(unsigned int x)///int size)
{
tByte yed=80;
	HowManyFpages=0;
	tByte temp256[256];
	memset(temp256,0xff,256);
	printf("\nNoOFPages=%d",x);
	for (int i=0;i<x;i++)
	{


		if(FMemorypages[i].DataPresent==true)
			{
			cout<<"\nData Present @ PageNo.=,0x";
			cout<<hex << i<<endl;
			HowManyFpages++;
			////usleep(1000000);
			yed=getNVM_CRC((tByte *)FMemorypages[i].PageData,256, yed);
			}
		else
			yed=getNVM_CRC(temp256,256, yed);
	}
	printf("\nTotal Fpages=%d\n",HowManyFpages);
	printf("\nTotal CRC=%d\n",yed);return yed;

}
   int charToByteArray(long int x,char *hexstring){
         int i;
int blen=0;
int offset=0;
int pn=0;
         int str_len = strlen(hexstring);

     	memset(bytearray, 0xFF, sizeof(bytearray));
     	blen=((str_len / 2));

         for (i = 0; i < ((str_len / 2)); i++) {
             sscanf(hexstring + 2*i, "%02x", &bytearray[i]);
         }
////printf("\nconverted bytes=");
offset=x%256;
pn=x/256;
////printf("\npn=%d, offset=%d , len=%d",pn,offset,blen);
int y=0;
        for(i=0;i<blen;i++)
        {

       	 y=offset+i;
       	 if(y>255)
       	 {
       		 FMemorypages[pn+1].PageData[0] =bytearray[i];
       		 printf("\nExceeding the array\n");
       		// sleep(3);

       	 }
       	 else
       		 FMemorypages[pn].PageData[offset+i] =bytearray[i];
       ///	BufferFpages[400][256];
       	//// cout<<hex<<bytearray[i]<<endl;
        }


         return 1;
     }
string getFirstWord(string text)
     {
            bool firstWordFound = false;
            string firstWord = "";

            for(int i = 0; i <= text.length() && firstWordFound == false; i++)
            {
                  if (text[i] = ' ')
                  {
                              firstWordFound = true;

                              for (int j = 0; j <= i; j++)
                              {
                                  firstWord += text[j];
                              }
                  }
            }

            return firstWord;
     }
tByte Crc_Buffer[2];
void InitFBufferData()
{
	uint16_t p=0;
	int *bytearray=new int[16];
		string word;
		 char *temp1;
		char *e;
		int k=0;

		string fw="";
		int crc_count=0;
		long int Spi_adresses=0;
		long int temp_adress=0;
		struct timeval t1, t2;
		    double elapsedTime;
	string strg2=";CRC=";

	std::cout <<"Read Clem line by line\n";

 ifstream infile;
infile.open ("/root/Melc/");///home/0004564.j2A");
aclem=0;
	 HowManyFpages=0;
//////////////////////////////
	// printf("\nTargetMemory.NoOfPages=%l",TargetMemory.NoOfPages);
	//sleep(3);
	    		 SizeofMemory[FLASH_MEMORY]=TargetMemory.NoOfPages*256;//1024*1024;
	 SizeofMemory[FLASH_MEMORY]=TargetMemory.NoOfPages*256;//1024*1024;
	    		 InitPages(2048);///4096);
	    	     while(aclem<1) // To get you all the lines.
	    	    		     		      {
	    	    		     		          getline(infile,strg); // Saves the line in STRING.
	    	    		     		       //   if (STRING != previousLine)
	    	    		     		          {
	    	    		     		             // previousLine=strg;
	    	    		     		              // Prints our STRING.
	    	    		     		              ///////////////////

	    	    		     		              if(strg[0]!=';')// && strg[1])//20084BA1//
	    	    		     		              {
	    	    		     		            	  if(getFirstWord(strg).length()<9)
	    	    		     		            	  {
	    	    		     		            		  if(strg[0]=='2')
	    	    		     		            		  {
	    	    		     		            			 if(strstr(strg.c_str(),strgff.c_str()))
	;
	    	    		     		            			 else
	    	    		     		            				{
	    	    		     		            				 printf("\nLine=");
	    	    		     		            				cout<<strg<<endl;
	    	    		     		            				 	 	 //fw=getFirstWord(strg);
	    	    		     		            				 stringstream stream(strg);
	    	    		     		            				 k=0;
	    	    		     		            			    while( getline(stream, word, ' ') )
	    	    		     		            			    {
	    	    		     		            			    	if(k==0)
	    	    		     		            			    	{
	    	    		     		            			    		 word=word.erase(0,1);
	    	    		     		            			    		      printf("\nLine[0]=");
	    	    		     		            			    		    	    		     		             cout<<word<<endl;
	    	    		     		            			    		 Spi_adresses = strtol (word.c_str(), &e, 16);

	    	    		     		            			    		  	printf("\nSpi Adres=%ld",Spi_adresses);
	    	    		     		            	       				printf("\nPageNo=%ld\n",(Spi_adresses>>8));
	    	    		     		            	       				//if(0x87C==(Spi_adresses>>8))
	    	    		     		            	       				//	sleep(3);
	    	    		     		            	       			//FMemorypages[i].DataPresent==true
	    	    		     		            			              				FMemorypages[Spi_adresses/256].DataPresent=true;//	FMemoryPages.DataPresent=true;

	    	    		     		            			    	}
	    	    		     		            			    	if(k==1)
	    	    		     		            			    	{
	    	    		     		            			    	//	usleep(500000);
	    	    		     		            			    		//temp1=word;
	    	    		     		            			    		temp1 = new char[word.length() +1];
	    	    		     		            			    		strcpy(temp1, word.c_str());

	    	    		     		            			    		//eventually, remember to delete cstr

	    	    		     		            			    		charToByteArray(Spi_adresses,temp1);


	    	    		     		            			    		//delete[] cstr;

	    	    		     		            			    	}
	    	    		     		            			    	k++;
	    	    		     		            			    	//cout << word << "\n";
	    	    		     		            			    }





	    	    		     		            				/// cout <<hex<<strtoul(getFirstWord(strg).substr(0, 2).c_str(), 0, 16)<<endl;

	    	    		     		            				}

	    	    		     		            			  //
	    	    		     		            			  /////////////////////////////////////
	    	    		     		            		  }
	    	    		     		            	  }
	    	    		     		              }
	    	    		     		              if(strstr(strg.c_str(),strg2.c_str()))//comparing two string if they contains
	    	    		     		              {

	    	    		     		                // crc_count++;
	    	    		     		               //  if( crc_count>1)
	    	    		     		                	 {
	    	    		     		                		 cout << " Merge Clem Flash-Data Reading is completed\n";
	    	    		     		                 break;
	    	    		     		                	 }
	    	    		     		              }

	    	    		     		              ////////////////////
	    	    		     		          }

	    	    		     		      }
	    	    		     		      infile.close();
	//sleep(3);
	    	    		     		     Crc_Buffer[0]=PrintFPageHavingCRC(2048);///4096);//  PrintFPagesHavingData(4096);
	//sleep(3);
	    	    		     		   // cout<<"Pls Enter Buffer Fpage No."<<endl;
	    	    		     		    	    		 //   	cin >> p;
	    	    		     		    	    	////ReadInitFBuffer( p);
}

void Init_ChipSS(){

	for (tShort i=0;i<sizeof(ChipSS);i++){
	//export the pins. I have used then pins described in the table above
	 gpio_export_spi(ChipSS[i]);


	// set them as HIgh. The SS pins should be high when idle(no commuication)

	 gpio_set_dir_spi(ChipSS[i], OUTPUT_PIN);


	gpio_set_value_spi(ChipSS[i], HIGH);


	}
}
void PrintFPagesHavingData(int x)///int size)
{

	HowManyFpages=0;
	printf("\nNoOFPages=%d",x);
	for (int i=0;i<x;i++)
	{
		if(FMemorypages[i].DataPresent==true)
			{
			cout<<"\nData Present @ PageNo.=,0x";
			cout<<hex << i<<endl;
			HowManyFpages++;
			////usleep(1000000);
			}
	}
	printf("\nTotal Fpages=%d\n",HowManyFpages);


}
static void Simultanious24Write(int fd,unsigned long int NoOfPAges)
{

	time_t time_t0=time(NULL);
	int ret;
char addresshigh,addressmid,addresslow;
	    		// for(unsigned long k=0;k<512;k++)
printf("\nNo. OF Pages=%d",NoOfPAges);
	for (unsigned long int k;k<NoOfPAges;k++)
	{
		if(FMemorypages[k].DataPresent==true)

		{


			Spi_address=k<<8;//FClem256[k]<<8;
	    		        		   addresshigh= Spi_address>>16;//
	    		        		    		        		addressmid= Spi_address>>8;//
	    		        		    		        		 addresslow =Spi_address;
	    		         printf("\nAdress=0x%.2X%.2X%.2X, Adress=%ld", addresshigh,addressmid,addresslow,Spi_address );
	   char writeenable[1] = {WREN, };
	   char writecommand[4] = {WRITE, addresshigh, addressmid,addresslow, };

	   ///setcharbuffer(TemCharBuffer,256,set);




	   struct spi_ioc_transfer message[1] = {0, };         //setup spi transfer data structure

	   message[0].tx_buf = (unsigned long)writeenable;      //send the write enable command
	   message[0].rx_buf = (unsigned long)NULL;
	   message[0].len = sizeof(writeenable);
	   message[0].cs_change = 1;
	   //chip select needs to be released
	   usleep(500);
	   gpio_set_value_spi(SS2, LOW);
	   gpio_set_value_spi(SS0, LOW);
	   gpio_set_value_spi(SS4, LOW);
	   gpio_set_value_spi(SS6, LOW);
	     ret = ioctl(fd, SPI_IOC_MESSAGE(1), &message);      //spi check if sent
	     if (ret < 1)
	        pabort("can't send spi message");
	     gpio_set_value_spi(SS2, HIGH);
	     gpio_set_value_spi(SS0, HIGH);
	     gpio_set_value_spi(SS4, HIGH);
	    gpio_set_value_spi(SS6, HIGH);
	   ///////////////////////////////////////////////////////
	     struct spi_ioc_transfer message1[2] = {0, };         //setup spi transfer data structure

	     message1[0].tx_buf = (unsigned long)writecommand;   //send the write command and address
	     message1[0].rx_buf = (unsigned long)NULL;
	     message1[0].len = sizeof(writecommand);
	     message1[0].cs_change = 1;
	     message1[1].tx_buf = (unsigned long)FMemorypages[k].PageData;;//data;//TempByteBuffer;//data;         //send the data
	       message1[1].rx_buf = (unsigned long)NULL;
	       message1[1].len = sizeof(FMemorypages[k].PageData);//data);//TempByteBuffer);
	       message1[1].cs_change = 1;                     //release the chip select line
	        usleep(500);
	        gpio_set_value_spi(SS2, LOW);
	        gpio_set_value_spi(SS0, LOW);
	        gpio_set_value_spi(SS4, LOW);
	       gpio_set_value_spi(SS6, LOW);

	          ret = ioctl(fd, SPI_IOC_MESSAGE(2), &message1);      //spi check if sent
	          if (ret < 1)
	             pabort("can't send spi message");

	          gpio_set_value_spi(SS2, HIGH);
	          gpio_set_value_spi(SS0, HIGH);
	          gpio_set_value_spi(SS4, HIGH);
	          gpio_set_value_spi(SS6, HIGH);
	          usleep(5000);
	    /////////////////////////////////////////////////////////////////////////////////////
	}


	}
	  printf ("\n**Execution time= %ld\n", time(NULL)-time_t0);
}
static void Simultanious16Write(int fd,unsigned long int NoOfPAges);
// MemoryPage *FMemorypages= new MemoryPage[NO_OF_FPAGES];
  //      MemoryPage *NMemorypages= new MemoryPage[NO_OF_NPAGES];
void MenuSelection(int fdd)
{unsigned long tempadress;
unsigned int pagen;
	    	tShort sss;
string word;
	 char *temp1;
	 time_t tcclem;
	char *e;
	int k=0;
	string fw="";
	int crc_count=0;
	long int Spi_adresses=0;
	long int temp_adress=0;
	struct timeval t1, t2;
	    double elapsedTime;
string strg2=";CRC=";
	//char *SelectionItems[]={"Erase",
	int selection,selection1,selection2;
	uint16_t p=0;
	uint32_t a=0;
	uint16_t offset=0;
	int v=0;
	int ret=0;
	//
	int selection3=0;
	const char* timeserverip="ntpdate 172.16.15.101";

	 const char * mountpath="mount -t cifs //databia1/ppe_soft/ -o username=abdulr,password=Sanjays@12345 /mnt/share";

 ////system(timeserverip);
/////	 system(mountpath);
	// int *bytearray=new int[16];
std::string enteredcmd;
///	MemoryPage *FMemorypages= new MemoryPage[NO_OF_FPAGES];
///	MemoryPage *NMemorypages= new MemoryPage[NO_OF_NPAGES];
	/*for (ret=0;ret<NO_OF_NPAGES;ret++)
	{
		for (selection=0;selection<256;selection++)

	NMemorypages[ret].PageData[selection]=0xff;
	}*/
	selection=0;
	ret=0;

	 std::cout << "FLASH Buffered Data: \n";
	 InitFBufferData();
	 Init_ChipSS();
//	fdd= SpiInit(fdd);
	std::vector<std::string> sentences;
//	    std::string s;
	    std::string s;
mainmenu:
	 do
	    {
	        std::cout << "Please make a selection: \n";
	        std::cout << "1) Erase\n";
	        std::cout << "2) Program\n";
	        std::cout << "3) Verify\n";
	        std::cout << "4) PageRead\n";
	        std::cout << "5) PageWrite\n";
	        std::cout << "6) Full DataCheck\n";
	        std::cout << "7) Page DataCheck\n";
	        std::cout << "8) Write Byte\n";
	        std::cout << "9) Smart Erase\n";
	        std::cout << "10) FILL NVM WITH ZEROS\n";
	        std::cout << "11) Write MAC ID\n";
	        std::cout<< "12) Update MAC ID Value\n";
	        std::cout<< "13) LCD Test\n";
	        std::cout<< "14) Read Status Register\n";
	        std::cout<< "15) Read Flash ID\n";
	        std::cout << "16) Clem\n";
	        std::cout << "17) List Mounted Drive\n";
	        std::cout << "18) Read Buffer Page\n";
	        std::cout << "19) Read Buffer CRC\n";
	        std::cout << "20) Program the Clem\n";
	        std::cout << "21) Read & Erase SS0\n";//Read Buffer Page\n";
	        std::cout << "22) Read & Erase SS2\n";
	        std::cout << "23) Read & Erase SS4\n";
	        std::cout << "24) Read & Erase SS6\n";
	       std::cout << "25)  Read24 CRC SS0\n";
	       std::cout << "26)  Read24 CRC SS2\n";
	       std::cout << "27)  Read24 CRC SS4\n";
	       std::cout << "28)  Read24 CRC SS6\n";

	       std::cout << "29)  Simultaneous Program Clem\n";
	       std::cout << "30) Premier300 Download & Verify\n";
	       std::cout << "31) 64KNVM Dwownload & Verify\n";
	       std::cout << "32) Initialize Target by Product ID\n";
	       std::cout << "33) Buffer To Target\n";
	       std::cout<<"34) Read Flash ID\n";
 std::getline (std::cin,enteredcmd);
	     ///   std::cout<< "13) Write a Single Byte \n";
	     ////  scanf ("%x", &selection);
	     //  std::string MyString;
	      // getline (cin, MyString);
	     //   cout << "Hello= " << MyString << ".\n";
if(enteredcmd.compare("READ_BUFFERED_FLASH")==0)
{
 PrintFPagesHavingData(256);



}
else	if(enteredcmd.compare("WRITE_CCA_FLASH")==0)
	{cout<<"ACK"<<endl;break;}
		Simultanious16Write(fdd,256);
                  cout<<"ACK"<<endl;break;

	       ///printf("\n..........selection=%d",selection);
	     // selection=stoi(MyString);
	      ///  cin.ignore(numeric_limits<streamsize>::max(),'\n');
	    }
	    while (1);//selection>33 && selection==0);//selection!=21 && selection!=20 && selection!=19 && selection!=18 && selection!=17 && selection!=16 && selection !=15 && selection !=14 && selection !=13 &&selection !=12 && selection != 11 && selection != 10 && selection != 9 && selection != 8 && selection != 1 && selection != 2 &&
	       // selection != 3 && selection != 4 && selection != 5&& selection != 6 && selection != 7);

	    // do something with selection here
	    // such as a switch statement

	    /////std::cout << "Writing....."<<endl;
///MenuSelection(fd);
}
/////
static void Simultanious16Write(int fd,unsigned long int NoOfPAges)
{

////	time_t time_t0=time(NULL);
	int ret;
Spi_address=0;
char addresshigh,addressmid,addresslow;
	    		// for(unsigned long k=0;k<512;k++)
printf("\nNo. OF Pages=%d",NoOfPAges);
	for (unsigned long int k=0;k<NoOfPAges;k++)
	{
		if(FMemorypages[k].DataPresent==true)

		{


			//Spi_address=k<<8;//FClem256[k]<<8;
	    		  //      		   addresshigh= Spi_address>>16;//
	    		        		    		        		addressmid= Spi_address>>8;//
	    		        		    		        		 addresslow =Spi_address;
	    		         printf("\nAdress=0x%.2X%.2X, Adress=%ld",addressmid,addresslow,Spi_address );
	   char writeenable[1] = {WREN, };
	   char writecommand[4] = {WRITE,  addressmid,addresslow, };

	   ///setcharbuffer(TemCharBuffer,256,set);




	   struct spi_ioc_transfer message[1] = {0, };         //setup spi transfer data structure

	   message[0].tx_buf = (unsigned long)writeenable;      //send the write enable command
	   message[0].rx_buf = (unsigned long)NULL;
	   message[0].len = sizeof(writeenable);
	   message[0].cs_change = 1;
	   //chip select needs to be released
	   usleep(500);
	   gpio_set_value_spi(SS2, LOW);
	   gpio_set_value_spi(SS0, LOW);
	   gpio_set_value_spi(SS4, LOW);
	   gpio_set_value_spi(SS6, LOW);
	     ret = ioctl(fd, SPI_IOC_MESSAGE(1), &message);      //spi check if sent
	     if (ret < 1)
	        pabort("can't send spi message");
	     gpio_set_value_spi(SS2, HIGH);
	     gpio_set_value_spi(SS0, HIGH);
	     gpio_set_value_spi(SS4, HIGH);
	    gpio_set_value_spi(SS6, HIGH);
	   ///////////////////////////////////////////////////////
	     struct spi_ioc_transfer message1[2] = {0, };         //setup spi transfer data structure

	     message1[0].tx_buf = (unsigned long)writecommand;   //send the write command and address
	     message1[0].rx_buf = (unsigned long)NULL;
	     message1[0].len = sizeof(writecommand);
	     message1[0].cs_change = 1;
	     message1[1].tx_buf = (unsigned long)FMemorypages[k].PageData;;//data;//TempByteBuffer;//data;         //send the data
	       message1[1].rx_buf = (unsigned long)NULL;
	       message1[1].len = sizeof(FMemorypages[k].PageData);//data);//TempByteBuffer);
	       message1[1].cs_change = 1;                     //release the chip select line
	        usleep(500);
	        gpio_set_value_spi(SS2, LOW);
	        gpio_set_value_spi(SS0, LOW);
	        gpio_set_value_spi(SS4, LOW);
	       gpio_set_value_spi(SS6, LOW);

	          ret = ioctl(fd, SPI_IOC_MESSAGE(2), &message1);      //spi check if sent
	          if (ret < 1)
	             pabort("can't send spi message");

	          gpio_set_value_spi(SS2, HIGH);
	          gpio_set_value_spi(SS0, HIGH);
	          gpio_set_value_spi(SS4, HIGH);
	          gpio_set_value_spi(SS6, HIGH);
	          usleep(5000);
	    /////////////////////////////////////////////////////////////////////////////////////
	}


	}
	///  printf ("\n**Execution time= %ld\n", time(NULL)-time_t0);
}

int main()///(int argc, char *argv[])



{
   int ret = 0;
   int fd;
   memset(TempTxData128,0x96,sizeof(TempTxData128));
   initLCD_cd();
   Init_16messageRead256();
Init_16messageRead();
   Init_messageRead();
   Init_WriteCommand();
   Init_Write16Command();
   Init_messageWREN();
   Init_ReadStatusReg();
   Init_WriteCommand16FF();
   Init_WriteCommand16Data();
//////////////////
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

		    		   puts("");
		    		   printf("The spi mode is set to: %d\n", mode);      //output successful settings to the terminal
		    		   printf("Bits per word: %d\n", bits);
		    		   speed=10000;
		    		   printf("Max speed is set to: %d Hz (%d KHz) (%d MHz)\n", speed, speed/1000, speed/1000000);

//	   ReadFlashID_Cs(fd,P9_15);
//Spi_Scan_Erase_Cs(2048,fd,P9_15);;
///\EraseFlashFull_Cs(fd, P9_15);
	   //close(fd);

////MenuSelection(fd);
InitFBufferData();
         Init_ChipSS();
///////////////////////////////
std::string enteredcmd;
while(1){
 std::getline (std::cin,enteredcmd);
             ///   std::cout<< "13) Write a Single Byte \n";
             ////  scanf ("%x", &selection);
             //  std::string MyString;
              // getline (cin, MyString);
             //   cout << "Hello= " << MyString << ".\n";
if(enteredcmd.compare("ERASE_FLASH_CCA1")==0)
{
system("ERASE_FLASH_CCA1");
}
else if(enteredcmd.compare("UPDATE_CRC_FLASH_CCA1")==0)
{
system("UPDATE_CRC_FLASH_CCA1");
}
else if(enteredcmd.compare("GET_COMPARE_CRC_FLASH_CCA1")==0)
{
system("GET_COMPARE_CRC_FLASH_CCA1");
}
else if(enteredcmd.compare("HELP")==0){
cout<<"ERASE_FLASH_CCA1"<<endl;
cout<<"WRITEON_FLASH"<<endl;
cout<<"UPDATE_CRC_FLASH_CCA1"<<endl;
cout<<"GET_COMPARE_CRC_FLASH_CCA1"<<endl;

}
else
if(enteredcmd.compare("WRITEON_FLASH")==0)
{


/////////////////////////////
Spi_address=0;
char addressmid,addresslow,addresshigh;

// ERASE_FLASH_CCA1 
//ACK

 //UPDATE_CRC_FLASH_CCA1 
//GET_COMPARE_CRC_FLASH_CCA1 

	    		// for(unsigned long k=0;k<512;k++)
///printf("\nNo. OF Pages=%d",NoOfPAges);

////////////
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

                                   puts("");
                                   printf("The spi mode is set to: %d\n", mode);      //output successful settings to the terminal
                                   printf("Bits per word: %d\n", bits);
                                   speed=10000;
                                   printf("Max speed is set to: %d Hz (%d KHz) (%d MHz)\n", speed, speed/1000, speed/1000000);

////////////
	for (unsigned long int k=0;k<2048;k++)
	{
		if(FMemorypages[k].DataPresent==true)

		{


			Spi_address=k<<8;//FClem256[k]<<8;
	    	addresshigh= Spi_address>>16;//
	    		        		    		        		addressmid= Spi_address>>8;//
	    		        		    		        		 addresslow =Spi_address;
	    		         ///printf("\nWriting..Adress=%.2X %.2X%.2X, Adress=%ld",addresshigh, addressmid,addresslow,Spi_address );
	   char writeenable[1] = {WREN, };
	   char writecommand[4] = {WRITE,addresshigh, addressmid,addresslow, };

	   ///setcharbuffer(TemCharBuffer,256,set);




	   struct spi_ioc_transfer message[1] = {0, };         //setup spi transfer data structure

	   message[0].tx_buf = (unsigned long)writeenable;      //send the write enable command
	   message[0].rx_buf = (unsigned long)NULL;
	   message[0].len = sizeof(writeenable);
	   message[0].cs_change = 0;
	   //chip select needs to be released
	   usleep(500);
	   gpio_set_value_spi(SS2, LOW);
	   gpio_set_value_spi(SS0, LOW);
	   gpio_set_value_spi(SS4, LOW);
	   gpio_set_value_spi(SS6, LOW);
	     ret = ioctl(fd, SPI_IOC_MESSAGE(1), &message);      //spi check if sent
	     if (ret < 1)
	        pabort("can't send spi message");
	     gpio_set_value_spi(SS2, HIGH);
	     gpio_set_value_spi(SS0, HIGH);
	     gpio_set_value_spi(SS4, HIGH);
	    gpio_set_value_spi(SS6, HIGH);
	   ///////////////////////////////////////////////////////
	     struct spi_ioc_transfer message1[2] = {0, };         //setup spi transfer data structure

	     message1[0].tx_buf = (unsigned long)writecommand;   //send the write command and address
	     message1[0].rx_buf = (unsigned long)NULL;
	     message1[0].len = sizeof(writecommand);
	     message1[0].cs_change = 0;
	     message1[1].tx_buf = (unsigned long)FMemorypages[k].PageData;;//data;//TempByteBuffer;//data;         //send the data
	       message1[1].rx_buf = (unsigned long)NULL;
	       message1[1].len = sizeof(FMemorypages[k].PageData);//data);//TempByteBuffer);
	       message1[1].cs_change = 1;                     //release the chip select line
	        usleep(500);
	        gpio_set_value_spi(SS2, LOW);
	        gpio_set_value_spi(SS0, LOW);
	        gpio_set_value_spi(SS4, LOW);
	       gpio_set_value_spi(SS6, LOW);

	          ret = ioctl(fd, SPI_IOC_MESSAGE(2), &message1);      //spi check if sent
	          if (ret < 1)
	             pabort("can't send spi message");

	          gpio_set_value_spi(SS2, HIGH);
	          gpio_set_value_spi(SS0, HIGH);
	          gpio_set_value_spi(SS4, HIGH);
	          gpio_set_value_spi(SS6, HIGH);
	          usleep(5000);
	    /////////////////////////////////////////////////////////////////////////////////////
	}


	}
////////////////////////crc16//
///
close(fd);
}
else
{
{
cout<<"ERASE_FLASH_CCA1"<<endl;
cout<<"WRITEON_FLASH"<<endl;
cout<<"UPDATE_CRC_FLASH_CCA1"<<endl;
cout<<"GET_COMPARE_CRC_FLASH_CCA1"<<endl;

}


}

}
return 0;
}
	static int Flash24ReadSingle_Crc_Cs(int fd ,unsigned long int NoOfPages,tByte Css){//int readnoofdevices) {

			char addresshigh,addressmid,addresslow;
			int ret;
			ReadFCS[0]=SS0;
			ReadFCS[1]=SS2;//,SS2,SS4,SS6};
			ReadFCS[2]=SS4;
				ReadFCS[3]=SS6;

			tByte yeed=80;

		//	for (int card=from;card<to;card++)
			{

			    for(unsigned long int k=0;k<NoOfPages;k++)
			    {
			        		        		 Spi_address=k<<8;//FClem256[k]<<8;
			        		        		   addresshigh= Spi_address>>16;//
			        		        		    		        		addressmid= Spi_address>>8;//
			        		        		    		        		 addresslow =Spi_address;
			        		//// printf("\nAdress=0x%.2X%.2X%.2X, Adress=%ld", addresshigh,addressmid,addresslow,Spi_address );

			        		        		    		        	       /// 		 printf("\nAdress=0x%.2X%.2X%.2X, Adress=%ld", addresshigh,addressmid,addresslow,Spi_address );
			        		        		    		        	   char readcommand[4] = {READ, addresshigh, addressmid,addresslow ,};

			        		        		    		        	   uint8_t rx[256] = {0, };                        //create an array of data to be received

			        		        		    		        	   struct spi_ioc_transfer message[2] = {0, };         //setup spi transfer data structure

			        		        		    		        	   message[0].tx_buf = (unsigned long)readcommand;
			        		        		    		        	   message[0].rx_buf = (unsigned long)NULL;
			        		        		    		        	   message[0].len = sizeof(readcommand);
			        		        		    		        	   message[0].cs_change = 1;//0 working for P9_17

			        		        		    		        	   message[1].tx_buf = (unsigned long)NULL;
			        		        		    		        	   message[1].rx_buf = (unsigned long)rx;
			        		        		    		        	   message[1].len = sizeof(rx);
			        		        		    		        	   message[1].cs_change = 1;
			        		        		    		        	   gpio_set_value_spi(Css, LOW);

			        		        		    		        	   usleep(5000);
			        		        		    		        	   ret = ioctl(fd, SPI_IOC_MESSAGE(2), &message);      //spi check if sent

			        		        		    		        	            gpio_set_value_spi(Css, HIGH);


			   if (ret < 1)
			      pabort("can't send spi message");

			  /* for (ret = 0; ret < sizeof(rx); ret++) {         //prints returned array on the screen
			      if (!(ret % 32))
			         puts("");
			      printf("%.2X ", rx[ret]);
			   }
			   puts("");
			   puts("");*/
			   yeed=getNVM_CRC(rx,256, yeed);
			//usleep(100000);
			}
			    printf("\nFlash Card No. =%d Crc=%d,0x%.2X ",Css, yeed,yeed);
			    if(Crc_Buffer[0]==yeed)
			    	return 1;
			    else
			    	return 0;
			}
		}
