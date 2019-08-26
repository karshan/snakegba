#include "tonc.h"

enum dir {
    left,
    right,
    up,
    down
};

typedef struct {
    int dir;
    int len;
} tail_chunk;

volatile struct {
    int x;
    int y;
    int dir;
    tail_chunk tail[256];
} snake;

volatile struct {
    int x;
    int y;
} fruit;

#define MAP_WIDTH  48
#define MAP_HEIGHT 32

void init_game() {
    snake.x = 24;
    snake.y = 16;
    snake.dir = left;

    do {
        fruit.x = qran_range(0, MAP_WIDTH);
        fruit.y = qran_range(0, MAP_HEIGHT);
    } while (fruit.x == snake.x && fruit.y == snake.y);
}

void init_timers() {
    // ~0.5s timer
    REG_TM2D = -0x2000;
    REG_TM2CNT = TM_FREQ_1024 | TM_ENABLE;
}

void key() {
    if (KEY_DOWN_NOW(KEY_LEFT)) {
        snake.dir = left;
        return;
    } if (KEY_DOWN_NOW(KEY_UP)) {
        snake.dir = up;
        return;
    } if (KEY_DOWN_NOW(KEY_RIGHT)) {
        snake.dir = right;
        return;
    } if (KEY_DOWN_NOW(KEY_DOWN)) {
        snake.dir = down;
        return;
    }
}

void tick() {
    // clear snake
    M4_CLEAR();

    // move snake
    switch (snake.dir) {
        case up:
            snake.y = snake.y - 1;
            if (snake.y < 0) {
                snake.y = MAP_HEIGHT - 1;
            }
            break;
        case down:
            snake.y = snake.y + 1;
            if (snake.y >= MAP_HEIGHT) {
                snake.y = 0;
            }
            break;
        case right:
            snake.x = snake.x + 1;
            if (snake.x >= MAP_WIDTH) {
                snake.x = 0;
            }
            break;
        case left:
            snake.x = snake.x - 1;
            if (snake.x < 0) {
                snake.x = MAP_WIDTH - 1;
            }
            break;
    }

    // draw snake and fruit
    m4_rect(snake.x * 5, snake.y * 5, (snake.x + 1) * 5, (snake.y + 1) * 5, 1);
    m4_rect(fruit.x * 5, fruit.y * 5, (fruit.x + 1) * 5, (fruit.y + 1) * 5, 2);
    vid_flip();
}

void init_palette() {
    pal_bg_mem[0] = RGB15(0, 0, 0);
    pal_bg_mem[1] = RGB15(31, 31, 31);
    pal_bg_mem[2] = RGB15(31, 0, 0);
}

void debugf() {
    REG_DISPCNT= DCNT_MODE0 | DCNT_BG0;
    tte_init_se_default(0, BG_CBB(0)|BG_SBB(31));
    tte_init_con();
    tte_printf("#{P:72,64}");
}

void init_rand() {
    u32 seed = sram_mem[0] |
            (sram_mem[1] << 8) |
            (sram_mem[2] << 16) |
            (sram_mem[3] << 24);
    seed++;
    sram_mem[0] = seed & 0xff;
    sram_mem[1] = (seed >> 8) & 0xff;
    sram_mem[2] = (seed >> 16) & 0xff;
    sram_mem[3] = (seed >> 24) & 0xff;
    sqran(seed);
    qran(); // first value is garbage???
}

int main()
{
    REG_DISPCNT = DCNT_MODE4 | DCNT_BG2;

    init_palette();
    init_rand();
    init_game();
    init_timers();

    irq_init(isr_master);
    irq_add(II_TIMER2, tick);
    irq_enable(II_TIMER2);
    irq_add(II_KEYPAD, key);
    REG_KEYCNT = KCNT_OR | KEY_DIR;
    irq_enable(II_KEYPAD); // TODO ensure this is lower prio than timer

    while(1);
    return 0;
}
