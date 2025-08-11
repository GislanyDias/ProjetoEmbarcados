idf_component_register(SRCS "hello_world_main.c"
                       PRIV_REQUIRES spi_flash
                       INCLUDE_DIRS ""
                       REQUIRES mpu6050 ssd1306 buzzer games)
