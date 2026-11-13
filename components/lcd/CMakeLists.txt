
idf_component_register(
    SRCS "lcd.c"
    INCLUDE_DIRS "include"
    REQUIRES
        driver
        i2cdev
        pcf8574
        hd44780
)
