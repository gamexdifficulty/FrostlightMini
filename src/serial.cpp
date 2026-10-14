#include "driver/usb_serial_jtag.h"
#include <stdio.h>
#include <string.h>
#include "serial.h"
#include <string>
#include <vector>
#include <ranges>

std::vector<std::string> split(const std::string& s, char delim) {
    std::vector<std::string> result;

    for (auto part : std::views::split(s, delim)) {
        result.emplace_back(part.begin(), part.end());
    }

    return result;
}

Serial::Serial(){
    usb_serial_jtag_driver_config_t config = {};
    config.rx_buffer_size = 256;
    config.tx_buffer_size = 256;

    ESP_ERROR_CHECK(usb_serial_jtag_driver_install(&config));
}

void Serial::update() {
    len = usb_serial_jtag_read_bytes(buf, sizeof(buf), 20 / portTICK_PERIOD_MS);
}

std::vector<std::string> Serial::read() {
    if (len == 0) { return {}; }

    if (len >= sizeof(buf)) {
        len = sizeof(buf) - 1;
    }

    buf[len] = '\0';
    std::string input(reinterpret_cast<char*>(buf));
    return split(input, ' ');
}

void Serial::write(const char* output) {
    usb_serial_jtag_write_bytes((const uint8_t *)output, strlen(output), portMAX_DELAY);
}