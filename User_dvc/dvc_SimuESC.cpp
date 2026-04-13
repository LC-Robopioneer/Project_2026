#include "dvc_SimuESC.h"

Class_SimuESC SimuESC;
Class_SimuESC_FSM FSM_SimuESC;

static const uint8_t crc8_table[256] = {
    0x00, 0x07, 0x0E, 0x09, 0x1C, 0x1B, 0x12, 0x15,
    0x38, 0x3F, 0x36, 0x31, 0x24, 0x23, 0x2A, 0x2D,
    0x70, 0x77, 0x7E, 0x79, 0x6C, 0x6B, 0x62, 0x65,
    0x48, 0x4F, 0x46, 0x41, 0x54, 0x53, 0x5A, 0x5D,
    0xE0, 0xE7, 0xEE, 0xE9, 0xFC, 0xFB, 0xF2, 0xF5,
    0xD8, 0xDF, 0xD6, 0xD1, 0xC4, 0xC3, 0xCA, 0xCD,
    0x90, 0x97, 0x9E, 0x99, 0x8C, 0x8B, 0x82, 0x85,
    0xA8, 0xAF, 0xA6, 0xA1, 0xB4, 0xB3, 0xBA, 0xBD,
    0xC7, 0xC0, 0xC9, 0xCE, 0xDB, 0xDC, 0xD5, 0xD2,
    0xFF, 0xF8, 0xF1, 0xF6, 0xE3, 0xE4, 0xED, 0xEA,
    0xB7, 0xB0, 0xB9, 0xBE, 0xAB, 0xAC, 0xA5, 0xA2,
    0x8F, 0x88, 0x81, 0x86, 0x93, 0x94, 0x9D, 0x9A,
    0x27, 0x20, 0x29, 0x2E, 0x3B, 0x3C, 0x35, 0x32,
    0x1F, 0x18, 0x11, 0x16, 0x03, 0x04, 0x0D, 0x0A,
    0x57, 0x50, 0x59, 0x5E, 0x4B, 0x4C, 0x45, 0x42,
    0x6F, 0x68, 0x61, 0x66, 0x73, 0x74, 0x7D, 0x7A,
    0x89, 0x8E, 0x87, 0x80, 0x95, 0x92, 0x9B, 0x9C,
    0xB1, 0xB6, 0xBF, 0xB8, 0xAD, 0xAA, 0xA3, 0xA4,
    0xF9, 0xFE, 0xF7, 0xF0, 0xE5, 0xE2, 0xEB, 0xEC,
    0xC1, 0xC6, 0xCF, 0xC8, 0xDD, 0xDA, 0xD3, 0xD4,
    0x69, 0x6E, 0x67, 0x60, 0x75, 0x72, 0x7B, 0x7C,
    0x51, 0x56, 0x5F, 0x58, 0x4D, 0x4A, 0x43, 0x44,
    0x19, 0x1E, 0x17, 0x10, 0x05, 0x02, 0x0B, 0x0C,
    0x21, 0x26, 0x2F, 0x28, 0x3D, 0x3A, 0x33, 0x34,
    0x4E, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5C, 0x5B,
    0x76, 0x71, 0x78, 0x7F, 0x6A, 0x6D, 0x64, 0x63,
    0x3E, 0x39, 0x30, 0x37, 0x22, 0x25, 0x2C, 0x2B,
    0x06, 0x01, 0x08, 0x0F, 0x1A, 0x1D, 0x14, 0x13,
    0xAE, 0xA9, 0xA0, 0xA7, 0xB2, 0xB5, 0xBC, 0xBB,
    0x96, 0x91, 0x98, 0x9F, 0x8A, 0x8D, 0x84, 0x83,
    0xDE, 0xD9, 0xD0, 0xD7, 0xC2, 0xC5, 0xCC, 0xCB,
    0xE6, 0xE1, 0xE8, 0xEF, 0xFA, 0xFD, 0xF4, 0xF3};

