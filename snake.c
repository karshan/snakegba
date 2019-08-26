#include "tonc.h"

enum dir {
    left,
    right,
    up,
    down
};

struct {
    int x;
    int y;
    int len;
    u8 dir;
    u8 tail[256];
} snake;

void init_snake() {
    snake.x = 20;
    snake.y = 20;
    snake.len = 1;
    snake.dir = up;
}

void init_timers() {
    // ~0.5s timer
    REG_TM2D = -0x2000;
    REG_TM2CNT = TM_FREQ_1024 | TM_ENABLE;
}

volatile int ticktock = 0;
void tick() {
    ticktock ^= 1;
}

int main()
{
    REG_DISPCNT = DCNT_MODE4 | DCNT_BG2;

    pal_bg_mem[0] = RGB15(0, 0, 0);
    pal_bg_mem[1] = RGB15(31, 31, 31);

    init_snake();
    init_timers();
    irq_init(isr_master);
    irq_add(II_TIMER2, tick);
    irq_enable(II_TIMER2);

    while(1) {
        vid_vsync();
        int l = 20 * ticktock;
        int cl = 20 * (ticktock ^ 1);
        m4_rect(l, 20, l + 30, 50, 1);
        m4_rect(cl, 20, cl + 30, 50, 0);
        vid_flip();
    }

    return 0;
}
