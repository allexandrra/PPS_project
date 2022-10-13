#ifndef INTERFACE_H
#define INTERFACE_H

#include <string.h>
#include <stdint.h>
#include <vector.h>

namespace Interface {
    class Interface {
        public:
            Interface(string name, string IP, string mask);
            
            void set_status(bool status);
            string get_int_name();
            string get_int_IP();
            string get_int_mask();

        private:
            string name;
            string IP_addr;
            string mask;
            bool status;

    };
}

#endif