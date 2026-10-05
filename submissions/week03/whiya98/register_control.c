#include <stdint.h>

uint8_t enable(uint8_t ctrl)
{
    return ctrl | 0x01u;
}

uint8_t set_mode(uint8_t ctrl, uint8_t mode)
{
    return (ctrl & 0xF9u) | (uint8_t)(mode << 1); // 0xF9 = 1111 1001
}

uint8_t disable(uint8_t ctrl)
{
    return ctrl & 0xFEu; // 0xFE = 1111 1110 
}

uint8_t is_ready(uint8_t status)
{
    return (status & 0x08u) >> 3;
}