// General includes
#include <fstream>
#include <iostream>
#include <iomanip>
#include <cstring>
#include <algorithm>
#include "ns3/config-store.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/tcp-header.h"
#include "ns3/log.h"
#include "ns3/simulator.h"

#include "../include/BGP-server.h"
#include "../include/MessageHeader.h"
#include "../include/Router.h"
#include "../include/MessageOpen.h"
#include "../include/MessageNotification.h"
#include "../include/MessageUpdate.h"

namespace ns3 {
	NS_LOG_COMPONENT_DEFINE ("BGPServer");
  	NS_OBJECT_ENSURE_REGISTERED(BGPServer);

	// Constructor
	BGPServer::BGPServer () {
		NS_LOG_FUNCTION (this);
		m_socket = 0;
	}

	BGPServer::~BGPServer() {
		NS_LOG_FUNCTION (this);
	}

	TypeId BGPServer::GetTypeId() {
        static TypeId tid = TypeId("ns3::BGPServer")
            .AddConstructor<BGPServer>()
            .SetParent<TCPCommon>();
        return tid;
    }

	/** Setup function - 2 arguments
 	* The socket is NOT passed to the function. The socket will be allocated in the Start Application function.
 	*/
	void BGPServer::Setup(uint16_t serverPort,  Time startTime) {
		m_port      = serverPort;
		m_startTime = startTime;
	}


