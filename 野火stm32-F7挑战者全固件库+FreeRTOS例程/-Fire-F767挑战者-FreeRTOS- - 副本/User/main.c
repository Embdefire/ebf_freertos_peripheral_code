/**
  *********************************************************************
  * @file    main.c
  * @author  fire
  * @version V1.0
  * @date    2018-xx-xx
  * @brief   FreeRTOS v9.0.0 + STM32 �̼�������
  *********************************************************************
  * @attention
  *
  * ʵ��ƽ̨:Ұ�� STM32ȫϵ�п����� 
  * ��̳    :http://www.firebbs.cn
  * �Ա�    :https://fire-stm32.taobao.com
  *
  **********************************************************************
  */ 
 
/*
*************************************************************************
*                             ������ͷ�ļ�
*************************************************************************
*/ 
#include "stm32f7xx.h"
#include "systemclock.h"
#include "bsp_led.h"
#include "bsp_key.h" 
#include "core_delay.h"   
#include "bsp_debug_usart.h"
#include <stdlib.h>
#include "./i2c/i2c.h"
#include "./mpu6050/mpu6050.h"
//�����Ƿ�ʹ��LCD������ʾ������Ҫ�Ļ��������ע�͵�����
#define USE_LCD_DISPLAY

#ifdef USE_LCD_DISPLAY
 #include "./lcd/bsp_lcd.h"
#endif
/* FreeRTOSͷ�ļ� */
#include "FreeRTOS.h"
#include "task.h"

/**************************** ������ ********************************/
/* 
 * ��������һ��ָ�룬����ָ��һ�����񣬵����񴴽���֮�����;�����һ��������
 * �Ժ�����Ҫ��������������Ҫͨ�������������������������������Լ�����ô
 * ����������ΪNULL��
 */
static TaskHandle_t AppTaskCreate_Handle = NULL;/* ���������� */
static TaskHandle_t Test_Task_Handle = NULL;/* LED������ */
static TaskHandle_t KEY_Task_Handle = NULL;/* KEY������ */

/********************************** �ں˶����� *********************************/
/*
 * �ź�������Ϣ���У��¼���־�飬�����ʱ����Щ�������ں˵Ķ���Ҫ��ʹ����Щ�ں�
 * ���󣬱����ȴ����������ɹ�֮��᷵��һ����Ӧ�ľ����ʵ���Ͼ���һ��ָ�룬������
 * �ǾͿ���ͨ��������������Щ�ں˶���
 *
 * �ں˶���˵���˾���һ��ȫ�ֵ����ݽṹ��ͨ����Щ���ݽṹ���ǿ���ʵ��������ͨ�ţ�
 * �������¼�ͬ���ȸ��ֹ��ܡ�������Щ���ܵ�ʵ��������ͨ��������Щ�ں˶���ĺ���
 * ����ɵ�
 * 
 */


/******************************* ȫ�ֱ������� ************************************/
/*
 * ��������дӦ�ó����ʱ�򣬿�����Ҫ�õ�һЩȫ�ֱ�����
 */


/*
*************************************************************************
*                             ��������
*************************************************************************
*/
static void AppTaskCreate(void);/* ���ڴ������� */

static void Test_Task(void* pvParameters);/* Test_Task����ʵ�� */
static void KEY_Task(void* pvParameters);/* KEY_Task����ʵ�� */

static void BSP_Init(void);/* ���ڳ�ʼ�����������Դ */

/*****************************************************************
  * @brief  ������
  * @param  ��
  * @retval ��
  * @note   ��һ����������Ӳ����ʼ�� 
            �ڶ���������APPӦ������
            ������������FreeRTOS����ʼ���������
  ****************************************************************/
int main(void)
{	
  BaseType_t xReturn = pdPASS;/* ����һ��������Ϣ����ֵ��Ĭ��ΪpdPASS */
  
  /* ������Ӳ����ʼ�� */
  BSP_Init();
  
  printf("����һ��[Ұ��]-STM32ȫϵ�п�����-FreeRTOS�̼������̣�\n\n");
  printf("����KEY1�������񣬰���KEY2�ָ�����\n");
  
   /* ����AppTaskCreate���� */
  xReturn = xTaskCreate((TaskFunction_t )AppTaskCreate,  /* ������ں��� */
                        (const char*    )"AppTaskCreate",/* �������� */
                        (uint16_t       )512,  /* ����ջ��С */
                        (void*          )NULL,/* ������ں������� */
                        (UBaseType_t    )1, /* ��������ȼ� */
                        (TaskHandle_t*  )&AppTaskCreate_Handle);/* ������ƿ�ָ�� */ 
  /* ����������� */           
  if(pdPASS == xReturn)
    vTaskStartScheduler();   /* �������񣬿������� */
  else
    return -1;  
  
  while(1);   /* ��������ִ�е����� */    
}


