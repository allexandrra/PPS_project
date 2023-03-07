#ifndef NS3_MESSAGE_OPEN_H
#define NS3_MESSAGE_OPEN_H

#include <stdint.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <cassert>

#include "MessageHeader.h"

/**
 * @brief Define the minimum length of the BGP message open
 */
#define MIN_MESSAGE_OPEN_LEN 10


/*
 * @brief Define the MessageOpen class by extending the MessageHeader class
*/
class MessageOpen : public MessageHeader{
    private:

        /**
         * @brief Version of the BGP protocol
         */
        uint16_t version;

        /**
         * @brief Autonomous system number of the BGP message open
         */
        uint16_t my_AS;

        /**
         * @brief Hold time of the BGP message open
         */
        uint16_t hold_time;

        /**
         * @brief BGP identifier of the BGP message open (following the standard it will have the same form of an IPv4 address)
         */
        std::string BGP_id;

        /**
         * @brief Optional parameters length of the BGP message open
         */
        uint16_t opt_param_len;

        /**
         * @brief Optional parameters of the BGP message open
         */
        std::vector<uint8_t> opt_param;

    public:

        /**
         * @brief Constructors for MessageOpen with different parameters
         */
        MessageOpen(uint16_t AS_number, uint16_t hold_time, std::string BGP_id, uint16_t opt_param_len, std::vector<uint8_t> opt_param);
        MessageOpen(uint16_t AS_number, uint16_t hold_time, std::string BGP_id);
        MessageOpen();

        /**
         * @brief Getters for the attributes of the class
         */
        uint16_t get_version();
        uint16_t get_AS();
        uint16_t get_hold_time();
        std::string get_BGP_id();
        uint16_t get_opt_param_len();
        std::vector<uint8_t> get_opt_param();


        /**
         * @brief Overload of the << operator to print the MessageOpen fields into a binary bitstream
         * @param stream the output stream
         * @param msg the MessageOpen to print
         * @return the output stream
        */
        friend std::ostream& operator<<(std::ostream& stream, const MessageOpen& msg);

        /**
         * @brief Overload of the >> operator to read the MessageOpen fields from a binary bitstream
         * @param stream the input stream
         * @param msg the MessageOpen to read
         * @return the input stream
        */
        friend std::istream & operator>>(std::istream & stream, MessageOpen& msg);
};

#endif