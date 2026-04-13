#include "tsk_config_and_callback.h"
#include "dvc_SimuESC.h"
#include "drv_tim.h"

void Task_Init(void)
{
    TIM_Init(&htim1, TIM_1ms_PeriodElapsedCallback);

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
    // SimuESC.TIM_Communication_PeriodElapsedCallback();
    SimuESC.FSM_SimuESC.SimuESC_TIM_Status_PeriodElapsedCallback();
}
