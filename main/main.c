#include <stdio.h>
#include "../components/IRSend/include/IRSend.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define RECV_PIN      2

void app_main()
{
  IRSendInit(RECV_PIN, 1);

  while(1)
  {
    IRSendIR(69);
    vTaskDelay(2000 / portTICK_RATE_MS);
  }
}
