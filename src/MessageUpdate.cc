#include "MessageBGP.h"
#include "Router.h"

#include <fstream>
#include <iostream>
#include <sstream> 
#include <iomanip>
#include <cstring>
#include <vector>

struct NLRI {
    uint8_t prefix_lenght;
    string prefix;
};

struct Path_atrs {
    int type;
    int lenght;
    string value;
    uint8_t optional;
    uint8_t transitive;
    uint8_t partial;
    uint8_t extended_lenght;
};

struct Route {
    NLRI nlri;
    std::vector<Path_atrs> path_atr;
};

class MessageUpdate : public MessageBGP {
    private:
        uint16_t unfeasable_route_len;
        std::vector<int> withdrawn_routes;
        uint16_t total_path_atr_len;
        std::vector<Path_atrs> path_atr;
        std::vector<NLRI> NLRIs;

    public:
        MessageUpdate(uint16_t unfeasable_route_len, std::vector<int> withdraw_routes, uint16_t total_path_atr_len, 
        std::vector<Path_atrs> path_atr, std::vector<NLRI> NLRI) {
            this->unfeasable_route_len = unfeasable_route_len;
            this->withdrawn_routes = withdrawn_routes;
            this->total_path_atr_len = total_path_atr_len;
            this->path_atr = path_atr;
            this->NLRIs = NLRI;
        }

        uint16_t get_unfeasable_route_len() { return unfeasable_route_len; }
        std::vector<int> get_withdrawn_routes() { return withdrawn_routes; }
        uint16_t get_total_path_atr_len() { return total_path_atr_len; }
        std::vector<Path_atrs> get_path_atr() { return path_atr; }
        std::vector<NLRI> get_NLRI() { return NLRIs; }

        bool check_neighbour(Router router, int req_router) {
            for (int n : router.get_router_neigh()) {
                if (n == req_router) 
                    return true;
            }
            return false;
        }

        vector<Route> check_preferences(vector<Route> new_routes, vector<Peer> routing_table) {
            vector<Route> loc_RIB;
            for (Route r : new_routes) {
                for (Peer p : routing_table) {
                    if (r.nlri.prefix.compare(p.network) == 0) {
                        // compare preferences
                        for (Path_atrs atr : r.path_atr) {
                            switch (atr.type) {
                            case 1: //weight
                                // compara greutatea
                                // daca mai mare treci mai departe
                                // daca mai mica inlocuieste ruta actuala cu ruta noua in tabelul de rutare
                                break;
                            case 2: //loc_pref
                                break;
                            case 3: // 
                                break;
                            default:
                                break;
                            }
                        }
                    } else {
                        loc_RIB.push_back(r);
                        break;
                    }
                }
            }

            return loc_RIB;
        }

        void apply_policy() {

        }

        void add_to_RT() {

        }

        void add_to_RIBin() {
            
        }

        void add_to_RIBout() {

        }

        void add_to_FIB() { 

        }
};
