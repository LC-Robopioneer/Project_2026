#ifndef DVC_SIMUESC_H
#define DVC_SIMUESC_H
/*---------------------includes-------------------------*/
#include "dvc_dwt.h"
#include "dvc_MT6816.h"
#include "alg_fsm.h"
#include "drv_can.h"

/*---------------------变量声明-------------------------*/
class Class_SimuESC;
/*---------------------枚举-------------------------*/

enum Enum_SimuESC_Status
{
    SimuESC_NORMAL = 0,
    SimuESC_SETTING,
};

/*---------------------结构体-------------------------*/
#pragma pack(push, 1) // 1字节对齐
struct Data_Package   // 数据包
{
    uint8_t raw_angle_high; // 角度数据高8位
    uint8_t raw_angle_low;  // 角度数据低6位
    uint8_t status;         // 状态寄存器
    uint8_t no_mag_warning; // 弱磁报警标志
    uint8_t over_speed;     // 超速报警标志
    uint8_t parity_error;   // 奇偶校验错误标志
    uint8_t checksum;       // 校验和
    uint8_t reserved;       // 保留字节
};
#pragma pack(pop) // 恢复默认对齐

/*---------------------类定义-------------------------*/
class Class_SimuESC_FSM : public Class_FSM
{
public:
    Class_SimuESC *SimuESC;

    void SimuESC_TIM_Status_PeriodElapsedCallback();

		Enum_SimuESC_Status SimuESC_Status = SimuESC_NORMAL;
    
    SoftTimer_t setting_timeout_timer;

    SoftTimer_t led_timer;

    SoftTimer_t button_debounce_timer;
};

class Class_SimuESC
{
public:
    Class_MT6816 MT6816;
    Class_SimuESC_FSM FSM_SimuESC;

    void Init();
    void TIM_Communication_PeriodElapsedCallback();

    void LED_Toggle();
    void LED_On();
    void LED_Off();

    bool Button_Check();
    void Send_CAN_Message();

    inline uint16_t Get_ID() const { return ID; }
    inline void Set_ID(uint16_t new_id) { ID = new_id; }

    uint16_t LED_Blink_Count = 0;
    uint16_t Button_Click_Count = 0;

    bool button_last_clicked = false;
    bool debounced = false;

protected:
    uint16_t ID = 1;
    constexpr static uint16_t ID_Base = 0x200;

    Data_Package data_package; // 数据包
    // 常量定义区

    GPIO_TypeDef *CS_GPIO_Port = GPIOA;
    uint16_t CS_GPIO_Pin = GPIO_PIN_4;
    SPI_HandleTypeDef *hspi = &hspi1;

    GPIO_TypeDef *LED_GPIO_Port = GPIOA;
    uint16_t LED_GPIO_Pin = GPIO_PIN_3;

    GPIO_TypeDef *Button_GPIO_Port = GPIOA;
    uint16_t Button_GPIO_Pin = GPIO_PIN_2;

    CAN_HandleTypeDef *_hcan = &hcan;
};

extern Class_SimuESC SimuESC;
extern Class_SimuESC_FSM FSM_SimuESC;
#endif