set(target_name SPI_HAL)

set(${target_name}_src  Spi.cpp
)

set(${target_name}_hpp  ../ISpi.hpp
                        Spi.hpp
)

configure_library(${target_name}
    SOURCES ${${target_name}_src}
    HEADERS ${${target_name}_hpp}
)

configure_embedded_target(${target_name})
configure_HAL_target(${target_name})
