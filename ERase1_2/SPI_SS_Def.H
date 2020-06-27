/*
 * SPI_SS_Def.H
 *
 *  Created on: Jun 6, 2016
 *      Author: root
 */

#ifndef SPI_SS_DEF_H_
#define SPI_SS_DEF_H_

# define P9_11 30
# define P9_12 60
# define P9_13 31
# define P9_14 50
# define P9_15 48
# define P9_16 51
# define P9_17 5
# define P9_18 4

# define P9_21 3
# define P9_22 2
# define P9_23 49
# define P9_24 15
# define P9_25 117
# define P9_26 14
# define P9_27 115
# define P9_28 113
# define P9_29 111
# define P9_30 112
# define P9_31 110
# define P9_41 20
# define P9_42 7

#define SSS  P9_17
#define SS0  P9_15
#define SS1  P9_23
#define SS2 P9_12
#define SS3  P9_27
#define SS4  P9_41
#define SS5  P9_42
#define SS6  P9_30
#define SS7  P9_31
extern unsigned char Rxd[];

char *PinNamesSSS[]={"P9_15","P9_23","P9_12","P9_27","P9_41","P9_42","P9_30","P9_31"};
#define ONE_MB (1024*1024)
#define LENGTH_OF_1MB (ONE_MB/sizeof(Rxd))
void msleep(unsigned long msec);

#endif /* SPI_SS_DEF_H_ */
