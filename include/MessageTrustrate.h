#ifndef NS3_MESSAGE_TRUSTRATE_H
#define NS3_MESSAGE_TRUSTRATE_H

#include "MessageHeader.h"

#include <stdint.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <cassert>

#define DEFAULT_TRUST 0.5
#define MIN_MESSAGE_TRUSTRATE_LEN 1

/*
- After discovering a new host, send your inherent trust to this host.
- Store your trust into a new variable. One upgrade would be to store political,
financial, technological trust and have a function to weight those metrics.
- New metric in routing table : trust to take into account to choose best path.
*/

/*
 * @brief Define the MessageTrustRate class by extending the MessageHeader class
*/
class MessageTrustrate : public MessageHeader {
    private:
        // Trust value: between 0 and 1
        float trust;

    public:
        /**
         * @brief Constructors for MessageTrustrate with different parameters
         */
        MessageTrustrate();
        MessageTrustrate(float trust);
        
        /**
         * @brief Getters for the trust attribute of the class
         * @return the trust value
        */
        float get_trust();

        /**
         * @brief Overload of the << operator to print the MessageTrustrate fields into a binary bitstream
         * @param stream the output stream
         * @param msg the MessageOpen to print
         * @return the output stream
        */
        friend std::ostream& operator<<(std::ostream& stream, const MessageTrustrate& msg);

        /**
         * @brief Overload of the >> operator to read the MessageTrustrate fields from a binary bitstream
         * @param stream the input stream
         * @param msg the MessageOpen to read
         * @return the input stream
        */
        friend std::istream & operator>>(std::istream & stream, MessageTrustrate& msg);

};
#endif