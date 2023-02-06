// General includes
#include <fstream>
#include <iostream>
#include <sstream> 
#include <iomanip>
#include <cstring>
#include "ns3/config-store.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/tcp-header.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "../include/TCP-common.h"



namespace ns3 {
	NS_LOG_COMPONENT_DEFINE("TCPCommon");
	NS_OBJECT_ENSURE_REGISTERED(TCPCommon);

	// Constructor
	// Constructor
	TCPCommon::TCPCommon() {
		NS_LOG_FUNCTION (this);
	}

	// Destructor
	TCPCommon::~TCPCommon(){
		NS_LOG_FUNCTION (this);
	}

  	// Send function
	void TCPCommon::Send(Ptr<Socket> socket, Ptr<Packet> packet) {
		NS_LOG_FUNCTION_NOARGS();

		Time rightNow = Simulator::Now();

		// Get sending address
		Address from;
		socket->GetSockName(from);
		InetSocketAddress fromAddress = InetSocketAddress::ConvertFrom(from);

		// Get receiving address
		Address to;
		socket->GetPeerName(to);
		InetSocketAddress toAddress = InetSocketAddress::ConvertFrom(to);

		// TODO: remove
		// Debug only
		uint8_t *buffer = new uint8_t[packet->GetSize ()];
        packet->CopyData(buffer, packet->GetSize ());
        std::string packetData = std::string((char*)buffer);

		NS_LOG_INFO("[" << fromAddress.GetIpv4() 
					<< ":" << fromAddress.GetPort()
					<< "] sending packet '"
					<< packetData  
					<< "' at time "
					<< rightNow.GetSeconds() 
					<< " to " << toAddress.GetIpv4() 
					<< ":" << toAddress.GetPort()
					<< " of size "
					<< packet->GetSize());

		// Send
		socket->Send(packet);
    }

	std::string TCPCommon::HandleRead (Ptr<Socket> socket){
		NS_LOG_FUNCTION(this << socket);

		Ptr<Packet> packet;
		Address from, to;
		std::string packetData;

		// Get receiving address
		socket->GetSockName(to);
		InetSocketAddress toAddress = InetSocketAddress::ConvertFrom(to);

		while (packet = socket->RecvFrom(from)) {

			if (packet->GetSize () == 0) //EOF
				break;
			uint8_t *buffer = new uint8_t[packet->GetSize ()];
			packet->CopyData(buffer, packet->GetSize ());
			packetData = std::string((char*)buffer);

			Time rightNow = Simulator::Now() - m_startTime;

			InetSocketAddress fromAddress = InetSocketAddress::ConvertFrom (from);

			// Log print
			NS_LOG_INFO( "[" << toAddress.GetIpv4() 
						<< ":" << toAddress.GetPort()
						<< "] receiving packet '"
						<< packetData
						<< "' at time "
						<< rightNow.GetSeconds()
						<< " from "
						<< fromAddress.GetIpv4 ()
						<< ":" << fromAddress.GetPort ()  
						<< " of size: "
						<< packet->GetSize()
			);
    	}

		return packetData;
  	}

	void TCPCommon::DoDispose (void){
		NS_LOG_FUNCTION (this);
		
		// chain up
		Application::DoDispose ();
	}		

} // namespace ns3