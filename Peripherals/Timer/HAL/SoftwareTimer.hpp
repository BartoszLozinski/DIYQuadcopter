#pragma once

#include "TimerBase.hpp"
#include <cstdint>

namespace HAL
{
    class SoftwareTimer : public TimerBase<SoftwareTimer>
    {
    friend TimerBase<SoftwareTimer>;
    private:
        [[nodiscard]] uint32_t Now_Impl() const;

    public:
        SoftwareTimer(const uint32_t delay_);
    };
};