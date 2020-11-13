#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "espressif/esp_common.h"
#include "esp/gpio.h"
#include "esp/uart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "esp8266.h"
#include <esp/uart.h>
#include <esp/uart_regs.h>

#include "drivers/thermalprinter/thermal-printer.c"

// TODO: if dev
#include "gdbstub.h"

#define ONBOARD_LED_PIN 16

#define DIP_A_PIN 8
#define DIP_B_PIN 11
#define DIP_C_PIN 7
#define DIP_D_PIN 6

#define THERMAL_PRINTER_RX_PIN 2 // yellow
#define THERMAL_PRINTER_TX_PIN 3 // green

uint8_t printDensity = 14; // yields 120% density, experimentally determined to be good
uint8_t printBreakTime = 4; // From page 23 of data sheet. Testing shows the max helps darken text
uint8_t heatTime = 80; //80 is default from page 23 of datasheet. Controls speed of printing and darkness
uint8_t heatInterval = 2; // 2 is default from page 23 of datasheet. Controls speed of printing and darkness

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

void printTask(void *pvParameters) {
  thermalprinter_t printer = {
    .rx_pin = THERMAL_PRINTER_RX_PIN,
    .tx_pin = THERMAL_PRINTER_TX_PIN
  };

  thermalprinter_init(&printer);

  thermalprinter_sleep(5);

  thermalprinter_reset();

  thermalprinter_default_printing_settings();
  thermalprinter_density((printBreakTime << 5) | printDensity);

  while(1) {
    thermalprinter_align_mode(left);
    writeString("Hello!");
    thermalprinter_line_feed();

    thermalprinter_align_mode(right);
    writeString("A B C 1 2 3");
    thermalprinter_line_feed();

    uart_flush_txfifo(1);

    vTaskDelay(30000 / portTICK_PERIOD_MS);
  }

  // TODO: clean-up task by calling thermalprinter_offline()
}

void user_init(void) {
  uart_set_baud(0, 115200);

  // TODO: if dev
  gdbstub_init();

  printf("SDK version:%s\n", sdk_system_get_sdk_version());

  xTaskCreate(pollConfigurationTask, "Poll Configuration", 256, NULL, 3, NULL);
  xTaskCreate(printTask, "Print", 1024, NULL, 2, NULL);
}
