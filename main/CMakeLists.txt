idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES
        wifi
        websocket
        sensor
        nvs_flash
)