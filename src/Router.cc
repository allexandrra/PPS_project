#include "../include/Router.h"
#include "ns3/log.h"

namespace ns3 {

  // Define the NS_LOG_COMPONENT to be used by the logging system
  NS_LOG_COMPONENT_DEFINE("Router");

  /**
   * @brief Constructors for the Router class with different parameters
   * Each of the constructurs calls the constructor of the NodeContainer class with the passed node
   */
  Router::Router(std::string router_ID, int AS, std::vector<Interface> interfaces,
                std::vector<int> neighbours, std::vector<Peer> routing_table, 
                NodeContainer node, Ipv4Address AS_ip, Ipv4Mask AS_mask) : NodeContainer(node){
    this->router_ID = router_ID;
    this->AS = AS;
    this->interfaces = interfaces;
    this->neighbours = neighbours;
    this->routing_table = routing_table;
    this->AS_ip = AS_ip;
    this->AS_mask = AS_mask;
  }

  Router::Router(int AS, NodeContainer node, Ipv4Address AS_ip, Ipv4Mask AS_mask) : NodeContainer(node) {
    this->AS = AS;
    //Convert ASip to string to store it as BGP ID
    std::stringstream ip;
    this->router_ID = ip.str();
    this->AS_ip = AS_ip;
    this->AS_mask = AS_mask;
  }


  /**
   * @brief Standard getters for the Router class attributes
   */
  std::string Router::get_router_ID() { return this->router_ID; }
  int Router::get_router_AS() { return this->AS; }
  std::vector<Interface> Router::get_router_int() { return this->interfaces; }
  std::vector<int> Router::get_router_neigh() { return this->neighbours; }
  std::vector<Peer> Router::get_router_rt() { return this->routing_table; }
  NodeContainer Router::get_router_node() { return NodeContainer::Get(0); }
  Ipv4Address Router::get_router_ASip() { return this->AS_ip; }
  Ipv4Mask Router::get_router_ASmask() { return this->AS_mask; }

  /**
   * @brief Method for getting an interface struct instance from the interface name 
   * @param num name of the interface (the name is composed by the number passed as parameter and the prefix string "eth")
   * @return Interface struct instance
  */
  Interface Router::get_router_int_from_name(int num) { 
    // Iterate over the interfaces vector and return the interface with the name passed as parameter
    for(int i = 0; i < (int) interfaces.size(); i++) {
      // name is composed by eth plus the number passed as parameter
      std::string if_name = "eth"+std::to_string(num);
      if(interfaces[i].name == if_name) {
        return interfaces[i];
      }
    }
    // if not found return the first interface
    return this->interfaces[0]; 
  }

  /**
   * @brief Method for getting the interface index inside the vector of Interfaces of the router from the interface name
   * @param num number of the interface (the name is composed by the number passed as parameter and the prefix string "eth")
   * @return Interface index inside the vector of Interfaces of the router
   */    
  int Router::get_router_int_num_from_name(int num) {
    // Iterate over the interfaces vector and return the index of the interface with the name passed as parameter
    for(int i = 0; i < (int) interfaces.size(); i++) {
      // name is composed by eth plus the number passed as parameter
      std::string if_name = "eth"+std::to_string(num);
      if(interfaces[i].name == if_name) {
        return i;
      }
    }
    // if not found return 0
    return 0; 
  }

  /**
   * @brief Method for getting the interface index inside the vector of Interfaces of the router from the interface IP address
   * @param ip IP address of the interface
   * @return Interface index inside the vector of Interfaces of the router
  */
  int Router::get_router_int_num_from_ip(Ipv4Address ip) {
    // Iterate over the interfaces vector and return the index of the interface with the IP address passed as parameter
    for(int i = 0; i < (int) interfaces.size(); i++) {
      if(interfaces[i].ip_address == ip) {
        return i;
      }
    }
    // if not found return 0
    return 0; 
  }


  /**
   * @brief Methods for updating the routing table of the router
  */
  void Router::push_new_route(Peer new_route) {
    this->routing_table.push_back(new_route);
    //NS_LOG_INFO("Added route " << new_route.network << " to router " << this->router_ID);
    //NS_LOG_INFO("Router " << this->router_ID << " now has " << this->neighbours.size() << " neighbours");
    //NS_LOG_INFO(this->routing_table.size());
  }

