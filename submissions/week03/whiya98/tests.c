#include <stdio.h>
#include <stdint.h>

/* register_control.c에 구현된 함수 선언 */
uint8_t enable(uint8_t ctrl);
uint8_t set_mode(uint8_t ctrl, uint8_t mode);
uint8_t disable(uint8_t ctrl);
uint8_t is_ready(uint8_t status);

int main(void)
{
    int failures = 0;
    uint8_t actual;

    /* 1. enable(0xA8) -> 0xA9 */
    actual = enable(0xA8u);
    printf("enable(0xA8)      expected: 0xA9, actual: 0x%02X\n",
           (unsigned int)actual);

    if (actual != 0xA9u)
    {
        failures++;
    }

    /* 2. enable(0xA9) -> 0xA9 */
    actual = enable(0xA9u);
    printf("enable(0xA9)      expected: 0xA9, actual: 0x%02X\n",
           (unsigned int)actual);

    if (actual != 0xA9u)
    {
        failures++;
    }

    /* 3. set_mode(0xAB, 2) -> 0xAD */
    actual = set_mode(0xABu, 2u);
    printf("set_mode(0xAB, 2) expected: 0xAD, actual: 0x%02X\n",
           (unsigned int)actual);

    if (actual != 0xADu)
    {
        failures++;
    }

    /* 4. set_mode(0xAF, 2) -> 0xAD */
    actual = set_mode(0xAFu, 2u);
    printf("set_mode(0xAF, 2) expected: 0xAD, actual: 0x%02X\n",
           (unsigned int)actual);

    if (actual != 0xADu)
    {
        failures++;
    }

    /* 5. disable(0xAD) -> 0xAC */
    actual = disable(0xADu);
    printf("disable(0xAD)     expected: 0xAC, actual: 0x%02X\n",
           (unsigned int)actual);

    if (actual != 0xACu)
    {
        failures++;
    }

    /* 6. is_ready(0x09) -> 1 */
    actual = is_ready(0x09u);
    printf("is_ready(0x09)    expected: 1,    actual: %u\n",
           (unsigned int)actual);

    if (actual != 1u)
    {
        failures++;
    }

    /* 7. is_ready(0x01) -> 0 */
    actual = is_ready(0x01u);
    printf("is_ready(0x01)    expected: 0,    actual: %u\n",
           (unsigned int)actual);

    if (actual != 0u)
    {
        failures++;
    }

    if (failures == 0)
    {
        printf("\nAll tests passed.\n");
        return 0;
    }

    printf("\n%d test(s) failed.\n", failures);
    return 1;
}