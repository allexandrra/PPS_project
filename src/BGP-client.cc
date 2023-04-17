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

namespace ns3 {
	NS_LOG_COMPONENT_DEFINE("BGPClient");
  	NS_OBJECT_ENSURE_REGISTERED(BGPClient);

	// Constructor
	BGPClient::BGPClient () {
		m_socket = 0;
        m_running = false;
	}

	BGPClient::~BGPClient() {
        m_socket = 0;
	}


	/** Setup
   	* Function that allows the initial setup
    */
	void BGPClient::Setup (Ptr<Socket> socket, Address address, Time startTime, Time stopTime ){
		m_socket  = socket;
		m_peer    = address;
		m_start   = startTime;
		m_stop    = stopTime;
	}

	void BGPClient::HandleRead (Ptr<Socket> socket){
		NS_LOG_FUNCTION(this << socket);

		std::string packet = TCPCommon::HandleRead(socket);

		MessageHeader msg;
		std::stringstream(packet) >> msg;

		Router *r = this->GetRouter();
		Address to;
		socket->GetSockName(to);
		InetSocketAddress toAddress = InetSocketAddress::ConvertFrom(to);
		int int_num = r->get_router_int_num_from_ip(toAddress.GetIpv4());
		Interface intf = r->get_router_int()[int_num];

		//NS_LOG_INFO("entro");

		if (msg.get_type() == 0){
			//NS_L5OG_INFO("Received KEEPALIVE message");

			if(Simulator::Now().GetSeconds() - intf.get_last_update_time() <= intf.get_max_hold_time()) {
				std::cout << " KEEPALIVE message " << std::endl;
				//std::cout << this->GetRouter()->get_router_AS() << std::endl;
				intf.set_last_update_time(Simulator::Now().GetSeconds());
				r->set_interface(intf, int_num);
				//NS_LOG_INFO("Interface " << intf.name << " of router " << r->get_router_AS() << " has now start time equal to " << intf.get_start_time() << " [KEEPALIVE READ CLIENT]");
				//NS_LOG_INFO("Interface " << intf.name << " of router " << r->get_router_AS() << " has status " << intf.status << " and has client " << intf.client.has_value() << " and has server " << intf.server.has_value() <<  " at time " << Simulator::Now().GetSeconds() << " [CLIENT KEEPALIVE READ]");
			} else {
				NS_LOG_INFO("Interface " << intf.name << " of router " << r->get_router_AS() << " has expired hold time  at time " << Simulator::Now().GetSeconds() << " [KEEPALIVE READ CLIENT]");

				//send notification msg
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
			
		} else if(msg.get_type() == 1){
			//NS_LOG_INFO("Received OPEN message");
			MessageOpen msgRcv;
			std::stringstream(packet) >> msgRcv;

			std::cout << " OPEN message with content  AS: " << msgRcv.get_AS() << " \t HOLD TIME: " << msgRcv.get_hold_time() << "\t BGP ID: " <<  binaryToDottedNotation(msgRcv.get_BGP_id()) << std::endl;

			
			intf.set_max_hold_time(max((int)msgRcv.get_hold_time(), (int)intf.get_max_hold_time()));
			intf.set_last_update_time(Simulator::Now().GetSeconds());
			r->set_interface(intf, int_num);

			//NS_LOG_INFO("BGP state: OPEN CONFIRM");
			//Simulator::Schedule (Seconds(0), &Interface::increment_hold_time, &intf);

			std::vector<EventId> events;
			intf.client.value()->AddPacketsToQueuePeriodically(events);

		} else if(msg.get_type() == 2){
			MessageUpdate msgRcv;
			std::stringstream(packet) >> msgRcv;

			std::cout << " UPDATE message " << std::endl;

			std::vector<Route> ribIn = msgRcv.add_to_RIBin(msgRcv.get_path_atr(), msgRcv.get_NLRI());
			std::vector<Route> locRib = msgRcv.check_preferences(ribIn, r->get_router_rt());
			msgRcv.add_to_RT(*r, locRib);
		} else if(msg.get_type() == 3){
			MessageNotification msgRcv;
			std::stringstream(packet) >> msgRcv;

			std::cout << " NOTIFICATION message with content  ERROR CODE: " << msgRcv.get_error_code() << " \t ERROR SUBCODE: " << msgRcv.get_error_subcode() << " closing the TCP connection" << std::endl;

			// Stop the application in case the error code is 6 (Cease) or 4 (Hold time expired)
			if(msgRcv.get_error_code() == 6 || msgRcv.get_error_code() == 4) {
				this->StopApplication();
			}
			
		} else {
			NS_LOG_INFO("Received another type of message");
		}
  	}


	/** Start Application
 	* Function that starts the TCP Client application
	*/
	void BGPClient::StartApplication (void)
	{
		NS_LOG_FUNCTION(this);

		// Initialize
		m_running = true;

		// Create the socket
		if( InetSocketAddress::IsMatchingType(m_peer) || PacketSocketAddress::IsMatchingType(m_peer)) {
			m_socket->Bind();
		}

		// Setup connection
		m_socket->Connect(m_peer);
		m_socket->SetAllowBroadcast(true);

		m_socket->SetConnectCallback (
			MakeCallback (&BGPClient::ConnectionSucceeded, this),
			MakeCallback (&BGPClient::ConnectionFailed, this));

		// Set callback since the client receives feedback packets
		m_socket->SetRecvCallback (
			MakeCallback (&BGPClient::HandleRead, this));

		//NS_LOG_INFO("BGP Client started at time " << Simulator::Now());

		SendMyPackets();
	}


	/** Stop Application
   * Function to stop the application called by simulator from main()
   */
	void BGPClient::StopApplication (void) {
		//NS_LOG_INFO("Stopping BGP Client Application");
		m_running = false;

		if (m_sendEvent.IsRunning()) {
			Simulator::Cancel(m_sendEvent);
		}

		if (m_socket) {
			m_socket->Close();
		}
	}



	/** Service functions Connection Succeeded and failed
   */
	void BGPClient::ConnectionSucceeded (Ptr<Socket> socket) {
		NS_LOG_FUNCTION (this << socket);
		m_running = true;
	}


	void BGPClient::ConnectionFailed (Ptr<Socket> socket) {
		NS_LOG_FUNCTION (this << socket);
	}


	void BGPClient::SendMyPackets(){
		while(!m_pktDelayList.empty()){

			//NS_LOG_INFO("List lenght: " << m_pktDelayList.size());

			Time atTime = m_pktDelayList.front();
			m_pktDelayList.pop_front();
			//NS_LOG_INFO("Sending packet at time " << atTime << " with queue size " << m_pktList.size());

			Ptr<Packet> packet = m_pktList.front();
			m_pktList.pop_front();

			if(m_running) {
				//NS_LOG_INFO("Sending packet at time " << atTime);
				m_sendEvent = Simulator::Schedule (atTime, &BGPClient::Send, this, m_socket, packet);
			}
		}

		//NS_LOG_INFO("Sent");
	}

	Ptr<Socket> BGPClient::GetSocket(void) const {
		return m_socket;
	}

	void BGPClient::AddPacketsToQueue(std::stringstream& msg, Time atTime) {
		Ptr<Packet> packet = Create<Packet>((uint8_t*) msg.str().c_str(), msg.str().length()+1);
		m_pktDelayList.push_back(atTime);
		m_pktList.push_back(packet);
	}

	void BGPClient::AddPacketsToQueuePeriodically(std::vector<EventId> events) {
		Router *r = this->GetRouter();
		Address to;
		m_socket->GetSockName(to);
		InetSocketAddress toAddress = InetSocketAddress::ConvertFrom(to);
		int int_num = r->get_router_int_num_from_ip(toAddress.GetIpv4());
		Interface intf = r->get_router_int()[int_num];

		//NS_LOG_INFO("ciaoooo");

		if(m_running) {

			//NS_LOG_INFO("Interface " << intf.name << " of router " << r->get_router_AS() << " has status " << intf.status << " [KEEPALIVE]");
			if(Simulator::Now().GetSeconds() - intf.get_last_update_time() <= intf.get_max_hold_time()) {
				if(intf.status) {
					std::stringstream msgStream;
					MessageHeader msg = MessageHeader(0);
					msgStream << msg << '\0';

					Ptr<Packet> packet = Create<Packet>((uint8_t*) msgStream.str().c_str(), msgStream.str().length()+1);

					Time atTime = Simulator::Now()+Seconds(1.5);
					//NS_LOG_INFO("Sending packet at time " << atTime.GetSeconds() << " from intf " << intf.name);
					m_sendEvent = Simulator::Schedule (atTime, &BGPClient::Send, this, m_socket, packet);	
					events.push_back(m_sendEvent);
					//NS_LOG_INFO("Events length: " << events.size());
					//std::cout << "Event id" << m_sendEvent.GetUid() << std::endl;
					//NS_LOG_INFO("entro al tempo " << Simulator::Now().GetSeconds() << " e la coda è lunga: " << m_pktList.size() << " e la lista è lunga: " << m_pktDelayList.size());
					//TODO: mettere 1/6 il tempo di hold time 
					Simulator::Schedule(Seconds(intf.get_max_hold_time()/6), &BGPClient::AddPacketsToQueuePeriodically, this, events);
					//NS_LOG_INFO("[CLIENT] Now: " << Simulator::Now().GetSeconds() << " " << " start time " << intf.get_start_time() << " max hold time " << intf.get_max_hold_time());
					//NS_LOG_INFO("Interface " << intf.name << " of router " << r->get_router_AS() << " has status " << intf.status << " and has client " << intf.client.has_value() << " and has server " << intf.server.has_value() <<  " at time " << Simulator::Now().GetSeconds() << " [CLIENT KEEPALIVE SCHEDULE]");

				} else {

					NS_LOG_INFO("[KEEPALIVE] Interface " << intf.name << " of router " << r->get_router_AS() << " is down, sending a notification message");

					std::stringstream msgStream;
					MessageNotification msg = MessageNotification(6,0);
					msgStream << msg << '\0';

					Ptr<Packet> packet = Create<Packet>((uint8_t*) msgStream.str().c_str(), msgStream.str().length()+1);
					//this->AddPacketsToQueue(msgStream, Simulator::Now());
					//m_sendEvent = Simulator::Schedule (Simulator::Now(), &BGPClient::Send, this, m_socket, packet);
					this->Send(m_socket, packet);

					//m_running = false;
					//m_socket = 0;

					for (int i = 0; i < (int) events.size(); i++)
					{
						if (!Simulator::IsExpired(events[i])) {
							Simulator::Cancel(events[i]);
						}
					}

					//m_pktDelayList.clear();
					//m_pktList.clear();

					intf.client.reset();
					intf.server.reset();

					//this->StopApplication();

					r->set_interface_status(int_num, false);
					r->set_interface(intf, int_num);
				}			
			} else {
				NS_LOG_INFO("Hold time expired on interface " << intf.name << " of router " << r->get_router_AS() << " at time " << Simulator::Now().GetSeconds() << " [SCHEDULE PERIODIC KEEPALIVE]");
		
				std::stringstream msgStreamNotification;
				MessageNotification msg = MessageNotification(4,0);
				msgStreamNotification << msg << '\0';

				Ptr<Packet> packetNotification = Create<Packet>((uint8_t*) msgStreamNotification.str().c_str(), msgStreamNotification.str().length()+1);
				this->Send(m_socket, packetNotification);

				for (int i = 0; i < (int) events.size(); i++)
				{
					if (!Simulator::IsExpired(events[i])) {
						Simulator::Cancel(events[i]);
					}
				}

				intf.client.reset();
				intf.server.reset();

				r->set_interface_status(int_num, false);
				r->set_interface(intf, int_num);
				this->StopApplication();
			}
		}

	}


} // namespace ns3