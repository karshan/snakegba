PREFIX  := arm-none-eabi-
CC      := $(PREFIX)gcc
LD      := $(PREFIX)gcc
OBJCOPY := $(PREFIX)objcopy

CFLAGS  := -mthumb-interwork -mthumb -O2 -Wall -fno-strict-aliasing
LDFLAGS := -mthumb-interwork -mthumb -specs=gba.specs

TARGET := snake
OBJS   := snake.o

build: $(TARGET).gba

# Strip and fix header (step 3,4)
$(TARGET).gba : $(TARGET).elf
	$(OBJCOPY) -v -O binary $< $@
	-@gbafix $@

# Link (step 2)
$(TARGET).elf : $(OBJS)
	$(LD) $^ $(LDFLAGS) -o $@

# Compile (step 1)
$(OBJS) : %.o : %.c
	$(CC) -c $< $(CFLAGS) -o $@
		
clean : 
	@rm -fv *.gba
	@rm -fv *.elf
	@rm -fv *.o
