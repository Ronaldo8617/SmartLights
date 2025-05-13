# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ================= VS Code Pico Extension =================
if (WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 1.5.1)
set(toolchainVersion 13_2_Rel1)
set(picotoolVersion 2.0.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ========================================================

# Defina a sua placa
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Importa o SDK do Pico (obrigatório antes de project())
include(pico_sdk_import.cmake)

project(SmartLights C CXX ASM)

# Inicializa o SDK
pico_sdk_init()

# Executável principal
add_executable(SmartLights 
    SmartLights.c
    lib/rgb.c
    lib/ssd1306.c
    lib/buttons.c
    lib/display_init.c
    lib/matrixws.c
)

pico_set_program_name(SmartLights "SmartLights")
pico_set_program_version(SmartLights "0.1")

# UART/USB
pico_enable_stdio_uart(SmartLights 0)
pico_enable_stdio_usb(SmartLights 1)

# Bibliotecas vinculadas
target_link_libraries(SmartLights
    pico_stdlib
    hardware_pio
    hardware_pwm
    hardware_gpio
    hardware_adc
    hardware_i2c
    pico_cyw43_arch_lwip_threadsafe_background
    pico_lwip
    pico_cyw43_driver
)

# Diretórios de include
target_include_directories(SmartLights PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${PICO_SDK_PATH}/lib/lwip/src/include
    ${PICO_SDK_PATH}/lib/lwip/src/include/arch
    ${PICO_SDK_PATH}/lib/lwip/src/include/lwip
    ${PICO_SDK_PATH}/lib/lwip/src/apps/http
)

# Fontes adicionais do servidor HTTP (se usar)
target_sources(SmartLights PRIVATE
    ${PICO_SDK_PATH}/lib/lwip/src/apps/http/httpd.c
    ${PICO_SDK_PATH}/lib/lwip/src/apps/http/fs.c
)

# Gera UF2, HEX, etc.
pico_add_extra_outputs(SmartLights)
