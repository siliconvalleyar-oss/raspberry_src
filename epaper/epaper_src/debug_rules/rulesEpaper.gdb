# Cargar el archivo ejecutable
file ./bin/epaper_app

# Establecer puntos de interrupción en funciones del namespace GPIO
break GPIO::Gpio_t::file_open_and_write_value
break GPIO::Gpio_t::gpio_export
break GPIO::Gpio_t::gpio_unexport
break GPIO::Gpio_t::gpio_set_direction
break GPIO::Gpio_t::gpio_set_value
break GPIO::Gpio_t::gpio_set_edge
break GPIO::Gpio_t::gpio_get_fd_to_value

break GPIO::Gpio_t::CloseGpios
break GPIO::Gpio_t::pinMode
break GPIO::Gpio_t::digitalWrite
break GPIO::Gpio_t::digitalRead


break EPAPER::EPD_Driver::COG_initial
break EPAPER::EPD_Driver::sendIndexData

break SPI::Spi_t::Spi_t        
break SPI::Spi_t::init
break SPI::Spi_t::Transfer1bytes
break SPI::Spi_t::settings_spi

# RULES ENABLE
break GPIO::Gpio_t::settings


# Configurar comandos para imprimir valores importantes
command 1
    print pin
    print str_v
    print fileTmp.is_open()
end

# Continuar ejecución automáticamente después de un punto de interrupción
commands
    continue
end

# Configurar la visualización del código fuente
set listsize 10
