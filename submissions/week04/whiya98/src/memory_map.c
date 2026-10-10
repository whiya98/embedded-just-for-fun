#include "mission.h"

#define FLASH_START   0x08000000u
#define FLASH_END     0x0800FFFFu

#define SRAM_START    0x20000000u
#define SRAM_END      0x20001FFFu

#define GPIOA_START   0x48000000u
#define GPIOA_END     0x480003FFu

#define RCC_START     0x40021000u
#define RCC_END       0x400213FFu

MemoryRegion memory_region(uint32_t address)
{
    /* TODO M1: Datasheet 근거로 Flash/SRAM/GPIOA/RCC 경계를 반영하세요.
       단일 주소 숫자만 분류합니다. Pointer 변환이나 역참조는 금지합니다. */

    if ((address >= FLASH_START) && (address <= FLASH_END))
    {
        return MEMORY_FLASH;
    }

    if ((address >= SRAM_START) && (address <= SRAM_END))
    {
        return MEMORY_SRAM;
    }

    if ((address >= GPIOA_START) && (address <= GPIOA_END))
    {
        return MEMORY_GPIOA;
    }

    if ((address >= RCC_START) && (address <= RCC_END))
    {
        return MEMORY_RCC;
    }

    return MEMORY_UNKNOWN;

}
