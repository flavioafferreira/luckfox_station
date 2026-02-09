#ifndef _LORA_STATION_FIFO_H
#define _LORA_STATION_FIFO_H

#include <linux/types.h>   // para u8, u16, u32, u64, s64, etc.


struct mpu_sample {
    s64 ts_ns;   // timestamp em ns (ktime_get_ns)
    s32 ax, ay, az; // ou mm/s², o que preferir
    s32 gx, gy, gz; // mdps
    s32 temp_mC;    // milicelsius
};


#endif /* _LORA_STATION_FIFO_H */