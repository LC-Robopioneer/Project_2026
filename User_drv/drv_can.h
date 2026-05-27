#ifndef DRV_CAN_H
#define DRV_CAN_H

#include "stm32f1xx_hal.h"
#include "can.h"
/**
 * @brief CAN接收的信息结构体
 *
 */
struct Struct_CAN_Rx_Buffer
{
    CAN_RxHeaderTypeDef Header;
    uint8_t Data[8];
};

/**
 * @brief CAN通信接收回调函数数据类型
 *
 */
typedef void (*CAN_Call_Back)(Struct_CAN_Rx_Buffer *);

/**
 * @brief CAN通信处理结构体
 *
 */
struct Struct_CAN_Manage_Object
{
    CAN_HandleTypeDef *CAN_Handler;
    Struct_CAN_Rx_Buffer Rx_Buffer;
    CAN_Call_Back Callback_Function;
};

extern Struct_CAN_Manage_Object CAN1_Manage_Object;

// extern uint8_t CAN1_Tx_Data[]; //暂时废除

void CAN_Init(CAN_HandleTypeDef *hcan, CAN_Call_Back Callback_Function);
uint8_t CAN_Send_Data(CAN_HandleTypeDef *hcan, uint16_t ID, uint8_t *Data, uint16_t Length);

#endif
