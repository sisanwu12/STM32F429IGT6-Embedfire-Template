#include "ser_ultrasonic.h"

#include "FreeRTOS.h"
#include "task.h"

#include "dev_ultrasonic.h"

static volatile uint32_t s_latest_mm = 0;
static volatile uint8_t s_latest_valid = 0;

static void ultrasonic_task(void *argument)
{
  (void)argument;

  (void)dev_ultrasonic_init();

  for (;;)
  {
    uint32_t mm = 0;
    if (dev_ultrasonic_measure_mm(&mm))
    {
      s_latest_mm = mm;
      s_latest_valid = 1u;
    }
    else
    {
      s_latest_valid = 0u;
    }

    vTaskDelay(pdMS_TO_TICKS(120));
  }
}

void ser_ultrasonic_start(void)
{
  (void)xTaskCreate(ultrasonic_task, "ultra", 512, NULL,
                    tskIDLE_PRIORITY + 1, NULL);
}

bool ser_ultrasonic_get_latest_mm(uint32_t *mm)
{
  if (mm == NULL)
  {
    return false;
  }

  if (!s_latest_valid)
  {
    return false;
  }

  *mm = s_latest_mm;
  return true;
}
