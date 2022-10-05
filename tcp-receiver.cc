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
#include "tcp-receiver.h"


// Constants
const uint8_t SKT_CONTROL_FLAGS = 0;


namespace ns3 {
  NS_LOG_COMPONENT_DEFINE ("TcpReceiver");
  NS_OBJECT_ENSURE_REGISTERED(TcpReceiver);

  // Constructor
  TcpReceiver::TcpReceiver ()
  {
    NS_LOG_FUNCTION (this);
    m_socket          = 0;
    m_totalRx         = 0;
    m_packetsReceived = 0;
    m_dataSize        = 0;
    m_FBSent          = 0;
    m_readReturn      = 0;
    m_toComplete      = 0;
    //NS_LOG_INFO("Constructor receiver");
  }

  // Destructor
  TcpReceiver::~TcpReceiver()
  {
    NS_LOG_FUNCTION (this);
    //free(m_recvBuffer);
  }

  /** Setup function - 2 arguments
  * The socket is NOT passed to the function. The socket will be allocated in the Start Application
  * function.
  */
  void
  TcpReceiver::Setup(uint16_t serverPort,  Time startTime)
  {
    //NS_LOG_INFO("Setup receiver");
    m_port      = serverPort;
    m_startTime = startTime;

    std::ostringstream msg;
    msg << "Hello World!" << '\0';
    uint16_t packetSize = msg.str().length()+1;
    pktDimensionVector.push_back((uint32_t)packetSize);

    msg << "Ciao" << '\0';
    packetSize = msg.str().length()+1;
    pktDimensionVector.push_back((uint32_t)packetSize);

    msg << "Hola amigos :)" << '\0';
    packetSize = msg.str().length()+1;
    pktDimensionVector.push_back((uint32_t)packetSize);
  }

  // Getters
  uint32_t
  TcpReceiver::GetTotalRx ()
  {
    NS_LOG_FUNCTION (this);
    return m_totalRx;
  }

  void TcpReceiver::DoDispose (void)
  {
    NS_LOG_FUNCTION (this);
    m_socket = 0;
    m_socketList.clear ();

    // chain up
    Application::DoDispose ();
  }

  /** Application Methods
  * Called at time specified by Start
  */
  void
  TcpReceiver::StartApplication ()
  {
    NS_LOG_FUNCTION (this);
    //NS_LOG_INFO("Start receiver");

    // Create the socket if not already existing
    if (m_socket == 0)
    {
      TypeId tid = TypeId::LookupByName ("ns3::TcpSocketFactory");
      m_socket = Socket::CreateSocket (GetNode(), tid);
      InetSocketAddress listenAddress = InetSocketAddress (Ipv4Address::GetAny(), m_port);
      m_socket->Bind (listenAddress);
      m_socket->Listen();
    }

    // AcceptCallback
    m_socket->SetAcceptCallback (
      MakeNullCallback<bool, Ptr<Socket>, const Address &> (),
      MakeCallback (&TcpReceiver::HandleAccept, this)
    );
    // RecvCallback
    m_socket->SetRecvCallback (MakeCallback (&TcpReceiver::HandleRead2, this));
    // CloseCallback
    m_socket->SetCloseCallbacks (
      MakeCallback (&TcpReceiver::HandlePeerClose, this),
      MakeCallback (&TcpReceiver::HandlePeerError, this)
    );
  }

  /** Stop Application
   * Called by simulator to stop the server
   */
  void TcpReceiver::StopApplication ()
  {
    NS_LOG_FUNCTION (this);
    while(!m_socketList.empty ()) //these are accepted sockets, close them
    {
      Ptr<Socket> acceptedSocket = m_socketList.front ();
      m_socketList.pop_front ();
      acceptedSocket->Close ();
    }
    if (m_socket)
    {
      m_socket->Close ();
      m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
    }
  }

  /** Handle Accept
   * Function called as soon as there is an accept
   */
  void TcpReceiver::HandleAccept (Ptr<Socket> socket, const Address& from)
  {
    NS_LOG_FUNCTION (this << socket << from);
    socket->SetRecvCallback (MakeCallback (&TcpReceiver::HandleRead2, this));
    // Saving socket in a list
    m_socketList.push_back (socket);
    // CloseCallback
    socket->SetCloseCallbacks(MakeCallback (&TcpReceiver::HandleSuccessClose, this),
                        MakeNullCallback<void, Ptr<Socket> > () );
  }

