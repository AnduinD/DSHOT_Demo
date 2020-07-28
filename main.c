#include "main.h"
extern uint16_t throttle;//油门值
/**
  * @brief  Main Function
  * @param  None
  * @retval None
  */
int main()
{
    throttle=0;                //给一段时间的油门零点检测（解锁）
    TIM8_Configuration();      //摩擦轮通信的DSHOT协议用
    delay_ms(80000);
    throttle=200;              //随便输出一个油门值先
//	delay_ms(40000);
//  throttle=200;              //改一下油门值
//  delay_ms(40000);
	
//    int tick=0;
    while(1){
//        tick++;
//        throttle=(tick%2)? 100: 200;
//        delay_ms(8000);
    }
} 

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
