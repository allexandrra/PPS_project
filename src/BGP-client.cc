// General includes
#include <fstream>
#include <iostream>
#include <iomanip>
#include <cstring> 

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

#include "../include/BGP-client.h"
#include "../include/Router.h"
#include "../include/MessageHeader.h"
#include "../include/MessageOpen.h"
#include "../include/MessageNotification.h"
#include "../include/MessageUpdate.h"
#include "../include/MessageTrustrate.h"

namespace ns3 {
	NS_LOG_COMPONENT_DEFINE("BGPClient");
  	NS_OBJECT_ENSURE_REGISTERED(BGPClient);

	/**
	 * @brief Constructor for BGPClient
	*/
	BGPClient::BGPClient () {
		m_socket = 0;
        m_running = false;
	}

	/**
	 * @brief Destructor for BGPClient
	*/
	BGPClient::~BGPClient() {
        m_socket = 0;
	}


	/**
	 * @brief Method to setup and initialize the BGPClient
	 * @param socket Socket to be used by the BGPClient
	 * @param address Address of the BGPClient
	 * @param startTime Time at which the BGPClient should start
	 * @param stopTime Time at which the BGPClient should stop
	*/
	void BGPClient::Setup (Ptr<Socket> socket, Address address, Time startTime, Time stopTime ){
		m_socket  = socket;
		m_peer    = address;
		m_start   = startTime;
		m_stop    = stopTime;
	}

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

