#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/*
to compile: 
make -f Makefile.pitch_roll

  run on board /bin/angulo to download from wsl to linux board

[root@luckfox root]# /root/pitch_roll
ACC: ax=6.955 ay=-3.857 az=6.126 m/s2
Pitch: -43.85 graus
Roll : -32.19 graus

  */

  void calc_pitch_roll_ms2(float ax, float ay, float az,
                         float *pitch_deg, float *roll_deg)
{
    float denom = sqrtf(ay*ay + az*az);

    if (denom == 0.0f) {
        *pitch_deg = 0.0f;
    } else {
        *pitch_deg = atan2f(-ax, denom) * 180.0f / (float)M_PI;
    }

    *roll_deg = atan2f(ay, az) * 180.0f / (float)M_PI;
}

int main(void)
{
    FILE *f;
    float ax, ay, az;
    float pitch, roll;

    f = fopen("/proc/mpu6050", "r");
    if (!f) {
        perror("fopen /proc/mpu6050");
        return 1;
    }

    /* Lê só o início da linha:
       "ACC: 7.740 -1.817 6.045 m/s2 ..."
       O fscanf com "%f %f %f" funciona porque ele para nos espaços.
    */
    if (fscanf(f, "ACC: %f %f %f", &ax, &ay, &az) != 3) {
        fprintf(stderr, "Erro ao ler ACC de /proc/mpu6050_fifo\n");
        fclose(f);
        return 1;
    }

    fclose(f);

    calc_pitch_roll_ms2(ax, ay, az, &pitch, &roll);

    printf("ACC: ax=%.3f ay=%.3f az=%.3f m/s2\n", ax, ay, az);
    printf("Pitch: %.2f graus\n", pitch);
    printf("Roll : %.2f graus\n", roll);

    return 0;
}
