#pragma once

#include <cstdint>
#include <optional>

namespace Device
{
    class LPS25HBBase
    {
    protected:
        struct Registers
        {
            static constexpr uint16_t ADDR = 0x5D; // 7-bit I2C address
            static constexpr uint16_t WHO_AM_I = 0x0F;
            static constexpr uint16_t CTRL_REG1 = 0x20;
            static constexpr uint16_t CTRL_REG2 = 0x21;
            static constexpr uint16_t CTRL_REG3 = 0x22;
            static constexpr uint16_t CTRL_REG4 = 0x23;
            static constexpr uint16_t PRESS_OUT_XL = 0x28;
            static constexpr uint16_t PRESS_OUT_L = 0x29;
            static constexpr uint16_t PRESS_OUT_H = 0x2A;
            static constexpr uint16_t TEMP_OUT_L = 0x2B;
            static constexpr uint16_t TEMP_OUT_H = 0x2C;
            static constexpr uint16_t CTRL_REG1_PD = 0x80;
            static constexpr uint16_t CTRL_REG1_ODR2 = 0x40;
            static constexpr uint16_t CTRL_REG1_ODR1 = 0x20;
            static constexpr uint16_t CTRL_REG1_ODR0 = 0x10;
        };

        static constexpr uint8_t addressOffset = 1;
        
        static constexpr uint8_t AUTO_INCREMENT = 0x80; //TEMP_OUT_L and TEMP_OUT_H are in sequence, so we can read them in one go by setting the auto-increment bit
                                                        //PRESS_OUT_XL, PRESS_OUT_L, and PRESS_OUT_H are in sequence, so we can read them in one go by setting the auto-increment bit
        [[nodiscard]] float RecalculateRawTemperature(const int16_t rawTemperature) const;
        [[nodiscard]] int32_t RecalculateRawPressure(const int32_t rawPressure) const;

    public:
        virtual ~LPS25HBBase() = default;
        enum class MeasurementFrequency
        {
            OneShot,
            Hz1,
            Hz7,
            Hz12_5,
            Hz25
        };

        [[nodiscard]] virtual std::optional<uint8_t> ReadWhoAmI() = 0;
        [[nodiscard]] virtual std::optional<float> ReadTemperature() = 0;
        [[nodiscard]] virtual std::optional<int32_t> ReadPressure() = 0;
        virtual void WakeUp() = 0;
        //TODO: add WakeUp and SetMeasurementFrequency
    };
};
