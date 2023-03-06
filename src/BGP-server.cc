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

			//NS_LOG_INFO("Before new start time - Now: " << Simulator::Now().GetSeconds() << " " << " start time " << intf.get_start_time() << " max hold time " << intf.get_max_hold_time());

			if(Simulator::Now().GetSeconds() - intf.get_start_time() <= intf.get_max_hold_time()) {
				//Server send KEEPALIVE message
				std::stringstream msgStream;
				MessageHeader msg = MessageHeader(0);
				msgStream << msg << '\0';

				if(intf.status) {
					Ptr<Packet> packet = Create<Packet>((uint8_t*) msgStream.str().c_str(), msgStream.str().length()+1);
					this->Send(socket, packet);

					// Reset holdtime time
					intf.set_start_time(Simulator::Now().GetSeconds());
					r->setInterface(intf, int_num);
					//NS_LOG_INFO("After new start time - Now: " << Simulator::Now().GetSeconds() << " " << " start time " << intf.get_start_time() << " max hold time " << intf.get_max_hold_time());
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

					r->setInterfaceStatus(int_num, false);
					r->setInterface(intf, int_num);
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

				r->setInterfaceStatus(int_num, false);
				r->setInterface(intf, int_num);
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

				r->setInterfaceStatus(int_num, false);
				this->StopApplication();
			}

    		intf.set_max_hold_time(max_hold_time);
			intf.set_start_time(Simulator::Now().GetSeconds());
			r->setInterface(intf, int_num);

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