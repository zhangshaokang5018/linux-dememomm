#ifndef __DEMO_NEN_H_
#define __DEMO_MEM_H_

#include <linux/ioctl.h>

#define DEMO_MEM_TYPE           'K'
#define DEMO_MEM_CLEAN          _IO(DEMO_MEM_TYPE, 0X10)
#define DEMO_MEM_SETVAL         _IOW(DEMO_MEM_TYPE, 0x11, int)
#define DEMO_MEM_GETVAL         _IOR(DEMO_MEM_TYPE,0X12,int)


#endif