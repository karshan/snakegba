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

struct {
    int x;
    int y;
    int dir;
    tail_chunk tail[256];
} snake;

#define MAP_WIDTH  48
#define MAP_HEIGHT 32

void init_snake() {
    snake.x = 24;
    snake.y = 16;
    snake.dir = left;
}

void init_timers() {
    // ~0.5s timer
    REG_TM2D = -0x2000;
    REG_TM2CNT = TM_FREQ_1024 | TM_ENABLE;
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

    // draw snake
    m4_rect(snake.x * 5, snake.y * 5, (snake.x + 1) * 5, (snake.y + 1) * 5, 1);
    vid_flip();
}

int main()
{
    REG_DISPCNT = DCNT_MODE4 | DCNT_BG2;

    pal_bg_mem[0] = RGB15(0, 0, 0);
    pal_bg_mem[1] = RGB15(31, 31, 31);
    pal_bg_mem[2] = RGB15(31, 0, 0);

    init_snake();
    init_timers();
    irq_init(isr_master);
    irq_add(II_TIMER2, tick);
    irq_enable(II_TIMER2);

    while(1);
    return 0;
}
