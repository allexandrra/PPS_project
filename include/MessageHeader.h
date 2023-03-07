#ifndef NS3_MESSAGE_HEADER_H
#define NS3_MESSAGE_HEADER_H

#include <stdint.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <cassert>


/**
 * @brief Define the standard length of the BGP message header
 */
#define MESSAGE_HEADER_LEN 19

class MessageHeader {
    protected:

        /**
         * @brief Marker of the BGP message header
        */
        std::vector<uint8_t> marker;

        /**
         * @brief Actual lenght of the BGP message header
        */
        uint16_t lenght;

        /**
         * @brief Type of the BGP message
        */
        uint16_t type;

    public:

        /**
         * @brief Constructors for MessageHeader with different parameters
        */
        MessageHeader(std::vector<uint8_t> marker, uint16_t lenght, uint16_t type);
        MessageHeader(uint16_t lenght, uint16_t type);
        MessageHeader(uint16_t type);
        MessageHeader();

        /**
         * @brief Getters and setters for the attributes of the class
        */
        std::vector<uint8_t> get_marker();
        uint16_t get_lenght();
        uint16_t get_type();
        void set_market(std::vector<uint8_t> marker);
        void set_lenght(uint16_t lenght);
        void set_type(uint16_t type);

        /**
         * @brief Overload of the << operator to print the MessageHeader fields into a binary bitstream 
         * @param stream the output stream
         * @param msg the MessageHeader to print
         * @return the output stream
        */
        friend std::ostream& operator<<(std::ostream& stream, const MessageHeader& msg);

        /**
         * @brief Overload of the >> operator to read the MessageHeader fields from a binary bitstream 
         * @param stream the input stream
         * @param msg the MessageHeader to read
         * @return the input stream
        */
        friend std::istream& operator>>(std::istream & stream, MessageHeader& msg);
};

#endif