	std::vector<NLRIs> buildNLRI(Router r, std::string not_send_ip) {
		std::vector<NLRIs> nlri;

		NLRIs new_nlri;

		for (Peer p : r.get_router_rt()) {
			if (p.next_hop != not_send_ip) {
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
		}

		return nlri;
	}

	std::vector<Path_atrs> buildPA(Router r, std::string not_send_ip) {
		std::vector<Path_atrs> path_atributes;

		for (Peer p : r.get_router_rt()) {
			// weight 1, loc pref 2, next hop 3, AS 4, MED 5, trust 6
			if (p.next_hop != not_send_ip) {
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

				Path_atrs atrib_trust;
				atrib_trust.type = 6;
				atrib_trust.lenght = 0;
				atrib_trust.value = to_string((int)(p.trust * 100));
				atrib_trust.optional = 0;
				atrib_trust.transitive = 0;
				atrib_trust.partial = 0;
				atrib_trust.extended_lenght = 0;

				path_atributes.push_back(atrib_w);
				path_atributes.push_back(atrib_lf);
				path_atributes.push_back(atrib_nh);
				path_atributes.push_back(atrib_as);
				path_atributes.push_back(atrib_med);
				path_atributes.push_back(atrib_trust);
			}
		}

		return path_atributes;
	}

	/**
	 * @brief Method to read the stream of data received by the socket
	 * @param socket Socket used to receive the data
	*/
	void BGPClient::HandleRead (Ptr<Socket> socket){
		NS_LOG_FUNCTION(this << socket);

		// call to the TCPCommon method to read the data
		// this method of the subclass is used to differentiate the action of the server from the action of the client
		std::string packet = TCPCommon::HandleRead(socket);

		// unpack the packet read from the socket into a MessageHeader object to understand the type of the message
		MessageHeader msg;
		std::stringstream(packet) >> msg;

		// router that is receiving the message
		Router *r = this->get_router();

		// interface of the router that is receiving the message
		Address to;
		socket->GetSockName(to);
		InetSocketAddress toAddress = InetSocketAddress::ConvertFrom(to);
		int int_num = r->get_router_int_num_from_ip(toAddress.GetIpv4());
		Interface intf = r->get_router_int()[int_num];

		// differentite the action to be taken based on the type of the message
		// 1 -> OPEN
		// 2 -> UPDATE
		// 3 -> NOTIFICATION
		// 4 -> KEEPALIVE
		// 5 -> TRUSTRATE
		if (msg.get_type() == 4){

			// check if the current time is greater than the last update time + the hold time
			// if it is, the interface has expired and the client should be stopped
			if(Simulator::Now().GetSeconds() - intf.get_last_update_time() <= intf.get_max_hold_time()) {
				std::cout << " KEEPALIVE message " << std::endl;

				// update the last update time because a keepalive message has been received
				intf.set_last_update_time(Simulator::Now().GetSeconds());
				r->set_interface(intf, int_num);

			} else {
				NS_LOG_INFO("Interface " << intf.name << " of router " << r->get_router_AS() << " has expired hold time  at time " << Simulator::Now().GetSeconds() << " [KEEPALIVE READ CLIENT]");

				//send notification message as the interface hold time has expired
				std::stringstream msgStreamNotification;
				MessageNotification msg = MessageNotification(6,0);
				msgStreamNotification << msg << '\0';

				Ptr<Packet> packetNotification = Create<Packet>((uint8_t*) msgStreamNotification.str().c_str(), msgStreamNotification.str().length()+1);
				this->Send(socket, packetNotification);

				// reset the client and server application pointers on the interface
				intf.client.reset();
				intf.server.reset();

				r->set_interface_status(int_num, false);
				r->set_interface(intf, int_num);

				// stop the application
				this->StopApplication();
			}
			
		} else if(msg.get_type() == 1){
			// unpack the packet into a real MessageOpen object as the type in the header is 1 (OPEN)
			MessageOpen msgRcv;
			std::stringstream(packet) >> msgRcv;

			std::cout << " OPEN message with content  AS: " << msgRcv.get_AS() << " \t HOLD TIME: " << msgRcv.get_hold_time() << "\t BGP ID: " <<  binary_to_dotted_notation(msgRcv.get_BGP_id()) << std::endl;

			//set the max hold time of the interface based on the value prefered by the receiving client (90) and the value received in the OPEN message
			intf.set_max_hold_time(max((int)msgRcv.get_hold_time(), (int)intf.get_max_hold_time()));
			intf.set_last_update_time(Simulator::Now().GetSeconds());
			r->set_interface(intf, int_num);

			// schedule the first keepalive message to be sent
			// events is a list of all the events scheduled for the periodical sending of the keepalive messages
			std::vector<EventId> events;
			intf.client.value()->AddPacketsToQueuePeriodically(events);
			std::vector<EventId> trustEvents;
			intf.client.value()->exchangeTrust(trustEvents);

			//send initial update message
			std::stringstream msgStreamUpdate;
			std::vector<Path_atrs> path_atr = buildPA(*r, r->make_string_from_IP(toAddress.GetIpv4()));
			std::vector<NLRIs> nlri = buildNLRI(*r, r->make_string_from_IP(toAddress.GetIpv4()));

			MessageUpdate msg = MessageUpdate(path_atr.size(), path_atr, nlri);
			msgStreamUpdate << msg << "\0";

			Ptr<Packet> packetUpdate = Create<Packet>((uint8_t*) msgStreamUpdate.str().c_str(), msgStreamUpdate.str().length()+1);
			this->Send(socket, packetUpdate);

		} else if(msg.get_type() == 2){
			MessageUpdate msgRcv;
			std::stringstream(packet) >> msgRcv;

			std::cout << " Client UPDATE message with " << msgRcv.get_unfeasable_route_len() << " routes to remove and " << 
				msgRcv.get_total_path_atr_len()/6 << " new routes."<< std::endl;

			std::vector<NLRIs> new_nlri;
			std::vector<Path_atrs> new_pa;
			std::vector<NLRIs> new_wr;

			if (msgRcv.get_unfeasable_route_len() > 0) {
				new_wr = r->remove_routes_if_necessary(msgRcv.get_withdrawn_routes(), r->make_string_from_IP(intf.ip_address));
			}

			if(msgRcv.get_total_path_atr_len() > 0) {
				std::vector<Route> ribIn = msgRcv.add_to_RIBin(msgRcv.get_path_atr(), msgRcv.get_NLRI());
				std::vector<Route> locRib = msgRcv.check_preferences(ribIn, r->get_router_rt());
					
				for (int i = 0; i < (int)locRib.size(); i++) {
					for (int j = 0; j < (int)locRib[i].path_atr.size(); j++) {
						if (locRib[i].path_atr[j].type == 4) {
							locRib[i].path_atr[j].value.append("");
							locRib[i].path_atr[j].value.append(std::to_string(r->get_router_AS()));
							locRib[i].path_atr[j].lenght += 1;
						}
						if (locRib[i].path_atr[j].type == 3) {
							locRib[i].path_atr[j].value = r->make_string_from_IP(intf.ip_address);
						}
						new_pa.push_back(locRib[i].path_atr[j]);
					}
					new_nlri.push_back(locRib[i].nlri);
				}
					
				Address from;
				socket->GetPeerName(from);
				InetSocketAddress fromAddress = InetSocketAddress::ConvertFrom(from);
				r->add_to_RT(locRib, r->make_string_from_IP(fromAddress.GetIpv4()));
			}
			
			std::stringstream msgStream;

			if(intf.status) {
				MessageUpdate msgToSend;

				if (new_wr.size() > 0 || new_pa.size() > 0) {
					if (new_wr.size() > 0 && new_pa.size() > 0 && new_wr.size() < msgRcv.get_unfeasable_route_len()) {
						msgToSend = MessageUpdate(new_wr.size(), new_wr, new_pa.size(), new_pa, new_nlri);
					} else if (new_wr.size() > 0 && new_wr.size() < msgRcv.get_unfeasable_route_len()) {
						msgToSend = MessageUpdate(new_wr.size(), new_wr);
					} else if (new_pa.size() > 0) {
						msgToSend = MessageUpdate(new_pa.size(), new_pa, new_nlri);
					}

					msgStream << msgToSend << '\0';
					Ptr<Packet> packet = Create<Packet>((uint8_t*) msgStream.str().c_str(), msgStream.str().length()+1);
					this->Send(socket,packet);

				} else {
					NS_LOG_INFO("No new updates to send.");
				}
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
			// unpack the packet into a real MessageNotification object as the type in the header is 3 (NOTIFICATION)
			MessageNotification msgRcv;
			std::stringstream(packet) >> msgRcv;

			std::cout << " NOTIFICATION message with content  ERROR CODE: " << msgRcv.get_error_code() << " \t ERROR SUBCODE: " << msgRcv.get_error_subcode() << " closing the TCP connection" << std::endl;

			// stop the application in case the error code is 6 (Cease) or 4 (Hold time expired)
			if(msgRcv.get_error_code() == 6 || msgRcv.get_error_code() == 4) {
				this->StopApplication();
			}
			
		} else if(msg.get_type() == 5) {
			// check if the current time is greater than the last update time + the hold time
			// if it is, the interface has expired and the client should be stopped
			if(Simulator::Now().GetSeconds() - intf.get_last_update_time() <= intf.get_max_hold_time()) {

				MessageTrustrate msgRcv;
				std::stringstream(packet) >> msgRcv;

				std::cout << " TRUSTRATE message with content TRUST: " <<  msgRcv.get_trust() << std::endl;

				// update the last update time because a trust message has been received
				intf.set_last_update_time(Simulator::Now().GetSeconds());
				r->set_interface(intf, int_num);

				if(intf.voted_trust == 0) {
					if(intf.direct_trust == 0) {
						// Initialize the trust values
						// The two values of trust are weighted in the same way(50% each)
						// The value of observed trust is initialized to 0.5 as the communication between the two interfaces is not yet established						
						float observed_trust = 0.5;
						
						// New trust value = (1 - α) * Existing trust value + α * New trust value
						// α = 0.3
						/*std::cout << "No voted and direct trust yet. Received value: " << msgRcv.get_trust() 
								  << " - old inherited trust value: "  << intf.inherited_trust 
								  << " - observed trust value: " << observed_trust;*/

						intf.inherited_trust = (1 - 0.3) * intf.inherited_trust + 0.3 * msgRcv.get_trust();

						//std::cout << " - new inherited trust value: " << intf.inherited_trust;
						// Weighted average of the two trust values	
						// The weight of the two values is 50% each

						intf.direct_trust = intf.inherited_trust * 0.5 + observed_trust * 0.5;
						
						//std::cout << " - new direct trust value: " << intf.direct_trust << std::endl
						//		  << std::endl;
					} else {
						// New trust value = (1 - α) * Existing trust value + α * New trust value
						// α = 0.3
						//std::cout << "No voted trust yet. Received value: " << msgRcv.get_trust() 
						//		  << " - old direct trust value: "  << intf.direct_trust;

						intf.direct_trust = (1 - 0.3) * intf.direct_trust + 0.3 * msgRcv.get_trust();
						
						//std::cout << " - new direct trust value: "  << intf.direct_trust 
						//		  << std::endl;
					} 

					intf.total_trust = intf.direct_trust;
				} else {
					/*std::cout << "Value received: " << msgRcv.get_trust() 
							  << " - old direct trust value: "  << intf.direct_trust 
							  << " - old voted trust value: " << intf.voted_trust 
							  << " - old total trust value: " << intf.total_trust; */

					intf.direct_trust = (1 - 0.3) * intf.direct_trust + 0.3 * msgRcv.get_trust();

					// αTd +(1−α)V where where Td = ω1 ∗ It +ω2 ∗ Ot
					// α = 0.4
					intf.total_trust = 0.4 * intf.direct_trust + (1 - 0.4) * intf.voted_trust;
					
					/*std::cout << " - new direct trust value: "  << intf.direct_trust 
							  << " - new total trust value: " << intf.total_trust
						      << std::endl; */
				}

				// update the interface	
				r->set_interface(intf, int_num);

				// TODO: update the routing table using the value of total trust
				//std::cout <<"\n\n ajunf aici vreodata? \n\n";
				r->update_trust(r->make_string_from_IP(intf.ip_address), int_num);
				

			} else {
				NS_LOG_INFO("Interface " << intf.name << " of router " << r->get_router_AS() << " has expired hold time  at time " << Simulator::Now().GetSeconds() << " [TRUST READ CLIENT]");

				//send notification message as the interface hold time has expired
				std::stringstream msgStreamNotification;
				MessageNotification msg = MessageNotification(6,0);
				msgStreamNotification << msg << '\0';

				Ptr<Packet> packetNotification = Create<Packet>((uint8_t*) msgStreamNotification.str().c_str(), msgStreamNotification.str().length()+1);
				this->Send(socket, packetNotification);

				// reset the client and server application pointers on the interface
				intf.client.reset();
				intf.server.reset();

				r->set_interface_status(int_num, false);
				r->set_interface(intf, int_num);

				// stop the application
				this->StopApplication();
			}

		} else {
			NS_LOG_INFO("Received another type of message");
		}
  	}


	/**
	 * @brief Method to start the client application
	*/
	void BGPClient::StartApplication (void)
	{
		NS_LOG_FUNCTION(this);

		// initialize the socket
		m_running = true;

		// Create the socket
		if( InetSocketAddress::IsMatchingType(m_peer) || PacketSocketAddress::IsMatchingType(m_peer)) {
			m_socket->Bind();
		}

		// setup connection
		m_socket->Connect(m_peer);
		m_socket->SetAllowBroadcast(true);

		// set the callbacks for the different phases of the connection
		m_socket->SetConnectCallback (
			MakeCallback (&BGPClient::ConnectionSucceeded, this),
			MakeCallback (&BGPClient::ConnectionFailed, this));

		// set callback since the client receives feedback packets
		m_socket->SetRecvCallback (
			MakeCallback (&BGPClient::HandleRead, this));

		// function used to send the packets already scheduled in the queue
		SendMyPackets();
	}


	/**
	 * @brief Method to stop the client application
	*/
	void BGPClient::StopApplication (void) {
		// setting the running to false
		m_running = false;

		// cancel the last event of sending the packets
		if (m_sendEvent.IsRunning()) {
			Simulator::Cancel(m_sendEvent);
		}

		// close the socket
		if (m_socket) {
			m_socket->Close();
		}
	}



	/**
	 * @brief Callback function called when the a successful connection is established
	 * @param socket the socket of the connection
	*/
	void BGPClient::ConnectionSucceeded (Ptr<Socket> socket) {
		NS_LOG_FUNCTION (this << socket);
		m_running = true;
	}


	/**
	 * @brief Callback function called when the connection fails
	 * @param socket the socket of the connection
	*/
	void BGPClient::ConnectionFailed (Ptr<Socket> socket) {
		NS_LOG_FUNCTION (this << socket);
	}


	/**
	 * @brief Method to send a packet to the server
	*/
	void BGPClient::SendMyPackets(){

		//send all the packets in the queue
		while(!m_pktDelayList.empty()){

			Time atTime = m_pktDelayList.front();
			m_pktDelayList.pop_front();

			Ptr<Packet> packet = m_pktList.front();
			m_pktList.pop_front();

			// check if the socket is still running
			if(m_running) {
				// schedule the sending of the packet
				m_sendEvent = Simulator::Schedule (atTime, &BGPClient::Send, this, m_socket, packet);
			}
		}
	}


	/**
	 * @brief Method to return the socket of the client
	 * @return the socket of the client
	 */
	Ptr<Socket> BGPClient::get_socket(void) const {
		return m_socket;
	}

	/**
	 * @brief Method to add new packets to the queue, starting from a msg stream and a time
	 * @param msg the message stream that will be converted into a packet
	 * @param atTime the time at which the packet will be sent
	*/
	void BGPClient::AddPacketsToQueue(std::stringstream& msg, Time atTime) {
		Ptr<Packet> packet = Create<Packet>((uint8_t*) msg.str().c_str(), msg.str().length()+1);
		m_pktDelayList.push_back(atTime);
		m_pktList.push_back(packet);
	}

	/**
	 * @brief Method to add a new KeepAlive packets to the queue after 1/3 of the hold time
	 * @param events the list of all the events scheduled for the periodical sending of the keepalive messages
	*/
	void BGPClient::AddPacketsToQueuePeriodically(std::vector<EventId> events) {
		
		// get the information about the interface and the router where the client is running
		Router *r = this->get_router();
		Address to;
		m_socket->GetSockName(to);
		InetSocketAddress toAddress = InetSocketAddress::ConvertFrom(to);
		int int_num = r->get_router_int_num_from_ip(toAddress.GetIpv4());
		Interface intf = r->get_router_int()[int_num];

		// check if the socket is still running
		if(m_running) {

			// check if the hold time is expired
			if(Simulator::Now().GetSeconds() - intf.get_last_update_time() <= intf.get_max_hold_time()) {

				// check if the interface is up
				if(intf.status) {

					// create the keepalive message
					std::stringstream msgStream;
					MessageHeader msg = MessageHeader(4);
					msgStream << msg << '\0';

					Ptr<Packet> packet = Create<Packet>((uint8_t*) msgStream.str().c_str(), msgStream.str().length()+1);

					// add a small dealy in the time used for the scheduling of the packet
					Time atTime = Simulator::Now()+Seconds(1.5);

					// schedule the sending of the packet and save the event id
					m_sendEvent = Simulator::Schedule (atTime, &BGPClient::Send, this, m_socket, packet);	
					events.push_back(m_sendEvent);

					// schedule the next sending of the keepalive message 
					Simulator::Schedule(Seconds(intf.get_max_hold_time()/6), &BGPClient::AddPacketsToQueuePeriodically, this, events);
					
				} else {

					// if the interface is down, send a notification message
					NS_LOG_INFO("[KEEPALIVE] Interface " << intf.name << " of router " << r->get_router_AS() << " is down, sending a notification message");

					std::stringstream msgStream;
					MessageNotification msg = MessageNotification(6,0);
					msgStream << msg << '\0';

					Ptr<Packet> packet = Create<Packet>((uint8_t*) msgStream.str().c_str(), msgStream.str().length()+1);
					this->Send(m_socket, packet);

					// cancel all the events scheduled for the periodical sending of the keepalive messages
					for (int i = 0; i < (int) events.size(); i++){
						if (!Simulator::IsExpired(events[i])) {
							Simulator::Cancel(events[i]);
						}
					}

					// reset the interface and update the interface status
					intf.client.reset();
					intf.server.reset();
					r->set_interface_status(int_num, false);
					r->set_interface(intf, int_num);
				}			
			} else {

				// if the hold time is expired, send a notification message
				NS_LOG_INFO("Hold time expired on interface " << intf.name << " of router " << r->get_router_AS() << " at time " << Simulator::Now().GetSeconds() << " [SCHEDULE PERIODIC KEEPALIVE]");
		
				std::stringstream msgStreamNotification;
				MessageNotification msg = MessageNotification(4,0);
				msgStreamNotification << msg << '\0';

				Ptr<Packet> packetNotification = Create<Packet>((uint8_t*) msgStreamNotification.str().c_str(), msgStreamNotification.str().length()+1);
				this->Send(m_socket, packetNotification);

				for (int i = 0; i < (int) events.size(); i++){
					if (!Simulator::IsExpired(events[i])) {
						Simulator::Cancel(events[i]);
					}
				}

				// reset the interface and update the interface status
				intf.client.reset();
				intf.server.reset();
				r->set_interface_status(int_num, false);
				r->set_interface(intf, int_num);
				this->StopApplication();
			}
		}

	}


	/**
	 * @brief Method to add a new trust packet to the queue every 2 minutes
	 * @param events the list of all the events scheduled for the periodical sending of the trust messages
	*/
	void BGPClient::exchangeTrust(std::vector<EventId> events) {
		
		// get the information about the interface and the router where the client is running
		Router *r = this->get_router();
		Address to;
		m_socket->GetSockName(to);
		InetSocketAddress toAddress = InetSocketAddress::ConvertFrom(to);
		int int_num = r->get_router_int_num_from_ip(toAddress.GetIpv4());
		Interface intf = r->get_router_int()[int_num];

		// check if the socket is still running
		if(m_running) {

			// check if the hold time is expired
			if(Simulator::Now().GetSeconds() - intf.get_last_update_time() <= intf.get_max_hold_time()) {

				// check if the interface is up
				if(intf.status) {

					// create the keepalive message
					std::stringstream msgStream;
					MessageTrustrate msg;

					if(intf.voted_trust == 0) {
						if (intf.direct_trust == 0) {
							// the first time, when the trust is not yet initialized, the trust value sent to the peer in only the inherited trust
							//NS_LOG_INFO("Inh trust: " << intf.inherited_trust);
							msg = MessageTrustrate(intf.inherited_trust);
						} else {
							// the other times the trust value is both the inherited trust and the observed trust
							msg = MessageTrustrate(intf.direct_trust);
						}
					} else {
						// the other times the trust value is both the inherited trust and the observed trust
						msg = MessageTrustrate(intf.total_trust);
					}
					
					msgStream << msg << '\0';

					Ptr<Packet> packet = Create<Packet>((uint8_t*) msgStream.str().c_str(), msgStream.str().length()+1);

					// add a small dealy in the time used for the scheduling of the packet
					Time atTime = Simulator::Now()+Seconds(10);

					// schedule the sending of the packet and save the event id
					m_sendEvent = Simulator::Schedule (atTime, &BGPClient::Send, this, m_socket, packet);	
					events.push_back(m_sendEvent);

					// schedule the next sending of the message 
					Simulator::Schedule(Seconds(60.0), &BGPClient::exchangeTrust, this, events);
					
				} else {

					// if the interface is down, send a notification message
					NS_LOG_INFO("[TRUSTRATE] Interface " << intf.name << " of router " << r->get_router_AS() << " is down, sending a notification message");

					std::stringstream msgStream;
					MessageNotification msg = MessageNotification(6,0);
					msgStream << msg << '\0';

					Ptr<Packet> packet = Create<Packet>((uint8_t*) msgStream.str().c_str(), msgStream.str().length()+1);
					this->Send(m_socket, packet);

					// cancel all the events scheduled for the periodical sending of the keepalive messages
					for (int i = 0; i < (int) events.size(); i++){
						if (!Simulator::IsExpired(events[i])) {
							Simulator::Cancel(events[i]);
						}
					}

					// reset the interface and update the interface status
					intf.client.reset();
					intf.server.reset();
					r->set_interface_status(int_num, false);
					r->set_interface(intf, int_num);
				}			
			} else {

				// if the hold time is expired, send a notification message
				NS_LOG_INFO("Hold time expired on interface " << intf.name << " of router " << r->get_router_AS() << " at time " << Simulator::Now().GetSeconds() << " [SCHEDULE PERIODIC TRUSTRATE]");
		
				std::stringstream msgStreamNotification;
				MessageNotification msg = MessageNotification(4,0);
				msgStreamNotification << msg << '\0';

				Ptr<Packet> packetNotification = Create<Packet>((uint8_t*) msgStreamNotification.str().c_str(), msgStreamNotification.str().length()+1);
				this->Send(m_socket, packetNotification);

				for (int i = 0; i < (int) events.size(); i++){
					if (!Simulator::IsExpired(events[i])) {
						Simulator::Cancel(events[i]);
					}
				}

				// reset the interface and update the interface status
				intf.client.reset();
				intf.server.reset();
				r->set_interface_status(int_num, false);
				r->set_interface(intf, int_num);
				this->StopApplication();
			}
		}

	}


} // namespace ns3