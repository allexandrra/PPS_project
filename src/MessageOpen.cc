#include <stdint.h>
#include <vector>
#include <bits/stdc++.h> 
#include <stdio.h>
#include <fstream>

#include "../include/MessageOpen.h"

MessageOpen::MessageOpen(uint16_t AS_number, uint16_t hold_time, uint32_t BGP_id, uint16_t opt_param_len, std::vector<uint8_t> opt_param) 
: MessageHeader(1) {
    version = 4;
    my_AS = AS_number;
    this->hold_time = hold_time;
    this->BGP_id = BGP_id;
    this->opt_param_len = opt_param_len;
    this->opt_param.assign(opt_param.begin(), opt_param.end()); 
}

MessageOpen::MessageOpen(uint16_t AS_number, uint16_t hold_time, uint32_t BGP_id, uint16_t opt_param_len) 
: MessageHeader(1) {
    version = 4;
    my_AS = AS_number;
    this->hold_time = hold_time;
    this->BGP_id = BGP_id;
    this->opt_param_len = opt_param_len;
    this->opt_param.resize(0); 
}

MessageOpen::MessageOpen() 
: MessageHeader(1) {
    version = 4;
    my_AS = 0;
    this->hold_time = 0;
    this->BGP_id = 0;
    this->opt_param_len = 0;
    this->opt_param.resize(0); 
}

uint16_t MessageOpen::get_version() { return version; }
uint16_t MessageOpen::get_AS() { return my_AS; }
uint16_t MessageOpen::get_hold_time() { return hold_time; }
uint32_t MessageOpen::get_BGP_id() { return BGP_id; }
uint16_t MessageOpen::get_opt_param_len() { return opt_param_len; }
std::vector<uint8_t> MessageOpen::get_opt_param() { return opt_param; }

std::ostream& operator<<(std::ostream& stream, const MessageOpen& msg) {
    std::stringstream strStreamMarker;
    std::copy(msg.marker.begin(), msg.marker.end(), std::ostream_iterator<int8_t>(strStreamMarker));

    std::stringstream strStreamOptParam;
    std::copy(msg.opt_param.begin(), msg.opt_param.end(), std::ostream_iterator<int8_t>(strStreamOptParam));
    
    //std::cout << "version: " << msg.version << " " << std::bitset<8>(msg.version).to_string() << std::endl;

    stream << std::bitset<128>(strStreamMarker.str()).to_string() << " " 
            << std::bitset<16>(msg.lenght).to_string() << " " 
            << std::bitset<8>(msg.type).to_string() << " " 
            << std::bitset<8>(msg.version).to_string() << " " 
            << std::bitset<16>(msg.my_AS).to_string() << " " 
            << std::bitset<16>(msg.hold_time).to_string() << " " 
            << std::bitset<32>(msg.BGP_id).to_string() << " " 
            << std::bitset<8>(msg.opt_param_len).to_string(); 
            
    if (msg.opt_param_len > 0) {
        stream << std::bitset<8>(strStreamOptParam.str()).to_string();
    }

    return stream;
}

std::istream & operator>>(std::istream & stream, MessageOpen& msg) {
    std::bitset<128> bit_marker;
    std::bitset<16> bit_lenght;
    std::bitset<8> bit_type;
    std::bitset<8> bit_version;
    std::bitset<16> bit_my_AS;
    std::bitset<16> bit_hold_time;
    std::bitset<32> bit_BGP_id;
    std::bitset<8> bit_opt_param_len;
    // TODO check this length
    std::bitset<128> bit_opt_param;

    stream >> bit_marker >> bit_lenght >> bit_type >> bit_version >> bit_my_AS >> bit_hold_time >> bit_BGP_id >> bit_opt_param_len >> bit_opt_param;

    //std::cout << "bit_version : " << bit_version << std::endl;

    std::vector<uint8_t> marker(128);
    for (int i = 0; i < (int)marker.size(); i++) {
        marker[i] = (bit_marker[i] == 1) ? '1' : '0';
    }

    // TODO: check this lenght
    std::vector<uint8_t> opt_param(128);
    for (int i = 0; i < (int)opt_param.size(); i++) {
        opt_param[i] = (bit_opt_param[i] == 1) ? '1' : '0';
    }

    //msg = MessageOpen(bit_my_AS.to_ulong(), bit_hold_time.to_ulong(), bit_BGP_id.to_ulong(), bit_opt_param_len.to_ulong(), opt_param);
    msg.marker = marker;
    msg.lenght = (uint16_t)bit_lenght.to_ulong();
    msg.type = (uint16_t)bit_type.to_ulong();
    msg.version = (uint16_t)bit_version.to_ulong();
    //std::cout << "version : " << msg.version << std::endl;
    msg.my_AS = (uint16_t)bit_my_AS.to_ulong();
    msg.hold_time = (uint16_t)bit_hold_time.to_ulong();
    msg.BGP_id = (uint32_t)bit_BGP_id.to_ulong();
    msg.opt_param_len = (uint16_t)bit_opt_param_len.to_ulong();
    msg.opt_param = opt_param; 
    //std::cout << "opt param len : " << msg.opt_param_len << bit_opt_param_len.to_ulong() << std::endl;
    return stream;
}   