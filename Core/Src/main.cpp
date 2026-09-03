#include <stdio.h>
#include <cstring>
#include <string_view>
#include <array>

#include "stm32CubeMXGenerated.h"

#include "../../Peripherals/Timer/RegisterLevel/SoftwareTimer.hpp"
#include "../../Peripherals/I2C/HAL/I2C_IT.hpp"
#include "../../Peripherals/UART/HAL/UartIT.hpp"
#include "../../Peripherals/UART/LineParser.hpp"
#include "../../Peripherals/Timer/HAL/Pwm.hpp"
#include "../../Devices/LPS25HB_Async.hpp"
#include "../../Devices/MCP23S08.hpp"

#include "../Inc/peripheralsDefinition.h"

#include "../../Scheduler/Task.hpp"
#include "../../Scheduler/Scheduler.hpp"

Peripherals::HAL::I2C_IT i2c1IT{ hi2c1 };
Device::LPS25HB_Async lps25hbAsync{ i2c1IT };
Peripherals::HAL::Pwm tim3_ch1_pa6{ htim3, TIM_CHANNEL_1 }; //PA6

//TODO add ICM-20948 accelerometer (i2C then SPI)

int main()
{
    /* MCU Configuration--------------------------------------------------------*/
    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();
    SystemClock_Config();
    /* Initialize all configured peripherals */

    MX_I2C1_Init();
    MX_TIM3_Init();
    
    volatile float temperature = 0.0f;
    volatile uint32_t pressure = 0;

    Task<RegisterLevel::SoftwareTimer> readLPS25HBSensorTask{ 50, [&]()
    {
        if (lps25hbAsync.IsAwake())
        {
            if (const auto readTemp = lps25hbAsync.ReadTemperature(); readTemp)
                temperature = readTemp.value();

            if (const auto readPressure = lps25hbAsync.ReadPressure(); readPressure)
                pressure = readPressure.value();
        }
        else
        {
            lps25hbAsync.WakeUp();
        }
    }};

    Task<RegisterLevel::SoftwareTimer> printTemperaturePressureTask{ 500, [&]()
    {
        char messageBuffer[48];
        snprintf(messageBuffer, sizeof(messageBuffer), "Temp: %.2f C, Pressure: %lu hPa\r\n", temperature, pressure);
        uart2.Transmit(reinterpret_cast<const uint8_t*>(messageBuffer), strlen(messageBuffer));
    }};

    Task<RegisterLevel::SoftwareTimer> ld2Task{ 150, [&]()
    {
        ld2.Toggle();
    }};

    Task<RegisterLevel::SoftwareTimer> ledControllerTask{ 10, [&]()
    {
        volatile const auto currentPulse = tim3_ch1_pa6.GetPulse();
        const auto readValue = uart1.Read();

        auto ledControlFunction = [&]()
        {
            static constexpr uint32_t restPulse = 799;
            static constexpr uint32_t restPulseIncreasement = 25;
            if (currentPulse > restPulse)
            {
                const uint32_t calculatedPulse = currentPulse - restPulseIncreasement;
                tim3_ch1_pa6.SetPulse(calculatedPulse < restPulse ? restPulse : calculatedPulse);
            }
            else if (currentPulse < restPulse)
            {
                const uint32_t calculatedPulse = currentPulse + restPulseIncreasement;
                tim3_ch1_pa6.SetPulse(calculatedPulse > restPulse ? restPulse : calculatedPulse);
            }
        };

        // TODO:
        // add turning off option
        // and minimal pulse while turned on (simulation of not dropping too fast)
        if (readValue)
        {
            static constexpr int32_t minPulse = 0;
            static constexpr uint32_t increasement = 100;

            if (readValue.value() == 'w')
            {
                tim3_ch1_pa6.SetPulse(currentPulse + increasement);
            }
            else if (readValue.value() == 's')
            {
                const int32_t calculatedPulse = currentPulse - increasement;
                tim3_ch1_pa6.SetPulse(calculatedPulse < minPulse ? minPulse : calculatedPulse);
            }
            else
            {
                ledControlFunction();
            }            
        }
        else
        {
            ledControlFunction();
        }
    }};
    
    Scheduler<Task<RegisterLevel::SoftwareTimer>, 4> scheduler{ { readLPS25HBSensorTask
                                                      , printTemperaturePressureTask
                                                      , ld2Task
                                                      , ledControllerTask } };

    // LPS25HB test
    uart1.ConfigureInterruptsPriority(IRQn_Type::USART1_IRQn, 1);
    uart1.Init(uart1Tx, uart1Rx, 9600);
    uart2.ConfigureInterruptsPriority(IRQn_Type::USART2_IRQn, 2);
    uart2.Init(uart2Tx, uart2Rx, 115200);
    ld2.Init();

    tim3_ch1_pa6.Start();

    while (true)
    {
        // UART 1 Test - Bluetooth HC-06
        // connection at linux
        // sudo rfcomm connect 0 <Address>
        // on second terminal window: screen /dev/rfcomm0 9600

        uart1.ProcessTx();
        uart2.ProcessTx();
        
        scheduler.Run();
    }
}

void Error_Handler(void)
{
    /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */
    __disable_irq();
    while (1)
    {
    }
    /* USER CODE END Error_Handler_Debug */
}

extern "C" void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    if (hi2c->Instance == I2C1)
    {
        lps25hbAsync.OnRxComplete();
    }
}

extern "C" void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    if (hi2c->Instance == I2C1)
    {
        lps25hbAsync.OnTxComplete();
    }
}

extern "C" void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    if (hi2c->Instance == I2C1)
    {
        lps25hbAsync.OnTxComplete();
    }
}

extern "C" void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
    if (hi2c->Instance == I2C1)
    {
        // Optionally inspect hi2c->ErrorCode here for future debug.
    }
}

extern "C" void USART1_IRQHandler(void)
{
    uart1.IRQHandler();
}

extern "C" void USART2_IRQHandler(void)
{
    uart2.IRQHandler();
}

