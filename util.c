#include "tonc.h"

// usage: debugf(); tte_printf("something: %d", something); while(1);
void debugf() {
    REG_DISPCNT = DCNT_MODE0 | DCNT_BG0;
    tte_init_se_default(0, BG_CBB(0)|BG_SBB(31));
    tte_init_con();
    tte_printf("#{P:72,64}");
}

void abort() {
    debugf();
    tte_printf("ABORT");
    while(1);
}
