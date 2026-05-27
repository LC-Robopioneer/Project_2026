#ifndef __TSK_CONFIG_AND_CALLBACK_H
#define __TSK_CONFIG_AND_CALLBACK_H

#include "stm32f1xx_hal.h"

#ifdef __cplusplus
extern "C"
{
#endif
    void Task_Init(void);
    void Task_Loop(void);

    void TIM_1ms_PeriodElapsedCallback();

    void SPI_Callback(uint8_t *tx_data, uint8_t *rx_data, uint16_t size);
#ifdef __cplusplus
}
#endif
#endif