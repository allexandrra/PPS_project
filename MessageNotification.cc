#include <stdint.h>
#include <vector>

class MessageNotification {
    private:
        int8_t error_code;
        int8_t error_subcode;
        char* data;

    public:
        MessageNotification(int8_t error_code, int8_t error_subcode, char* data) {
            error_code = error_code;
            error_subcode = error_subcode;
            data = data;
        }

        int8_t get_error_code() { return error_code; }
        int8_t get_error_subcode() { return error_subcode; }
        char* get_data() { return data; }
};