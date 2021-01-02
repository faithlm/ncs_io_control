# ncs_io_control
ncs简单的IO控制demo，演示控制IO口为输出状态进行LED闪烁和配置为输入状态检测按键
demo以nrf52dk_nrf52832板子为目标设备

#### 使用要求
需要安装nrf-connect-sdk开发环境，并能正常进行编译下载

#### 编译方法：
进入`io_control`路径，打开命令行，输入
`west build -b nrf52dk_nrf52832 -p`
进行编译，编译完成后，输入
`west flash`
进行下载，即可看到现象。

#### 演示说明：
通过main.c里面的`CONFIG_IO_CONTROL_OUTPUT`进行控制，定义该宏定义时，demo为led闪烁功能
取消该宏定义时，demo功能为按键检测功能
