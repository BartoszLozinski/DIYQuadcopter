set(target_name STM32_HAL)

add_library(${target_name}   STATIC)

file(GLOB HAL_SOURCES
    STM32L4xx_HAL_Driver/Src/*.c
)

cmake_path(GET CMAKE_CURRENT_SOURCE_DIR PARENT_PATH parent_dir)


target_sources(${target_name}
    PRIVATE
        ${HAL_SOURCES}
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS ${parent_dir}
)

configure_embedded_target(${target_name})

target_include_directories(${target_name}
    PRIVATE
        STM32L4xx_HAL_Driver/Legacy
    PUBLIC
        ${CMAKE_SOURCE_DIR}/Core/Inc
        CMSIS/Device/ST/STM32L4xx/Include
        CMSIS/Include
        STM32L4xx_HAL_Driver/Inc
)

target_compile_definitions(${target_name}
    PRIVATE
        USE_HAL_DRIVER
)