	/** Application Methods
	 * Called at time specified by Start
	 */
	void BGPServer::StartApplication() {
		NS_LOG_FUNCTION (this);

		// Create the socket if not already
		if (m_socket == 0) {
			m_tid = TypeId::LookupByName("ns3::TcpSocketFactory");
			m_socket = Socket::CreateSocket(GetNode(), m_tid);
			InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), m_port);
			m_socket->Bind(local);
			m_socket->Listen();
		}

		// AcceptCallback
		m_socket->SetAcceptCallback (
			MakeNullCallback<bool, Ptr<Socket>, const Address &> (),
			MakeCallback(&BGPServer::HandleAccept, this));
		
		// RecvCallback
		m_socket->SetRecvCallback (
			MakeCallback(&BGPServer::HandleRead, this));
		
		// CloseCallback
		m_socket->SetCloseCallbacks (
			MakeCallback(&BGPServer::HandlePeerClose, this),
			MakeCallback(&BGPServer::HandlePeerError, this));
	}


	 /** Stop Application
	 * Called by simulator to stop the server
	 */
	void BGPServer::StopApplication ()
	{
		NS_LOG_FUNCTION (this);
		while(!m_socketList.empty ()) { //these are accepted sockets, close them
			Ptr<Socket> acceptedSocket = m_socketList.front ();
			m_socketList.pop_front ();
			acceptedSocket->Close ();
		}
		if (m_socket) {
			m_socket->Close ();
			m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
		}
	}


	//--------------------------
	inline bool isSetBit(int n, int index){
		return n & (1 << index);
	}

	int popcount1(int n){
		int count = 0;
		int digits = static_cast<int>(std::floor(std::log2(n))) + 1;
		for(int i = 0 ; i < digits ; ++i){
			if(isSetBit(n,i)) ++count;
		}
		return count;
	}

	std::vector<NLRIs> buildWR() {
		std::vector<NLRIs> withdrawn_routes;

		return withdrawn_routes;
	}

	std::vector<NLRIs> buildNLRI(Router r) {
		std::vector<NLRIs> nlri;

		NLRIs new_nlri;

		for (Peer p : r.get_router_rt()) {
			int len = 0;
			size_t pos = 0;
			std::string token;
			std::string network = p.network;
			std::string mask = p.mask;
			pos = mask.find(".");
			while ((pos = mask.find(".")) != std::string::npos) {
				token = mask.substr(0,pos);
				if (stoi(token) == 255) 
					len += 8;
				else {
					len += popcount1(stoi(token));
				}

				mask.erase(0, pos+1);
			}
			
			new_nlri.prefix = network;
			new_nlri.prefix_lenght = len;

			nlri.push_back(new_nlri);
		}

		return nlri;
	}

	std::vector<Path_atrs> buildPA(Router r) {
		std::vector<Path_atrs> path_atributes;

		for (Peer p : r.get_router_rt()) {
			// weight 1, loc pref 2, next hop 3, AS 4, MED 5
			Path_atrs atrib_w;
			atrib_w.type = 1;
			atrib_w.lenght = 0;
			atrib_w.value = to_string(p.weight);
			atrib_w.optional = 0;
			atrib_w.transitive = 0;
			atrib_w.partial = 0;
			atrib_w.extended_lenght = 0;

			Path_atrs atrib_lf;
			atrib_lf.type = 2;
			atrib_lf.lenght = 0;
			atrib_lf.value = to_string(p.loc_pref);
			atrib_lf.optional = 0;
			atrib_lf.transitive = 0;
			atrib_lf.partial = 0;
			atrib_lf.extended_lenght = 0;

			Path_atrs atrib_nh;
			atrib_nh.type = 3;
			atrib_nh.lenght = 0;
			atrib_nh.value = p.next_hop;
			atrib_nh.optional = 0;
			atrib_nh.transitive = 0;
			atrib_nh.partial = 0;
			atrib_nh.extended_lenght = 0;

			Path_atrs atrib_as;
			atrib_as.type = 4;
			atrib_as.lenght = p.AS_path_len;
			atrib_as.value = p.path;
			atrib_as.optional = 0;
			atrib_as.transitive = 0;
			atrib_as.partial = 0;
			atrib_as.extended_lenght = 0;

			Path_atrs atrib_med;
			atrib_med.type = 5;
			atrib_med.lenght = 0;
			atrib_med.value = to_string(p.MED);
			atrib_med.optional = 0;
			atrib_med.transitive = 0;
			atrib_med.partial = 0;
			atrib_med.extended_lenght = 0;

			path_atributes.push_back(atrib_w);
			path_atributes.push_back(atrib_lf);
			path_atributes.push_back(atrib_nh);
			path_atributes.push_back(atrib_as);
			path_atributes.push_back(atrib_med);
		}

		return path_atributes;
	}
	//-------------------------

	void BGPServer::HandleRead (Ptr<Socket> socket){
		NS_LOG_FUNCTION(this << socket);

		std::string packet = TCPCommon::HandleRead(socket);

		MessageHeader msg;
		std::stringstream(packet) >> msg;

		if (msg.get_type() == 0){
			//In teory, the sending ok keepalive message should be done indipendtly for each router, indipendtly from a received message of its peer
			//So, ech router sends keepalive messages to its peer at regular intervals to maintain the connection, but it does not expect any response from the peer
			//To simplify the implementation, the server we will send the keepalive message only when it receives the keepalive form the client peer

			//NS_LOG_INFO("Received KEEPALIVE message");
			std::cout << " KEEPALIVE message " << std::endl;

			Router *r = this->GetRouter();
			Address to;

			// Get receiving address
			socket->GetSockName(to);
			InetSocketAddress toAddress = InetSocketAddress::ConvertFrom(to);
			int int_num = r->get_router_int_num_from_ip(toAddress.GetIpv4());
			Interface intf = r->get_router_int()[int_num];

			//NS_LOG_INFO("Before new start time - Now: " << Simulator::Now().GetSeconds() << " " << " start time " << intf.get_last_update_time() << " max hold time " << intf.get_max_hold_time());

			if(Simulator::Now().GetSeconds() - intf.get_last_update_time() <= intf.get_max_hold_time()) {
				//Server send KEEPALIVE message
				std::stringstream msgStream;
				MessageHeader msg = MessageHeader(0);
				msgStream << msg << '\0';

				if(intf.status) {
					Ptr<Packet> packet = Create<Packet>((uint8_t*) msgStream.str().c_str(), msgStream.str().length()+1);
					this->Send(socket, packet);

					// Reset holdtime time
					intf.set_last_update_time(Simulator::Now().GetSeconds());
					r->set_interface(intf, int_num);
					//NS_LOG_INFO("After new start time - Now: " << Simulator::Now().GetSeconds() << " " << " start time " << intf.get_last_update_time() << " max hold time " << intf.get_max_hold_time());
					//NS_LOG_INFO("Interface " << intf.name << " of router " << r->get_router_AS() << " has status " << intf.status << " and has client " << intf.client.has_value() << " and has server " << intf.server.has_value() <<  " at time " << Simulator::Now().GetSeconds() << " [SERVER KEEPALIVE READ]");
				} else {
					NS_LOG_INFO("Interface " << intf.name << " of router " << r->get_router_AS() << " is down [KEEPALIVE READ SERVER] ");
		
					std::stringstream msgStreamNotification;
					MessageNotification msg = MessageNotification(6,0);
					msgStreamNotification << msg << '\0';

					Ptr<Packet> packetNotification = Create<Packet>((uint8_t*) msgStreamNotification.str().c_str(), msgStreamNotification.str().length()+1);
					this->Send(socket, packetNotification);

					intf.client.reset();
					intf.server.reset();

					r->set_interface_status(int_num, false);
					r->set_interface(intf, int_num);
					this->StopApplication();
				}

			} else {
				NS_LOG_INFO("Hold time for interface " << intf.name << " of router " << r->get_router_AS() << " expired at time " << Simulator::Now().GetSeconds() << " [KEEPALIVE READ SERVER]");
		
				std::stringstream msgStreamNotification;
				MessageNotification msg = MessageNotification(4,0);
				msgStreamNotification << msg << '\0';

				Ptr<Packet> packetNotification = Create<Packet>((uint8_t*) msgStreamNotification.str().c_str(), msgStreamNotification.str().length()+1);
				this->Send(socket, packetNotification);

				intf.client.reset();
				intf.server.reset();

				r->set_interface_status(int_num, false);
				r->set_interface(intf, int_num);
				this->StopApplication();
			}

		}
		else if(msg.get_type() == 1){
			//NS_LOG_INFO("Received OPEN message");
			MessageOpen msgRcv;
			std::stringstream(packet) >> msgRcv;
			Address to;
			Router *r = this->GetRouter();

			std::cout << " OPEN message with content  AS: " << msgRcv.get_AS() << " \t HOLD TIME: " << msgRcv.get_hold_time() << "\t BGP ID: " <<  binaryToDottedNotation(msgRcv.get_BGP_id()) << std::endl;

			// Get receiving address
			socket->GetSockName(to);
			InetSocketAddress toAddress = InetSocketAddress::ConvertFrom(to);
			//int int_num = r->get_router_int_num_from_ip(toAddress.GetIpv4());
			//Interface intf = r->get_router_int()[int_num];
			//NS_LOG_INFO("Router AS: " << r->get_router_AS());
			int max_hold_time = max((int)msgRcv.get_hold_time(), 90);
			
			std::stringstream msgStream;
			//HOLD TIME = 90s, KEEPALIVE = 30s (1/3 the hold time)

			//r->set_hold_time(toAddress, max_hold_time);
			//Simulator::Schedule (Seconds(1.0), &Interface::increment_hold_time, &intf);
			int int_num = r->get_router_int_num_from_ip(toAddress.GetIpv4());
			Interface intf = r->get_router_int()[int_num];

			if(intf.status) {
				MessageOpen msgToSend = MessageOpen(r->get_router_AS(), max_hold_time, r->get_router_ID());
				msgStream << msgToSend << '\0';
				Ptr<Packet> packet = Create<Packet>((uint8_t*) msgStream.str().c_str(), msgStream.str().length()+1);
				this->Send(socket,packet);
			} else {
				NS_LOG_INFO("Interface " << intf.name << " of router " << r->get_router_AS() << " is down [OPEN READ SERVER]");

				std::stringstream msgStream;
				MessageNotification msg = MessageNotification(6,0);
				msgStream << msg << '\0';

				Ptr<Packet> packet = Create<Packet>((uint8_t*) msgStream.str().c_str(), msgStream.str().length()+1);
				this->Send(socket, packet);

				intf.client.reset();
				intf.server.reset();

				r->set_interface_status(int_num, false);
				this->StopApplication();
			}

    		intf.set_max_hold_time(max_hold_time);
			intf.set_last_update_time(Simulator::Now().GetSeconds());
			r->set_interface(intf, int_num);

		} else if(msg.get_type() == 2) {
			MessageUpdate msgRcv;
			std::stringstream(packet) >> msgRcv;
			Address to;
			Router *r = this->GetRouter();

			//std::cout << " OPEN message with content  AS: " << msgRcv.get_AS() << " \t HOLD TIME: " << msgRcv.get_hold_time() << "\t BGP ID: " <<  binaryToDottedNotation(msgRcv.get_BGP_id()) << std::endl;

			// Get receiving address
			socket->GetSockName(to);
			InetSocketAddress toAddress = InetSocketAddress::ConvertFrom(to);
			//int int_num = r->get_router_int_num_from_ip(toAddress.GetIpv4());
			//Interface intf = r->get_router_int()[int_num];
			//NS_LOG_INFO("Router AS: " << r->get_router_AS());
			//int max_hold_time = max((int)msgRcv.get_hold_time(), 90);
			
			std::stringstream msgStream;
			//HOLD TIME = 90s, KEEPALIVE = 30s (1/3 the hold time)

			//r->set_hold_time(toAddress, max_hold_time);
			//Simulator::Schedule (Seconds(1.0), &Interface::increment_hold_time, &intf);
			int int_num = r->get_router_int_num_from_ip(toAddress.GetIpv4());
			Interface intf = r->get_router_int()[int_num];

			if(intf.status) {
				//MessageOpen msgToSend = MessageOpen(r->get_router_AS(), max_hold_time, r->get_router_ID());
				std::vector<Path_atrs> path_atr = buildPA(*r);
    			std::vector<NLRIs> nlri = buildNLRI(*r);

    			MessageUpdate msgToSend = MessageUpdate(path_atr.size(), path_atr, nlri);

				msgStream << msgToSend << '\0';
				Ptr<Packet> packet = Create<Packet>((uint8_t*) msgStream.str().c_str(), msgStream.str().length()+1);
				this->Send(socket,packet);
			} else {
				NS_LOG_INFO("Interface " << intf.name << " of router " << r->get_router_AS() << " is down [OPEN READ SERVER]");

				std::stringstream msgStream;
				MessageNotification msg = MessageNotification(6,0);
				msgStream << msg << '\0';

				Ptr<Packet> packet = Create<Packet>((uint8_t*) msgStream.str().c_str(), msgStream.str().length()+1);
				this->Send(socket, packet);

				intf.client.reset();
				intf.server.reset();

				r->set_interface_status(int_num, false);
				this->StopApplication();
			}

    		//intf.set_max_hold_time(max_hold_time);
			intf.set_last_update_time(Simulator::Now().GetSeconds());
			r->set_interface(intf, int_num);
		} else if(msg.get_type() == 3){ 
			MessageNotification msgRcv;
			std::stringstream(packet) >> msgRcv;

			std::cout << " NOTIFICATION message with content  ERROR CODE: " << msgRcv.get_error_code() << " \t ERROR SUBCODE: " << msgRcv.get_error_subcode() << " closing the TCP connection" << std::endl;

			// Stop the application in case the error code is 6 (Cease) or 4 (Hold time expired)
			/*if(msgRcv.get_error_code() == 6 || msgRcv.get_error_code() == 4) {
				//TODO: check why this broke everything 
				//this->StopApplication();
			}*/

		} else {
			NS_LOG_INFO("Received another type of message");
		}	

  	}

	void BGPServer::DoDispose (void){
		NS_LOG_FUNCTION (this);
		m_socket = 0;
		m_socketList.clear ();

		// chain up
		TCPCommon::DoDispose ();
	}

	/** Handle Peer Close
	 * Called by the TCP stack when the peer closes the connection
	 */
	void BGPServer::HandlePeerClose (Ptr<Socket> socket) {	
		NS_LOG_FUNCTION (this << socket);
	}

	void BGPServer::HandlePeerError (Ptr<Socket> socket) {
    	NS_LOG_FUNCTION (this << socket);
  	}

	/** Handle Accept
   * Function called as soon as there is an accept
   */
	void BGPServer::HandleAccept (Ptr<Socket> socket, const Address& from)
	{
		NS_LOG_FUNCTION (this << socket << from);

		socket->SetRecvCallback (
			MakeCallback (&BGPServer::HandleRead, this));

		// Saving socket in a list
		m_socketList.push_back (socket);

		// CloseCallback
		socket->SetCloseCallbacks(
			MakeCallback (&BGPServer::HandleSuccessClose, this),
			MakeNullCallback<void, Ptr<Socket> > ());
	}


	/** Handle Success Close
	 * This function is called as soon as a close pkt arrives
	 */
	void BGPServer::HandleSuccessClose(Ptr<Socket> socket)
	{
		NS_LOG_FUNCTION (this << socket);
		NS_LOG_LOGIC ("Client close received");

		socket->Close();
		socket->SetRecvCallback (
			MakeNullCallback<void, Ptr<Socket> > ());

		socket->SetCloseCallbacks(
			MakeNullCallback<void, Ptr<Socket> > (),
			MakeNullCallback<void, Ptr<Socket> > ());
	}

	Ptr<Socket> BGPServer::GetSocket(void) const {
		return m_socket;
	}


} //namespace ns3