#include <stdlib.h>
#include <stdbool.h>
#include "espressif/esp_common.h"
#include "esp/uart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "esp8266.h"

#define ONBOARD_LED_PIN 16

#define DIP_A_PIN 8
#define DIP_B_PIN 11
#define DIP_C_PIN 7
#define DIP_D_PIN 6

void printConfigurationGroup(char* identifier, bool value) {
  char* readableValue = value ? "ON" : "OFF";
  printf("%s = %s\t\t", identifier, readableValue);
}

void readConfiguration() {
  bool configurationA = !gpio_read(DIP_A_PIN);
  bool configurationB = !gpio_read(DIP_B_PIN);
  bool configurationC = !gpio_read(DIP_C_PIN);
  bool configurationD = !gpio_read(DIP_D_PIN);

  printConfigurationGroup("A", configurationA);
  printConfigurationGroup("B", configurationB);
  printConfigurationGroup("C", configurationC);
  printConfigurationGroup("D", configurationD);
  printf("\n");
}

void pollConfigurationTask(void *pvParameters) {
  gpio_enable(ONBOARD_LED_PIN, GPIO_OUTPUT);
  gpio_set_pullup(DIP_A_PIN, true, false);
  gpio_set_pullup(DIP_B_PIN, true, false);
  gpio_set_pullup(DIP_C_PIN, true, false);
  gpio_set_pullup(DIP_D_PIN, true, false);

  while(1) {
    gpio_write(ONBOARD_LED_PIN, 0);
    vTaskDelay(500 / portTICK_PERIOD_MS);
    gpio_write(ONBOARD_LED_PIN, 1);

    readConfiguration();
    vTaskDelay(25000 / portTICK_PERIOD_MS);
  }
}

void user_init(void) {
  uart_set_baud(0, 115200);
  printf("SDK version:%s\n", sdk_system_get_sdk_version());

  xTaskCreate(pollConfigurationTask, "Poll Configuration", 256, NULL, 2, NULL);
}
