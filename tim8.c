#include "main.h"
#define ESC_BIT_1 14   /*800:400*/ /*32:16*/
#define ESC_BIT_0  7   /*800:400*/ /*32:16*/
#define SRC_DATA_LEN 32+4  //(�����ż���0���ӳ�)
uint16_t throttle;//=100;//0x7ff;//����㶨��һ������ֵ
uint8_t BIT[16];//DSHOT��16���ֽڵ�����
uint32_t SRC_Buffer[SRC_DATA_LEN];//������洢����ĸߵ͵�ƽ����
    
/**
  * @brief  TIM8����ģʽ����
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
    
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8,ENABLE);      //ʹ��TIM8ʱ��
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC,ENABLE);     //ʹ��GPIOCʱ��
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2,ENABLE);      //ʹ��DMA2ʱ��

	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_8|GPIO_Pin_9;      //PC8 Tim8--Ch3     PC9 Tim8--Ch4
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF;              //�������ģʽ  
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_DOWN;            //����(��GND)���
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;          //�������
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_100MHz;        //�������
	GPIO_Init(GPIOC,&GPIO_InitStructure);                   //���ոýṹ�����ö�Ӧ�˿�

	GPIO_PinAFConfig(GPIOC,GPIO_PinSource8,GPIO_AF_TIM8);   //������PC8����TIM8����AF3
	GPIO_PinAFConfig(GPIOC,GPIO_PinSource9,GPIO_AF_TIM8);   //������PC9����TIM8����AF3
	
    DMA_DeInit(DMA2_Stream1);                                               //TIM8_UP��DMAͨ��
    DMA_InitStructure.DMA_Channel =DMA_Channel_7;                           //����ͨ��ΪChannel_7,��TIM8_UP��
	DMA_InitStructure.DMA_PeripheralBaseAddr =(uint32_t)&(TIM8->DMAR);      //�������ַΪTIM8��DMAR�Ĵ���
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)SRC_Buffer;	        //�洢������ַΪSRC_Buffer������λ��
	DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;	                //��DMA�Ĵ��䷽��Ϊ�洢��������
	DMA_InitStructure.DMA_BufferSize = SRC_DATA_LEN;                        //Ҫ���������������32�����ݣ�ÿ��32λ�����ֵд��NDTR�Ĵ�������ȥ��(Ϊʲô���32��ֻ��15���������33���У�)
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;        //�����ַ������
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;                 //�洢����ַ����
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word; //�������ݴ�СΪһ���ֳ���32λ��
	DMA_InitStructure.DMA_MemoryDataSize = DMA_PeripheralDataSize_Word;     //�洢�����ݴ�СΪһ���ֳ���32λ��
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;                         //DMA������ѭ��ģʽ
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;                 //DMAͨ�����ȼ����ã�11b��
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Enable;                   //��ֹDMA��ֱ��ģʽ����������ģʽ����Ȼ��֪���Ǹ�ʲô�õģ���֪����д��DMDISλ��ȥ��
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;           //������ֵ����Ϊȫ����������4���ֵ�������
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;             //�洢��ͻ���������ã����δ��䣩
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;     //����ͻ���������ã����δ��䣩
	DMA_Init(DMA2_Stream1, &DMA_InitStructure);
				
    //ʱ�����ã�
        //ÿ32���������32��period�󣩲���һ��UP�жϣ�
        //�ڸ����ж����棬����һ�ε�DSHOT��16�������ú�
        //f(HSE)=168MHz,����Ԥ��Ƶ��PSC�󣬵õ�f(CNT)=24MHz�ļ�ʱ��Ԫ��
        //Ȼ����1200����ʱ��ԪΪһ�����ڣ�������T=1/f(CNT)*1200=50.0us,
        //��һ�����ڵ�1200����ʱ��Ԫ���棬���1200����ͬ�ĵ�ƽ�ߵͣ�����ʾ��Ϣ��
        //ʹ��DSHOTЭ�飬��Ϊ���ã�
        // digit(1)~800���ߵ�ƽ+400���͵�ƽ�� digit(0)~400���ߵ�ƽ+800���͵�ƽ��
	TIM_TimeBaseInitStruct.TIM_Prescaler=7-1;                               //ʱ���ṹ���Ԥ��Ƶ��24Mhz(168M/7=24M)
	TIM_TimeBaseInitStruct.TIM_CounterMode=TIM_CounterMode_Up;              //����ģʽ������������
	TIM_TimeBaseInitStruct.TIM_Period=/*1200-1;*/ 20-1;                     //�������Period=48��ʱ��f=500kHz ~ T=2.0us ~ 16:32��д��ARR��ȥ��
	TIM_TimeBaseInitStruct.TIM_ClockDivision=TIM_CKD_DIV1;                  //ʱ�ӷ�Ƶ��������Ƶ��
    TIM_TimeBaseInitStruct.TIM_RepetitionCounter=0;                         //�ظ���������д��0��RCR��ȥ�������ظ�����˼��
	TIM_TimeBaseInit(TIM8,&TIM_TimeBaseInitStruct);
	
    //��advanceTIM���DSHOT�������������16������ͬ��PWM�źţ�
    //Ҳ����Ҫ��ÿ���һ��PWM������CCR����дһ�������ֵ��
    //������ܵ�ʵ����Ҫ�����жϣ�
        //1�������һ�������źź��TIM��ʱ�������жϣ�
            //������ж�������DMA��ȡ��һ��д��CCR������ֵ.
        //2��DMA��������жϣ�
            //������ж������ȡ�´ε�16������ֵ����32��32λ����.
    TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;                          //����Ƚ�ģʽ�趨ΪPWM1
	TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;              //����Ƚ�ʹ�ܣ�д��CCER�Ĵ�������ȥ��
	TIM_OCInitStruct.TIM_Pulse = 0;                                         //д��CCR��ȥ���ȽϼĴ�����װ��ֵ��һ��Period���������������ΪDSHOT����������DMA�������ˣ�����Ҳ�Ͳ�������pulseֵ�ˣ�
	TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;                  //���ͨ��Ϊ�ߵ�ƽ��Ч
    TIM_OCInitStruct.TIM_OutputNState = TIM_OutputNState_Disable;           //���������ʹ��
    TIM_OC3Init(TIM8,&TIM_OCInitStruct), TIM_OC4Init(TIM8,&TIM_OCInitStruct);
    TIM_OC3PreloadConfig(TIM8,TIM_OCPreload_Enable), TIM_OC4PreloadConfig(TIM8,TIM_OCPreload_Enable);

	nvic.NVIC_IRQChannel = DMA2_Stream1_IRQn;                               //TIM8_UP���ŵ�DMA�ж����ȼ�����
	nvic.NVIC_IRQChannelPreemptionPriority = 0;                             //�����жϵ������ȼ�
	nvic.NVIC_IRQChannelSubPriority = 0;                                    //�����жϵ���ռ���ȼ�
	nvic.NVIC_IRQChannelCmd = ENABLE;                                       //�жϴ���ʹ��
	NVIC_Init(&nvic);

	TIM_DMAConfig(TIM8,TIM_DMABase_CCR3,TIM_DMABurstLength_2Transfers);      //��CCR3��ַ��ʼ�������Ĵ�����������
    //��������Ĺ��ܺ���Ҫ��������ʹ��DMA�ܹ������ݴ���CCR3��CCR4���ŵ�����reg��ȥ.
    //�ڶ��������Ǵ�DMAR�����ݹ����Ļ���ַ��DMAR������TIM���ղ����DMA���ݵļĴ�����;
    //������������ʾ�ӵڶ��������Ļ���ַλ�ÿ�ʼ��������ļ����Ĵ���������.
    //�������������Ĺ��ܵļ�Ϊ��
        //��DMA�����������ݵ�DMAR����һ�ΰ����ݶ���CCR3��ڶ��ΰ����ݶ���CCR4����CCR3�����һ��reg���Ȼ��Ų���һ�θ����ж�.
	
    TIM_DMACmd(TIM8, TIM_DMA_Update, ENABLE);                                //ʹ�ܸ���DMA����д��DIER��λ8��ȥ��
	TIM_ARRPreloadConfig(TIM8,ENABLE);                                       //ʹ��TIM8��ARRԤװ�ؼĴ���
    TIM_Cmd(TIM8,ENABLE);                                                    //ʹ��TIM8����
	TIM_CtrlPWMOutputs(TIM8,ENABLE);                                         //�����ʹ�ܣ�д��BDTR��ȥ����
	DMA_ITConfig(DMA2_Stream1,DMA_IT_TC,ENABLE);                             //ʹ��DMA2_Stream1�Ĵ�������ж�
	DMA_Cmd(DMA2_Stream1, ENABLE);                                           //ʹ��DMA2_Stream1
}


