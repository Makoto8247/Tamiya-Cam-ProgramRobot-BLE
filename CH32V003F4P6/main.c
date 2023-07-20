/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2022/08/08
 * Description        : Main program body.
*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for 
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
*******************************************************************************/

/*
 *@Note
 Multiprocessor communication mode routine:
 Master:USART1_Tx(PD5)\USART1_Rx(PD6).
 This routine demonstrates that USART1 receives the data sent by CH341 and inverts
 it and sends it (baud rate 115200).

 Hardware connection:PD5 -- Rx
                     PD6 -- Tx

*/

#include "debug.h"


/* Global define */
#define CHARA       "207023dc24964f3d9f71caa1a912b11b"
#define SUR_COMMAND "SUR," CHARA "\r\n"
// BLE GPIOD
#define BLE_TX      GPIO_Pin_5
#define BLE_RX      GPIO_Pin_6
#define BLE_SIZE    4
// MOTOR GPIOC
#define MOTOR_STBY  GPIO_Pin_7
#define MOTOR_A_ENB GPIO_Pin_6
#define MOTOR_A_PHA GPIO_Pin_5
#define MOTOR_B_ENB GPIO_Pin_4
#define MOTOR_B_PHA GPIO_Pin_3


/* Global Variable */
uint8_t Rn4020_Buffer[BLE_SIZE];

/*********************************************************************
 * @fn      USARTx_CFG
 *
 * @brief   Initializes the USART2 & USART3 peripheral.
 *
 * @return  none
 */
void Motor_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    GPIO_InitStructure.GPIO_Pin =
            GPIO_Pin_0 |
            MOTOR_STBY |
            MOTOR_A_ENB |
            MOTOR_A_PHA |
            MOTOR_B_ENB |
            MOTOR_B_PHA;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
}

void Motor_Stop(void)
{
    GPIO_WriteBit(GPIOC, GPIO_Pin_0, Bit_RESET);
    GPIO_WriteBit(GPIOC, MOTOR_STBY, Bit_RESET);
    GPIO_WriteBit(GPIOC, MOTOR_A_ENB, Bit_RESET);
    GPIO_WriteBit(GPIOC, MOTOR_A_PHA, Bit_RESET);
    GPIO_WriteBit(GPIOC, MOTOR_B_ENB, Bit_RESET);
    GPIO_WriteBit(GPIOC, MOTOR_B_PHA, Bit_RESET);
}

void Motor_Forward(void)
{
    GPIO_WriteBit(GPIOC, GPIO_Pin_0, Bit_SET);
    GPIO_WriteBit(GPIOC, MOTOR_STBY, Bit_SET);
    GPIO_WriteBit(GPIOC, MOTOR_A_ENB, Bit_SET);
    GPIO_WriteBit(GPIOC, MOTOR_A_PHA, Bit_RESET);
    GPIO_WriteBit(GPIOC, MOTOR_B_ENB, Bit_RESET);
    GPIO_WriteBit(GPIOC, MOTOR_B_PHA, Bit_SET);

}

void Motor_Back(void)
{
    GPIO_WriteBit(GPIOC, GPIO_Pin_0, Bit_SET);
    GPIO_WriteBit(GPIOC, MOTOR_STBY, Bit_SET);
    GPIO_WriteBit(GPIOC, MOTOR_A_ENB, Bit_RESET);
    GPIO_WriteBit(GPIOC, MOTOR_A_PHA, Bit_SET);
    GPIO_WriteBit(GPIOC, MOTOR_B_ENB, Bit_SET);
    GPIO_WriteBit(GPIOC, MOTOR_B_PHA, Bit_RESET);
}

void Motor_Right(void)
{
    GPIO_WriteBit(GPIOC, GPIO_Pin_0, Bit_SET);
    GPIO_WriteBit(GPIOC, MOTOR_STBY, Bit_SET);
    GPIO_WriteBit(GPIOC, MOTOR_A_ENB, Bit_RESET);
    GPIO_WriteBit(GPIOC, MOTOR_A_PHA, Bit_SET);
    GPIO_WriteBit(GPIOC, MOTOR_B_ENB, Bit_RESET);
    GPIO_WriteBit(GPIOC, MOTOR_B_PHA, Bit_SET);
}

void Motor_Left(void)
{
    GPIO_WriteBit(GPIOC, GPIO_Pin_0, Bit_SET);
    GPIO_WriteBit(GPIOC, MOTOR_STBY, Bit_SET);
    GPIO_WriteBit(GPIOC, MOTOR_A_ENB, Bit_SET);
    GPIO_WriteBit(GPIOC, MOTOR_A_PHA, Bit_RESET);
    GPIO_WriteBit(GPIOC, MOTOR_B_ENB, Bit_SET);
    GPIO_WriteBit(GPIOC, MOTOR_B_PHA, Bit_RESET);
}

void Usart_Ble_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    USART_InitTypeDef USART_InitStructure = {0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB2Periph_USART1, ENABLE);
    /* USART1 TX-->A.1   RX-->A.2 */
    // TX
    GPIO_InitStructure.GPIO_Pin = BLE_TX;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    // RX
    GPIO_InitStructure.GPIO_Pin = BLE_RX;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    // USART setting
    USART_InitStructure.USART_BaudRate = 2400;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;

    USART_Init(USART1, &USART_InitStructure);
    USART_Cmd(USART1, ENABLE);
}

void USARTx_SendStr(USART_TypeDef* pUSARTx, char* str)
{
    uint8_t i = 0;
    do
    {
        USART_SendData(pUSARTx, *(str+i));
        while(USART_GetFlagStatus(pUSARTx, USART_FLAG_TXE) == RESET);
        i++;
    }while(*(str+i) != '\0');
    while(USART_GetFlagStatus(pUSARTx, USART_FLAG_TC) == RESET);
}


/*********************************************************************
 * @fn      main
 *
 * @brief   Main program.
 *
 * @return  none
 */
int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    Delay_Init();
    Usart_Ble_Init();
    Motor_Init();
    Motor_Stop();

    // RN4020 setup delay
    Delay_Ms(2000);

    while(1)
    {
        USARTx_SendStr(USART1, SUR_COMMAND);
        /* RN4020 receive */
        int i = 0;
        do
        {
            if ((USART_GetFlagStatus(USART1, USART_FLAG_RXNE) != RESET) && (i < BLE_SIZE))
            {
                Rn4020_Buffer[i++] = USART_ReceiveData(USART1);
            }
        }while((Rn4020_Buffer[0] == 0x39) && (i != BLE_SIZE) && (Rn4020_Buffer[i-1] != '\r'));

        if(Rn4020_Buffer[0] == 0x39)
        {
            /* Motor */
            if(Rn4020_Buffer[2] == 0x31) Motor_Stop();         // 1
            else if(Rn4020_Buffer[2] == 0x32) Motor_Forward(); // 2
            else if(Rn4020_Buffer[2] == 0x33) Motor_Right();   // 3
            else if(Rn4020_Buffer[2] == 0x34) Motor_Left();    // 4
            else if(Rn4020_Buffer[2] == 0x35) Motor_Back();    // 5
        }
        Rn4020_Buffer[0] = 0;
        Delay_Ms(100);

    }
}