/***********************************************************************
  * @ ������  �� AppTaskCreate
  * @ ����˵���� Ϊ�˷���������е����񴴽����������������������
  * @ ����    �� ��  
  * @ ����ֵ  �� ��
  **********************************************************************/
static void AppTaskCreate(void)
{
  BaseType_t xReturn = pdPASS;/* ����һ��������Ϣ����ֵ��Ĭ��ΪpdPASS */
  
  taskENTER_CRITICAL();           //�����ٽ���
  
  /* ����Test_Task���� */
  xReturn = xTaskCreate((TaskFunction_t )Test_Task, /* ������ں��� */
                        (const char*    )"Test_Task",/* �������� */
                        (uint16_t       )512,   /* ����ջ��С */
                        (void*          )NULL,	/* ������ں������� */
                        (UBaseType_t    )2,	    /* ��������ȼ� */
                        (TaskHandle_t*  )&Test_Task_Handle);/* ������ƿ�ָ�� */
  if(pdPASS == xReturn)
    printf("����Test_Task����ɹ�!\r\n");
  /* ����KEY_Task���� */
  xReturn = xTaskCreate((TaskFunction_t )KEY_Task,  /* ������ں��� */
                        (const char*    )"KEY_Task",/* �������� */
                        (uint16_t       )512,  /* ����ջ��С */
                        (void*          )NULL,/* ������ں������� */
                        (UBaseType_t    )3, /* ��������ȼ� */
                        (TaskHandle_t*  )&KEY_Task_Handle);/* ������ƿ�ָ�� */ 
  if(pdPASS == xReturn)
    printf("����KEY_Task����ɹ�!\r\n");
  
  vTaskDelete(AppTaskCreate_Handle); //ɾ��AppTaskCreate����
  
  taskEXIT_CRITICAL();            //�˳��ٽ���
}



/**********************************************************************
  * @ ������  �� Test_Task
  * @ ����˵���� Test_Task��������
  * @ ����    ��   
  * @ ����ֵ  �� ��
  ********************************************************************/
static void Test_Task(void* parameter)
{	
	static short Acel[3];
	static short Gyro[3];
	static float Temp;
	
  while (1)
  {
		MPU6050ReadAcc(Acel);
		printf("���ٶȣ�%8d%8d%8d",Acel[0],Acel[1],Acel[2]);
		MPU6050ReadGyro(Gyro);
		printf("    ������%8d%8d%8d",Gyro[0],Gyro[1],Gyro[2]);
		MPU6050_ReturnTemp(&Temp);
		printf("    �¶�%8.2f\r\n",Temp);				
		
		
#ifdef USE_LCD_DISPLAY	
			{
				char cStr [ 70 ];
				sprintf ( cStr, "Acceleration:%8d%8d%8d",Acel[0],Acel[1],Acel[2] );	//���ٶ�ԭʼ����
				
				LCD_DisplayStringLine(7,(uint8_t* )cStr);			

				sprintf ( cStr, "Gyro        :%8d%8d%8d",Gyro[0],Gyro[1],Gyro[2] );	//��ԭʼ����

				LCD_DisplayStringLine(8,(uint8_t* )cStr);			

				sprintf ( cStr, "Temperture  :%8.2f",Temp );	//�¶�ֵ
				LCD_DisplayStringLine(9,(uint8_t* )cStr);			
			}
#endif
		
		//����һ�����ݣ��ɸ����Լ���������߲���Ƶ�ʣ���100ms����һ��
    vTaskDelay(500);   /* ��ʱ500��tick */
  }
}

/**********************************************************************
  * @ ������  �� KEY_Task
  * @ ����˵���� KEY_Task��������
  * @ ����    ��   
  * @ ����ֵ  �� ��
  ********************************************************************/