  /** Handle Success Close
   * This function is called as soon as a close pkt arrives
   */
  void TcpReceiver::HandleSuccessClose(Ptr<Socket> socket)
  {
    NS_LOG_FUNCTION (this << socket);
    NS_LOG_LOGIC ("Client close received");
    socket->Close();
    socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > () );
    socket->SetCloseCallbacks(MakeNullCallback<void, Ptr<Socket> > (),
                              MakeNullCallback<void, Ptr<Socket> > () );
  }

  /** Handle Read
   * This function is called when something is available at the TCP level.
   * It handles the incremental read waiting for the exact amount of bytes
   * before proceeding. Next it calles the "feedback" function which sends
   * back to the sender a packet containing the time of the receive
   */
  void TcpReceiver::HandleRead (Ptr<Socket> socket) {
    NS_LOG_FUNCTION(this << socket);

    // Get the dimension of the *current* packet
    int32_t currentPktSize = pktDimensionVector[m_packetsReceived];

    // Initialize variables to this value
    if(m_toComplete == 0) {
      m_toComplete = currentPktSize;
      m_recvBuffer = (uint8_t*) malloc((size_t)currentPktSize);
    }

    // While loop to perform a buffered read
    while(( m_readReturn = socket->Recv(m_recvBuffer, (uint32_t) m_toComplete , SKT_CONTROL_FLAGS)) >= 0 )
    {
      if (m_readReturn == 0)
      {
        // CASE 0: Recv returns 0 if there is nothing to be read
        break;
      }
      else
      {
        if((m_readReturn != 0) && (m_readReturn != m_toComplete)){
          // CASE 1: PARTIAL read.
          m_toComplete = m_toComplete - m_readReturn;
        }
        else if((m_readReturn != 0) && (m_readReturn == m_toComplete)){
          // CASE 2: Read COMPLETED. Update counters and move on
          m_toComplete = 0;
          m_totalRx += currentPktSize;
          m_packetsReceived++;
          Time rightNow = Simulator::Now() - m_startTime;
          // Log print
          NS_LOG_INFO( "[R] #" << m_packetsReceived << " '"
            << m_readReturn << "'"
            << " - RECV at: "
            << rightNow.GetSeconds()
            << " - Dimension: "
            << currentPktSize
          );

          // Call MyFBSend to schedule the Feedback packet send
          m_timeStringContainer = (char*) malloc ((size_t)318);
          snprintf(m_timeStringContainer, 318, "%f", rightNow.GetSeconds());
          SetFill(m_timeStringContainer);

          // Free buffers
          free(m_recvBuffer);
          free(m_timeStringContainer);

          // Call FeedBack Send
          MyFBSend();
        }
      }
    }
  }

  void TcpReceiver::HandleRead2 (Ptr<Socket> socket) {
    NS_LOG_FUNCTION(this << socket);

    Ptr<Packet> packet;
    Address from;
    Address localAddress;
    while (packet = socket->Recv()) {
      uint8_t *buffer = new uint8_t[packet->GetSize ()];
      packet->CopyData(buffer, packet->GetSize ());
      std::string packetData = std::string((char*)buffer);
      m_packetsReceived++;
      Time rightNow = Simulator::Now() - m_startTime;
      // Log print
      NS_LOG_INFO( "[R] #" << m_packetsReceived << " '"
        << packetData << "'"
        << " - RECV at: "
        << rightNow.GetSeconds()
        << " - Dimension: "
        << packet->GetSize()
      );

      // Call MyFBSend to schedule the Feedback packet send
      m_timeStringContainer = (char*) malloc ((size_t)318);
      snprintf(m_timeStringContainer, 318, "%f", rightNow.GetSeconds());
      SetFill(m_timeStringContainer);

      // Free buffers
      free(m_recvBuffer);
      free(m_timeStringContainer);

      // Call FeedBack Send
      MyFBSend();
    }
  }



  /** My FB Send
   * Function to schedule the SendNowFB call
   */
  void
  TcpReceiver::MyFBSend(void)
  {
    // Scheduling 'now' (0 seconds) but this can be changed to give a timeout
    m_sendEvent = Simulator::Schedule (Seconds(.0), &TcpReceiver::SendNowFB, this);
  }

  /** Set Fill
   * Function to fill the new packets to prepare the send
   */
  void
  TcpReceiver::SetFill (char* fill)
  {
    NS_LOG_FUNCTION (fill);
    uint32_t dataSize = sizeof(fill);
    m_data = new uint8_t [dataSize];
    m_dataSize = dataSize;
    // Actually copying the content inside the buffer
    memcpy (m_data, fill, dataSize);
  }

  /** Send Now FeedBack
   * Function that actually sends the FeedBack packet
   */
  void
  TcpReceiver::SendNowFB(void)
  {
    NS_LOG_FUNCTION_NOARGS ();

    Ptr<Packet> feedback;
    if (m_dataSize)
    {
      // SetFill has been called
      feedback = Create<Packet> (m_data, m_dataSize);
      delete[] m_data;
    }
    // TODO: Implement else: if SetFill has not been called what can we do? Write SimulatorNow() in pkt?
    /*else
    {
      // SetFill has NOT been called
      feedback = Create<Packet> (m_size);
    }*/

    // Retry the connected socket
    if(m_socketList.size() != 0) {
      Ptr <Socket> sock = m_socketList.front();
      Time rightNow = Simulator::Now() - m_startTime;
      m_FBSent++;
      NS_LOG_INFO("[F]-> #"
                  << m_FBSent << " Feedback '"
                  << rightNow.GetSeconds() << "' "
                  << " SENT from server at: "
                  << rightNow.GetSeconds()
                  << "s of size "
                  << feedback->GetSize()
      );

      // Send
      sock->Send(feedback);
    }
  }

  /** Handle Peer Close
   * Function to handle the close
   */
  void TcpReceiver::HandlePeerClose (Ptr<Socket> socket)
  {
    NS_LOG_FUNCTION (this << socket);
  }

  void TcpReceiver::HandlePeerError (Ptr<Socket> socket)
  {
    NS_LOG_FUNCTION (this << socket);
  }


} // namespace ns3