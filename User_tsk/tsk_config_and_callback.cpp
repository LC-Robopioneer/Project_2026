#include "tsk_config_and_callback.h"
#include "drv_tim.h"
#include "dvc_MT6816.h"
#include "dvc_SimuESC.h"


void Task_Init(void)
{
    TIM_Init(&htim1, TIM_1ms_PeriodElapsedCallback);
    // 初始化片选引脚为高电平（不选中）
    SPI_Init(&hspi1, SPI_Callback); // 这里可以传入回调函数指针，如果需要使用DMA或中断
    CAN_Init(&hcan,nullptr);
    DWT_Init();
    
    SimuESC.Init();

    HAL_TIM_Base_Start_IT(&htim1);
}

void Task_Loop(void)
{
}

void TIM_1ms_PeriodElapsedCallback()
{
    DWT_Update();
    SimuESC.TIM_Communication_PeriodElapsedCallback();
    SimuESC.FSM_SimuESC.SimuESC_TIM_Status_PeriodElapsedCallback();
}

void SPI_Callback(uint8_t *tx_data, uint8_t *rx_data, uint16_t size)
{
    //MT6816.status = Class_MT6816::IDLE; // 传输完成，设置状态为IDLE
}