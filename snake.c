#include "string.h"
#include "tonc.h"

#include "util.h"
#include "types.h"

// MAP_WIDTH  * CELL_SIZE == SCREEN_WIDTH
// MAP_HEIGHT * CELL_SIZE == SCREEN_HEIGHT
#define CELL_SIZE 8
#define MAP_WIDTH  (SCREEN_WIDTH/CELL_SIZE)
#define MAP_HEIGHT (SCREEN_HEIGHT/CELL_SIZE)
#define BOX(x, y, c) m4_rect(x * CELL_SIZE, y * CELL_SIZE, (x + 1) * CELL_SIZE, (y + 1) * CELL_SIZE, c)
#define INITIAL_SNAKE_SIZE 4 // minimum 4 due to subcell rendering code (maybe 3 =D)
#define MAX_SNAKE_SIZE 1024

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

void move_screen(point *p, dir d, int units) {
    switch(d) {
        case left:
            p->x = p->x - units;
            if (p->x < 0) p->x += SCREEN_WIDTH;
            break;
        case right:
            p->x = p->x + units;
            if (p->x >= SCREEN_WIDTH) p->x -= SCREEN_WIDTH;
            break;
        case up:
            p->y = p->y - units;
            if (p->y < 0) p->y += SCREEN_HEIGHT;
            break;
        case down:
            p->y = p->y + units;
            if (p->y >= SCREEN_HEIGHT) p->y -= SCREEN_HEIGHT;
            break;
    }
}

volatile struct {
    int dir;
    int idir; // last input
    point chunks[MAX_SNAKE_SIZE];
    int head;
    int size;
} snake;

volatile point fruit;

void new_fruit() {
    do {
        fruit.x = qran_range(0, MAP_WIDTH);
        fruit.y = qran_range(0, MAP_HEIGHT);
    } while (fruit.x == snake.chunks[snake.head].x && fruit.y == snake.chunks[snake.head].y);
}

void init_game() {
    int i;
    snake.head = 0;
    snake.size = INITIAL_SNAKE_SIZE;
    for (i = 0; i < INITIAL_SNAKE_SIZE; i++) {
        snake.chunks[i].x = MAP_WIDTH/2;
        snake.chunks[i].y = MAP_HEIGHT/2;
    }
    snake.dir = left;
    snake.idir = left;
    new_fruit();
}

void init_timers() {
    // ~0.2s/CELL_SIZE timer
    REG_TM2D = -(0x800/CELL_SIZE);
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

void death() {
    debugf();
    tte_printf("DEATH");
    while(1);
}

u32 ticks = 0;
void tick() {
    int i, j, pixels = ticks % CELL_SIZE;
    point t;

    M4_CLEAR();

    if (pixels == 0) {
        // move snake
        snake.head = snake.head - 1;
        if (snake.head < 0) snake.head = MAX_SNAKE_SIZE - 1;
        snake.chunks[snake.head] = snake.chunks[(snake.head + 1) % MAX_SNAKE_SIZE];
        move(&snake.chunks[snake.head], snake.dir);

        for (i = 1, j = (snake.head + 1) % MAX_SNAKE_SIZE; i < snake.size; i++, j = (j + 1) % MAX_SNAKE_SIZE) {
            if (memcmp(&snake.chunks[j], &snake.chunks[snake.head], sizeof(point)) == 0) {
                death();
            }
        }

        snake.dir = snake.idir;

        // if (ate fruit)
        if (snake.chunks[snake.head].x == fruit.x && snake.chunks[snake.head].y == fruit.y) {
            new_fruit();
            if (snake.size == MAX_SNAKE_SIZE) abort();
            snake.size++;
        }
    }

    // draw fruit and snake except last chunk
    BOX(fruit.x, fruit.y, 2);
    for (i = 0, j = snake.head; i < snake.size - 1; i++, j = (j + 1) % MAX_SNAKE_SIZE) {
        BOX(snake.chunks[j].x, snake.chunks[j].y, 1);
    }

    // draw snake head
    t.x = snake.chunks[snake.head].x * CELL_SIZE;
    t.y = snake.chunks[snake.head].y * CELL_SIZE;
    move_screen(&t, snake.dir, pixels);
    m4_rect(t.x, t.y, t.x + CELL_SIZE, t.y + CELL_SIZE, 1);

    // draw snake last chunk

    // if we are about to eat, draw the last chunk in full
    i = (snake.head + snake.size - 1) % MAX_SNAKE_SIZE;
    j = (snake.head + snake.size - 2) % MAX_SNAKE_SIZE;
    t = snake.chunks[snake.head];
    move(&t, snake.dir);
    if (t.x == fruit.x && t.y == fruit.y) {
        BOX(snake.chunks[i].x, snake.chunks[i].y, 1);
    } else {
        t.x = snake.chunks[i].x * CELL_SIZE;
        t.y = snake.chunks[i].y * CELL_SIZE;

        if ((snake.chunks[i].x + 1) % MAP_WIDTH == snake.chunks[j].x) {
            move_screen(&t, right, pixels);
        } else if ((snake.chunks[j].x + 1) % MAP_WIDTH == snake.chunks[i].x) {
            move_screen(&t, left, pixels);
        } else if ((snake.chunks[i].y + 1) % MAP_WIDTH == snake.chunks[j].y) {
            move_screen(&t, down, pixels);
        } else if ((snake.chunks[j].y + 1) % MAP_WIDTH == snake.chunks[i].y) {
            move_screen(&t, up, pixels);
        }
        m4_rect(t.x, t.y, t.x + CELL_SIZE, t.y + CELL_SIZE, 1);
    }

    vid_flip();
    ticks++;
}

void init_palette() {
    pal_bg_mem[0] = RGB15(0, 0, 0);
    pal_bg_mem[1] = RGB15(31, 31, 31);
    pal_bg_mem[2] = RGB15(31, 0, 0);
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
