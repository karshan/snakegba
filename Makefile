PREFIX  := arm-none-eabi-
CC      := $(PREFIX)gcc
LD      := $(PREFIX)gcc
OBJCOPY := $(PREFIX)objcopy

CFLAGS  := -mthumb-interwork -mthumb -O2 -Wall -fno-strict-aliasing -I/home/karshan/Downloads/code/tonclib/include
LDFLAGS := -mthumb-interwork -mthumb -specs=gba.specs /home/karshan/Downloads/code/tonclib/lib/libtonc.a

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