void Class_SimuESC_FSM::SimuESC_TIM_Status_PeriodElapsedCallback()
{
    Status[Now_Status_Serial].Count_Time++;

    switch (Now_Status_Serial)
    {
    case SimuESC_NORMAL:
        if (Is_Timer_ExpiredUs(&led_timer, Expire_Loop))
        {
            if (SimuESC->LED_Blink_Count < 2 * SimuESC->Get_ID())
            {
                led_timer.start_time = DWT_GetCurrentTimeUs();
                led_timer.expire_time = 200000; // 200ms间隔
                SimuESC->LED_Toggle();
                SimuESC->LED_Blink_Count++;
            }
            else
            {
                led_timer.start_time = DWT_GetCurrentTimeUs();
                led_timer.expire_time = 1200000; // 1.2s间隔
                SimuESC->LED_Blink_Count = 0;
            }
        }
        if (SimuESC->Button_Check())
        {
            Set_Status(SimuESC_SETTING);     // 切换FSM状态，触发状态初始化
            SimuESC->LED_Blink_Count = 0;    // 切换到SETTING模式时重置LED闪烁计数
            SimuESC->Button_Click_Count = 0; // 重置按钮点击计数

            // 初始化SETTING模式超时计时器（2秒无操作自动退回）
            setting_timeout_timer.start_time = DWT_GetCurrentTimeUs();
            setting_timeout_timer.expire_time = 2000000; // 2秒超时
        }
        break;

    case SimuESC_SETTING:
        // 检查是否2秒无操作超时
        if (Is_Timer_ExpiredUs(&setting_timeout_timer, Expire_Once))
        {
            // 2秒无操作，自动退回normal模式
            Set_Status(SimuESC_NORMAL);
            if (SimuESC->Button_Click_Count > 0)
            {
                SimuESC->Set_ID(SimuESC->Button_Click_Count);
            }
            SimuESC->LED_Off(); // 熄灭LED
            break;
        }

        if (SimuESC->Button_Check())
        {
            // 按钮被按下，重置超时计时器
            setting_timeout_timer.start_time = DWT_GetCurrentTimeUs();
            setting_timeout_timer.expire_time = 2000000; // 2秒超时

            SimuESC->LED_On(); // 按钮点击时点亮LED
        }
        else
        {
            SimuESC->LED_Off(); // 按钮未点击时熄灭LED
        }
        break;

    default:
        break;
    }
}

void Class_SimuESC::Init()
{
    // 初始化MT6816
    MT6816.Init(hspi, CS_GPIO_Port, CS_GPIO_Pin);
    CAN_Init(_hcan, nullptr);
    HAL_GPIO_WritePin(LED_GPIO_Port, LED_GPIO_Pin, GPIO_PIN_RESET); // 初始化LED状态

    FSM_SimuESC.led_timer.start_time = DWT_GetCurrentTimeUs();
    FSM_SimuESC.led_timer.expire_time = 200000;

    // 初始化FSM_SimuESC
    FSM_SimuESC.SimuESC = this;
}

void Class_SimuESC::TIM_Communication_PeriodElapsedCallback()
{
    MT6816.ReadAngle();
    Send_CAN_Message();
}

void Class_SimuESC::LED_Toggle()
{
    HAL_GPIO_TogglePin(LED_GPIO_Port, LED_GPIO_Pin);
}

void Class_SimuESC::LED_On()
{
    HAL_GPIO_WritePin(LED_GPIO_Port, LED_GPIO_Pin, GPIO_PIN_SET);
}

void Class_SimuESC::LED_Off()
{
    HAL_GPIO_WritePin(LED_GPIO_Port, LED_GPIO_Pin, GPIO_PIN_RESET);
}

bool Class_SimuESC::Button_Check()
{
    bool now_clicked = (HAL_GPIO_ReadPin(Button_GPIO_Port, Button_GPIO_Pin) == GPIO_PIN_RESET);
    if (now_clicked ^ button_last_clicked) // 检测到状态改变
    {
        if (now_clicked)
        {

            FSM_SimuESC.button_debounce_timer.start_time = DWT_GetCurrentTimeUs();
            FSM_SimuESC.button_debounce_timer.expire_time = 30000; // 30ms消抖时间
        }
        else
        {
            debounced = false; // 按钮释放，重置消抖状态，准备下一次点击
        }
    }
    else
    {
        if (Is_Timer_ExpiredUs(&FSM_SimuESC.button_debounce_timer, Expire_Once) && now_clicked) // 只有在消抖时间到且按钮仍然被按下时才认为是有效点击
        {
            if (!debounced) // 如果还没有被标记为消抖过，才增加点击计数,长按只算一次点击
            {
                Button_Click_Count = Button_Click_Count % 4 + 1; // 1-4循环计数
                debounced = true;                                // 标记为已消抖
            }
            button_last_clicked = now_clicked; // 更新上次点击状态
            return true;
        }
    }
    button_last_clicked = now_clicked; // 更新上次点击状态
    return false;                      // 默认返回未检测到有效点击
}

void Class_SimuESC::Send_CAN_Message()
{
    data_package.raw_angle_high = (MT6816.Get_AngleDataPtr()->raw_angle >> 8) & 0xFF;
    data_package.raw_angle_low = MT6816.Get_AngleDataPtr()->raw_angle & 0xFF;
    data_package.status = MT6816.Get_AngleDataPtr()->status;
    data_package.no_mag_warning = MT6816.Get_AngleDataPtr()->no_mag_warning;
    data_package.over_speed = MT6816.Get_AngleDataPtr()->over_speed;
    data_package.parity_error = MT6816.Get_AngleDataPtr()->parity_error;
    uint8_t crc = 0;
    for (uint8_t i = 0; i < 6; i++)
    {
        crc = crc8_table[crc ^ ((uint8_t *)&data_package)[i]];
    }
    data_package.checksum = crc;
    data_package.reserved = NULL; // 保留字节设置为0

    CAN_Send_Data(_hcan, ID + ID_Base, (uint8_t *)&data_package, sizeof(data_package));
}
