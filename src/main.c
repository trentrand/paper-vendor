#include <stdlib.h>
#include <stdbool.h>
#include "espressif/esp_common.h"
#include "esp/uart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "esp8266.h"

uint8_t const led_pin = 16;

uint8_t const configuration_a_pin = 5;
uint8_t const configuration_b_pin = 4;
uint8_t const configuration_c_pin = 0;
uint8_t const configuration_d_pin = 2;

void printConfigurationGroup(char* identifier, bool value) {
  char* readableValue = value ? "ON" : "OFF";
  printf("%s = %s\t\t", identifier, readableValue);
}

void readConfiguration() {
  bool configurationA = !gpio_read(configuration_a_pin);
  bool configurationB = !gpio_read(configuration_b_pin);
  bool configurationC = !gpio_read(configuration_c_pin);
  bool configurationD = !gpio_read(configuration_d_pin);

  printConfigurationGroup("A", configurationA);
  printConfigurationGroup("B", configurationB);
  printConfigurationGroup("C", configurationC);
  printConfigurationGroup("D", configurationD);
  printf("\n");
}

void pollConfigurationTask(void *pvParameters) {
  gpio_enable(led_pin, GPIO_OUTPUT);
  gpio_set_pullup(configuration_a_pin, true, false);
  gpio_set_pullup(configuration_b_pin, true, false);
  gpio_set_pullup(configuration_c_pin, true, false);
  gpio_set_pullup(configuration_d_pin, true, false);

  while(1) {
    gpio_write(led_pin, 0);
    vTaskDelay(500 / portTICK_PERIOD_MS);
    gpio_write(led_pin, 1);

    readConfiguration();
    vTaskDelay(5000 / portTICK_PERIOD_MS);
  }
}

void user_init(void) {
  uart_set_baud(0, 115200);
  printf("SDK version:%s\n", sdk_system_get_sdk_version());

  xTaskCreate(pollConfigurationTask, "Poll Configuration", 256, NULL, 2, NULL);
}
