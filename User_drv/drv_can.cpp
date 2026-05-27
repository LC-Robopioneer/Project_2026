#include "drv_can.h"
#include "main.h"

// 滤波器编号
#define CAN_FILTER(x) ((x) << 3)

// 接收队列
#define CAN_FIFO_0 (0 << 2)
#define CAN_FIFO_1 (1 << 2)

// 标准帧或扩展帧
#define CAN_STDID (0 << 1)
#define CAN_EXTID (1 << 1)

// 数据帧或遥控帧
#define CAN_DATA_TYPE (0 << 0)
#define CAN_REMOTE_TYPE (1 << 0)

Struct_CAN_Manage_Object CAN1_Manage_Object = {0};

// CAN通信发送缓冲区
// uint8_t CAN1_Tx_Data[8]; //暂时废除

/**
 * @brief 配置CAN的过滤器
 *
 * @param hcan CAN编号
 * @param Object_Para 编号 | FIFOx | ID类型 | 帧类型
 * @param ID ID
 * @param Mask_ID 屏蔽位(0x3ff, 0x1fffffff)
 */
void can_filter_mask_config(CAN_HandleTypeDef *hcan, uint8_t Object_Para, uint32_t ID, uint32_t Mask_ID)
{
  // 检测传参是否正确
  assert_param(hcan != NULL);

  // CAN过滤器初始化结构体
  CAN_FilterTypeDef can_filter_init_structure;
  // 滤波器序号, 0-27, 共28个滤波器
  can_filter_init_structure.FilterBank = Object_Para >> 3;
  // 滤波器模式，设置ID掩码模式
  can_filter_init_structure.FilterMode = CAN_FILTERMODE_IDMASK;

  if ((Object_Para & 0x02))
  {
    // 29位 拓展帧
    //  32位滤波
    can_filter_init_structure.FilterScale = CAN_FILTERSCALE_32BIT;
    // 验证码 高16bit
    can_filter_init_structure.FilterIdHigh = (ID << 3) >> 16;
    // 验证码 低16bit
    can_filter_init_structure.FilterIdLow = ID << 3 | (Object_Para & 0x03) << 1;
    // 屏蔽码 高16bit
    can_filter_init_structure.FilterMaskIdHigh = (Mask_ID << 3) >> 16;
    // 屏蔽码 低16bit
    can_filter_init_structure.FilterMaskIdLow = Mask_ID << 3 | (0x03) << 1;
  }
  else
  {
    // 11位 标准帧
    //  32位滤波
    can_filter_init_structure.FilterScale = CAN_FILTERSCALE_16BIT;
    // 标准帧验证码 高16bit不启用
    can_filter_init_structure.FilterIdHigh = 0x0000;
    // 验证码 低16bit
    can_filter_init_structure.FilterIdLow = ID << 5 | (Object_Para & 0x02) << 4;
    // 标准帧屏蔽码 高16bit不启用
    can_filter_init_structure.FilterMaskIdHigh = 0x0000;
    // 屏蔽码 低16bit
    can_filter_init_structure.FilterMaskIdLow = (Mask_ID << 5) | 0x01 << 4;
  }

  // 滤波器绑定FIFO0或FIFO1
  can_filter_init_structure.FilterFIFOAssignment = (Object_Para >> 2) & 0x01;
  // 从机模式选择开始单元 , 前14个在CAN1, 后14个在CAN2
  can_filter_init_structure.SlaveStartFilterBank = 14;
  // 使能滤波器
  can_filter_init_structure.FilterActivation = ENABLE;

  // 过滤器配置
  if (HAL_CAN_ConfigFilter(hcan, &can_filter_init_structure) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
 * @brief 初始化CAN总线
 *
 * @param hcan CAN编号
 * @param Callback_Function 处理回调函数
 */
void CAN_Init(CAN_HandleTypeDef *hcan, CAN_Call_Back Callback_Function)
{
  if (hcan->Instance == CAN1)
  {
    CAN1_Manage_Object.CAN_Handler = hcan;
    CAN1_Manage_Object.Callback_Function = Callback_Function;
    //         can_filter_mask_config(hcan, CAN_FILTER(0) | CAN_FIFO_0 | CAN_STDID | CAN_DATA_TYPE, 0x200 ,0x7F8);  //只接收0x200-0x207
    //         can_filter_mask_config(hcan, CAN_FILTER(1) | CAN_FIFO_1 | CAN_STDID | CAN_DATA_TYPE, 0x200, 0x7F8);
    can_filter_mask_config(hcan, CAN_FILTER(0) | CAN_FIFO_0 | CAN_STDID | CAN_DATA_TYPE, 0, 0);
    can_filter_mask_config(hcan, CAN_FILTER(1) | CAN_FIFO_1 | CAN_STDID | CAN_DATA_TYPE, 0, 0);
  }
  /*离开初始模式*/
  HAL_CAN_Start(hcan);

  /*开中断*/
  HAL_CAN_ActivateNotification(hcan, CAN_IT_RX_FIFO0_MSG_PENDING); // can 接收fifo 0不为空中断
  HAL_CAN_ActivateNotification(hcan, CAN_IT_RX_FIFO1_MSG_PENDING); // can 接收fifo 1不为空中断
}

/**
 * @brief 发送数据帧
 *
 * @param hcan CAN编号
 * @param ID ID
 * @param Data 被发送的数据指针
 * @param Length 长度
 * @return uint8_t 执行状态
 */
uint8_t CAN_Send_Data(CAN_HandleTypeDef *hcan, uint16_t ID, uint8_t *Data, uint16_t Length)
{
  CAN_TxHeaderTypeDef tx_header;
  uint32_t used_mailbox;

  // 检测传参是否正确
  assert_param(hcan != NULL);

  tx_header.StdId = ID;
  tx_header.ExtId = 0;
  tx_header.IDE = 0;
  tx_header.RTR = 0;
  tx_header.DLC = Length;

  return (HAL_CAN_AddTxMessage(hcan, &tx_header, Data, &used_mailbox));
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
  // 判断程序初始化完成
  // if (init_finished == false)
  // {
  //     return;
  // }

  // 选择回调函数
  if (hcan->Instance == CAN1)
  {
    HAL_CAN_GetRxMessage(hcan, CAN_FILTER_FIFO0, &CAN1_Manage_Object.Rx_Buffer.Header, CAN1_Manage_Object.Rx_Buffer.Data);
    if (CAN1_Manage_Object.Callback_Function != nullptr)
    {
      CAN1_Manage_Object.Callback_Function(&CAN1_Manage_Object.Rx_Buffer);
    }
  }
  /*else if (hcan->Instance == CAN2)
  {
    HAL_CAN_GetRxMessage(hcan, CAN_FILTER_FIFO0, &CAN2_Manage_Object.Rx_Buffer.Header, CAN2_Manage_Object.Rx_Buffer.Data);
    if (CAN2_Manage_Object.Callback_Function != nullptr)
    {
      CAN2_Manage_Object.Callback_Function(&CAN2_Manage_Object.Rx_Buffer);
    }
  }*/
}

/**
 * @brief HAL库CAN接收FIFO1中断
 *
 * @param hcan CAN编号
 */
void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
  // // 判断程序初始化完成
  // if (init_finished == false)
  // {
  //     return;
  // }

  // 选择回调函数
  if (hcan->Instance == CAN1)
  {
    HAL_CAN_GetRxMessage(hcan, CAN_FILTER_FIFO1, &CAN1_Manage_Object.Rx_Buffer.Header, CAN1_Manage_Object.Rx_Buffer.Data);
    if (CAN1_Manage_Object.Callback_Function != nullptr)
    {
      CAN1_Manage_Object.Callback_Function(&CAN1_Manage_Object.Rx_Buffer);
    }
  }
  /*  else if (hcan->Instance == CAN2)
    {
      HAL_CAN_GetRxMessage(hcan, CAN_FILTER_FIFO1, &CAN2_Manage_Object.Rx_Buffer.Header, CAN2_Manage_Object.Rx_Buffer.Data);
      if (CAN2_Manage_Object.Callback_Function != nullptr)
      {
        CAN2_Manage_Object.Callback_Function(&CAN2_Manage_Object.Rx_Buffer);
      }
    }*/
}
