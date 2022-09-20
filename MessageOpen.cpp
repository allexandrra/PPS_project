#include <stdint.h>
#include <vector>

class MessageOpen {
    private:
        uint8_t version;
        uint16_t my_AS;
        uint16_t hold_time;
        uint32_t BGP_id;
        uint8_t opt_param_len;
        vector<int> opt_param();

    public:
        MessageOpen(uint16_t AS_number, uint16_t hold_time, uint32_t BGP_id, uint8_t opt_param_len, vector<int> opt_param) {
            version = 4;
            my_AS = AS_number;
            hold_time = hold_time;
            BGP_id = BGP_id;
            opt_param_len = opt_param_len;
            opt_param = opt_param;
        }

        uint8_t get_version() { return version; }
        uint16_t get_AS() { return my_AS; }
        uint16_t get_hold_time() { return hold_time; }
        uint32_t get_BGP_id() { return BGP_id; }
        uint8_t get_opt_param_len() { return opt_param_len; }
        vector<int> get_opt_param() { return opt_param; }
};