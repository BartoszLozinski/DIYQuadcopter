#pragma once

#include <cstdint>
#include <optional>

namespace Device
{
    class ICM20948Base
    {
    protected:
        // documentation page 38/89

        enum class RegisterAddresses : uint8_t
        {
            //Bank 0 register map
            ADDR = 0x69, // 7-bit I2C address (AD0 high)
            WHO_AM_I = 0x00, // Value for ICM-20948 is 0xEA
            USER_CTRL = 0x03,
            LP_CONFIG = 0x05,
            PWR_MGMT_1 = 0x06,
            PWR_MGMT_2 = 0x07,
        };

    public:
        virtual ~ICM20948Base() = default;
        virtual void WakeUp() = 0;
    };
};
