#include <iostream>
#include "configuration_parser.h"

using namespace std;

int start_simulation()
{   
    std::vector<AS> network = load_configuration();
    return 0;
}

int send_ip_message()
{
    return 0;
}

int disable_router_link()
{
    return 0;
}

int enable_router_link()
{
    return 0;
}

int main()
{
    char c;

    start_simulation();

    while (1)
    {
        printf("\n\nSelect one of the following: \n\n"
               "[1] Add a new policy\n"
               "[2] Disable a link \n"
               "[3] Enable a link \n"
               "[4] Quit\n\n"
               "Choose a number: ");

        cin >> c;

        if (isdigit(c))
        {
            switch (c)
            {
            case '1':
                send_ip_message();
                break;

            case '2':
                disable_router_link();
                break;

            case '3':
                enable_router_link();
                break;

            case '4':
                exit(EXIT_SUCCESS);
                break;

            default:
                printf("Invalid number, choose a number between 1 and 4. ");
                break;
            }
        }
        else
        {
            printf("Invalid input, choose a number between 1 and 4. ");
        }
    }

    return 0;
}