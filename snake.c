#include "tonc.h"
#define BOX(x, y, c) m4_rect(x * 5, y * 5, (x + 1) * 5, (y + 1) * 5, c)
#define MAP_WIDTH  48
#define MAP_HEIGHT 32

typedef enum {
    left,
    right,
    up,
    down
} dir;

typedef struct {
    int x;
    int y;
} point;

void move(point *p, dir d) {
    switch(d) {
        case left:
            p->x = p->x - 1;
            if (p->x < 0) p->x = MAP_WIDTH - 1;
            break;
        case right:
            p->x = p->x + 1;
            if (p->x >= MAP_WIDTH) p->x = 0;
            break;
        case up:
            p->y = p->y - 1;
            if (p->y < 0) p->y = MAP_HEIGHT - 1;
            break;
        case down:
            p->y = p->y + 1;
            if (p->y >= MAP_HEIGHT) p->y = 0;
            break;
    }
}

volatile struct {
    int dir;
    int idir; // last input
    point head;
    int n; // sizeof tail
    point tail[256];
} snake;

volatile point fruit;

void new_fruit() {
    do {
        fruit.x = qran_range(0, MAP_WIDTH);
        fruit.y = qran_range(0, MAP_HEIGHT);
    } while (fruit.x == snake.head.x && fruit.y == snake.head.y);
}

void init_game() {
    snake.head.x = 24;
    snake.head.y = 16;
    snake.dir = left;
    snake.n = 0;
    new_fruit();
}

void init_timers() {
    // ~0.5s timer
    REG_TM2D = -0x2000;
    REG_TM2CNT = TM_FREQ_1024 | TM_ENABLE;
}

void key() {
    if (KEY_DOWN_NOW(KEY_LEFT) && snake.dir != right) {
        snake.idir = left;
        return;
    } if (KEY_DOWN_NOW(KEY_UP) && snake.dir != down) {
        snake.idir = up;
        return;
    } if (KEY_DOWN_NOW(KEY_RIGHT) && snake.dir != left) {
        snake.idir = right;
        return;
    } if (KEY_DOWN_NOW(KEY_DOWN) && snake.dir != up) {
        snake.idir = down;
        return;
    }
}

int eating = 0;
void tick() {
    int i;

    snake.dir = snake.idir;

    M4_CLEAR();

    // move snake
    if (eating == 1) {
        snake.n++;
        eating = 0;
    }
    for (i = snake.n - 1; i > 0; i--) {
        snake.tail[i] = snake.tail[i - 1];
    }
    snake.tail[0] = snake.head;
    move(&snake.head, snake.dir);

    if (snake.head.x == fruit.x && snake.head.y == fruit.y) {
        new_fruit();
        eating = 1;
    }

    // draw snake and fruit
    BOX(snake.head.x, snake.head.y, 1);
    for (i = 0; i < snake.n; i++) {
        BOX(snake.tail[i].x, snake.tail[i].y, 1);
    }
    BOX(fruit.x, fruit.y, 2);

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
