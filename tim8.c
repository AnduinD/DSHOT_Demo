#include "main.h"
#define ESC_BIT_1 14   /*800:400*/ /*32:16*/
#define ESC_BIT_0  7   /*800:400*/ /*32:16*/
#define SRC_DATA_LEN 32+4  //(最后面放几个0做延迟)
uint16_t throttle;//=100;//0x7ff;//先随便定的一个油门值
uint8_t BIT[16];//DSHOT的16个字节的数据
uint32_t SRC_Buffer[SRC_DATA_LEN];//该数组存储传输的高低电平个数
    
/**
  * @brief  TIM8工作模式配置
  * @param  None
  * @retval None
  */
void TIM8_Configuration(void)             
{
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_OCInitTypeDef TIM_OCInitStruct;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
	DMA_InitTypeDef DMA_InitStructure;
	NVIC_InitTypeDef  nvic;
    
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8,ENABLE);      //使能TIM8时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC,ENABLE);     //使能GPIOC时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2,ENABLE);      //使能DMA2时钟

	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_8|GPIO_Pin_9;      //PC8 Tim8--Ch3     PC9 Tim8--Ch4
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF;              //复用输出模式  
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_DOWN;            //下拉(到GND)输出
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;          //推挽输出
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_100MHz;        //输出速率
	GPIO_Init(GPIOC,&GPIO_InitStructure);                   //按照该结构体配置对应端口

	GPIO_PinAFConfig(GPIOC,GPIO_PinSource8,GPIO_AF_TIM8);   //用引脚PC8；把TIM8引到AF3
	GPIO_PinAFConfig(GPIOC,GPIO_PinSource9,GPIO_AF_TIM8);   //用引脚PC9；把TIM8引到AF3
	
    DMA_DeInit(DMA2_Stream1);                                               //TIM8_UP的DMA通道
    DMA_InitStructure.DMA_Channel =DMA_Channel_7;                           //输入通道为Channel_7,即TIM8_UP的
	DMA_InitStructure.DMA_PeripheralBaseAddr =(uint32_t)&(TIM8->DMAR);      //外设基地址为TIM8的DMAR寄存器
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)SRC_Buffer;	        //存储器基地址为SRC_Buffer变量的位置
	DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;	                //用DMA的传输方向为存储器到外设
	DMA_InitStructure.DMA_BufferSize = SRC_DATA_LEN;                        //要传输的数据项数（32个数据，每个32位，这个值写到NDTR寄存器里面去）(为什么设成32就只有15个波？设成33才行？)
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;        //外设地址不递增
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;                 //存储器地址递增
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word; //外设数据大小为一个字长（32位）
	DMA_InitStructure.DMA_MemoryDataSize = DMA_PeripheralDataSize_Word;     //存储器数据大小为一个字长（32位）
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;                         //DMA工作在循环模式
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;                 //DMA通道优先级配置（11b）
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Enable;                   //禁止DMA的直接模式，开启队列模式（虽然不知道是干什么用的，但知道是写到DMDIS位上去）
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;           //队列阈值设置为全队容量（即4个字的容量）
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;             //存储器突发传输配置（单次传输）
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;     //外设突发传输配置（单次传输）
	DMA_Init(DMA2_Stream1, &DMA_InitStructure);
				
    //时基配置：
        //每32个溢出（即32个period后）产生一个UP中断，
        //在更新中断里面，把下一次的DSHOT的16个数配置好
        //f(HSE)=168MHz,经过预分频器PSC后，得到f(CNT)=24MHz的计时单元，
        //然后以1200个计时单元为一个周期，该周期T=1/f(CNT)*1200=50.0us,
        //在一个周期的1200个计时单元里面，输出1200个不同的电平高低，来表示信息，
        //使用DSHOT协议，人为设置：
        // digit(1)~800个高电平+400个低电平； digit(0)~400个高电平+800个低电平。
	TIM_TimeBaseInitStruct.TIM_Prescaler=7-1;                               //时基结构体的预分频数24Mhz(168M/7=24M)
	TIM_TimeBaseInitStruct.TIM_CounterMode=TIM_CounterMode_Up;              //计数模式（递增计数）
	TIM_TimeBaseInitStruct.TIM_Period=/*1200-1;*/ 20-1;                     //（这个是Period=48的时候）f=500kHz ~ T=2.0us ~ 16:32（写到ARR里去）
	TIM_TimeBaseInitStruct.TIM_ClockDivision=TIM_CKD_DIV1;                  //时钟分频数（不分频）
    TIM_TimeBaseInitStruct.TIM_RepetitionCounter=0;                         //重复计数器：写入0到RCR里去（即不重复的意思）
	TIM_TimeBaseInit(TIM8,&TIM_TimeBaseInitStruct);
	
    //用advanceTIM输出DSHOT本质是连续输出16个脉宽不同的PWM信号，
    //也就需要在每输出一次PWM波后往CCR里重写一次脉宽的值。
    //这个功能的实现需要两个中断：
        //1、输出完一个数字信号后的TIM定时器更新中断；
            //在这个中断里面向DMA获取下一个写入CCR的脉宽值.
        //2、DMA传输完成中断；
            //在这个中断里面获取下次的16个脉宽值（即32个32位数）.
    TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;                          //输出比较模式设定为PWM1
	TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;              //输出比较使能（写到CCER寄存器里面去）
	TIM_OCInitStruct.TIM_Pulse = 0;                                         //写到CCR里去，比较寄存器的装载值（一个Period里的脉冲数）（因为DSHOT这里把脉宽从DMA传过来了，这里也就不用配置pulse值了）
	TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;                  //输出通道为高电平有效
    TIM_OCInitStruct.TIM_OutputNState = TIM_OutputNState_Disable;           //互补输出不使能
    TIM_OC3Init(TIM8,&TIM_OCInitStruct), TIM_OC4Init(TIM8,&TIM_OCInitStruct);
    TIM_OC3PreloadConfig(TIM8,TIM_OCPreload_Enable), TIM_OC4PreloadConfig(TIM8,TIM_OCPreload_Enable);

	nvic.NVIC_IRQChannel = DMA2_Stream1_IRQn;                               //TIM8_UP引脚的DMA中断优先级配置
	nvic.NVIC_IRQChannelPreemptionPriority = 0;                             //设置中断的主优先级
	nvic.NVIC_IRQChannelSubPriority = 0;                                    //设置中断的抢占优先级
	nvic.NVIC_IRQChannelCmd = ENABLE;                                       //中断处理使能
	NVIC_Init(&nvic);

	TIM_DMAConfig(TIM8,TIM_DMABase_CCR3,TIM_DMABurstLength_2Transfers);      //自CCR3地址开始的两个寄存器传输数据
    //这个函数的功能很重要，正是它使得DMA能够将数据传到CCR3、CCR4连着的两个reg里去.
    //第二个参数是从DMAR读数据过来的基地址（DMAR是这列TIM接收并存放DMA数据的寄存器）;
    //第三个参数表示从第二个参数的基地址位置开始，往后面的几个寄存器传数据.
    //这个函数在这里的功能的即为：
        //从DMA接收两次数据到DMAR，第一次把数据读到CCR3里，第二次把数据读到CCR4（即CCR3后面的一个reg里），然后才产生一次更新中断.
	
    TIM_DMACmd(TIM8, TIM_DMA_Update, ENABLE);                                //使能更新DMA请求（写到DIER的位8上去）
	TIM_ARRPreloadConfig(TIM8,ENABLE);                                       //使能TIM8的ARR预装载寄存器
    TIM_Cmd(TIM8,ENABLE);                                                    //使能TIM8外设
	TIM_CtrlPWMOutputs(TIM8,ENABLE);                                         //主输出使能（写到BDTR里去？）
	DMA_ITConfig(DMA2_Stream1,DMA_IT_TC,ENABLE);                             //使能DMA2_Stream1的传输完成中断
	DMA_Cmd(DMA2_Stream1, ENABLE);                                           //使能DMA2_Stream1
}


