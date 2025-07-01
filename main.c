/* main.c — ????????????? ISL29501 ? ?????? ?????? ???? ????????? */

#include "stm32f10x_conf.h"
#include "stm32f10x_i2c.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include <stdio.h>
#include "isl29501.h"

// ????????? ???????, ????? ??? assert_failed.c ? isl29501.c
void delay_ms(uint32_t ms);
void USART2_SendChar(char c);
void USART2_SendString(const char *s);
void init_GPIO_UART_I2C(void);

int main(void) {
    uint8_t id, en, st;
    char buf[64];

    // 1) ????????? GPIO/UART/I2C
    init_GPIO_UART_I2C();

    // 2) ???? ??????????????? ??????
    delay_ms(50);
    USART2_SendString("ISL29501 Read Test\r\n");

    // 3) Soft-reset ? ????????? ??????? (CEN=LOW ?????? ISL29501_Init)
    ISL29501_Init();

    // 4) ?????? ???? ?????? ? ?????? ?????????
    while (1) {
        id = ISL29501_Read8(ISL29501_REG_ID);
        en = ISL29501_Read8(ISL29501_REG_ENABLE);
        st = ISL29501_Read8(ISL29501_REG_STATUS);

        sprintf(buf,
                "ID=0x%02X, ENABLE=0x%02X, STATUS=0x%02X\r\n",
                id, en, st);
        USART2_SendString(buf);

        delay_ms(500);
    }
}

/* Simple software delay (~1 ms at 72 MHz) */
void delay_ms(uint32_t ms) {
    for (uint32_t i = 0; i < ms * 8000; i++) {
        __NOP();
    }
}

/* Send a single character over USART2 */
void USART2_SendChar(char c) {
    while (!(USART2->SR & USART_SR_TXE));
    USART2->DR = c;
}

/* Send a null-terminated string over USART2 */
void USART2_SendString(const char *s) {
    while (*s) {
        USART2_SendChar(*s++);
    }
}

/* Configure GPIO pins:
   PA2=TX, PA3=RX for UART2;
   PB10=SCL, PB11=SDA for I2C2;
   PA0=CEN (output), PA4=SS (output, unused) */
void init_GPIO_UART_I2C(void) {
    GPIO_InitTypeDef  gpio;
    USART_InitTypeDef usart;
    I2C_InitTypeDef   i2c;

    /* Enable clocks */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA |
                           RCC_APB2Periph_GPIOB |
                           RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2 |
                           RCC_APB1Periph_USART2, ENABLE);

    /* UART2 TX (PA2) */
    gpio.GPIO_Pin   = GPIO_Pin_2;
    gpio.GPIO_Mode  = GPIO_Mode_AF_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpio);

    /* UART2 RX (PA3) */
    gpio.GPIO_Pin  = GPIO_Pin_3;
    gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &gpio);

    /* USART2 config */
    usart.USART_BaudRate            = 115200;
    usart.USART_WordLength          = USART_WordLength_8b;
    usart.USART_StopBits            = USART_StopBits_1;
    usart.USART_Parity              = USART_Parity_No;
    usart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    usart.USART_Mode                = USART_Mode_Tx;
    USART_Init(USART2, &usart);
    USART_Cmd(USART2, ENABLE);

    /* I2C2 SCL/SDA (PB10/PB11) */
    gpio.GPIO_Pin   = GPIO_Pin_10 | GPIO_Pin_11;
    gpio.GPIO_Mode  = GPIO_Mode_AF_OD;
    GPIO_Init(GPIOB, &gpio);

    i2c.I2C_ClockSpeed            = 100000;
    i2c.I2C_Mode                  = I2C_Mode_I2C;
    i2c.I2C_DutyCycle             = I2C_DutyCycle_2;
    i2c.I2C_OwnAddress1           = 0x00;
    i2c.I2C_Ack                   = I2C_Ack_Enable;
    i2c.I2C_AcknowledgedAddress  = I2C_AcknowledgedAddress_7bit;
    I2C_Init(I2C2, &i2c);
    I2C_Cmd(I2C2, ENABLE);

    /* PA0 = CEN, PA4 = SS */
    gpio.GPIO_Pin   = GPIO_Pin_0 | GPIO_Pin_4;
    gpio.GPIO_Mode  = GPIO_Mode_Out_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpio);

    /* Drive CEN low to enable the sensor, SS high (unused) */
    GPIO_ResetBits(GPIOA, GPIO_Pin_0);
    GPIO_SetBits  (GPIOA, GPIO_Pin_4);
}
