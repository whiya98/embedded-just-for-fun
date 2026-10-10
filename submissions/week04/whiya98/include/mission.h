#ifndef MISSION_H
#define MISSION_H

#include <stdbool.h>
#include <stdint.h>

/* Fixed public interface. Do not edit this header for the submission. */
typedef enum {
    MEMORY_UNKNOWN = 0,
    MEMORY_FLASH,
    MEMORY_SRAM,
    MEMORY_GPIOA,
    MEMORY_RCC
} MemoryRegion;

/* Classify one numeric address. Never dereference it on the Host. */
MemoryRegion memory_region(uint32_t address);
bool board_clock_init(void);
void board_led_init(void);
void board_led_toggle(void);
bool app_init(void);
void app_step(void);

#endif
