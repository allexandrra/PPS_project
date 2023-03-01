#include "../include/Router.h"
#include "ns3/log.h"

namespace ns3 {
  NS_LOG_COMPONENT_DEFINE("Router");

  Router::Router(std::string router_ID, int AS, std::vector<Interface> interfaces,
                std::vector<int> neighbours, std::vector<Peer> routing_table, 
                NodeContainer node, Ipv4Address ASip, Ipv4Mask ASmask) : NodeContainer(node){
    this->router_ID = router_ID;
    this->AS = AS;
    this->interfaces = interfaces;
    this->neighbours = neighbours;
    this->routing_table = routing_table;
    //this->node = node;
    this->ASip = ASip;
    this->ASmask = ASmask;
  }

  Router::Router(int AS, NodeContainer node, Ipv4Address ASip, Ipv4Mask ASmask) : NodeContainer(node) {
    this->AS = AS;
    //Convert ASip to string
    std::stringstream ip;
    ASip.Print(ip);
    this->router_ID = ip.str();
    this->ASip = ASip;
    this->ASmask = ASmask;
  }

  std::string Router::get_router_ID() { return this->router_ID; }
  int Router::get_router_AS() { return this->AS; }
  std::vector<Interface> Router::get_router_int() { return this->interfaces; }
  std::vector<int> Router::get_router_neigh() { return this->neighbours; }
  std::vector<Peer> Router::get_router_rt() { return this->routing_table; }
  NodeContainer Router::get_router_node() { return NodeContainer::Get(0); }
  Ipv4Address Router::get_router_ASip() { return this->ASip; }
  Ipv4Mask Router::get_router_ASmask() { return this->ASmask; }

  Interface Router::get_router_int_from_name(int num) { 
    for(int i = 0; i < (int) interfaces.size(); i++) {
      std::string if_name = "eth"+std::to_string(num);
      //NS_LOG_INFO(if_name);
      if(interfaces[i].name == if_name) {
        //NS_LOG_INFO("Entro con " + if_name);
        return interfaces[i];
      }
    }
    return this->interfaces[0]; 
  }

  int Router::get_router_int_num_from_name(int num) {
    for(int i = 0; i < (int) interfaces.size(); i++) {
      std::string if_name = "eth"+std::to_string(num);
      //NS_LOG_INFO(if_name);
      if(interfaces[i].name == if_name) {
        //NS_LOG_INFO("Entro con " + if_name);
        return i;
      }
    }
    return 0; 
  }

  void Router::push_new_route(Peer new_route) {
    this->routing_table.push_back(new_route);
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

  void Router::add_interface(Interface interface) {
    this->interfaces.push_back(interface);
  }
  
  void Router::add_neighbour(int neighbour) {
    this->neighbours.push_back(neighbour);
    //NS_LOG_INFO("Added neighbour " << neighbour << " to router " << this->router_ID);
    //NS_LOG_INFO("Router " << this->router_ID << " now has " << this->neighbours.size() << " neighbours");
  }

  void Router::setServer(int int_num, Ptr<BGPServer> server) {
    this->interfaces[int_num].server = server;
    this->interfaces[int_num].isServer = true;
    this->interfaces[int_num].status = true;
  }
  
  void Router::setClient(int int_num, Ptr<BGPClient> client) {
    this->interfaces[int_num].client = client;
    this->interfaces[int_num].isServer = false;
    this->interfaces[int_num].status = true;
  }

  void Router::resetServer(int int_num) {
    this->interfaces[int_num].server.reset();
    this->interfaces[int_num].status = false;
  }
  
  void Router::resetClient(int int_num) {
    this->interfaces[int_num].client.reset();
    this->interfaces[int_num].status = false;
  }

  void Router::setInterfaceStatus(int int_num, bool status) {
    this->interfaces[int_num].status = status;
  }

  int Router::get_router_int_num_from_ip(Ipv4Address ip) {
    for(int i = 0; i < (int) interfaces.size(); i++) {
      if(interfaces[i].ip_address == ip) {
        //std::cout <<  "Entro con " << ip << " e " << interfaces[i].ip_address << std::endl;
        return i;
      }
    }
    return 0; 
  }

  void Router::set_router_neigh(std::vector<int> neighbours) {
    this->neighbours = neighbours;
  }

}  // namespace ns3

