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

		if (msg.get_type() == 0){
			//NS_LOG_INFO("Received KEEPALIVE message");
			std::cout << " KEEPALIVE message " << std::endl;
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
			int int_num = r->get_router_int_num_from_ip(toAddress.GetIpv4());
			Interface intf = r->get_router_int()[int_num];
			intf.set_max_hold_time(max((int)msgRcv.get_hold_time(), (int)intf.get_max_hold_time()));

			//NS_LOG_INFO("BGP state: OPEN CONFIRM");
			
			//Simulator::Schedule (Seconds(0), &Interface::increment_hold_time, &intf);
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

	void BGPClient::AddPacketsToQueue(std::stringstream& msg, Time atTime) {
		Ptr<Packet> packet = Create<Packet>((uint8_t*) msg.str().c_str(), msg.str().length()+1);
		m_pktDelayList.push_back(atTime);
		m_pktList.push_back(packet);
	}

	void BGPClient::AddPacketsToQueuePeriodically() {
		std::stringstream msgStream;
    	MessageHeader msg = MessageHeader(0);
    	msgStream << msg << '\0';

		Ptr<Packet> packet = Create<Packet>((uint8_t*) msgStream.str().c_str(), msgStream.str().length()+1);

		if(m_running) {
			Time atTime = Simulator::Now()+Seconds(1.5);
			//NS_LOG_INFO("Sending packet at time " << atTime.GetSeconds());
			m_sendEvent = Simulator::Schedule (atTime, &BGPClient::Send, this, m_socket, packet);
		}

		//NS_LOG_INFO("entro al tempo " << Simulator::Now().GetSeconds() << " e la coda è lunga: " << m_pktList.size() << " e la lista è lunga: " << m_pktDelayList.size());
		Simulator::Schedule(Seconds(15.0), &BGPClient::AddPacketsToQueuePeriodically, this);	
	}


} // namespace ns3