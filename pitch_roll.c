 #include <stdio.h>
 #include <stdlib.h>
 #include <math.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
 

#include <unistd.h>
#include <termios.h>
#include <poll.h>
#include <ctype.h>

 /*
 to compile:
 make -f Makefile.pitch_roll

   run on board /bin/angulo to download from wsl to linux board

 [root@luckfox root]# /root/pitch_roll
 ACC: ax=6.955 ay=-3.857 az=6.126 m/s2
 Pitch: -43.85 degrees
 Roll : -32.19 degrees

   */
 
int serial_open(const char *device)
{
    int fd = open(device, O_RDWR | O_NOCTTY | O_SYNC);
    if (fd < 0) {
        perror("open");
        return -1;
    }

    struct termios tty;
    memset(&tty, 0, sizeof tty);

    if (tcgetattr(fd, &tty) != 0) {
        perror("tcgetattr");
        close(fd);
        return -1;
    }

    cfsetospeed(&tty, B115200);
    cfsetispeed(&tty, B115200);

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8 bits
    tty.c_iflag = 0;
    tty.c_oflag = 0;
    tty.c_lflag = 0;                                // raw mode
    tty.c_cflag |= (CLOCAL | CREAD);                // enable receiver
    tty.c_cflag &= ~(PARENB | PARODD);              // no parity
    tty.c_cflag &= ~CSTOPB;                         // 1 stop bit
    tty.c_cflag &= ~CRTSCTS;                        // no flow control

    tty.c_cc[VMIN]  = 0;
    tty.c_cc[VTIME] = 0; // 0.5s timeout read

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        perror("tcsetattr");
        close(fd);
        return -1;
    }

    return fd;
}

static void serial_flush_rx(int fd)
{
    char tmp[128];
    for (;;) {
        ssize_t n = read(fd, tmp, sizeof(tmp));
        if (n <= 0) break;
    }
}


// lê até '\n' com timeout (ms). ignora '\r' (seu dispositivo manda no início).
// return: >0 bytes lidos, 0 timeout, -1 erro
static int serial_readline_timeout(int fd, char *out, size_t out_sz, int timeout_ms)
{
    if (out_sz < 2) return -1;
    size_t used = 0;

    struct pollfd pfd = { .fd = fd, .events = POLLIN };

    int r = poll(&pfd, 1, timeout_ms);
    if (r < 0) return -1;
    if (r == 0) { out[0] = '\0'; return 0; }

    while (used < out_sz - 1) {
        char c;
        ssize_t n = read(fd, &c, 1);
        if (n < 0) return -1;
        if (n == 0) break;

        if (c == '\r') continue;          // ignora CR no início (ou em qualquer lugar)
        out[used++] = c;
        out[used] = '\0';

        if (c == '\n') return (int)used;  // fim da resposta
        // se não veio '\n' ainda, continua lendo (o fd já está legível)
    }

    return (int)used;
}

// normaliza removendo espaços e \n final
static void strip_ws_and_eol(const char *in, char *out, size_t out_sz)
{
    size_t j = 0;
    for (size_t i = 0; in[i] && j < out_sz - 1; i++) {
        if (in[i] == '\n' || in[i] == '\r') break;
        if (isspace((unsigned char)in[i])) continue;
        out[j++] = in[i];
    }
    out[j] = '\0';
}

static int serial_send_wait_ack_retry(
    int fd,
    const char *cmd,
    const char *prefix,     // "ABC1" ou "ABC2"
    int timeout_ms,         // ex: 150
    int retries){             // ex: 3
   
    char line[128];
    char norm[64];
    size_t prelen = strlen(prefix);

    for (int attempt = 1; attempt <= retries; attempt++) {
        serial_flush_rx(fd);

        if (write(fd, cmd, strlen(cmd)) < 0) return -1;

        int n = serial_readline_timeout(fd, line, sizeof(line), timeout_ms);
        if (n <= 0) {
            // 0 = timeout, -1 = erro
            continue;
        }

        strip_ws_and_eol(line, norm, sizeof(norm));
        // esperado: "ABC11" / "ABC10" etc.
        if (strlen(norm) == prelen + 1 && strncmp(norm, prefix, prelen) == 0) {
            if (norm[prelen] == '1') return 1; // OK
            if (norm[prelen] == '0') return 0; // FAIL
        }

        // resposta inesperada -> retry
    }

    return -1; // estourou tentativas (timeout/resposta inválida)
}





void serial_send(int fd, const char *cmd)
{
    write(fd, cmd, strlen(cmd));
}

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
    int fd;
     float ax, ay, az;
     float pitch, roll;
    char buffer[128];
    const struct timespec interval = { .tv_sec = 0, .tv_nsec = 50 * 1000 * 1000 };
 
    fd = open("/proc/mpu6050_fifo", O_RDONLY | O_NONBLOCK);
    if (fd < 0) {
        perror("open /proc/mpu6050_fifo");
         return 1;
     }
 

   int serial_fd = serial_open("/dev/ttyS3");
    if (serial_fd < 0){
        return -1;
    }

    // exemplo fixo
    //serial_send(serial_fd, "ABC1 S 16*1\n");
    //esperar resposta  ABC11 se OK ou ABC10 SE FALHOU
    //usleep(50000); // 50ms
    //serial_send(serial_fd, "ABC2 V 33 -33*1\n");

    int ok1 = serial_send_wait_ack_retry(serial_fd, "ABC1 S 16*1\n", "ABC1", 5, 3);
    if (ok1 != 1) fprintf(stderr, "ABC1 não confirmou (ret=%d)\n", ok1);

    int ok2 = serial_send_wait_ack_retry(serial_fd, "ABC2 V 33 -33*1\n", "ABC2", 5, 3);
    if (ok2 != 1) fprintf(stderr, "ABC2 não confirmou (ret=%d)\n", ok2);

    close(serial_fd);

    for (;;) {
        ssize_t bytes;

        bytes = pread(fd, buffer, sizeof(buffer) - 1, 0);

        if (bytes < 0) {
            if (errno != EAGAIN) {
                fprintf(stderr, "Erro ao ler /proc/mpu6050_fifo: %s\n",
                        strerror(errno));
                close(fd);
                return 1;
            }
            nanosleep(&interval, NULL);
            continue;
        }
 
        if (bytes == 0) {
            nanosleep(&interval, NULL);
            continue;
        }
 
        buffer[bytes] = '\0';
 
        /* Read only the start of the line:
           "ACC: 7.740 -1.817 6.045 m/s2 ..."
           The sscanf with "%f %f %f" works because it stops at spaces.
        */
        if (sscanf(buffer, "ACC: %f %f %f", &ax, &ay, &az) != 3) {
            fprintf(stderr, "Erro ao ler ACC de /proc/mpu6050_fifo\n");
            nanosleep(&interval, NULL);
            continue;
        }
 
        calc_pitch_roll_ms2(ax, ay, az, &pitch, &roll);

        printf("ACC: ax=%.3f ay=%.3f az=%.3f m/s2\n", ax, ay, az);
        printf("Pitch: %.2f graus\n", pitch);
        printf("Roll : %.2f graus\n", roll);

        nanosleep(&interval, NULL);
    }
 }

