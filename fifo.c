#include <linux/kfifo.h>
#include <linux/wait.h>
#include <linux/poll.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/ktime.h>
#include <linux/spinlock.h>
#include <linux/errno.h>

#include "fifo.h"

#define MPU_FIFO_SIZE 128

/* FIFO de samples */
static DECLARE_KFIFO(mpu_fifo, struct mpu_sample, MPU_FIFO_SIZE);

/* Sincronização */
static spinlock_t mpu_fifo_lock;
static wait_queue_head_t mpu_wq;

int mpu_fifo_init(void)
{
    spin_lock_init(&mpu_fifo_lock);
    init_waitqueue_head(&mpu_wq);
    INIT_KFIFO(mpu_fifo);
    return 0;
}

void mpu_fifo_exit(void)
{
    /* nada especial por enquanto */
}

int mpu_fifo_push(const struct mpu_sample *s)
{
    unsigned long flags;
    unsigned int n;

    spin_lock_irqsave(&mpu_fifo_lock, flags);
    n = kfifo_in(&mpu_fifo, s, 1);
    spin_unlock_irqrestore(&mpu_fifo_lock, flags);

    if (n)
        wake_up_interruptible(&mpu_wq);

    return n ? 0 : -ENOSPC;
}

int mpu_fifo_pop(struct mpu_sample *s)
{
    unsigned long flags;
    unsigned int n;

    spin_lock_irqsave(&mpu_fifo_lock, flags);
    n = kfifo_out(&mpu_fifo, s, 1);
    spin_unlock_irqrestore(&mpu_fifo_lock, flags);

    return n ? 0 : -EAGAIN;
}
