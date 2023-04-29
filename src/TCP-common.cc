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
#include "../include/MessageUpdate.h"
#include "../include/MessageTrustrate.h"



namespace ns3 {
	NS_LOG_COMPONENT_DEFINE("TCPCommon");
	NS_OBJECT_ENSURE_REGISTERED(TCPCommon);

	/**
	 * @brief Constructor for the TCPCommon class
	*/
	TCPCommon::TCPCommon() {
		NS_LOG_FUNCTION (this);
		m_router = NULL;
	}

	/**
	 * @brief Destructor for the TCPCommon class
	*/
	TCPCommon::~TCPCommon(){
		NS_LOG_FUNCTION (this);
	}

  	/**
	 * @brief Function used to send a packet through a socket
	 * @param socket The socket used to send the packet
	 * @param packet The packet to send
	*/
	void TCPCommon::Send(Ptr<Socket> socket, Ptr<Packet> packet) {
		NS_LOG_FUNCTION_NOARGS();

		// Get current time of the simulation
		Time rightNow = Simulator::Now();

		// Get sending address
		Address from;
		socket->GetSockName(from);
		InetSocketAddress fromAddress = InetSocketAddress::ConvertFrom(from);

		// Get receiving address
		Address to;
		socket->GetPeerName(to);
		InetSocketAddress toAddress = InetSocketAddress::ConvertFrom(to);

		// Get the packet data (useful to print information about the packet that we will send)
		uint8_t *buffer = new uint8_t[packet->GetSize ()];
        packet->CopyData(buffer, packet->GetSize ());
        std::string packetData = std::string((char*)buffer);

		// Print information about the packet that we will send
		// If we want to see the binary data of the packet, we can use uncomment the line below
		std::cout   << "SEND [FROM: " << fromAddress.GetIpv4() 
					<< ":" << fromAddress.GetPort()
					<< "] " //sending packet '"
					//<< packetData  
					<< "[TO: " << toAddress.GetIpv4() 
					<< ":" << toAddress.GetPort()
					<< "]"
					<< " at time "
					<< rightNow.GetSeconds();

		MessageHeader msg;
		std::stringstream(packetData) >> msg;

		// Print different information about the packet that we will send based on the type of the packet
		if (msg.get_type() == 4){
			std::cout << " KEEPALIVE message " << std::endl;
		}
		else if(msg.get_type() == 1){
			MessageOpen msgRcv;
			std::stringstream(packetData) >> msgRcv;

			std::cout << " OPEN message with content  AS: " << msgRcv.get_AS() << " \t HOLD TIME: " << msgRcv.get_hold_time() << "\t BGP ID: " <<  binary_to_dotted_notation(msgRcv.get_BGP_id()) << std::endl;
		}else if(msg.get_type() == 2) {
			MessageUpdate msgRcv;
			std::stringstream(packetData) >> msgRcv;

			std::cout << " UPDATE message with " << msgRcv.get_unfeasable_route_len() << " routes to remove and " << 
				msgRcv.get_total_path_atr_len()/6 << " new routes."<< std::endl;
		} else if(msg.get_type() == 3){
			MessageNotification msgRcv;
			std::stringstream(packetData) >> msgRcv;

			std::cout << " NOTIFICATION message with content  ERROR CODE: " << msgRcv.get_error_code() << " \t ERROR SUBCODE: " << msgRcv.get_error_subcode() << std::endl;
		} else if (msg.get_type() == 5){
			MessageTrustrate msgRcv;
			std::stringstream(packetData) >> msgRcv;
			std::cout << " TRUSTRATE message with content TRUST: " << msgRcv.get_trust() << std::endl;
		} 

		// Send the packet
		int bytesSent = socket->Send(packet);
		//std::cout << " and sent " << bytesSent << " bytes" << std::endl;

		// Check for errors
		if (bytesSent == -1) {
			// An error occurred, handle it here
			NS_LOG_INFO("Error sending packet: " << socket->GetErrno());
		}
    }

	/**
	 * @brief Function used to receive a packet from a socket
	 * @param socket The socket used to receive the packet
	 * @return The data of the packet received
	*/
	std::string TCPCommon::HandleRead (Ptr<Socket> socket){
		NS_LOG_FUNCTION(this << socket);

		Ptr<Packet> packet;
		Address from, to;
		std::string packetData;

		// Get receiving address
		socket->GetSockName(to);
		InetSocketAddress toAddress = InetSocketAddress::ConvertFrom(to);

		// Read the data from the socket while there is still data to read
		while (packet = socket->RecvFrom(from)) {

			if (packet->GetSize () == 0) //EOF
				break;

			// Get the packet data from the stream read from the socket	
			uint8_t *buffer = new uint8_t[packet->GetSize ()];
			packet->CopyData(buffer, packet->GetSize ());
			packetData = std::string((char*)buffer);

			Time rightNow = Simulator::Now() - m_startTime;
			InetSocketAddress fromAddress = InetSocketAddress::ConvertFrom (from);

			// Print the information about the packet received
			std::cout << "RECV [TO: " << toAddress.GetIpv4() 
						<< ":" << toAddress.GetPort()
						<< "] "
						<< "[FROM: "
						<< fromAddress.GetIpv4 ()
						<< ":" << fromAddress.GetPort () << "] "
						<< " at time "
						<< rightNow.GetSeconds();
    	}
		return packetData;
  	}


	/**
	 * @brief Callback used to handle a dispose request
	 */ 
	void TCPCommon::DoDispose (void){
		NS_LOG_FUNCTION (this);
		
		// chain up to NS3 application base class
		Application::DoDispose ();
	}		

	/**
	 * @brief Function used to set the router of the TCPCommon object
	 * @param router The router to set
	 */
	void TCPCommon::set_router(Router *router) {
		NS_LOG_FUNCTION (this << router);
		this->m_router = router;
	}
	
	/**
	 * @brief Function used to get the router of the TCPCommon object
	 * @return The router of the TCPCommon object
	 */
	Router* TCPCommon::get_router() {
		NS_LOG_FUNCTION (this);
		return this->m_router;
	}


	/**
	 * @brief Method to convert a binary bitstream that represents an IP address to dotted notation string
	 * @param ipBinary IP address in binary bitstream
	 * @return IP address in dotted notation string
	*/
	std::string TCPCommon::binary_to_dotted_notation(std::string ipBinary) {
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