/**
  * @brief  DMA�жϴ�����
  * @param  
  * @retval None
  */
void DMA2_Stream1_IRQHandler(void)
{
    if(DMA_GetFlagStatus(DMA2_Stream1,DMA_IT_TCIF1)!=RESET)                    //��TCIF��־����λʱ����DMA��������жϣ����ж���NDTR�õ����Ǵ�����
	{
		TIM_DMACmd(TIM8, TIM_DMA_Update, DISABLE);                              //��ʱ�ر�TIM8�ĸ����¼�����
		DMA_ITConfig(DMA2_Stream1,DMA_IT_TC,DISABLE);                           //��ʱ�ر�DMA�Ĵ�������ж�
		DMA_Cmd(DMA2_Stream1, DISABLE);                                         //��ʱ�ر�DMA�Ĵ���ͨ��
		TIM_SetCompare3(TIM8,0);                                                //�ֶ�����д��CCR���װ��ֵ��д�㸴λ��
		TIM_SetCompare4(TIM8,0);                                                //�ֶ�����д��CCR���װ��ֵ��д�㸴λ��
	
         //������DSHOTЭ�����16λ������
        throttle=LIMIT_MAX_MIN(throttle,2047,0);
            //����11λ������λ������ֵ��
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
            //����һλ����������λ��Request��
		BIT[11] =0;//��1��ش�
            //���Ǽ���CRCУ���4λ
		BIT[12] = BIT[0]^BIT[4]^BIT[8];
		BIT[13] = BIT[1]^BIT[5]^BIT[9];
		BIT[14] = BIT[2]^BIT[6]^BIT[10];
		BIT[15] = BIT[3]^BIT[7]^BIT[11];
		for(int i=0 ; i<16 ; i++)
		{
			SRC_Buffer[2*i]=  ((BIT[i]==1)? ESC_BIT_1:ESC_BIT_0);   //OC3:���BIT��Ӧ�ĸ�λΪ1�����λ��ESC_BIT_1��������ESC_BIT_0��
			SRC_Buffer[2*i+1]=((BIT[i]==1)? ESC_BIT_1:ESC_BIT_0);   //OC4:���BIT��Ӧ�ĸ�λΪ1�����λ��ESC_BIT_1��������ESC_BIT_0��
		}
        
		DMA_ClearFlag(DMA2_Stream1, DMA_FLAG_TCIF1);    //���DMA������ɵ��жϱ�־λ(��ź�stream�ĺŶ�Ӧ)
  
		TIM_DMACmd(TIM8, TIM_DMA_Update, ENABLE);       //ʹ��TIM8���¶�DMA1�����ݴ�������
		DMA_ITConfig(DMA2_Stream1,DMA_IT_TC,ENABLE);    //ʹ��DMA�Ĵ�������ж�
		DMA_Cmd(DMA2_Stream1, ENABLE);
        
//      TIM_Cmd(TIM8, ENABLE);                                                  //ʹ��TIM8��
//		TIM_ClearFlag(TIM8, TIM_FLAG_Update);                                   //���TIM8�ĸ����¼���־
//		TIM_ITConfig(TIM8,TIM_IT_Update,ENABLE);                                //ʹ��TIM8�ĸ����ж�
//		DMA_ClearITPendingBit(DMA2_Stream1,DMA_IT_TCIF1);                       //���DMA��TCIF��־
	}
}


/**
  * @brief  TIM8�жϴ�����
  * @param  
  * @retval None
  */
void TIM8_IRQHandler(void)
{
	if ( TIM_GetITStatus(TIM8 , TIM_IT_Update) != RESET ) 
	{
		//TIM_Cmd(TIM8, DISABLE);                         //���ж������ȹر�һ��TIM8
		//TIM_ITConfig(TIM8,TIM_IT_Update,DISABLE);       //�Ѹ����¼��������ж�Ҳ��һ����
        

		//DMA_ClearFlag(DMA2_Stream1, DMA_FLAG_TCIF1);
		//TIM_DMACmd(TIM8, TIM_DMA_Update, ENABLE);
		//DMA_ITConfig(DMA2_Stream1,DMA_IT_TC,ENABLE);
		//DMA_Cmd(DMA2_Stream1, ENABLE);
        TIM_ClearFlag(TIM8, TIM_FLAG_Update);           //���TIM8�ĸ����¼���־
		 
	}	
}
