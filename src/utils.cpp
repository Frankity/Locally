#include <stdexcept>
#include <limits>
#include <iostream>
#include <cstdint>

uint16_t stringToUint16(const std::string& str) {
    try {
        unsigned long value = std::stoul(str);

        if (value > std::numeric_limits<uint16_t>::max()) {
            throw std::out_of_range("Valor fuera de rango para uint16_t");
        }

        return static_cast<uint16_t>(value);
    } catch (const std::exception& e) {
        std::cerr << "Error al convertir string a uint16_t: " << e.what() << std::endl;
        return 0;
    }
}
