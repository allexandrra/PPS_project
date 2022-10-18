#include <stdint.h>
#include <vector>

#include "MessageUpdate.h"

MessageUpdate::MessageUpdate(uint16_t unfeasable_route_len, std::vector<int> withdraw_routes, uint16_t total_path_atr_len, std::vector<int> path_atr, std::vector<int> NLRI) 
: MessageHeader() {
    this->unfeasable_route_len = unfeasable_route_len;
    this->withdrawn_routes = withdrawn_routes;
    this->total_path_atr_len = total_path_atr_len;
    this->path_atr = path_atr;
    this->NLRI = NLRI;
}

MessageUpdate::MessageUpdate() : MessageHeader() {
    //TODO
}


uint16_t MessageUpdate::get_unfeasable_route_len() { return unfeasable_route_len; }
std::vector<int> MessageUpdate::get_withdrawn_routes() { return withdrawn_routes; }
uint16_t MessageUpdate::get_total_path_atr_len() { return total_path_atr_len; }
std::vector<int> MessageUpdate::get_path_atr() { return path_atr; }
std::vector<int> MessageUpdate::get_NLRI() { return NLRI; }

std::ostream& operator<<(std::ostream& stream, const MessageUpdate& msg) {
    //TODO
}

std::istream & operator>>(std::istream & stream, MessageUpdate& msg) {
    //TODO
}
