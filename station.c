/**
 * @readme dev_tree
 * DEVICE TREE
 * /home/flavio/sdk/luckfox-pico/sysdrv/source/kernel/arch/arm/boot/dts/rv1106g-evb1-v10.dts
 * /home/flavio/sdk/luckfox-pico/sysdrv/source/kernel/arch/arm/boot/dts/rv1106-pinctrl.dtsi
 */


#include <linux/module.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/kthread.h>
#include <linux/delay.h>

#include <linux/proc_fs.h>
#include <linux/seq_file.h>


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Flavio");
MODULE_DESCRIPTION("MPU6050 I2C kthread");
MODULE_VERSION("1.0");

// --------------------------------------------------------------------
// Parâmetros de módulo
// --------------------------------------------------------------------

static int interval_ms = 500;   // período entre leituras

module_param(interval_ms, int, 0644);
MODULE_PARM_DESC(interval_ms, "Intervalo entre leituras em ms");


//callback

static int16_t last_ax, last_ay, last_az;
static int16_t last_gx, last_gy, last_gz;
static struct proc_dir_entry *mpu6050_proc_entry;

static int mpu6050_proc_show(struct seq_file *m, void *v)
{
    // Aqui você imprime o que quiser no "arquivo"
    seq_printf(m, "ACC: %6d %6d %6d\n", last_ax, last_ay, last_az);
    seq_printf(m, "GYR: %6d %6d %6d\n", last_gx, last_gy, last_gz);
    return 0;
}

static int mpu6050_proc_open(struct inode *inode, struct file *file)
{
    return single_open(file, mpu6050_proc_show, NULL);
}

static const struct proc_ops mpu6050_proc_ops = {
    .proc_open    = mpu6050_proc_open,
    .proc_read    = seq_read,
    .proc_lseek   = seq_lseek,
    .proc_release = single_release,
};

// --------------------------------------------------------------------
// I2C / MPU6050
// --------------------------------------------------------------------

// ATENÇÃO: ajuste conforme o seu hardware (i2c-0 -> 0, i2c-1 -> 1, etc.)
#define MPU6050_I2C_BUS   2      /* por exemplo, i2c-1 */
#define MPU6050_I2C_ADDR  0x68   /* endereço padrão do MPU6050 */

static struct i2c_client *mpu_client;
static struct task_struct *mpu_thread;

// Inicializa registradores do MPU6050:
//  - tira do sleep (0x6B = 0x00)
//  - ACCEL_CONFIG (0x1C = 0x00) -> ±2g
//  - GYRO_CONFIG  (0x1B = 0x00) -> ±250 °/s
static int mpu6050_hw_init(void)
{
    int ret;

    /* 0x6B <- 0x00 : tira do sleep (PWR_MGMT_1) */
    ret = i2c_smbus_write_byte_data(mpu_client, 0x6B, 0x00);
    if (ret < 0) {
        pr_err("mpu6050: falha ao escrever PWR_MGMT_1 (0x6B), ret=%d\n", ret);
        return ret;
    }

    /* 0x1C <- 0x00 : ACCEL_CONFIG = ±2g */
    ret = i2c_smbus_write_byte_data(mpu_client, 0x1C, 0x00);
    if (ret < 0) {
        pr_err("mpu6050: falha ao escrever ACCEL_CONFIG (0x1C), ret=%d\n", ret);
        return ret;
    }

    /* 0x1B <- 0x00 : GYRO_CONFIG = ±250°/s */
    ret = i2c_smbus_write_byte_data(mpu_client, 0x1B, 0x00);
    if (ret < 0) {
        pr_err("mpu6050: falha ao escrever GYRO_CONFIG (0x1B), ret=%d\n", ret);
        return ret;
    }

    pr_info("mpu6050: configurado (sleep off, accel ±2g, gyro ±250dps)\n");
    return 0;
}


// Lê acelerômetro e giroscópio de uma vez (14 bytes a partir de 0x3B)
static int mpu6050_read_accel_gyro(int16_t *ax, int16_t *ay, int16_t *az,
                                   int16_t *gx, int16_t *gy, int16_t *gz)
{
    int ret;
    u8 buf[14];

    if (!mpu_client)
        return -ENODEV;

    /*
     * Layout dos 14 bytes a partir de 0x3B:
     *  0: ACCEL_XOUT_H
     *  1: ACCEL_XOUT_L
     *  2: ACCEL_YOUT_H
     *  3: ACCEL_YOUT_L
     *  4: ACCEL_ZOUT_H
     *  5: ACCEL_ZOUT_L
     *  6: TEMP_OUT_H
     *  7: TEMP_OUT_L
     *  8: GYRO_XOUT_H
     *  9: GYRO_XOUT_L
     * 10: GYRO_YOUT_H
     * 11: GYRO_YOUT_L
     * 12: GYRO_ZOUT_H
     * 13: GYRO_ZOUT_L
     */
    ret = i2c_smbus_read_i2c_block_data(mpu_client, 0x3B,
                                        sizeof(buf), buf);
    if (ret < 0) {
        pr_err("mpu6050: falha ao ler accel/gyro, ret=%d\n", ret);
        return ret;
    }

    *ax = (int16_t)((buf[0] << 8) | buf[1]);
    *ay = (int16_t)((buf[2] << 8) | buf[3]);
    *az = (int16_t)((buf[4] << 8) | buf[5]);

    *gx = (int16_t)((buf[8]  << 8) | buf[9]);
    *gy = (int16_t)((buf[10] << 8) | buf[11]);
    *gz = (int16_t)((buf[12] << 8) | buf[13]);

    return 0;
}