static void KEY_Task(void* parameter)
{	
  while (1)
  {
    if( Key_Scan(KEY1_GPIO_PORT,KEY1_PIN) == KEY_ON )
    {/* K1 ������ */
      printf("����Test_Task����\n");
      vTaskSuspend(Test_Task_Handle);/* ����LED���� */
      printf("����Test_Task����ɹ���\n");
    } 
    if( Key_Scan(KEY2_GPIO_PORT,KEY2_PIN) == KEY_ON )
    {/* K2 ������ */
      printf("�ָ�Test_Task����\n");
      vTaskResume(Test_Task_Handle);/* �ָ�LED���� */
      printf("�ָ�Test_Task����ɹ���\n");
    }
    vTaskDelay(20);/* ��ʱ20��tick */
  }
}

/***********************************************************************
  * @ ������  �� BSP_Init
  * @ ����˵���� �弶�����ʼ�������а����ϵĳ�ʼ�����ɷ��������������
  * @ ����    ��   
  * @ ����ֵ  �� ��
  *********************************************************************/
static void BSP_Init(void)
{
  /* ϵͳʱ�ӳ�ʼ����400MHz */
	SystemClock_Config();
  
  /* ��ʼ��SysTick */
  HAL_SYSTICK_Config( HAL_RCC_GetSysClockFreq() / configTICK_RATE_HZ );	

	/* Ӳ��BSP��ʼ��ͳͳ�����������LED�����ڣ�LCD�� */
    
	/* LED �˿ڳ�ʼ�� */
	LED_GPIO_Config();	
	
	/* usart �˿ڳ�ʼ�� */
  DEBUG_USART_Config();

  /* KEY �˿ڳ�ʼ�� */
  Key_GPIO_Config();
  	
#ifdef USE_LCD_DISPLAY		
    /* LCD �˿ڳ�ʼ�� */ 
    LCD_Init();
    /* LCD ��һ���ʼ�� */ 
    LCD_LayerInit(0, LCD_FB_START_ADDRESS,ARGB8888);
	/* LCD �ڶ����ʼ�� */ 
    LCD_LayerInit(1, LCD_FB_START_ADDRESS+(LCD_GetXSize()*LCD_GetYSize()*4),ARGB8888);
    /* ʹ��LCD������������ */ 
    LCD_DisplayOn(); 

    /* ѡ��LCD��һ�� */
    LCD_SelectLayer(0);

    /* ��һ����������ʾȫ�� */ 
    LCD_Clear(LCD_COLOR_BLACK);  

    /* ѡ��LCD�ڶ��� */
    LCD_SelectLayer(1);

    /* �ڶ�����������ʾ͸�� */ 
    LCD_Clear(LCD_COLOR_TRANSPARENT);

    /* ���õ�һ�͵ڶ����͸����,��СֵΪ0�����ֵΪ255*/
    LCD_SetTransparency(0, 255);
    LCD_SetTransparency(1, 0);
	
	/* ѡ��LCD��һ�� */
    LCD_SelectLayer(0);
	/*����������ɫ������ı�����ɫ*/
	LCD_SetColors(LCD_COLOR_RED,LCD_COLOR_BLACK);	
#endif

	//��ʼ�� I2C
	I2cMaster_Init(); 

	printf("\r\n ��ӭʹ��Ұ��  STM32 F767 �����塣\r\n");		 

	printf("\r\n ����һ��I2C����(MPU6050)��д�������� \r\n");

 	//MPU6050��ʼ��
	MPU6050_Init();
	//���MPU6050
	if (MPU6050ReadID() == 1)
	{	
		LED2_TOGGLE;
	}
	else
	{
		printf("\r\nû�м�⵽MPU6050��������\r\n");
		LED_RED; 
#ifdef USE_LCD_DISPLAY			
		/*����������ɫ������ı�����ɫ*/
		LCD_SetColors(LCD_COLOR_BLUE,LCD_COLOR_BLACK);	

		LCD_DisplayStringLine(4,(uint8_t* )"No MPU6050 detected! ");			//Ұ���Դ���17*24��ʾ
		LCD_DisplayStringLine(5,(uint8_t* )"Please check the hardware connection! ");//Ұ���Դ���17*24��ʾ
#endif
		while(1);	
	}
}
 



/*********************************************END OF FILE**********************/

