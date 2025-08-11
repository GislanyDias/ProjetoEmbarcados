# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
#cmake_minimum_required(VERSION 3.16)

#include($ENV{IDF_PATH}/tools/cmake/project.cmake)
# "Trim" the build. Include the minimal set of components, main, and anything it depends on.
#idf_build_set_property(MINIMAL_BUILD ON)
#project(hello_world)

# Adicione esta linha para incluir seus componentes
#set(EXTRA_COMPONENT_DIRS components)
# Configurações opcionais
#set(COMPONENTS mpu6050 ssd1306 buzzer games)

cmake_minimum_required(VERSION 3.5)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
set(EXTRA_COMPONENT_DIRS components)
project(hello_world)