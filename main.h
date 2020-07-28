#ifndef __MAIN_H__
#define __MAIN_H__

#define ABS(x) ((x)>0? (x):(-(x))) 
#define LIMIT_MAX_MIN(x, max, min)	(((x) <= (min)) ? (min):(((x) >= (max)) ? (max) : (x)))  

//Standard Lib
#include <stm32f4xx.h>	 
#include <stm32f4xx_conf.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
//Mylib
//#include "laser.h"
//#include "MicroSw.h"
//#include "pid.h"
//#include "can1.h"
//#include "can2.h"
#include "tim8.h"
//#include "usart1.h"
//#include "usart6.h"
//#include "frictionwheel.h"
//#include "algorithmOfCRC.h"
//#include "SteeringEngine.h"
//Task lib
//#include "ZeroCheckTask.h"
//#include "DataSendTask.h"
//#include "DataReceivetask.h"
//#include "ActionTask.h"
//#include "GimbalTask.h"
//#include "ShootTask.h"
//#include "ChassisTask.h"

//typedef struct
//{
//	  unsigned short MagOpen;
//	  unsigned short MagClose;
//	  unsigned short Pitch_init;
//	  unsigned short Yaw_init;
//}RobotPRM_Struct;

//void InfantryidInit(void);
void System_Init(void);
void System_Configration(void);
void delay_ms(unsigned int t);
#endif
