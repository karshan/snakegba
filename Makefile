PREFIX  := arm-none-eabi-
CC      := $(PREFIX)gcc
LD      := $(PREFIX)gcc
OBJCOPY := $(PREFIX)objcopy

CFLAGS  := -mthumb-interwork -mthumb -O2 -Wall -fno-strict-aliasing -Itonclib/include
LDFLAGS := -mthumb-interwork -mthumb -specs=gba.specs tonclib/lib/libtonc.a

TARGET := snake
BMPS   := berry.bmp tae.bmp
C_BMPS  := $(foreach bmp, $(BMPS), $(basename $(bmp)).c) shared.c
H_BMPS  := $(foreach bmp, $(BMPS), $(basename $(bmp)).h) shared.h
O_BMPS  := $(foreach bmp, $(BMPS), $(basename $(bmp)).o) shared.o
OBJS   := snake.o util.o $(O_BMPS)

build: $(TARGET).gba

# Strip and fix header (step 4,5)
$(TARGET).gba : $(TARGET).elf
	$(OBJCOPY) -v -O binary $< $@
	-@gbafix $@

# Link (step 3)
$(TARGET).elf : $(OBJS)
	$(LD) $^ $(LDFLAGS) -o $@

# Compile (step 2)
$(OBJS) : %.o : %.c $(C_BMPS)
	$(CC) -c $< $(CFLAGS) -o $@

# Generate C files for bitmaps (step 1)
$(C_BMPS) : $(BMPS)
	grit $(BMPS) -ftc -gb -pn 32 -pS -O shared -gB8

clean :
	@rm -fv *.gba
	@rm -fv *.elf
	@rm -fv *.o
	@rm -fv $(C_BMPS)
	@rm -fv $(H_BMPS)
