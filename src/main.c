/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/gpio.h>


//本例程使用nrf52dk为开发环境，dts文件为nrf52dk_nrf52832.dts
//下面宏定义定义位置分别是 devicetree.h及gpio.h，可以在这里进行查询，注意这里gpio.h不是drivers下面的gpio.h
//devicetree.h 路径：ncs\zephyr\include\devicetree.h
//gpio.h 路径：ncs\zephyr\include\devicetree\gpio.h
//也可以参考下面链接
//https://blog.csdn.net/weixin_36224284/article/details/112004790
//https://blog.csdn.net/weixin_36224284/article/details/111918679

#define CONFIG_IO_CONTROL_OUTPUT 1   //控制测试条件，定义以后，进行output测试，否则进行input测试

#define SLEEP_TIME_MS   500 //休眠时间500ms

#ifdef CONFIG_IO_CONTROL_OUTPUT 
//IO control output test
#define LED0_NODE DT_ALIAS(led0)  				 //此处获取 led0 对应的node id
#define LED0	DT_GPIO_LABEL(LED0_NODE, gpios)  //用于设备驱动模型的绑定，获取led0 对应的gpios 属性的第一个值的label，本例中为"Green LED 0"
#define PIN	DT_GPIO_PIN(LED0_NODE, gpios)		 //获取led0 对应的gpios 属性的第一个值的pin引脚，本例中为 17
#define FLAGS	DT_GPIO_FLAGS(LED0_NODE, gpios)  //获取led0 对应的gpios 属性的第一个值的flag，本例中为 GPIO_ACTIVE_LOW

void io_control_output()
{
	printk("IO control output test\r\n");
	printk("LED0 will blinky\r\n");
	const struct device *dev_led0;
	int ret;

	//绑定led0到dev_led0上面,device_get_binding传入的参数为对应外设的 label属性值，本例中为52dk的led0，label属性为 "Green LED 0";
	dev_led0 = device_get_binding(LED0);
	if (dev_led0 == NULL) {
		return;
	}
	//配置led0对应的gpio及flag，此处配置P0.17为OUTPUT并且状态为ACTIVE。 
	//注意，由于前面获取到的FLAGS为GPIO_ACTIVE_LOW，因此设置为ACTIVE时，实际的输出信号时低电平，而设置为INACTIVE时，输出信号为高电平
	//当FLAGS为GPIO_ACTIVE_HIGH时，设置为ACTIVE，实际输出信号为高电平，设置为INACTIVE，输出信号为低电平
	ret = gpio_pin_configure(dev_led0, PIN, GPIO_OUTPUT_ACTIVE | FLAGS);
	if (ret < 0) {
		return;
	}

	while (1) {
		//将P0.17设置为0，PIN也可以直接写17
		gpio_pin_set(dev_led0, PIN, 0);
		k_msleep(SLEEP_TIME_MS);
		//将P0.17设置为1，PIN也可以直接写17
		gpio_pin_set(dev_led0, PIN, 1);
		k_msleep(SLEEP_TIME_MS);
	}
}
#else 
//IO control intput test
#define SW0_NODE	DT_ALIAS(sw0)							 	//此处获取 sw0 对应的node id
#define SW0_GPIO_LABEL	DT_GPIO_LABEL(SW0_NODE, gpios)		 	//用于设备驱动模型的绑定，获取 sw0 对应的gpios 属性的第一个值的label，本例中为"Push button switch 0"
#define SW0_GPIO_PIN	DT_GPIO_PIN(SW0_NODE, gpios)		 	//获取 sw0 对应的gpios 属性的第一个值的pin引脚，本例中为 13
#define SW0_GPIO_FLAGS	DT_GPIO_FLAGS(SW0_NODE, gpios)			//获取 sw0 对应的gpios 属性的第一个值的flag，本例中为 (GPIO_PULL_UP | GPIO_ACTIVE_LOW)

static struct gpio_callback button_cb_data;  					//button回调函数，用于绑定中断事件

//回调函数具体实现
void button_pressed(const struct device *dev, struct gpio_callback *cb,
		    uint32_t pins)
{
	printk("Button pressed at %" PRIu32 "\n", k_cycle_get_32());
}

void io_control_input()
{
	printk("IO control intput test\r\n");
	printk("Press button0 will triger interrupt");
	const struct device *dev_button;
	int ret;
	
	//绑定 sw0 到dev_button上面,device_get_binding传入的参数为对应外设的 label属性值，本例中为52dk的 sw0，label属性为 "Push button switch 0";
	dev_button = device_get_binding(SW0_GPIO_LABEL);
	if (dev_button == NULL) {
		printk("Error: didn't find %s device\n", SW0_GPIO_LABEL);
		return;
	}
	//配置SW0_GPIO_PIN脚为输入状态，并且默认上拉，active状态为low
	ret = gpio_pin_configure(dev_button, SW0_GPIO_PIN, (GPIO_INPUT | SW0_GPIO_FLAGS));
	if (ret != 0) {
		printk("Error %d: failed to configure %s pin %d\n",
		       ret, SW0_GPIO_LABEL, SW0_GPIO_PIN);
		return;
	}
	//配置SW0_GPIO_PIN引脚中断触发条件为引脚进入 active状态，也就是GPIO_ACTIVE_LOW
	ret = gpio_pin_interrupt_configure(dev_button,
					   SW0_GPIO_PIN,
					   GPIO_INT_EDGE_TO_ACTIVE);
	if (ret != 0) {
		printk("Error %d: failed to configure interrupt on %s pin %d\n",
			ret, SW0_GPIO_LABEL, SW0_GPIO_PIN);
		return;
	}
	//将 button_pressed函数绑定到SW0_GPIO_PIN的中断回调函数中，触发中断时，调用button_pressed函数
	// 注意这里用了 BIT(SW0_GPIO_PIN)，因为可能会存在多个gpio的中断，因此按位进行绑定，否则会影响其他位的IO中断配置
	gpio_init_callback(&button_cb_data, button_pressed, BIT(SW0_GPIO_PIN));
	gpio_add_callback(dev_button, &button_cb_data);
	while (1) {
		k_msleep(SLEEP_TIME_MS);
	}
}
#endif
void main(void)
{
	#ifdef CONFIG_IO_CONTROL_OUTPUT 
	io_control_output();
	#else
	io_control_input();
	#endif
}
