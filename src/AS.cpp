
#include "../include/AS.h"

AS::AS(string n)
{
    this->name = n;
};

void AS::add_link(string l){

    /* check if link is already existent */
    if( find(this->links.begin(), this->links.end(), l) != this->links.end() ) 
        this->links.push_back(l);

};


void AS::set_links(vector<string> links)
{
    this->links = links;
};