/**
  * @brief  DMA中断处理函数
  * @param  
  * @retval None
  */
void DMA2_Stream1_IRQHandler(void)
{
    if(DMA_GetFlagStatus(DMA2_Stream1,DMA_IT_TCIF1)!=RESET)                    //当TCIF标志被置位时（即DMA传输完成中断，该中断在NDTR置到零是触发）
	{
		TIM_DMACmd(TIM8, TIM_DMA_Update, DISABLE);                              //暂时关闭TIM8的更新事件请求
		DMA_ITConfig(DMA2_Stream1,DMA_IT_TC,DISABLE);                           //暂时关闭DMA的传输完成中断
		DMA_Cmd(DMA2_Stream1, DISABLE);                                         //暂时关闭DMA的传输通道
		TIM_SetCompare3(TIM8,0);                                                //手动重设写到CCR里的装载值（写零复位）
		TIM_SetCompare4(TIM8,0);                                                //手动重设写到CCR里的装载值（写零复位）
	
         //下面是DSHOT协议里的16位数据了
        throttle=LIMIT_MAX_MIN(throttle,2047,0);
            //这是11位的数据位（油门值）
		BIT[0]   = (throttle&0x400 ? 1 : 0);
		BIT[1]   = (throttle&0x200 ? 1 : 0);
		BIT[2]   = (throttle&0x100 ? 1 : 0);
		BIT[3]   = (throttle&0x80  ? 1 : 0);
		BIT[4]   = (throttle&0x40  ? 1 : 0);
		BIT[5]   = (throttle&0x20  ? 1 : 0);
		BIT[6]   = (throttle&0x10  ? 1 : 0);
		BIT[7]   = (throttle&0x8   ? 1 : 0);
		BIT[8]   = (throttle&0x4   ? 1 : 0);
		BIT[9]   = (throttle&0x2   ? 1 : 0);
		BIT[10]  = (throttle&0x1   ? 1 : 0);
            //这是一位的请求数据位（Request）
		BIT[11] =0;//置1则回传
            //这是计算CRC校验的4位
		BIT[12] = BIT[0]^BIT[4]^BIT[8];
		BIT[13] = BIT[1]^BIT[5]^BIT[9];
		BIT[14] = BIT[2]^BIT[6]^BIT[10];
		BIT[15] = BIT[3]^BIT[7]^BIT[11];
		for(int i=0 ; i<16 ; i++)
		{
			SRC_Buffer[2*i]=  ((BIT[i]==1)? ESC_BIT_1:ESC_BIT_0);   //OC3:如果BIT对应的该位为1，则高位传ESC_BIT_1个，否则传ESC_BIT_0个
			SRC_Buffer[2*i+1]=((BIT[i]==1)? ESC_BIT_1:ESC_BIT_0);   //OC4:如果BIT对应的该位为1，则高位传ESC_BIT_1个，否则传ESC_BIT_0个
		}
        
		DMA_ClearFlag(DMA2_Stream1, DMA_FLAG_TCIF1);    //清除DMA传输完成的中断标志位(编号和stream的号对应)
  
		TIM_DMACmd(TIM8, TIM_DMA_Update, ENABLE);       //使能TIM8更新对DMA1的数据传输请求
		DMA_ITConfig(DMA2_Stream1,DMA_IT_TC,ENABLE);    //使能DMA的传输完成中断
		DMA_Cmd(DMA2_Stream1, ENABLE);
        
//      TIM_Cmd(TIM8, ENABLE);                                                  //使能TIM8？
//		TIM_ClearFlag(TIM8, TIM_FLAG_Update);                                   //清除TIM8的更新事件标志
//		TIM_ITConfig(TIM8,TIM_IT_Update,ENABLE);                                //使能TIM8的更新中断
//		DMA_ClearITPendingBit(DMA2_Stream1,DMA_IT_TCIF1);                       //清除DMA的TCIF标志
	}
}


/**
  * @brief  TIM8中断处理函数
  * @param  
  * @retval None
  */
void TIM8_IRQHandler(void)
{
	if ( TIM_GetITStatus(TIM8 , TIM_IT_Update) != RESET ) 
	{
		//TIM_Cmd(TIM8, DISABLE);                         //在中断里面先关闭一下TIM8
		//TIM_ITConfig(TIM8,TIM_IT_Update,DISABLE);       //把更新事件产生的中断也关一下先
        

		//DMA_ClearFlag(DMA2_Stream1, DMA_FLAG_TCIF1);
		//TIM_DMACmd(TIM8, TIM_DMA_Update, ENABLE);
		//DMA_ITConfig(DMA2_Stream1,DMA_IT_TC,ENABLE);
		//DMA_Cmd(DMA2_Stream1, ENABLE);
        TIM_ClearFlag(TIM8, TIM_FLAG_Update);           //清除TIM8的更新事件标志
		 
	}	
}
