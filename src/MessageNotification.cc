#include "MessageBGP.h"

class MessageNotification : public MessageBGP {
    private:
        int8_t error_code;
        int8_t error_subcode;
        char* data;

    public: 
        MessageNotification(int8_t error_code, int8_t error_subcode, char* data) {
            this->error_code = error_code;
            this->error_subcode = error_subcode;
            this->data = data;
        }

        int8_t get_error_code() { return error_code; }
        int8_t get_error_subcode() { return error_subcode; }
        char* get_data() { return data; }
};