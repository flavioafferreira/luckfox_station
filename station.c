/*
DEVICE TREE
/home/flavio/sdk/luckfox-pico/sysdrv/source/kernel/arch/arm/boot/dts/rv1106g-evb1-v10.dts
/home/flavio/sdk/luckfox-pico/sysdrv/source/kernel/arch/arm/boot/dts/rv1106-pinctrl.dtsi
*/


#include <linux/module.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/timer.
#include <linux/jiffies.h>

static int in_gpio = -1;
static int out_gpio = -1;
static int interval_ms = 50;

module_param(in_gpio, int, 0444);
module_param(out_gpio, int, 0444);
module_param(interval_ms, int, 0644);

static struct timer_list mirror_timer;

static void mirror_timer_fn(struct timer_list *t)
{
    int v = gpio_get_value(in_gpio);
    gpio_set_value(out_gpio, v);
    mod_timer(&mirror_timer, jiffies + msecs_to_jiffies(interval_ms));
}

static int __init gpio_mirror_init(void)
{
    int ret;

    if (in_gpio < 0 || out_gpio < 0) {
        pr_err("gpio_mirror: insmod gpio_mirror.ko in_gpio=<N> out_gpio=<N> [interval_ms=<N>]\n");
        return -EINVAL;
    }

    if (!gpio_is_valid(in_gpio) || !gpio_is_valid(out_gpio)) {
        pr_err("gpio_mirror: invalid gpio numbers in=%d out=%d\n", in_gpio, out_gpio);
        return -EINVAL;
    }

    ret = gpio_request(in_gpio, "gpio_mirror_in");
    if (ret) { pr_err("gpio_mirror: gpio_request(in=%d) failed: %d\n", in_gpio, ret); return ret; }

    ret = gpio_direction_input(in_gpio);
    if (ret) { pr_err("gpio_mirror: gpio_direction_input(%d) failed: %d\n", in_gpio, ret); gpio_free(in_gpio); return ret; }

    ret = gpio_request(out_gpio, "gpio_mirror_out");
    if (ret) { pr_err("gpio_mirror: gpio_request(out=%d) failed: %d\n", out_gpio, ret); gpio_free(in_gpio); return ret; }

    ret = gpio_direction_output(out_gpio, 0);
    if (ret) { pr_err("gpio_mirror: gpio_direction_output(%d) failed: %d\n", out_gpio, ret); gpio_free(out_gpio); gpio_free(in_gpio); return ret; }

    timer_setup(&mirror_timer, mirror_timer_fn, 0);
    mod_timer(&mirror_timer, jiffies + msecs_to_jiffies(interval_ms));

    pr_info("gpio_mirror: loaded in_gpio=%d out_gpio=%d interval_ms=%d\n", in_gpio, out_gpio, interval_ms);
    return 0;
}

static void __exit gpio_mirror_exit(void)
{
    del_timer_sync(&mirror_timer);
    gpio_set_value(out_gpio, 0);
    gpio_free(out_gpio);
    gpio_free(in_gpio);
    pr_info("gpio_mirror: unloaded\n");
}

module_init(gpio_mirror_init);
module_exit(gpio_mirror_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Mirror one GPIO input to one GPIO output (polling)");

