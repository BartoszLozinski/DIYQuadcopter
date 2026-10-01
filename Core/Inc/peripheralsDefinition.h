#pragma once

#include "Peripherals/UART/RegisterLevel/Uart.hpp"
#include "Peripherals/GPIO/RegisterLevel/GpioAlternate.hpp"
#include "Peripherals/GPIO/RegisterLevel/GpioOutput.hpp"


Peripherals::RegisterLevel::GpioAlternate<9> uart1Tx{ GPIOA, Peripherals::RegisterLevel::Gpio::AlternateFunction::AF7 };
Peripherals::RegisterLevel::GpioAlternate<10> uart1Rx{ GPIOA, Peripherals::RegisterLevel::Gpio::AlternateFunction::AF7 };
Peripherals::RegisterLevel::GpioAlternate<2> uart2Tx{ GPIOA, Peripherals::RegisterLevel::Gpio::AlternateFunction::AF7 };
Peripherals::RegisterLevel::GpioAlternate<3> uart2Rx{ GPIOA, Peripherals::RegisterLevel::Gpio::AlternateFunction::AF7 };
Peripherals::RegisterLevel::Uart uart1{ USART1 };
Peripherals::RegisterLevel::Uart uart2{ USART2 };

Peripherals::RegisterLevel::GpioOutput<5> ld2{ GPIOA };
