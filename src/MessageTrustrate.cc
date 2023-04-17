#include <stdint.h>
#include <vector>
#include <bits/stdc++.h> 
#include <stdio.h>
#include <fstream>

#include "../include/MessageTrustrate.h"


MessageTrustrate::MessageTrustrate() 
: MessageHeader(MESSAGE_HEADER_LEN+MIN_MESSAGE_TRUSTRATE_LEN, 5) {
  this->trust = DEFAULT_TRUST;
}

MessageTrustrate::MessageTrustrate(float trust) 
: MessageHeader(MESSAGE_HEADER_LEN+MIN_MESSAGE_TRUSTRATE_LEN, 5) {
  this->trust = trust;
}

float MessageTrustrate::get_trust() { return trust; }

/**
 * @brief Overload of the << operator to print the MessageTrustrate fields into a binary bitstream
 * @param stream the output stream
 * @param msg the MessageNotification to print
 * @return the output stream
*/
std::ostream& operator<<(std::ostream& stream, const MessageTrustrate& msg) {
    // Convert into a binary bitstream the marker field of the header
    std::stringstream strStreamMarker;
    std::copy(msg.marker.begin(), msg.marker.end(), std::ostream_iterator<int8_t>(strStreamMarker));

    // Convert the trust float value into an int between 0 and 100
    int trust = (int) (msg.trust * 100);

    // Print the bitstream into the output stream
    stream << std::bitset<128>(strStreamMarker.str()).to_string() << " " 
            << std::bitset<16>(msg.lenght).to_string() << " " 
            << std::bitset<8>(msg.type).to_string() << " "
            << std::bitset<8>(trust).to_string();

    return stream;
}


/**
 * @brief Overload of the >> operator to read the MessageTrustrate fields from a binary bitstream
 * @param stream the input stream
 * @param msg the MessageNotification to read
 * @return the input stream
*/
std::istream & operator>>(std::istream & stream, MessageTrustrate& msg) {
    // declare the bitset variables
    std::bitset<128> bit_marker;
    std::bitset<16> bit_lenght;
    std::bitset<8> bit_type;
    std::bitset<8> bit_trust;
    
    // read the bitstream from the input stream
    stream >> bit_marker >> bit_lenght >> bit_type >> bit_trust;

    // convert the bitset variables into the MessageNotification fields
    // start with the marker field
    std::vector<uint8_t> marker(128);
    for (int i = 0; i < (int)marker.size(); i++) {
        marker[i] = (bit_marker[i] == 1) ? '1' : '0';
    }

    // convert the other fields
    msg.marker = marker;
    msg.type = (uint16_t)bit_type.to_ulong();
    msg.lenght = (uint16_t)bit_lenght.to_ulong();
    msg.trust = ((uint16_t)bit_trust.to_ulong())/100.0;

    return stream;
}   
