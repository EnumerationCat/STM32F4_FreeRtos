
//#include "FreeRTOSConfig.h"
#include "key.h"
#include "FreeRtos_demo.h"
#include "stm32f407xx.h"
#include <stdint.h>
#include <sys/types.h>




void MyTIM_Init(void)
{
	HAL_TIM_Base_Start_IT(&htim7);
	HAL_TIM_Base_Start_IT(&htim2);
}

volatile unsigned long ulHighFrequencyTimerTicks;
// 定时器更新中断回调函数
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  //定时器7每1ms触发一次
  if(htim->Instance == TIM7) 
  {


	Key_Tick(); // 调用按键处理函数
		
  }
  if(htim->Instance == TIM6)
  {
    HAL_IncTick();
  }
  if(htim->Instance == TIM2)
  {
	ulHighFrequencyTimerTicks++;

  }
  
}





/*                  7      6      5      4       3     2    1      0 */
/*uint8_t Key_Flag      REPEAT  LONG  DOUBLE  SINGLE  UP  DOWN  HOLD*/


uint8_t Key_Flag[KEY_COUNT] = {0}; // 按键事件标志数组，每个元素对应一个按键


uint8_t getKeyState(uint8_t keyIndex)
{
	if(keyIndex==0)
	{
		if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_15) == 0) // 按键按下
	    {
			return KEY_PRESSED;
	    }
	}
	else if(keyIndex==1)
	{
	
	}	
	
	return KEY_NOPRESSED;
}






uint8_t Key_Check(uint8_t keyIndex, uint8_t Flag)
{
	if(Key_Flag[keyIndex] & Flag) // 检查指定标志位是否被设置
	{
		if(Flag != KEY_HOLD) // 如果不是HOLD事件，清除该标志位
		{
			Key_Flag[keyIndex] &= ~Flag; // 清除该标志位

		}
		
		return 1; // 返回1表示该事件发生过
	}
	return 0; // 返回0表示该事件未发生

}



void Key_Tick(void)
{
	static uint8_t Counter; // 消抖计数
	static uint8_t currentKeyState[KEY_COUNT],lastKeyState[KEY_COUNT]; // 上次,当前的按键状态
	static uint8_t KeyState[KEY_COUNT]; // 按键事件状态机状态
	static uint16_t KeyTimer[KEY_COUNT]; // 按键事件计时器

	for(uint8_t i=0;i<KEY_COUNT;i++)
	{
		if(KeyTimer[i]>0)
		{
			KeyTimer[i]--; // 按键事件计时器递减
		}
	}
	
	Counter++;
	if(Counter>=20)
	{
		Counter = 0;
		for(uint8_t i=0;i<KEY_COUNT;i++)
		{
			lastKeyState[i] = currentKeyState[i]; // 更新上次按键状态
			currentKeyState[i] = getKeyState(i); // 获取当前按键状态

		    /*按键基础事件检测  按下，按住，抬起*/
		    if(currentKeyState[i] == KEY_PRESSED)
			{
			
				//HOLD=1;
				Key_Flag[i] |= KEY_HOLD; // 设置HOLD标志位

			}else
			{
				//HOLD=0;
				Key_Flag[i] &= ~KEY_HOLD; // 清除HOLD标志位
			}
			if(currentKeyState[i] == KEY_PRESSED && lastKeyState[i] == KEY_NOPRESSED)
			{
				//DOWN=1;
				Key_Flag[i] |= KEY_DOWN; // 设置DOWN标志位

			}
			if(currentKeyState[i] == KEY_NOPRESSED && lastKeyState[i] == KEY_PRESSED)
			{
				//UP=1;
				Key_Flag[i] |= KEY_UP; // 设置UP标志位

			}
			/******************************************************** */
			/*按键高级事件检测  单击，双击，长按*/
			switch(KeyState[i])
			{
				case KEY_IDLE:
				if(currentKeyState[i] == KEY_PRESSED)
				{
					KeyTimer[i] = KEY_TIME_LONG; // 设置长按时间阈值为2000ms
					KeyState[i] = KEY_HasBeenPressed; // 进入按下状态
				}
				
				break;

				case KEY_HasBeenPressed:
				if(currentKeyState[i] == KEY_NOPRESSED)
				{
					KeyTimer[i] = KEY_TIME_DOUBLE; // 设置双击时间阈值为200ms
					KeyState[i] = KEY_HasBeenLoosened; // 进入抬起状态
				}
				else if(KeyTimer[i] == 0) // 如果长按时间到达阈值
				{
					KeyTimer[i] = KEY_TIME_REPEAT; // 重置计时器，避免重复触发长按事件
					//LONG=1;
					Key_Flag[i] |= KEY_LONG; // 设置LONG标志位
					KeyState[i] = KEY_HasBeenLongPressed; // 进入长按状态
				}
				break;

				case KEY_HasBeenLoosened:
				if(currentKeyState[i] == KEY_PRESSED)
				{
					//DOUBLE=1;
					Key_Flag[i] |= KEY_DOUBLE; // 设置DOUBLE标志位
					KeyState[i] = KEY_HasBeenDoubleClicked; // 进入双击状态
				}
				else if(KeyTimer[i] == 0) // 如果双击时间到达阈值
				{
					//SINGLE=1;
					Key_Flag[i] |= KEY_SINGLE; // 设置SINGLE标志位
					KeyState[i] = KEY_IDLE; // 进入空闲状态
				}
				
				break;

				case KEY_HasBeenDoubleClicked:
				if(currentKeyState[i] == KEY_NOPRESSED)
				{
					KeyState[i] = KEY_IDLE; // 进入空闲状态
				}
				
				break;

				case KEY_HasBeenLongPressed:
				if(currentKeyState[i] == KEY_NOPRESSED)
				{
					KeyState[i] = KEY_IDLE; // 进入空闲状态
				}
				else if(KeyTimer[i] == 0) // 如果长按时间到达阈值
				{
					KeyTimer[i] = KEY_TIME_REPEAT; // 重置计时器，避免重复触发长按事件
					//REPEAT=1;
					Key_Flag[i] |= KEY_REPEAT; // 设置REPEAT标志位
					KeyState[i] = KEY_HasBeenLongPressed; // 继续保持长按状态
				
				}
				break;
			
				default:
				KeyState[i] = KEY_IDLE; // 进入空闲状态
				
				break;
				
		}

		}
			
	}
	
	
}


void Key_Scanf(void)
{
	for(uint8_t i=0;i<KEY_COUNT;i++)
	{
		if(Key_Check(i,KEY_SINGLE))
		{
			if(i==0)
			{
				usbprintf("按键%d被单击\r\n",i);

			}
			
		}

		if(Key_Check(i,KEY_DOUBLE))
		{
			if(i==0)
			{
				usbprintf("按键%d被双击\r\n",i);

			}
			
		}

		if(Key_Check(i,KEY_LONG))
		{
			if(i==0)
			{
				usbprintf("按键%d被长按\r\n",i);

			}
			
		}
		if(Key_Check(i,KEY_REPEAT))
		{
			if(i==0)
			{
				usbprintf("按键%d长按重复触发用于计数增加\r\n",i);

			}
			
		}


	}
		
}




