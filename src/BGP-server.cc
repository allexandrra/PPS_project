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
#include "../include/MessageTrustrate.h"

namespace ns3 {
	NS_LOG_COMPONENT_DEFINE ("BGPServer");
  	NS_OBJECT_ENSURE_REGISTERED(BGPServer);

	/**
	 * @brief Constructor for the BGPServer class
	*/
	BGPServer::BGPServer () {
		NS_LOG_FUNCTION (this);
		m_socket = 0;
	}


	/**
	 * @brief Destructor for the BGPServer class
	*/
	BGPServer::~BGPServer() {
		NS_LOG_FUNCTION (this);
	}

	/**
	 * @ Return the type ID of the class
	 * @return the object TypeId
	*/
	TypeId BGPServer::GetTypeId() {
        static TypeId tid = TypeId("ns3::BGPServer")
            .AddConstructor<BGPServer>()
            .SetParent<TCPCommon>();
        return tid;
    }

	/**
	 * @brief Setup function for the BGPServer class
	 * @param serverPort The port number of the server
	 * @param startTime The time at which the server starts
	*/
	void BGPServer::Setup(uint16_t serverPort,  Time startTime) {
		m_port      = serverPort;
		m_startTime = startTime;
	}


	/**
	 * @brief Start Application	method
	*/
	void BGPServer::StartApplication() {
		NS_LOG_FUNCTION (this);

		// create the socket if not already created
		if (m_socket == 0) {
			m_tid = TypeId::LookupByName("ns3::TcpSocketFactory");
			m_socket = Socket::CreateSocket(GetNode(), m_tid);
			InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), m_port);
			m_socket->Bind(local);
			m_socket->Listen();
		}

		// acceptCallback
		m_socket->SetAcceptCallback (
			MakeNullCallback<bool, Ptr<Socket>, const Address &> (),
			MakeCallback(&BGPServer::HandleAccept, this));
		
		// rcvCallback
		m_socket->SetRecvCallback (
			MakeCallback(&BGPServer::HandleRead, this));
		
		// closeCallback
		m_socket->SetCloseCallbacks (
			MakeCallback(&BGPServer::HandlePeerClose, this),
			MakeCallback(&BGPServer::HandlePeerError, this));
	}


	/**
	 * @brief Stop Application method
	*/
	void BGPServer::StopApplication ()
	{
		NS_LOG_FUNCTION (this);

		// close all pending sockets
		while(!m_socketList.empty ()) { 
			//these are accepted sockets, close them
			Ptr<Socket> acceptedSocket = m_socketList.front ();
			m_socketList.pop_front ();
			acceptedSocket->Close ();
		}

		// close the listening socket
		if (m_socket) {
			m_socket->Close ();
			m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
		}
	}


	/**
	 * @brief Callback used to read from the socket
	 * @param socket The socket of the server
	*/
	void BGPServer::HandleRead (Ptr<Socket> socket){
		NS_LOG_FUNCTION(this << socket);

		// Read the message from the socket
		// Using the TCPCommon::HandleRead method
		std::string packet = TCPCommon::HandleRead(socket);

		// Parse the message
		MessageHeader msg;
		std::stringstream(packet) >> msg;

		// Check the type of the message to execute different actions based on that
		if (msg.get_type() == 4){
			// In teory, the sending of keepalive message should be done indipendtly for each router, indipendtly from a received keepalive message of its peer
			// So, ech router sends keepalive messages to its peer at regular intervals to maintain the connection, but it does not expect any response from the peer
			// To simplify the implementation, the server we will send the keepalive message only when it receives the keepalive form the client peer

			std::cout << " KEEPALIVE message " << std::endl;

			// Get the interface of the router where the server application is installed
			Router *r = this->get_router();
			Address to;
			socket->GetSockName(to);
			InetSocketAddress toAddress = InetSocketAddress::ConvertFrom(to);
			int int_num = r->get_router_int_num_from_ip(toAddress.GetIpv4());
			Interface intf = r->get_router_int()[int_num];

			// Check if the holdtime is expired
			if(Simulator::Now().GetSeconds() - intf.get_last_update_time() <= intf.get_max_hold_time()) {
				
				//Create a new KEEPALIVE message
				std::stringstream msgStream;
				MessageHeader msg = MessageHeader(4);
				msgStream << msg << '\0';

				// Check if the interface is up
				if(intf.status) {
					Ptr<Packet> packet = Create<Packet>((uint8_t*) msgStream.str().c_str(), msgStream.str().length()+1);
					this->Send(socket, packet);

					// Reset holdtime time
					intf.set_last_update_time(Simulator::Now().GetSeconds());
					r->set_interface(intf, int_num);

				} else {

					// If the interface is down, send a NOTIFICATION message to the client peer
					NS_LOG_INFO("Interface " << intf.name << " of router " << r->get_router_AS() << " is down [KEEPALIVE READ SERVER] ");
		
					std::stringstream msgStreamNotification;
					MessageNotification msg = MessageNotification(6,0);
					msgStreamNotification << msg << '\0';

					Ptr<Packet> packetNotification = Create<Packet>((uint8_t*) msgStreamNotification.str().c_str(), msgStreamNotification.str().length()+1);
					this->Send(socket, packetNotification);

					// Reset the client and server application installed on the interface
					intf.client.reset();
					intf.server.reset();
					r->set_interface_status(int_num, false);
					r->set_interface(intf, int_num);
					this->StopApplication();
				}

			} else {

				// If the holdtime is expired, send a NOTIFICATION message to the client peer
				NS_LOG_INFO("Hold time for interface " << intf.name << " of router " << r->get_router_AS() << " expired at time " << Simulator::Now().GetSeconds() << " [KEEPALIVE READ SERVER]");
		
				std::stringstream msgStreamNotification;
				MessageNotification msg = MessageNotification(4,0);
				msgStreamNotification << msg << '\0';

				Ptr<Packet> packetNotification = Create<Packet>((uint8_t*) msgStreamNotification.str().c_str(), msgStreamNotification.str().length()+1);
				this->Send(socket, packetNotification);

				// Reset the client and server appllication installed on the interface
				intf.client.reset();
				intf.server.reset();
				r->set_interface_status(int_num, false);
				r->set_interface(intf, int_num);
				this->StopApplication();
			}

		}
		else if(msg.get_type() == 1){
			
			// Parse the OPEN message to extract all the correct fields
			MessageOpen msgRcv;
			std::stringstream(packet) >> msgRcv;
			

			std::cout << " OPEN message with content  AS: " << msgRcv.get_AS() << " \t HOLD TIME: " << msgRcv.get_hold_time() << "\t BGP ID: " <<  binary_to_dotted_notation(msgRcv.get_BGP_id()) << std::endl;

			// Get the router and the router interface where the server application is installed
			Address to;
			Router *r = this->get_router();
			socket->GetSockName(to);
			InetSocketAddress toAddress = InetSocketAddress::ConvertFrom(to);
			int int_num = r->get_router_int_num_from_ip(toAddress.GetIpv4());
			Interface intf = r->get_router_int()[int_num];

			//DECISION: HOLD TIME = 90s, KEEPALIVE = 30s (1/3 the hold time)
			// Compare the received hold time with the hold time decided for the server interfaces (90s)
			int max_hold_time = max((int)msgRcv.get_hold_time(), 90);

			std::stringstream msgStream;
		
			// Check if the interface is up
			if(intf.status) {

				// Create a new OPEN message and send it back to the client
				MessageOpen msgToSend = MessageOpen(r->get_router_AS(), max_hold_time, r->get_router_ID());
				msgStream << msgToSend << '\0';
				Ptr<Packet> packet = Create<Packet>((uint8_t*) msgStream.str().c_str(), msgStream.str().length()+1);
				this->Send(socket,packet);
			} else {

				// If the interface is down, send a NOTIFICATION message to the client peer
				NS_LOG_INFO("Interface " << intf.name << " of router " << r->get_router_AS() << " is down [OPEN READ SERVER]");

				std::stringstream msgStream;
				MessageNotification msg = MessageNotification(6,0);
				msgStream << msg << '\0';

				Ptr<Packet> packet = Create<Packet>((uint8_t*) msgStream.str().c_str(), msgStream.str().length()+1);
				this->Send(socket, packet);

				// Reset the client and server application installed on the interface
				intf.client.reset();
				intf.server.reset();
				r->set_interface_status(int_num, false);
				this->StopApplication();
			}

			// Set the hold time of the interface and the current time as the last update time
    		intf.set_max_hold_time(max_hold_time);
			intf.set_last_update_time(Simulator::Now().GetSeconds());
			r->set_interface(intf, int_num);

		} else if(msg.get_type() == 3){ 

			// Parse the NOTIFICATION message to extract the error code and the error subcode
			MessageNotification msgRcv;
			std::stringstream(packet) >> msgRcv;

			std::cout << " NOTIFICATION message with content  ERROR CODE: " << msgRcv.get_error_code() << " \t ERROR SUBCODE: " << msgRcv.get_error_subcode() << " closing the TCP connection" << std::endl;

		} else if (msg.get_type() == 5) {
			MessageTrustrate msgRcv;
			std::stringstream(packet) >> msgRcv;

			std::cout << " TRUSTRATE message with TRUST: " << msgRcv.get_trust() << std::endl;

			// Get the interface of the router where the server application is installed
			Router *r = this->get_router();
			Address to;
			socket->GetSockName(to);
			InetSocketAddress toAddress = InetSocketAddress::ConvertFrom(to);
			int int_num = r->get_router_int_num_from_ip(toAddress.GetIpv4());
			Interface intf = r->get_router_int()[int_num];

			// Check if the holdtime is expired
			if(Simulator::Now().GetSeconds() - intf.get_last_update_time() <= intf.get_max_hold_time()) {
				
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
								  << " - observed trust value: " << observed_trust; */
						
						intf.inherited_trust = (1 - 0.3) * intf.inherited_trust + 0.3 * msgRcv.get_trust();

						//std::cout << " - new inherited trust value: " << intf.inherited_trust;

						// Weighted average of the two trust values	
						// The weight of the two values is 50% each
						intf.direct_trust = intf.inherited_trust * 0.5 + observed_trust * 0.5;
						
						/*std::cout << " - new direct trust value: " << intf.direct_trust << std::endl
								  << std::endl; */

					} else {
						// New trust value = (1 - α) * Existing trust value + α * New trust value
						// α = 0.3
						/*std::cout << "No voted trust yet. Received value: " << msgRcv.get_trust() 
								  << " - old direct trust value: "  << intf.direct_trust; */

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

				// TODO: update the routing table using the value of the total trust
				
				//Create a new TRUSTRATE message
				std::stringstream msgStream;
				MessageTrustrate msg = MessageTrustrate(intf.total_trust);
				msgStream << msg << '\0';

				// Check if the interface is up
				if(intf.status) {
					Ptr<Packet> packet = Create<Packet>((uint8_t*) msgStream.str().c_str(), msgStream.str().length()+1);
					this->Send(socket, packet);

					// Reset holdtime time
					intf.set_last_update_time(Simulator::Now().GetSeconds());
					r->set_interface(intf, int_num);

				} else {

					// If the interface is down, send a NOTIFICATION message to the client peer
					NS_LOG_INFO("Interface " << intf.name << " of router " << r->get_router_AS() << " is down [TRUSTRATE READ SERVER] ");
		
					std::stringstream msgStreamNotification;
					MessageNotification msg = MessageNotification(6,0);
					msgStreamNotification << msg << '\0';

					Ptr<Packet> packetNotification = Create<Packet>((uint8_t*) msgStreamNotification.str().c_str(), msgStreamNotification.str().length()+1);
					this->Send(socket, packetNotification);

					// Reset the client and server application installed on the interface
					intf.client.reset();
					intf.server.reset();
					r->set_interface_status(int_num, false);
					r->set_interface(intf, int_num);
					this->StopApplication();
				}

			} else {

				// If the holdtime is expired, send a NOTIFICATION message to the client peer
				NS_LOG_INFO("Hold time for interface " << intf.name << " of router " << r->get_router_AS() << " expired at time " << Simulator::Now().GetSeconds() << " [TRUSTRATE READ SERVER]");
		
				std::stringstream msgStreamNotification;
				MessageNotification msg = MessageNotification(4,0);
				msgStreamNotification << msg << '\0';

				Ptr<Packet> packetNotification = Create<Packet>((uint8_t*) msgStreamNotification.str().c_str(), msgStreamNotification.str().length()+1);
				this->Send(socket, packetNotification);

				// Reset the client and server appllication installed on the interface
				intf.client.reset();
				intf.server.reset();
				r->set_interface_status(int_num, false);
				r->set_interface(intf, int_num);
				this->StopApplication();
			}


		} else {
			NS_LOG_INFO("Received another type of message");
		}	

  	}

	/**
	 * @brief Handle a disposition of the socket
	*/
	void BGPServer::DoDispose (void){
		NS_LOG_FUNCTION (this);
		m_socket = 0;
		m_socketList.clear ();

		// chain up to TCP common DoDispose
		TCPCommon::DoDispose ();
	}

	/** 
	 * @brief Handle Peer Close callback (called by the TCP stack when the peer closes the connection)
	 * @param socket the socket of the peer that closed the connection
	 */
	void BGPServer::HandlePeerClose (Ptr<Socket> socket) {	
		NS_LOG_FUNCTION (this << socket);
	}

	/** 
	 * @brief Handle Peer Error callback (called by the TCP stack when an error occurs)
	 * @param socket the socket of the peer
	 */
	void BGPServer::HandlePeerError (Ptr<Socket> socket) {
    	NS_LOG_FUNCTION (this << socket);
  	}

	/**
	 * @brief Handle Accept callback (called as soon as there is an accepted connection)
	 * @param socket the socket of the accepted connection
	 * @param from the address of the peer
   */
	void BGPServer::HandleAccept (Ptr<Socket> socket, const Address& from)
	{
		NS_LOG_FUNCTION (this << socket << from);

		// Set the callback for reading data
		socket->SetRecvCallback (
			MakeCallback (&BGPServer::HandleRead, this));

		// Saving the client socket in a list
		m_socketList.push_back (socket);

		// CloseCallback
		socket->SetCloseCallbacks(
			MakeCallback (&BGPServer::HandleSuccessClose, this),
			MakeNullCallback<void, Ptr<Socket> > ());
	}


	/** 
	 * @brief Handle Success Close callback (called as soon as a close pkt arrives)
	 * @param socket the socket of the peer that closed the connection
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

	/**
	 * @brief Return the socket of the server
	 * @return the socket of the server
	 */
	Ptr<Socket> BGPServer::get_socket(void) const {
		return m_socket;
	}


} //namespace ns3