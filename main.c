#include "main.h"
//RobotPRM_Struct Infantry;
//Status_t *status;
//extern Status_t Status;
//extern short FrictionWheel_speed;
extern uint16_t throttle;//����ֵ
/**
  * @brief  Main Function
  * @param  None
  * @retval None
  */
int main()
{
	//System_Configration();
	//System_Init();
    //SysTick_Config(168000);
    //delay_ms(7000);
    throttle=0;                //��һ��ʱ�����������⣨������
    TIM8_Configuration();      //Ħ����ͨ�ŵ�DSHOTЭ����
    delay_ms(80000);
    throttle=200;              //������һ������ֵ��
//	delay_ms(40000);
//  throttle=200;              //��һ������ֵ
//  delay_ms(40000);
	
//    int tick=0;
    while(1){
//        tick++;
//        throttle=(tick%2)? 100: 200;
//        delay_ms(8000);
    }
} 
/**
  * @brief  Configuration 
  * @param  None
  * @retval None
  */
void System_Configration()
{
//	LASER_Configration();
//	delay_ms(100);
	//TIM8_Configuration();      //Ħ����ͨ�ŵ�DSHOTЭ����
	//delay_ms(100);
//	FrictionWheel_Configration();
//    delay_ms(100);
//	TIM8_Configuration();
//	USART1_Configuration();    //��ң����
//	delay_ms(100);
//    USART6_Configuration();    //��Tx2
//	delay_ms(100);
//  CAN1_Configration();
//	delay_ms(100);
//	CAN2_Configration();
//	delay_ms(10000);
}
/**
  * @brief  Parameter 
  * @param  None
  * @retval None
  */
void System_Init(void)
{
//	ZeroCheck_Init();
//	InfantryidInit();
//	Pid_ChassisWheelInit();
//	Pid_ChassisPosition();
//	Pid_BodanMotorPos();
//	Pid_BodanMotorSpeed();
//	Pid_YawGyroPosSpeed();
//	Pid_PitchGyroPosSpeed();
//	Pid_Pitch_MotorPos_GyroSpeed();
//	Pid_Yaw_MotorPos_GyroSpeed();
//	status = GetStatusAdress();
}
/**
  * @brief  System timer 
  * @param  None
  * @retval None
  */
//extern short RC_DisConnect;
//extern short F105_DisConect;
void SysTick_Handler(void)
{
//	/*���������*/
//	ZeroCheck_cal();
//	/*״̬��*/
//	Status_Act();
//	/*Ħ���ֿ���*/
//	FrictionWheel_Set(FrictionWheel_speed);
//	/*��̨���̷����������*/
//	if(Status.GimbalMode != Gimbal_Powerdown_Mode)
//        Gimbal_CurrentPid_Cal();
//	BodanMotor_CurrentPid_Cal();
//	Chassis_CurrentPid_Cal();
//	/*Tx2ͨ��*/
//	USART6_SendtoPC();
//	/*ң�������߼��*/
//	if(RC_DisConnect>1000)
//		RC_Rst();
//	RC_DisConnect++;
//	/*���ݰ���߼��*/
//	if(F105_DisConect>1500)
//		F105_Rst();
//	F105_DisConect++;
}

///**
//  * @brief  Infantry parameter initialization
//  * @param  None
//  * @retval None
//  */
//void InfantryidInit(void)
//{
//	Infantry.Yaw_init=1374;
//	Infantry.Pitch_init=6419;  
//	Infantry.MagClose=2900;
//	Infantry.MagOpen=1300;
//}
/**
  * @brief  Millisecond delay
  * @param  None
  * @retval None
  */
void delay_ms(unsigned int t)
{
	int i;
	for( i=0;i<t;i++)
	{
		int a=10300;
 		while(a--);
	}
}
