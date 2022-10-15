#include <stdint.h>
#include <vector>

#include "MessageHeader.h"

class MessageUpdate : public MessageHeader {
    private:
        uint16_t unfeasable_route_len;
        std::vector<int> withdrawn_routes;
        uint16_t total_path_atr_len;
        std::vector<int> path_atr;
        std::vector<int> NLRI;

    public:
        MessageUpdate(uint16_t unfeasable_route_len, std::vector<int> withdraw_routes, uint16_t total_path_atr_len, std::vector<int> path_atr, std::vector<int> NLRI) 
        : MessageHeader() {
            this->unfeasable_route_len = unfeasable_route_len;
            this->withdrawn_routes = withdrawn_routes;
            this->total_path_atr_len = total_path_atr_len;
            this->path_atr = path_atr;
            this->NLRI = NLRI;
        }

        uint16_t get_unfeasable_route_len() { return unfeasable_route_len; }
        std::vector<int> get_withdrawn_routes() { return withdrawn_routes; }
        uint16_t get_total_path_atr_len() { return total_path_atr_len; }
        std::vector<int> get_path_atr() { return path_atr; }
        std::vector<int> get_NLRI() { return NLRI; }
};