PREFIX   = arm-none-eabi-
CC       = $(PREFIX)gcc
OBJCOPY  = $(PREFIX)objcopy

BUILD_DIR = build

CFLAGS = -mcpu=cortex-m3 -mthumb -g -Wall
CFLAGS += -Isrc -I. -ICMSIS
CFLAGS += -IFreeRTOS/include -IFreeRTOS/portable/GCC/ARM_CM3
CFLAGS += -Isrc/adcConfig -Isrc/adcConfig/filtro_digital -Isrc/salidasSemaforo -Isrc/uartConfigSemaforo
CFLAGS += -DSTM32F103xB
CFLAGS += -ffunction-sections -fdata-sections
CFLAGS += -MMD -MP

LDFLAGS  = -T ld/STM32F103XB_FLASH.ld -nostartfiles
LDFLAGS += --specs=nano.specs --specs=nosys.specs
LDFLAGS += -u _printf_float
LDFLAGS += -Wl,--gc-sections -Wl,-Map=$(BUILD_DIR)/app.map
LDFLAGS += -lm

SRCS = CMSIS/startup_stm32f103xb.s \
       CMSIS/system_stm32f1xx.c \
       CMSIS/syscalls.c \
       src/main.c \
       src/adcConfig/init_adc.c \
       src/adcConfig/filtro_digital/biquad.c \
       src/salidasSemaforo/pines_init.c \
       src/uartConfigSemaforo/uart_init.c \
       FreeRTOS/list.c \
       FreeRTOS/queue.c \
       FreeRTOS/tasks.c \
       FreeRTOS/timers.c \
       FreeRTOS/portable/MemMang/heap_4.c \
       FreeRTOS/portable/GCC/ARM_CM3/port.c


OBJS  = $(addprefix $(BUILD_DIR)/,$(SRCS:.c=.o))
OBJS := $(OBJS:.s=.o)
DEPS  = $(OBJS:.o=.d)

ELF = $(BUILD_DIR)/app.elf
BIN = $(BUILD_DIR)/app.bin
HEX = $(BUILD_DIR)/app.hex

all: $(ELF) $(BIN) $(HEX)

# Regla para C
$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

# Regla para ensamblador
$(BUILD_DIR)/%.o: %.s
	@mkdir -p $(dir $@)
	$(CC) -c -mcpu=cortex-m3 -mthumb -o $@ $<

$(ELF): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

$(BIN): $(ELF)
	$(OBJCOPY) -O binary $< $@

$(HEX): $(ELF)
	$(OBJCOPY) -O ihex $< $@

flash: $(ELF)
	openocd -f interface/stlink.cfg -f target/stm32f1x.cfg -c "program $(ELF) verify reset exit"

clean:
	rm -rf $(BUILD_DIR)

-include $(DEPS)

.PHONY: all clean flash