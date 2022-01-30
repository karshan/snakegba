PREFIX  := arm-none-eabi-
CC      := $(PREFIX)gcc
LD      := $(PREFIX)gcc
OBJCOPY := $(PREFIX)objcopy

CFLAGS  := -mthumb-interwork -mthumb -O2 -Wall -fno-strict-aliasing -Itonclib/include
LDFLAGS := -mthumb-interwork -mthumb -specs=gba.specs tonclib/lib/libtonc.a

TARGET := snake
BMP_DIR := gfx
BMPS   := berry.bmp tae.bmp snakehead.bmp snakebody.bmp
F_BMPS  := $(foreach bmp, $(BMPS), $(BMP_DIR)/$(bmp))
C_BMPS  := $(foreach bmp, $(BMPS), $(BMP_DIR)/$(basename $(bmp)).c) $(BMP_DIR)/shared.c
H_BMPS  := $(foreach bmp, $(BMPS), $(BMP_DIR)/$(basename $(bmp)).h) $(BMP_DIR)/shared.h
O_BMPS  := $(foreach bmp, $(BMPS), $(BMP_DIR)/$(basename $(bmp)).o) $(BMP_DIR)/shared.o
OBJS   := snake.o util.o $(O_BMPS)

build: $(TARGET).gba

# Strip and fix header (step 4,5)
$(TARGET).gba : $(TARGET).elf
	$(OBJCOPY) -v -O binary $< $@
	-@gbafix $@

# Link (step 3)
$(TARGET).elf : $(OBJS)
	$(LD) $^ $(LDFLAGS) -o $@

$(OBJS): $(C_BMPS)

# Compile (step 2)
$(OBJS) : %.o : %.c
	$(CC) -c $< $(CFLAGS) -o $@

# Generate C files for bitmaps (step 1)
$(C_BMPS) : $(F_BMPS)
	cd gfx && grit $(BMPS) -ftc -gb -pS -O shared -gB8

clean :
	@rm -fv *.gba
	@rm -fv *.elf
	@rm -fv $(OBJS)
	@rm -fv $(C_BMPS)
	@rm -fv $(H_BMPS)
