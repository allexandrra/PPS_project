#include <stdint.h>
#include <vector>

class MessageUpdate {
    private:
        uint16_t unfeasable_route_len;
        vector<int> withdrawn_routes();
        uint16_t total_path_atr_len;
        vector<int> path_atr();
        vector<int> NLRI();

    public:
        MessageUpdate(uint16_t unfeasable_route_len, vector<int> withdraw_routes, uint16_t total_path_atr_len, vector<int> path_atr, vector<int> NLRI) {
            unfeasable_route_len = unfeasable_route_len;
            withdrawn_routes = withdrawn_routes;
            total_path_atr_len = total_path_atr_len;
            path_atr = path_atr;
            NLRI = NLRI;
        }

        uint16_t get_unfeasable_route_len() { return unfeasable_route_len; }
        vector<int> get_withdrawn_routes() { return withdrawn_routes; }
        uint16_t get_total_path_atr_len() { return total_path_atr_len; }
        vector<int> get_path_atr() { return path_atr; }
        vector<int> get_NLRI() { return NLRI; }
};