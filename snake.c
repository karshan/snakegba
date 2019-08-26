#include "tonc.h"

int main()
{
    REG_DISPCNT = DCNT_MODE4 | DCNT_BG2;

    pal_bg_mem[0] = RGB15(0, 0, 0);
    pal_bg_mem[1] = RGB15(31, 31, 31);

    m4_rect(20, 20, 50, 50, 1);
    vid_flip();

    while(1);
    return 0;
}
