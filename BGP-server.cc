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

#include "BGP-server.h"


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


} //namespace ns3