#ifndef AS_H
#define AS_H

#include <string>
#include <vector>
#include <algorithm>

using namespace std;

class AS
{
public:
    string name;
    vector<string> links;

    /**
     * @brief Construct a new AS object
     *
     * @param n
     */
    AS(string n);

    /**
     * @brief Add a physical link to another AS
     * @pre
     * @param router name of another AS to link to
     */
    void add_link(string router);

    /**
     * @brief Set physical links the router is connected to
     * 
     * @param links 
     */
    void set_links(vector<string> links);

};

#endif