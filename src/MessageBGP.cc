#include <stdint.h>
#include <vector>

class MessageBGP {
    private:
        int8_t* marker;
        int16_t lenght;
        int8_t type;

    public:
        MessageBGP(int8_t* marker, int16_t lenght, int8_t type) {
            marker = marker;
            lenght = lenght;
            type = type;
        }

        int8_t* get_marker() { return marker; }
        int16_t get_lenght() { return lenght; }
        int8_t get_type() { return type; }
}; 