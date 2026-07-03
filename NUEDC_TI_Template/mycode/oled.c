#include "oled.h"
#include "oledfont.h"

// 假设在 SysConfig 中配置的 I2C 模块为 myI2C0，基地址为 I2CA_BASE
#define OLED_I2C_BASE   I2CA_BASE
#define OLED_SLAVE_ADDR 0x3C // 7位地址 (0x78 >> 1)

/**********************************************
// IIC 发送数据序列：[Start] -> [Addr+W] -> [Control Byte] -> [Data] -> [Stop]
**********************************************/
void I2C_Write_Byte(uint8_t control_byte, uint8_t data)
{
    // 1. 设置从机地址
    I2C_setSlaveAddress(OLED_I2C_BASE, OLED_SLAVE_ADDR);

    // 2. 设置要发送的数据量：1字节控制位 + 1字节数据
    I2C_setDataCount(OLED_I2C_BASE, 2);

    // 3. 发送模式设置：起始位 + 停止位 + 主机发送
    I2C_setConfig(OLED_I2C_BASE, I2C_MASTER_SEND_MODE);
    I2C_sendStartCondition(OLED_I2C_BASE);
    I2C_sendStopCondition(OLED_I2C_BASE);

    // 4. 发送控制字节 (0x00命令或0x40数据)
    I2C_putData(OLED_I2C_BASE, control_byte);

    // 5. 发送实际数据
    I2C_putData(OLED_I2C_BASE, data);

    // 6. 等待发送完成 (实际应用中建议使用 FIFO 检查或非阻塞方式)
    while (I2C_isBusBusy(OLED_I2C_BASE))
        ;
}

void Write_IIC_Command(unsigned char i2c_cmd)
{
    I2C_Write_Byte(0x00, i2c_cmd);
}

void Write_IIC_Data(unsigned char i2c_data)
{
    I2C_Write_Byte(0x40, i2c_data);
}

void OLED_WR_Byte(unsigned char dat, unsigned char cmd)
{
    if (cmd)
        Write_IIC_Data(dat);
    else
        Write_IIC_Command(dat);
}

/****************核心显示函数移植 (逻辑与原版基本一致)****************/

void OLED_Set_Pos(unsigned char x, unsigned char y)
{
    OLED_WR_Byte(0xb0 + y, OLED_CMD);
    OLED_WR_Byte(((x & 0xf0) >> 4) | 0x10, OLED_CMD);
    OLED_WR_Byte((x & 0x0f), OLED_CMD);
}

void OLED_Display_On(void)
{
    OLED_WR_Byte(0X8D, OLED_CMD);
    OLED_WR_Byte(0X14, OLED_CMD);
    OLED_WR_Byte(0XAF, OLED_CMD);
}

void OLED_Display_Off(void)
{
    OLED_WR_Byte(0X8D, OLED_CMD);
    OLED_WR_Byte(0X10, OLED_CMD);
    OLED_WR_Byte(0XAE, OLED_CMD);
}

void OLED_Clear(void)
{
    uint8_t i, n;
    for (i = 0; i < 8; i++)
    {
        OLED_WR_Byte(0xb0 + i, OLED_CMD);
        OLED_WR_Byte(0x00, OLED_CMD);
        OLED_WR_Byte(0x10, OLED_CMD);
        for (n = 0; n < 128; n++)
            OLED_WR_Byte(0, OLED_DATA);
    }
}

void OLED_ShowChar(uint8_t x, uint8_t y, uint8_t chr, uint8_t Char_Size)
{
    unsigned char c = 0, i = 0;
    c = chr - ' ';
    if (x > 128 - 1)
    {
        x = 0;
        y = y + 2;
    }
    if (Char_Size == 16)
    {
        OLED_Set_Pos(x, y);
        for (i = 0; i < 8; i++)
            OLED_WR_Byte(F8X16[c * 16 + i], OLED_DATA);
        OLED_Set_Pos(x, y + 1);
        for (i = 0; i < 8; i++)
            OLED_WR_Byte(F8X16[c * 16 + i + 8], OLED_DATA);
    }
    else
    {
        OLED_Set_Pos(x, y);
        for (i = 0; i < 6; i++)
            OLED_WR_Byte(F6x8[c][i], OLED_DATA);
    }
}

void OLED_ShowString(uint8_t x, uint8_t y, uint8_t *chr, uint8_t Char_Size)
{
    unsigned char j = 0;
    while (chr[j] != '\0')
    {
        OLED_ShowChar(x, y, chr[j], Char_Size);
        x += 8;
        if (x > 120)
        {
            x = 0;
            y += 2;
        }
        j++;
    }
}

// ... OLED_ShowNum, OLED_ShowCHinese, OLED_DrawBMP 逻辑同理，只需确保调用 OLED_WR_Byte 即可 ...

void OLED_Init(void)
{
    // TI 平台上建议在初始化前给一点延时确保 OLED 供电稳定
    DEVICE_DELAY_US(100000);

    OLED_WR_Byte(0xAE, OLED_CMD); // Display Off
    OLED_WR_Byte(0x00, OLED_CMD); // Set Low Column Address
    OLED_WR_Byte(0x10, OLED_CMD); // Set High Column Address
    OLED_WR_Byte(0x40, OLED_CMD); // Set Start Line Address
    OLED_WR_Byte(0xB0, OLED_CMD); // Set Page Address
    OLED_WR_Byte(0x81, OLED_CMD); // Contract Control
    OLED_WR_Byte(0xFF, OLED_CMD);
    OLED_WR_Byte(0xA1, OLED_CMD); // Set Segment Remap
    OLED_WR_Byte(0xA6, OLED_CMD); // Normal / Reverse
    OLED_WR_Byte(0xA8, OLED_CMD); // Set Multiplex Ratio
    OLED_WR_Byte(0x3F, OLED_CMD);
    OLED_WR_Byte(0xC8, OLED_CMD); // Com Scan Direction
    OLED_WR_Byte(0xD3, OLED_CMD); // Set Display Offset
    OLED_WR_Byte(0x00, OLED_CMD);
    OLED_WR_Byte(0xD5, OLED_CMD); // Set Osc Division
    OLED_WR_Byte(0x80, OLED_CMD);
    OLED_WR_Byte(0xD9, OLED_CMD); // Set Pre-Charge Period
    OLED_WR_Byte(0xF1, OLED_CMD);
    OLED_WR_Byte(0xDA, OLED_CMD); // Set Com Pin Config
    OLED_WR_Byte(0x12, OLED_CMD);
    OLED_WR_Byte(0xDB, OLED_CMD); // Set Vcomh
    OLED_WR_Byte(0x30, OLED_CMD);
    OLED_WR_Byte(0x8D, OLED_CMD); // Set Charge Pump Enable
    OLED_WR_Byte(0x14, OLED_CMD);
    OLED_WR_Byte(0xAF, OLED_CMD); // Display On

    OLED_Clear();
}