// --------------------------------------------------------------------
// Thread de leitura
// --------------------------------------------------------------------



static int mpu6050_thread_fn(void *data){
    int16_t ax, ay, az, gx, gy, gz;
    int ret;
    int local_interval;

    pr_info("mpu6050_thread: iniciado (interval_ms=%d)\n", interval_ms);

    while (!kthread_should_stop()) {

        if (!mpu_client) {
            pr_err("mpu6050_thread: mpu_client NULL\n");
            break;
        }

        ret = mpu6050_read_accel_gyro(&ax, &ay, &az, &gx, &gy, &gz);
        if (ret < 0) {
            pr_err("mpu6050_thread: erro na leitura: %d\n", ret);
            // aqui você decide: break, continue, ou um retry simples
            continue;
        }

        // Log opcional
        //pr_info("mpu6050: ACC [%6d %6d %6d]  GYR [%6d %6d %6d]\n",ax, ay, az, gx, gy, gz);

        // Atualiza valores para o /proc
        last_ax = ax;
        last_ay = ay;
        last_az = az;
        last_gx = gx;
        last_gy = gy;
        last_gz = gz;

        // Evita intervalo zero ou negativo
        local_interval = (interval_ms > 0) ? interval_ms : 100;

        if (kthread_should_stop())
            break;

        msleep(local_interval);
    }

    pr_info("mpu6050_thread: finalizado\n");
    return 0;
}




// --------------------------------------------------------------------
// Init / Exit do módulo
// --------------------------------------------------------------------

static int __init mpu6050_module_init(void)
{
    int ret;
    struct i2c_adapter *adap;

    pr_info("mpu6050_module: init (interval_ms=%d)\n", interval_ms);

    // Pega adapter I2C
    adap = i2c_get_adapter(MPU6050_I2C_BUS);
    if (!adap) {
        pr_err("mpu6050: não conseguiu obter adapter I2C %d\n",
               MPU6050_I2C_BUS);
        return -ENODEV;
    }

    // Cria cliente "dummy" para o MPU6050
    mpu_client = i2c_new_dummy_device(adap, MPU6050_I2C_ADDR);
    i2c_put_adapter(adap);

    if (IS_ERR(mpu_client)) {
        ret = PTR_ERR(mpu_client);
        pr_err("mpu6050: erro ao criar dummy device (addr=0x%02x), ret=%d\n",
               MPU6050_I2C_ADDR, ret);
        mpu_client = NULL;
        return ret;
    }

    // Configura o MPU6050 (equivalente aos i2cset)
    ret = mpu6050_hw_init();
    if (ret < 0) {
        pr_err("mpu6050: falha na inicialização de hardware\n");
        i2c_unregister_device(mpu_client);
        mpu_client = NULL;
        return ret;
    }

    // Cria thread de leitura
    mpu_thread = kthread_run(mpu6050_thread_fn, NULL, "mpu6050_thread");
    if (IS_ERR(mpu_thread)) {
        ret = PTR_ERR(mpu_thread);
        pr_err("mpu6050: falha ao criar thread, ret=%d\n", ret);
        i2c_unregister_device(mpu_client);
        mpu_client = NULL;
        return ret;
    }


   // Cria entrada em /proc
    mpu6050_proc_entry = proc_create("mpu6050", 0444, NULL, &mpu6050_proc_ops);
    if (!mpu6050_proc_entry) {
        pr_err("mpu6050: falha ao criar entrada /proc/mpu6050\n");
        // se quiser fazer tudo certinho, desfaz o que já foi feito:
        kthread_stop(mpu_thread);
        mpu_thread = NULL;
        i2c_unregister_device(mpu_client);
        mpu_client = NULL;
        return -ENOMEM;
    }


    pr_info("mpu6050_module: módulo carregado com sucesso\n");
    return 0;
}

static void __exit mpu6050_module_exit(void)
{
    pr_info("mpu6050_module: exit\n");

  // Remove /proc
    if (mpu6050_proc_entry) {
        proc_remove(mpu6050_proc_entry);
        mpu6050_proc_entry = NULL;
    }

    // Para thread
    if (mpu_thread) {
        kthread_stop(mpu_thread);
        mpu_thread = NULL;
    }

    // Libera dispositivo I2C
    if (mpu_client) {
        i2c_unregister_device(mpu_client);
        mpu_client = NULL;
    }
}

module_init(mpu6050_module_init);
module_exit(mpu6050_module_exit);
