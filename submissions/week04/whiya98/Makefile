# Host-only build. No ARM compiler, board flashing or MCU startup is performed.
CC ?= cc
CPPFLAGS += -Iinclude -Ihost
CFLAGS ?= -std=c11 -Wall -Wextra -Werror -pedantic -O0 -g
ASFLAGS ?= -g
LDFLAGS ?=
LDLIBS ?=
EXEEXT ?=
BUILD := build
MISSION_SRC := src/memory_map.c src/clock.c src/led.c src/app.c
SUPPORT_SRC := host/fake_hal.c tests/test_mission.c tests/test_student.c
SOURCES := $(MISSION_SRC) $(SUPPORT_SRC)
OBJECTS := $(patsubst %.c,$(BUILD)/%.o,$(SOURCES))
PREPROCESSED := $(patsubst %.c,$(BUILD)/%.i,$(SOURCES))
ASSEMBLY := $(patsubst %.c,$(BUILD)/%.s,$(SOURCES))
HEADERS := include/mission.h host/stm32f0xx_hal.h host/fake_hal.h
TARGET := $(BUILD)/mission-tests$(EXEEXT)

.PHONY: all test stages clean
# Keep intermediates available for inspection after both test and stages.
.SECONDARY: $(PREPROCESSED) $(ASSEMBLY)
all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJECTS) $(LDLIBS) -o $@

$(BUILD)/%.o: $(BUILD)/%.s
	@mkdir -p $(dir $@)
	$(CC) $(ASFLAGS) -c $< -o $@

$(BUILD)/%.i: %.c $(HEADERS)
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -E $< -o $@

$(BUILD)/%.s: $(BUILD)/%.i
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -S $< -o $@

test: $(TARGET)
	./$(TARGET)

stages: $(PREPROCESSED) $(ASSEMBLY) $(TARGET)
	@echo "Generated: build/src/*.i, *.s, *.o and Host executable $(TARGET)"
	@echo "Executable format depends on the Host OS; this is not an MCU firmware image."

clean:
	rm -rf $(BUILD)
