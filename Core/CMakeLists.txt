set(MAP_FILE "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.map")
set(LD_SCRIPT ${CMAKE_SOURCE_DIR}/STM32L476RGTX_FLASH.ld)

add_executable(${CMAKE_PROJECT_NAME})
target_sources(${CMAKE_PROJECT_NAME}
    PRIVATE
        Src/stm32CubeMXGenerated.c
        Src/main.cpp
        Src/stm32l4xx_hal_msp.c
        Src/stm32l4xx_it.c
        Src/syscalls.c
        Src/sysmem.c
        Src/system_stm32l4xx.c
        Startup/startup_stm32l476rgtx.s
)

target_include_directories(${CMAKE_PROJECT_NAME}
    PRIVATE
        Core/Inc)
        
target_link_libraries(${CMAKE_PROJECT_NAME}
    PRIVATE
        I2C_HAL
        # SPI_HAL
        Gpio_RegisterLevel
        # Gpio_HAL
        # ADC_HAL
        Timer
        Timer_RegisterLevel
        UART_HAL
        UART_RegisterLevel
        Devices
        Scheduler
)

configure_embedded_target(${CMAKE_PROJECT_NAME})
configure_HAL_target(${CMAKE_PROJECT_NAME})

target_link_options(${CMAKE_PROJECT_NAME}
    PRIVATE
        -T${LD_SCRIPT}
        -Wl,-Map=${MAP_FILE},-cref
        -Wl,--gc-sections
)
