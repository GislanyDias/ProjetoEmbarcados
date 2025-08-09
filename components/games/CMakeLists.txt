idf_component_register(SRCS 
                    "menu.c"
                    "dodge.c"
                    "tilt_maze.c"
                    "snake.c"
                    "pong.c"
                    INCLUDE_DIRS "include"
                    REQUIRES mpu6050 ssd1306 buzzer button)