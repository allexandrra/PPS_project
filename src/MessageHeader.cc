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

MessageHeader::MessageHeader(std::vector<uint8_t> marker, uint16_t lenght, uint16_t type) {
    this->marker.resize(128);
    this->marker.assign(marker.begin(), marker.end()); 
    this->lenght = lenght;
    this->type = type;
}

MessageHeader::MessageHeader(uint16_t lenght, uint16_t type) {
    this->marker.resize(128, '1');
    this->lenght = lenght;
    this->type = type;
}

MessageHeader::MessageHeader(uint16_t type) {
    this->marker.resize(128, '1');
    this->lenght = 0;
    this->type = type;
}

MessageHeader::MessageHeader() {
    this->marker.resize(128, '1');
    this->lenght = 0;
    this->type = 0;
}

std::vector<uint8_t> MessageHeader::get_marker() { return marker; }
uint16_t MessageHeader::get_lenght() { return lenght; }
uint16_t MessageHeader::get_type() { return type; }

void MessageHeader::set_market(std::vector<uint8_t> marker) { 
    this->marker.resize(128);
    this->marker.assign(marker.begin(), marker.end());
}
void MessageHeader::set_lenght(uint16_t lenght) { this->lenght = lenght; }
void MessageHeader::set_type(uint16_t type) { this->type = type; }

std::ostream& operator<<(std::ostream& stream, const MessageHeader& msg) {
    std::cout << "entro serialize" << std::endl;

    /*for (int i = 0; i < (int)msg.marker.size(); i++)
        std::cout << ' ' << msg.marker[i];
    std::cout << std::endl; */

    std::stringstream strStream;
    std::copy(msg.marker.begin(), msg.marker.end(), std::ostream_iterator<uint8_t>(strStream));

    stream << std::bitset<128>(strStream.str()).to_string() << std::bitset<16>(msg.lenght).to_string() << std::bitset<8>(msg.type).to_string();

    std::cout << "esco serialize" << std::endl;
    return stream;
}

std::istream & operator>>(std::istream & stream, MessageHeader& msg) {
    std::cout << "entro deserialized" << std::endl;
    std::bitset<128> bit_marker;
    std::bitset<16> bit_lenght;
    std::bitset<8> bit_type;

    stream >> bit_marker >> bit_lenght >> bit_type;

    //std::cout << bit_marker << " " << bit_lenght << " " << bit_type << std::endl;

    std::vector<uint8_t> marker(128);
    for (int i = 0; i < (int)marker.size(); i++) {
        marker[i] = (bit_marker[i] == 1) ? '1' : '0';
    }

    msg.marker = marker;
    //TODO: method for check that the lenght is between 19 and 4096
    msg.lenght = (uint16_t)bit_lenght.to_ulong();
    msg.type = (uint16_t)bit_type.to_ulong();

    /*std::cout << msg.lenght << " " << msg.type << std::endl;
    for (int i = 0; i < (int)msg.marker.size(); i++)
        std::cout << ' ' << msg.marker[i];
    std::cout << std::endl; */

    return stream;
}