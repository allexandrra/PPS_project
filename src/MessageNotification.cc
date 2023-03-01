#include <stdint.h>
#include <vector>
#include <bits/stdc++.h> 
#include <stdio.h>
#include <fstream>

#include "../include/MessageNotification.h"

// ERROR CODES
//  4 = hold time expire = let see if we are able to make the hold time working
// 	1 = wrong lenght in the msg header = N/A due to the fact that we create the messages (we do not perform the check because otherwise the unpacking will not work)
// 	2/3 = OPEN/UPDATE Message Error Handling = N/A due to the fact that we create the messages (we do not perform the check because otherwise the unpacking will not work)
// 	5 = Finite State Machine Error Handling = it should not happen that we receive an expected msg (we designed the simulation so we are in control)
// 	BGP Connection Collision Detection = it should not happen due to the fact that we have fixed roles for client/server
// 	6 = Cease: we implemented the fact that a router could drop the tcp connection (by disabling the link)


MessageNotification::MessageNotification(int16_t error_code, int16_t error_subcode, std::string data) 
: MessageHeader(MESSAGE_HEADER_LEN+MIN_MESSAGE_NOTIFICATION_LEN+data.length(), 3) {
    this->error_code = error_code;
    this->error_subcode = error_subcode;
    this->data = data;
}

MessageNotification::MessageNotification(int16_t error_code, int16_t error_subcode) 
: MessageHeader(MESSAGE_HEADER_LEN+MIN_MESSAGE_NOTIFICATION_LEN, 3) {
    this->error_code = error_code;
    this->error_subcode = error_subcode;
    this->data = "";
}

MessageNotification::MessageNotification() : 
MessageHeader(MESSAGE_HEADER_LEN+MIN_MESSAGE_NOTIFICATION_LEN, 3) {
    this->error_code = 0;
    this->error_subcode = 0;
    this->data = "";
}

int16_t MessageNotification::get_error_code() { return error_code; }
int16_t MessageNotification::get_error_subcode() { return error_subcode; }
std::string MessageNotification::get_data() { return data; }

std::ostream& operator<<(std::ostream& stream, const MessageNotification& msg) {
    std::stringstream strStreamMarker;
    std::copy(msg.marker.begin(), msg.marker.end(), std::ostream_iterator<int8_t>(strStreamMarker));

    std::stringstream strStreamData;
    for (int i = 0; i < (int)msg.data.size(); i++) {
        strStreamData << std::bitset<8>(msg.data[i]).to_string();
    }

    stream << std::bitset<128>(strStreamMarker.str()).to_string() << " " 
            << std::bitset<16>(msg.lenght).to_string() << " " 
            << std::bitset<8>(msg.type).to_string() << " "
            << std::bitset<8>(msg.error_code).to_string() << " "
            << std::bitset<8>(msg.error_subcode).to_string() << " "
            // TODO change the length of the data field
            << strStreamData.str();

    return stream;
}

std::istream & operator>>(std::istream & stream, MessageNotification& msg) {
    std::bitset<128> bit_marker;
    std::bitset<16> bit_lenght;
    std::bitset<8> bit_type;
    std::bitset<8> bit_error_code;
    std::bitset<8> bit_error_subcode;
    std::bitset<(256-21)*8> bit_data;

    stream >> bit_marker >> bit_lenght >> bit_type >> bit_error_code >> bit_error_subcode >> bit_data;

    //std::cout << "bit_version : " << bit_version << std::endl;

    std::vector<uint8_t> marker(128);
    for (int i = 0; i < (int)marker.size(); i++) {
        marker[i] = (bit_marker[i] == 1) ? '1' : '0';
    }

    msg.lenght = (uint16_t)bit_lenght.to_ulong();
    
    //std::cout << bit_data << std::endl;
    msg.data = "";
    int dataLen = msg.lenght - MIN_MESSAGE_NOTIFICATION_LEN - MESSAGE_HEADER_LEN;

    for (int i=dataLen; i > 0 ; i--) {
        std::bitset<8> byte;
        for(int j=0; j<8; j++) {
            byte[7-j] = bit_data[i*8-j-1]; 
        }
        //std::cout << (uint8_t)byte.to_ulong() << std::endl;
        msg.data += (uint8_t)byte.to_ulong();
    }

    msg.marker = marker;
    msg.type = (uint16_t)bit_type.to_ulong();
    msg.error_code = (uint16_t)bit_error_code.to_ulong();
    msg.error_subcode = (uint16_t)bit_error_subcode.to_ulong();

    return stream;
}   