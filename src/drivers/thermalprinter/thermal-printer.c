#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>
#include <esp/gpio.h>
#include <esp/iomux_regs.h>
#include <esp/uart.h>
#include <esp/uart_regs.h>
#include "FreeRTOS.h"

typedef struct {
  uint8_t rx_pin;
  uint8_t tx_pin;
} thermalprinter_t;

void thermalprinter_init(const thermalprinter_t *device) {
  if (device->rx_pin != 2) {
    printf("Error: Printer RX must be connected to GPIO 2 (UART1_TXD)");
    return;
  }

  gpio_set_iomux_function(device->rx_pin, IOMUX_GPIO2_FUNC_UART1_TXD);

  // Configure uart to printer controller board specifications (19200,8,N,1)
  uart_set_baud(1, 19200);
  uart_set_byte_length(1, UART_BYTELENGTH_8);
  uart_set_parity_enabled(1, false);
  uart_set_stopbits(1, UART_STOPBITS_1);
}

void write(uint8_t count, ...) {
  va_list commands;
  va_start(commands, count);

  uint8_t command;
  for (int i = 0; i < count; ++i) {
    command = (uint8_t)va_arg(commands, int);
    uart_putc(1, command);
  }
  va_end(commands);
}

void writeCharacter(char character) {
  uart_putc(1, character);
}

void writeString(char *str) {
  while(*str) {
    uart_putc(1, *str++);
  }
  uart_putc(1, '\n'); // (LF) Print buffer and line feed
  /* lineFeed(); */
}

/**
 * Print commands (Section 5.2.1)
 */
void thermalprinter_line_feed() {
  write(1, 0x0A);
}

void thermalprinter_tab() {
  write(1, 0x09);
}

void thermalprinter_feed_dots(uint8_t numOfDots) {
  write(3, 0x1B, 0x4A, numOfDots);
}

void thermalprinter_feed_lines(uint8_t numOfLines) {
  write(3, 0x1B, 0x64, numOfLines);
}

void thermalprinter_online() {
  write(3, 0x1B, 0x3D, 0x01);
}

void thermalprinter_offline() {
  write(3, 0x1B, 0x3D, 0x00);
}

/**
 * Line Spacing Setting commands (Section 5.2.2)
 */

void thermalprinter_default_line_spacing() {
  write(2, 0x1B, 0x32); // default = 32 dots
}

void thermalprinter_line_spacing(uint8_t numOfDots) {
  write(3, 0x1B, 0x33, numOfDots);
}

typedef enum {
  left = 0x30,
  middle = 0x31,
  right = 0x32,
} thermalprinter_align_mode_t;

void thermalprinter_align_mode(thermalprinter_align_mode_t alignMode) {
  if (alignMode == left || alignMode == middle || alignMode == right) {
    write(3, 0x1B, 0x61, alignMode);
  }
}

void thermalprinter_indent(uint8_t numOfCharacters) {
  if (numOfCharacters >= 0 && numOfCharacters <= 47) {
    write(3, 0x1B, 0x42, numOfCharacters);
  }
}

/**
 * Character commands (Section 5.2.3)
 */

typedef enum {
  reverse = 0x02,
  updown = 0x04,
  emphasized = 0x08,
  doubleHeight = 0x10,
  doubleWidth = 0x20,
  deleteLine = 0x40,
} thermalprinter_print_mode_t;

// Multiple modes can be set by combining thermalprinter_print_mode_t using the bit-wise or operator
void thermalprinter_print_mode(thermalprinter_print_mode_t printMode) {
  write(3, 0x1B, 0x21, printMode);
}

typedef enum {
  height = 0x0F,
  width = 0xF0,
} thermalprinter_enlarge_t;

// Multiple modes can be set by combining thermalprinter_enlarge_font_t using the bit-wise or operator
void thermalprinter_enlarge(thermalprinter_enlarge_t enlargeMode) {
  write(3, 0x1D, 0x21, enlargeMode);
}

void thermalprinter_enable_bold(bool enable) {
  if (enable) {
    write(3, 0x1B, 0x45, 0x01);
  } else {
    write(3, 0x1B, 0x45, 0x00);
  }
}

void thermalprinter_enable_double_width(bool enable) {
  if (enable) {
    write(2, 0x1B, 0x0E);
  } else {
    write(2, 0x1B, 0x14);
  }
}

void thermalprinter_enable_updown(bool enable) {
  if (enable) {
    write(3, 0x1B, 0x7B, 0x01);
  } else {
    write(3, 0x1B, 0x7B, 0x00);
  }
}

void thermalprinter_enable_reverse(bool enable) {
  if (enable) {
    write(3, 0x1D, 0x42, 0x01);
  } else {
    write(3, 0x1D, 0x42, 0x00);
  }
}

typedef enum {
  none = 0x00,
  thin = 0x01,
  thick = 0x02,
} thermalprinter_underline_t;

