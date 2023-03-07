#include <stdint.h>
#include <vector>

#include "MessageHeader.h"

/**
 * @brief Define the minimum length of the BGP message notification
 */
#define MIN_MESSAGE_NOTIFICATION_LEN 2

/**
 * @brief Define the MessageNotification class by extending the MessageHeader class
 */
class MessageNotification : public MessageHeader {

    private:

        /**
         * @brief Error code of the BGP message notification
         */
        int16_t error_code;

        /**
         * @brief Error subcode of the BGP message notification
         */        
        int16_t error_subcode;

        /**
         * @brief Data related to the error of the BGP message notification
         */
        std::string data;

    public: 

        /**
         * @brief Constructors for MessageNotification with different parameters
         */
        MessageNotification(int16_t error_code, int16_t error_subcode, std::string data);
        MessageNotification(int16_t error_code, int16_t error_subcode);
        MessageNotification();

        /**
         * @brief Getters for the attributes of the class
         */
        int16_t get_error_code();
        int16_t get_error_subcode();
        std::string get_data();

        /**
         * @brief Overload of the << operator to print the MessageNotification fields into a binary bitstream
         * @param stream the output stream
         * @param msg the MessageNotification to print
         * @return the output stream
        */
        friend std::ostream& operator<<(std::ostream& stream, const MessageNotification& msg);

        /**
         * @brief Overload of the >> operator to read the MessageNotification fields from a binary bitstream
         * @param stream the input stream
         * @param msg the MessageNotification to read
         * @return the input stream
        */
        friend std::istream & operator>>(std::istream & stream, MessageNotification& msg);
};