idf_component_register(SRCS "hello_world_main.c"
                       REQUIRES  key_deteced
                       PRIV_REQUIRES spi_flash key_deteced
                       INCLUDE_DIRS "")
