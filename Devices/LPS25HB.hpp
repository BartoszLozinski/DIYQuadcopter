#pragma once

#include "Peripherals/I2C/I2CBase.hpp"
#include "LPS25HBBase.hpp"
#include <optional>

namespace Device
{
    class LPS25HB : public LPS25HBBase
    {
    private:
        
        Peripherals::I2CBase& i2c;
        void WriteRegister(const uint8_t reg, uint8_t value);
        [[nodiscard]] std::optional<uint8_t> ReadRegister(uint8_t reg) const;        

    public:

        explicit LPS25HB(Peripherals::I2CBase& i2c_);
        [[nodiscard]] std::optional<uint8_t> ReadWhoAmI() override;
        [[nodiscard]] std::optional<float> ReadTemperature() override;
        [[nodiscard]] std::optional<int32_t> ReadPressure() override;
        void SetMeasurementFrequency(const MeasurementFrequency freq);
        void WakeUp() override;
    };
}
