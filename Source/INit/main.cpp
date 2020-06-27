#include "SimpleGPIO_SPI.h"
#include "SPI_SS_Def.H"
const unsigned char ChipSS[]={SS0,SS1,SS2,SS3,SS4,SS5,SS6,SS7};


void Init_ChipSS(){

        for (int i=0;i<sizeof(ChipSS);i++){
        //export the pins. I have used then pins described in the table above
         gpio_export_spi(ChipSS[i]);


        // set them as HIgh. The SS pins should be high when idle(no commuication)

         gpio_set_dir_spi(ChipSS[i], OUTPUT_PIN);


        gpio_set_value_spi(ChipSS[i], HIGH);


        }
}
#include<fstream>

using namespace std;
#define PATH1 "/root/CLem/REad/ReadCrc/Source_ReadCrcCCA1/crcflashcca"
#define PATH2 "/root/CLem/REad/ReadCrc/Source_ReadCrcCCA2/crcflashcca"
#define PATH3 "/root/CLem/REad/ReadCrc/Source_ReadCrcCCA3/crcflashcca"
#define PATH4 "/root/CLem/REad/ReadCrc/Source_ReadCrcCCA4/crcflashcca"
int main()
{

Init_ChipSS();

fstream ofs;

ofs.open(PATH1, ios::out | ios::trunc);
ofs.close(); 
ofs.open(PATH2, ios::out | ios::trunc);
ofs.close(); 
ofs.open(PATH3, ios::out | ios::trunc);
ofs.close(); 
ofs.open(PATH4, ios::out | ios::trunc);
ofs.close(); 
return 0;



}
