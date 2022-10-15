#include <stdint.h>
#include <vector>

#include "MessageHeader.h"

class MessageNotification : public MessageHeader {

    private:
        int16_t error_code;
        int16_t error_subcode;
        std::string data;

    public: 
        MessageNotification(int16_t error_code, int16_t error_subcode, std::string data);
        MessageNotification();

        int16_t get_error_code();
        int16_t get_error_subcode();
        std::string get_data();

        friend std::ostream& operator<<(std::ostream& stream, const MessageNotification& msg);
        friend std::istream & operator>>(std::istream & stream, MessageNotification& msg);
};