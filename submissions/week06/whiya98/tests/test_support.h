#ifndef TEST_SUPPORT_H
#define TEST_SUPPORT_H
#include <stdbool.h>
void test_check(bool result, const char *expression, const char *file, int line);
#define CHECK(expression) test_check((expression), #expression, __FILE__, __LINE__)
void test_student_cases(void);
#endif