void thermalprinter_underline(thermalprinter_underline_t underlineMode) {
  write(3, 0x1B, 0x2D, underlineMode);
}

void thermalprinter_enable_user_defined_characters(bool enable) {
  if (enable) {
    write(3, 0x1B, 0x25, 0x01);
  } else {
    write(3, 0x1B, 0x25, 0x00);
  }
}

// TODO: Add method for setting user-defined characters (ESC & s n m w d1 d2 ... dx)

typedef enum {
  usa = 0x00,
  france = 0x01,
  germany = 0x02,
  uk = 0x03,
  denmark = 0x04,
  sweden = 0x05,
  italy = 0x06,
  spain = 0x07,
  japan = 0x08,
  norway = 0x09,
  denmark2 = 0x0A,
  spain2 = 0x0B,
  latinAmerica = 0x0C,
  korea = 0x0D,
} thermalprinter_character_set_t;

void thermalprinter_character_set(thermalprinter_character_set_t characterSet) {
  write(3, 0x1B, 0x52, characterSet);
}

typedef enum {
  pc437 = 0x00,
  pc850 = 0x01,
} thermalprinter_character_code_table_t;

void thermalprinter_character_code_table(thermalprinter_character_code_table_t characterCodeTable) {
  write(3, 0x1B, 0x74, characterCodeTable);
}

/**
 * Bit Image commands (Section 5.2.4)
 */

// TODO: Add commands for bit images

/**
 * Key Control command (Section 5.2.5)
 */

void thermalprinter_enable_panel_key(bool enable) {
  if (enable) {
    write(4, 0x1B, 0x63, 0x35, 0x00); // enabled by default
  } else {
    write(4, 0x1B, 0x63, 0x35, 0x01);
 }
}

/**
 * Init command (Section 5.2.6)
 */

// Initializes the printer
// - Clear print buffer
// - Reset the param to default value
// - Return to standard mode
// - Delete user-defined characters
void thermalprinter_reset() {
  write(2, 0x1B, 0x40);
}

/**
 * Status command (Section 5.2.7)
 */

// Transmit board status to host in format `P<Paper>V<Voltage>T<Degree>
// For example, "P1V72T30" means "Paper ready, Current voltage 7.2V, Temperature 30 degrees"
void thermalprinter_transmit_status_to_host() {
  write(3, 0x1B, 0x76, 0x00);
}

typedef enum {
  enableAsb = 0x04,
  enableRts = 0x20,
} thermalprinter_automatic_status_back_mode_t;

// Multiple modes can be set by combining thermalprinter_automatic_status_back_mode_t using the bit-wise or operator
void thermalprinter_enable_automatic_status_back(thermalprinter_automatic_status_back_mode_t mode) {
  write(3, 0x1D, 0x61, mode);
}

/**
 * Bar Code command (Section 5.2.8)
 */

// TODO: Add commands for bar code

/**
 * Control Parameter command (Section 5.2.9)
 */

// Set print quality settings
// - maxPrintingDots [0-255] Unit (8 dots), default: 8 (64 dots)
//  Increased heating dots correlates to faster printing speed at an expense of
//  higher peak current while printing.
//  and faster printing speed. The max heating dots is 8*(n+1).
// - heatingTime [3-255] Unit (10us), default: 80 (800us)
//  Increased heating time correlates to higher print density at an expense of
//  slower print speed. If heating time is too short, blank page may occur.
// - heatingInterval [0-255] Unit (10us), default: 2 (20us)
//  Increated heating interval correlates with higher clarity at an expense of
//  slower print speed.
void thermalprinter_print_settings(uint8_t maxPrintingDots, uint8_t heatingTime, uint8_t heatingInterval) {
  if (heatingTime >= 3 && heatingTime <= 255) {
    write(5, 0x1B, 0x37, maxPrintingDots, heatingTime, heatingInterval);
  }
}

void thermalprinter_default_printing_settings() {
  thermalprinter_print_settings(0x14, 0xFF, 0xFA);
}

void thermalprinter_sleep(uint8_t numOfSeconds) {
  write(3, 0x1B, 0x38, numOfSeconds);
}

void thermalprinter_prevent_sleep() {
  thermalprinter_sleep(0x00);
}

// When control board is in sleep mode, host must send wake and wait
void thermalprinter_wake() {
  write(1, 0xFF);
  vTaskDelay(50 / portTICK_PERIOD_MS);
}

// Set printing density
// - D4..D0 of n is used to set the printing density
//  Density is 50% + 5% * n(D4-D0) printing density
// - D7..D5 of n is used to set the printing break time
//  Break time is n(D7-D5)*250us
void thermalprinter_density(uint8_t densityAndBreakTime) {
  write(3, 0x12, 0x23, densityAndBreakTime);
}

void thermalprinter_print_test_page() {
  write(2, 0x12, 0x54);
}
