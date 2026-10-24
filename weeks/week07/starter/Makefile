CC ?= cc
EXEEXT ?=
CPPFLAGS ?= -Iinclude -Ihost -Itests
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -Werror -O2
LDFLAGS ?=
COMMON = src/uart_config.c src/frame_codec.c src/stream_rx.c src/stream_tx.c host/fake_platform.c
HEADERS = include/mission.h include/platform.h host/fake_platform.h tests/test_support.h
TEST_BIN = build/mission-tests$(EXEEXT)
DEMO_BIN = build/mission-demo$(EXEEXT)

.PHONY: all test demo clean
all: $(TEST_BIN) $(DEMO_BIN)
$(TEST_BIN): $(COMMON) tests/test_mission.c tests/test_student.c $(HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(COMMON) tests/test_mission.c tests/test_student.c $(LDFLAGS) -o $@
$(DEMO_BIN): $(COMMON) host/demo.c $(HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(COMMON) host/demo.c $(LDFLAGS) -o $@
build:
	mkdir -p build
test: $(TEST_BIN)
	./$(TEST_BIN)
demo: $(DEMO_BIN)
	./$(DEMO_BIN)
clean:
	rm -rf build