  void Router::update_routing_table(std::string network, std::vector<Path_atrs> atrib) {
    for(Peer p : this->routing_table) {
      if(p.network == network) {
        for(Path_atrs atr : atrib) {
          if(atr.type == 1) {
            p.weight = std::stoi(atr.value);
          } else if(atr.type == 2) {
            p.AS_path_len = atr.lenght;
            p.path = atr.value;
          } else if(atr.type == 3) {
            p.next_hop = atr.value;
          } else if(atr.type == 4) {
            p.MED = std::stoi(atr.value);
          } else if(atr.type == 5) {
            p.loc_pref = std::stoi(atr.value);
          }
        }
      }
    }
  }

  void Router::apply_policy(Router router, Route update_route) {
    router.update_routing_table(update_route.nlri.prefix, update_route.path_atr);
  }

  /**
   * @brief Method for adding a new interface to the router Infaces vector
   * @param interface Interface struct instance to be added
  */
  void Router::add_interface(Interface interface) {
    this->interfaces.push_back(interface);
  }
  
  /**
   * @brief Method for adding a new neighbour to the router Neighbours vector
   * @param neighbour AS number of the neighbour router to be added
  */
  void Router::add_neighbour(int neighbour) {
    this->neighbours.push_back(neighbour);
  }

  /**
   * @brief Methods for setting the server and client applications of a single router interface
   * @param int_num number of the interface in which the server or client application is to be set
   * @param server pointer to the new server application
  */
  void Router::set_server(int int_num, Ptr<BGPServer> server) {
    this->interfaces[int_num].server = server;
    this->interfaces[int_num].isServer = true;
    this->interfaces[int_num].status = true;
  }
  
  /**
   * @brief Methods for setting the server and client applications of a single router interface
   * @param int_num number of the interface in which the server or client application is to be set
   * @param client pointer to the new client application
  */
  void Router::set_client(int int_num, Ptr<BGPClient> client) {
    this->interfaces[int_num].client = client;
    this->interfaces[int_num].isServer = false;
    this->interfaces[int_num].status = true;
  }

  /**
   * @brief Methods for resetting the server and client applications of a single router interface
   * @param int_num number of the interface in which the server or client application is to be reset
  */
  void Router::reset_server(int int_num) {
    this->interfaces[int_num].server.reset();
    this->interfaces[int_num].status = false;
  }
  
  void Router::reset_client(int int_num) {
    this->interfaces[int_num].client.reset();
    this->interfaces[int_num].status = false;
  }

  /**
   * @brief Method for setting the status of a single router interface
   * @param int_num number of the interface in which the status is to be set
   * @param status boolean indicating the new status of the interface
  */
  void Router::set_interface_status(int int_num, bool status) {
    this->interfaces[int_num].status = status;
  }

  /**
   * @brief Method for updating and setting of new router neighbours vector
   * @param neighbours new vector of neighbours AS numbers
  */
  void Router::set_router_neigh(std::vector<int> neighbours) {
    this->neighbours = neighbours;
  }

  /**
   * @brief Method for updating and setting of new router interface
   * @param interface new interface struct instance
   * @param num index of the interface to be updated
   * 
  */
  void Router::set_interface(Interface interface, int num) {
    this->interfaces[num] = interface;
  }

  std::vector<NLRIs> Router::remove_route(std::vector<NLRIs> wr) {

    for(int i = 0; i < wr.size(); i++) {
      for (int j = 0; j < this->routing_table.size(); j++) {
        if (wr[i].prefix.compare(this->routing_table[j].network) == 0) {
          this->routing_table.erase(this->routing_table.begin() + j);
        }
      }
    }
  }


  /**
   * @brief Method for getting the interface trust value from the interface name
   * @param num name of the interface (the name is composed by the number passed as parameter and the prefix string "eth")
   * @return Interface trust value
  */
  float Router::get_trust_from_interface_name(int num) {
    // Iterate over the interfaces vector and return the index of the interface with the name passed as parameter
    for(int i = 0; i < (int) interfaces.size(); i++) {
      // name is composed by eth plus the number passed as parameter
      std::string if_name = "eth"+std::to_string(num);
      if(interfaces[i].name == if_name) {
        std::cout << "entro ";
        if(interfaces[i].total_trust == 0) {
          std::cout << " direct trust: " << interfaces[i].direct_trust << std::endl;
          return interfaces[i].direct_trust;
        }
        else {
          std::cout << " total trust: " << interfaces[i].total_trust << std::endl;
          return interfaces[i].total_trust;
        }
      }
    }
    // if not found return 0
    return 0; 
  }
}  // namespace ns3

