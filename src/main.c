#include <stdlib.h>
#include "espressif/esp_common.h"
#include "esp/uart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "esp8266.h"

const int gpio = 16;

void printTask(void *pvParameters) {
  gpio_enable(gpio, GPIO_OUTPUT);

  while(1) {
    printf("Executing task\n");
    gpio_write(gpio, 1);
    vTaskDelay(10000 / portTICK_PERIOD_MS);
    gpio_write(gpio, 0);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void user_init(void) {
  uart_set_baud(0, 115200);
  printf("SDK version:%s\n", sdk_system_get_sdk_version());

  xTaskCreate(printTask, "printTask", 256, NULL, 2, NULL);
}
