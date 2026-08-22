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

Peripherals::HAL::UartIT btHC06Uart{ huart1 }; //PA9 (TX), PA10 (RX)
Peripherals::HAL::I2C_IT i2c1IT{ hi2c1 };
Device::LPS25HB_Async lps25hbAsync{ i2c1IT };
Peripherals::HAL::Pwm tim3_ch1_pa6{ htim3, TIM_CHANNEL_1 }; //PA6


int main()
{
    /* MCU Configuration--------------------------------------------------------*/
    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();
    SystemClock_Config();
    /* Initialize all configured peripherals */
    MX_GPIO_Init();
    //MX_USART2_UART_Init();
    MX_USART1_UART_Init();
    MX_I2C1_Init();
    MX_TIM3_Init();
    MX_SPI2_Init();
    
    RegisterLevel::SoftwareTimer btUartResetTimer{ 2000 };
    RegisterLevel::SoftwareTimer btUartPollTimer{ 1 };
    //UcCommunication::LineParser lineParser{ uart2 };
    UcCommunication::LineParser btLineParser{ btHC06Uart };

    float temperature = 0.0f;
    uint32_t pressure = 0;

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

    /*
    Task<RegisterLevel::SoftwareTimer> uart2ReadLineTask{ 1, [&]()
    {
        if (const auto lineOpt = lineParser.ReadLine())
        {
            const auto line = *lineOpt;
            const char* prefix = "RX Uart2IT: ";
            uart2.Transmit(reinterpret_cast<const uint8_t*>(prefix), strlen(prefix));
            uart2.Transmit(reinterpret_cast<const uint8_t*>(line.data()), line.size());
            uart2.Transmit(reinterpret_cast<const uint8_t*>("\r\n"), 2);
        }
    }};
    */

    std::optional<uint8_t> previousReadValue = std::nullopt;
    Task<RegisterLevel::SoftwareTimer> ledControllerTask{ 10, [&]()
    {
        volatile const auto currentPulse = tim3_ch1_pa6.GetPulse();
        const auto readValue = uart2.Read();

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
        else if (!previousReadValue.has_value())
        {
            ledControlFunction();   
        }

        previousReadValue = readValue;
    }};
    
    Scheduler<Task<RegisterLevel::SoftwareTimer>, 4> scheduler{ { readLPS25HBSensorTask
                                                      , printTemperaturePressureTask
                                                      , ld2Task
                                                      , ledControllerTask } };

    // LPS25HB test

    uart2.ConfigureInterruptsPriority(IRQn_Type::USART2_IRQn, 1);
    uart2.Init(uart2Tx, uart2Rx, 115200);
    ld2.Init();

    ioexp_cs.Init();
    ioexp_cs.Set();
    //pc2 - spi2_miso
    //pc3 - spi2_mosi
    //pb10 - spi2_sck`
    RegisterLevel::SoftwareTimer mcp23s08ToggleTimer{ 250 };
    static constexpr uint8_t mcp23s08WriteAddress = 0x40;
    uint8_t mcp23s08_pg0_on = 0x01;
    uint8_t gpio_config[3] { mcp23s08WriteAddress, static_cast<uint8_t>(Device::MCP23S08Register::IODIR), 0xFE }; //all input but pg0
    ioexp_cs.Clear();
    HAL_SPI_Transmit(&hspi2, gpio_config, sizeof(gpio_config), HAL_MAX_DELAY);
    ioexp_cs.Set();

    tim3_ch1_pa6.Start();

    while (true)
    {
        // UART 1 Test - Bluetooth HC-06
        // connection at linux
        // sudo rfcomm connect 0 <Address>
        // on second terminal window: screen /dev/rfcomm0 9600

        uart2.ProcessTx();
        btHC06Uart.ProcessTx();
        //uart2.ProcessRx();
        btHC06Uart.ProcessRx();

        //TODO: fix bthc06 reciever
        if (const auto lineOpt = btLineParser.ReadLine())
        {
            const auto line = *lineOpt;
            const char* prefix = "RX: ";
            btHC06Uart.Transmit(reinterpret_cast<const uint8_t*>(prefix), strlen(prefix));
            btHC06Uart.Transmit(reinterpret_cast<const uint8_t*>(line.data()), line.size());
            btHC06Uart.Transmit(reinterpret_cast<const uint8_t*>("\r\n"), 2);
        }
        
        if (btUartResetTimer.IsExpired())
        {  
            btUartResetTimer.Reset();
            std::string_view resetMsg = "BT UART Reset\r\n";
            btHC06Uart.Transmit(reinterpret_cast<const uint8_t*>(resetMsg.data()), resetMsg.size());
        }
        
        // End of UART Test
        scheduler.Run();

        if (mcp23s08ToggleTimer.IsExpired())
        {
            mcp23s08ToggleTimer.Reset();
            if (mcp23s08_pg0_on == 0x01)
                mcp23s08_pg0_on = 0x00;
            else
                mcp23s08_pg0_on = 0x01;

            uint8_t led_command[3] = { mcp23s08WriteAddress, static_cast<uint8_t>(Device::MCP23S08Register::OLAT), mcp23s08_pg0_on };

            ioexp_cs.Clear();
            HAL_SPI_Transmit(&hspi2, led_command, sizeof(led_command), HAL_MAX_DELAY);
            ioexp_cs.Set();
        }
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

extern "C" void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        btHC06Uart.RxCpltCallback();
    }

    /*
    if (huart->Instance == USART2)
    {
        uart2.RxCpltCallback();
    }
    */
}

extern "C" void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        btHC06Uart.TxCpltCallback();
    }

    /*
    if (huart->Instance == USART2)
    {
        uart2.TxCpltCallback();
    }
    */
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

extern "C" void USART2_IRQHandler(void)
{
    uart2.IRQHandler();
}

