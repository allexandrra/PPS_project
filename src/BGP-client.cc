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
		// 0 -> KEEPALIVE
		// 1 -> OPEN
		// 2 -> UPDATE
		// 3 -> NOTIFICATION
		if (msg.get_type() == 0){

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

		} else if(msg.get_type() == 3){
			// unpack the packet into a real MessageNotification object as the type in the header is 3 (NOTIFICATION)
			MessageNotification msgRcv;
			std::stringstream(packet) >> msgRcv;

			std::cout << " NOTIFICATION message with content  ERROR CODE: " << msgRcv.get_error_code() << " \t ERROR SUBCODE: " << msgRcv.get_error_subcode() << " closing the TCP connection" << std::endl;

			// stop the application in case the error code is 6 (Cease) or 4 (Hold time expired)
			if(msgRcv.get_error_code() == 6 || msgRcv.get_error_code() == 4) {
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
					MessageHeader msg = MessageHeader(0);
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


} // namespace ns3