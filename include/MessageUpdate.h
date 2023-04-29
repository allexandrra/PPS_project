#ifndef NS3_MESSAGE_UPDATE_H
#define NS3_MESSAGE_UPDATE_H

#include <stdint.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <cassert>

#include "MessageHeader.h"
#include "Router.h"

/**
 * @brief Define the minimum length of the BGP message update
 */
#define MIN_MESSAGE_UPDATE_LEN 16

/*
 * @brief Define the MessageUpdate class by extending the MessageHeader class
*/
class MessageUpdate : public MessageHeader{
    private:

        /**
         * @brief Define the length of the  unfeasible routes in BGP message update
        */
        uint16_t unfeasable_route_len;

        /**
         * @brief Define the withdrawn routes in BGP message update
        */
        std::vector<NLRIs> withdrawn_routes;

        /**
         * @brief Define the total path attribute length in BGP message update
        */
        uint16_t total_path_atr_len;

        /**
         * @brief Define the path attribute in BGP message update
        */
        std::vector<Path_atrs> path_atr;

        /**
         * @brief Define the NLRI in BGP message update
        */
        std::vector<NLRIs> NLRI;

    public:

        /**
         * @brief Constructors for MessageUpdate with different parameters
        */
        MessageUpdate(uint16_t unfeasable_route_len, std::vector<NLRIs> withdraw_routes, uint16_t total_path_atr_len, std::vector<Path_atrs> path_atr, std::vector<NLRIs> NLRI);
        MessageUpdate(uint16_t total_path_atr_len, std::vector<Path_atrs> path_atr, std::vector<NLRIs> NLRI);
        MessageUpdate(uint16_t unfeasable_route_len, std::vector<NLRIs> withdraw_routes);
        MessageUpdate();

        /**
         * @brief Define the getters of the various param of the class
        */
        uint16_t get_unfeasable_route_len();
        std::vector<NLRIs> get_withdrawn_routes();
        uint16_t get_total_path_atr_len();
        std::vector<Path_atrs> get_path_atr();
        std::vector<NLRIs> get_NLRI();

        /**
         * @brief Overload of the << operator to print the MessageTrustrate fields into a binary bitstream
         * @param stream the output stream
         * @param msg the MessageOpen to print
         * @return the output stream
        */
        friend std::ostream& operator<<(std::ostream& stream, const MessageUpdate& msg);

        /**
         * @brief Overload of the >> operator to read the MessageTrustrate fields from a binary bitstream
         * @param stream the input stream
         * @param msg the MessageOpen to read
         * @return the input stream
        */
        friend std::istream& operator>>(std::istream & stream, MessageUpdate& msg);


        /**
         * @brief Function to check if the router is a neighbour
         * @param router the router to check
         * @param req_router index of router to check
         * @return true if the router is a neighbour, false otherwise
        */
        bool check_neighbour(Router router, int req_router);


        /**
         * @brief Function to check local the preferences of the router
         * @param new_routes the new routes to check
         * @param routing_table the routing table of the router
         * @return the new routes with the preferences
         */
        std::vector<Route> check_preferences(std::vector<Route> new_routes, std::vector<Peer> routing_table);

        /**
         * @brief Function to add the new routes to the RIBin
         * @param path_atr the path attributes of the new routes
         * @param nlri the nlri of the new routes
         * @return the new routes
         */
        std::vector<Route> add_to_RIBin(std::vector<Path_atrs> path_atr, std::vector<NLRIs> nlri);
};
#endif