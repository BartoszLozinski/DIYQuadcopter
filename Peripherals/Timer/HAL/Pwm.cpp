#include "Pwm.hpp"
#include "Drivers/CMSIS/Device/ST/STM32L4xx/Include/stm32l4xx.h"

namespace Peripherals
{
    namespace HAL
    {
        Pwm::Pwm(TIM_HandleTypeDef& timer_, const uint32_t channel_)
            : timer(timer_)
            , channel(channel_)
        {}; //cube initialized

        uint16_t Pwm::GetCounter_Impl() const
        {
            return __HAL_TIM_GET_COUNTER(&timer);
        };

        uint16_t Pwm::GetMaxCounter_Impl() const
        {
            return __HAL_TIM_GET_AUTORELOAD(&timer);
        }

        PwmState Pwm::GetState_Impl() const
        {
            return static_cast<PwmState>(HAL_TIM_PWM_GetState(&timer));
        };

        void Pwm::Start_Impl()
        {
            HAL_TIM_PWM_Start(&timer, channel);
        }

        void Pwm::Stop_Impl()
        {
            HAL_TIM_PWM_Stop(&timer, channel);
        };

        void Pwm::SetPulse_Impl(const uint32_t pulse)
        {
            const uint32_t saturatedPulse = pulse >= timer.Init.Period ? timer.Init.Period : pulse;
            __HAL_TIM_SET_COMPARE(&timer, channel, saturatedPulse);
        }

        uint32_t Pwm::GetPulse() const
        {
            return __HAL_TIM_GET_COMPARE(&timer, channel);
        }

        void Pwm::ResetCounter_Impl()
        {
            __HAL_TIM_SET_COUNTER(&timer, 0);
        }
    }
};