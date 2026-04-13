#ifndef __TSK_CONFIG_AND_CALLBACK_H
#define __TSK_CONFIG_AND_CALLBACK_H

#ifdef __cplusplus
extern "C"
{
#endif
    void Task_Init(void);
    void Task_Loop(void);

    void TIM_1ms_PeriodElapsedCallback();
#ifdef __cplusplus
}
#endif
#endif