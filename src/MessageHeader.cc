#include <stdint.h>
#include <vector>
#include <bits/stdc++.h> 
#include <stdio.h>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <cstring>

#include "ns3/config-store.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/tcp-header.h"
#include "ns3/log.h"
#include "ns3/simulator.h"

#include "../include/MessageHeader.h"

/**
 * @brief Construct a with all the params of the Message Header
*/
MessageHeader::MessageHeader(std::vector<uint8_t> marker, uint16_t lenght, uint16_t type) {
    this->marker.resize(128);
    this->marker.assign(marker.begin(), marker.end()); 
    this->lenght = lenght;
    this->type = type;
}

/**
 * @brief Other different types of construct for the Message Header with different params
*/
MessageHeader::MessageHeader(uint16_t lenght, uint16_t type) {
    // the marker is initialized with 128 '1'
    this->marker.resize(128, '1');
    this->lenght = lenght;
    this->type = type;
}

MessageHeader::MessageHeader(uint16_t type) {
    this->marker.resize(128, '1');
    // the lenght is initialized with 19 (the min len of the header)
    this->lenght = MESSAGE_HEADER_LEN;
    this->type = type;
}

MessageHeader::MessageHeader() {
    this->marker.resize(128, '1');
    this->lenght = 0;
    this->type = 0;
}


/**
 * @brief Getters for the attributes of the class
*/
std::vector<uint8_t> MessageHeader::get_marker() { return marker; }
uint16_t MessageHeader::get_lenght() { return lenght; }
uint16_t MessageHeader::get_type() { return type; }

/**
 * @brief Setters for the attributes of the class
*/
void MessageHeader::set_market(std::vector<uint8_t> marker) { 
    this->marker.resize(128);
    this->marker.assign(marker.begin(), marker.end());
}
void MessageHeader::set_lenght(uint16_t lenght) { this->lenght = lenght; }
void MessageHeader::set_type(uint16_t type) { this->type = type; }

/**
 * @brief Overload of the << operator to print the MessageHeader fields into a binary bitstream 
 * @param stream the output stream
 * @param msg the MessageHeader to print
 * @return the output stream
*/
std::ostream& operator<<(std::ostream& stream, const MessageHeader& msg) {

    std::stringstream strStream;
    // copy the vector that rapresent the marker a string stream of bits
    std::copy(msg.marker.begin(), msg.marker.end(), std::ostream_iterator<uint8_t>(strStream));

    // print the marker, the lenght and the type of the message as bits
    stream << std::bitset<128>(strStream.str()).to_string() << std::bitset<16>(msg.lenght).to_string() << std::bitset<8>(msg.type).to_string();

    return stream;
}

/**
 * @brief Overload of the >> operator to read the MessageHeader fields from a binary bitstream 
 * @param stream the input stream
 * @param msg the MessageHeader to read
 * @return the input stream
*/
std::istream & operator>>(std::istream & stream, MessageHeader& msg) {
    // define the bitset that rapresent the marker, the lenght and the type of the message
    std::bitset<128> bit_marker;
    std::bitset<16> bit_lenght;
    std::bitset<8> bit_type;

    // read the marker, the lenght and the type of the message as bits
    stream >> bit_marker >> bit_lenght >> bit_type;

    // copy the bitset that rapresent the marker into a vector of uint8_t
    std::vector<uint8_t> marker(128);
    for (int i = 0; i < (int)marker.size(); i++) {
        marker[i] = (bit_marker[i] == 1) ? '1' : '0';
    }

    msg.marker = marker;
    
    msg.lenght = (uint16_t)bit_lenght.to_ulong();

    std::cout << "\n         " << msg.lenght << std::endl;

    if (msg.lenght < 19 || msg.lenght > 4096) {
        std::cerr << "The lenght of the message is not valid" << std::endl;
    }
    
    msg.type = (uint16_t)bit_type.to_ulong();

    return stream;
}