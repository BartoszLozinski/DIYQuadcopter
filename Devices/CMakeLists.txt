set(target_name Devices)

set(${target_name}_src  LPS25HBBase.cpp
                        LPS25HB.cpp
                        LPS25HB_Async.cpp
                        ICM20948_Async.cpp)

set(${target_name}_hpp  LPS25HBBase.hpp
                        LPS25HB.hpp
                        LPS25HB_Async.hpp
                        ICM20948_Async.hpp
                        MCP23S08.hpp
                        Measurement.hpp)


configure_library(${target_name}
    SOURCES ${${target_name}_src}
    HEADERS ${${target_name}_hpp}
)

configure_embedded_target(${target_name})
