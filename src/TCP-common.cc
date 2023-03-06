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
#include "../include/MessageHeader.h"
#include "../include/MessageOpen.h"
#include "../include/MessageNotification.h"



namespace ns3 {
	NS_LOG_COMPONENT_DEFINE("TCPCommon");
	NS_OBJECT_ENSURE_REGISTERED(TCPCommon);

	// Constructor
	// Constructor
	TCPCommon::TCPCommon() {
		NS_LOG_FUNCTION (this);
		m_router = NULL;
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

		std::cout   << "SEND [FROM: " << fromAddress.GetIpv4() 
					<< ":" << fromAddress.GetPort()
					<< "] " //sending packet '"
					//<< packetData  
					<< "[TO: " << toAddress.GetIpv4() 
					<< ":" << toAddress.GetPort()
					<< "]" //of size "
					<< " at time "
					<< rightNow.GetSeconds();
					//<< packet->GetSize());

		
		MessageHeader msg;
		std::stringstream(packetData) >> msg;

		if (msg.get_type() == 0){
			std::cout << " KEEPALIVE message " << std::endl;
		}
		else if(msg.get_type() == 1){
			MessageOpen msgRcv;
			std::stringstream(packetData) >> msgRcv;

			std::cout << " OPEN message with content  AS: " << msgRcv.get_AS() << " \t HOLD TIME: " << msgRcv.get_hold_time() << "\t BGP ID: " <<  binaryToDottedNotation(msgRcv.get_BGP_id()) << std::endl;
		} else if(msg.get_type() == 3){
			MessageNotification msgRcv;
			std::stringstream(packetData) >> msgRcv;

			std::cout << " NOTIFICATION message with content  ERROR CODE: " << msgRcv.get_error_code() << " \t ERROR SUBCODE: " << msgRcv.get_error_subcode() << std::endl;
		} 

		// Send
		int bytesSent = socket->Send(packet);

		// Check for errors
		if (bytesSent == -1) {
			// An error occurred, handle it here
			NS_LOG_INFO("Error sending packet: " << socket->GetErrno());
		}
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
			std::cout << "RECV [TO: " << toAddress.GetIpv4() 
						<< ":" << toAddress.GetPort()
						<< "] "
						<< "[FROM: "
						<< fromAddress.GetIpv4 ()
						<< ":" << fromAddress.GetPort () << "] "
						<< " at time "
						<< rightNow.GetSeconds();
						//<< " of size: "
						//<< packet->GetSize();
    	}

		return packetData;
  	}

	void TCPCommon::DoDispose (void){
		NS_LOG_FUNCTION (this);
		
		// chain up
		Application::DoDispose ();
	}		


	void TCPCommon::SetRouter(Router *router) {
		NS_LOG_FUNCTION (this << router);
		this->m_router = router;
	}
	
	Router* TCPCommon::GetRouter() {
		NS_LOG_FUNCTION (this);
		return this->m_router;
	}


	std::string TCPCommon::binaryToDottedNotation(std::string ipBinary) {
		std::string dottedNotation;

		// Split the binary string into 4 octets
		std::string octet1 = ipBinary.substr(0, 8);
		std::string octet2 = ipBinary.substr(8, 8);
		std::string octet3 = ipBinary.substr(16, 8);
		std::string octet4 = ipBinary.substr(24, 8);

		// Convert each octet from binary to decimal
		int decimalOctet1 = std::stoi(octet1, nullptr, 2);
		int decimalOctet2 = std::stoi(octet2, nullptr, 2);
		int decimalOctet3 = std::stoi(octet3, nullptr, 2);
		int decimalOctet4 = std::stoi(octet4, nullptr, 2);

		// Concatenate the decimal values separated by dots
		dottedNotation = std::to_string(decimalOctet1) + "." + std::to_string(decimalOctet2) + "." + std::to_string(decimalOctet3) + "." + std::to_string(decimalOctet4);

		return dottedNotation;
	}


} // namespace ns3