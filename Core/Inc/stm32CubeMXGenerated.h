#pragma once

#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

extern I2C_HandleTypeDef hi2c1;
extern UART_HandleTypeDef huart1;
extern TIM_HandleTypeDef htim3;

// CUBE GENERATED
/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_USART1_UART_Init(void);
void MX_I2C1_Init(void);
void MX_TIM3_Init(void);

//END - CUBE GENERATED

#ifdef __cplusplus
}
#endif
