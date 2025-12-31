/* 引用文件 */
#include "FreeRTOS.h"
#include "stm32f4xx_hal.h"
#include "task.h"

int main(void)
{
  HAL_Init();

  vTaskStartScheduler();
}
