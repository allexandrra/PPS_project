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
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "tcp-sender.h"

// Namespace
using namespace ns3;

// Globals Define
#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS 0
#endif
#ifndef EXIT_FAILURE
#define EXIT_FAILURE -1
#endif

#define FB_SIZE       318

/**
 * Class to handle ad-hoc runtime exceptions.
 * Throw as: throw customException("<message>")
 */
class customException : public std::exception
{
    std::string message;
public:
    customException(std::string m) : message(m) {}
    ~customException() throw() {}
    virtual const char* what() const throw()
  {
    return message.c_str();
  }
};

// Constants
const uint8_t SKT_CONTROL_FLAGS = 0;

/******************* TCP CLIENT ******************/
/** Class TcpSender
 * TCP client application implementation
 */
namespace ns3 {
  NS_LOG_COMPONENT_DEFINE ("TcpSender");
  NS_OBJECT_ENSURE_REGISTERED(TcpSender);

  /** Constructor
  */
  TcpSender::TcpSender ()
  {
    m_socket          = 0;
    m_running         = false;
    m_packetsSent     = 0;
    m_FBReceived      = 0;
    m_readReturn      = FB_SIZE;
  }

  /** Destructor
  */
  TcpSender::~TcpSender()
  {
    m_socket = 0;
  }

  /** Setup
   * Function that allows the initial setup
   */
  void
  TcpSender::Setup (
      Ptr<Socket> socket,
      Address address,
      Time startTime,
      Time stopTime
  ){
    m_socket  = socket;
    m_peer    = address;
    m_start   = startTime;
    m_stop    = stopTime;

    m_pktDelayList.push_back(2);
    m_pktDelayList.push_back(4);
    m_pktDelayList.push_back(6);

    std::ostringstream msg;
    msg << "Hello World!" << '\0';
    uint16_t packetSize = msg.str().length()+1;
    pktContentVector.push_back((std::ostringstream)msg.str().c_str());
    pktDimensionVector.push_back((uint32_t)packetSize);

    msg.str("");
    msg << "Ciao" << '\0';
    packetSize = msg.str().length()+1;
    pktContentVector.push_back((std::ostringstream)msg.str().c_str());
    pktDimensionVector.push_back((uint32_t)packetSize);

    msg.str("");
    msg << "Hola amigos :)" << '\0';
    packetSize = msg.str().length()+1;
    pktContentVector.push_back((std::ostringstream)msg.str().c_str());
    pktDimensionVector.push_back((uint32_t)packetSize);
  }

  /** Start Application
   * Function that starts the TCP Client application
   */
  void
  TcpSender::StartApplication (void)
  {
    NS_LOG_FUNCTION (this);

    // Initialize
    m_running     = true;
    m_packetsSent = 0;

    // Create the socket
    if( InetSocketAddress::IsMatchingType (m_peer) || PacketSocketAddress::IsMatchingType (m_peer))
    {
      m_socket->Bind ();
    }

    // Setup connection
    m_socket->Connect (m_peer);
    m_socket->SetAllowBroadcast (true);

    m_socket->SetConnectCallback (
        MakeCallback (&TcpSender::ConnectionSucceeded, this),
        MakeCallback (&TcpSender::ConnectionFailed, this));

    // Set callback since the client receives feedback packets
    m_socket->SetRecvCallback (MakeCallback (&TcpSender::HandleReadFB2, this));

    // Call the default function to start scheduling packets send
    //NS_LOG_INFO("Start sending packets");
    MySendPacket();
  }

  /** Service functions Connection Succeeded and failed
   */
  void TcpSender::ConnectionSucceeded (Ptr<Socket> socket)
  {
    NS_LOG_FUNCTION (this << socket);
    m_running = true;
  }

  void TcpSender::ConnectionFailed (Ptr<Socket> socket)
  {
    NS_LOG_FUNCTION (this << socket);
  }

  /** Stop Application
   * Function to stop the application called by simulator from main()
   */
  void
  TcpSender::StopApplication (void)
  {
    m_running = false;

    if (m_sendEvent.IsRunning ())
    {
      Simulator::Cancel (m_sendEvent);
    }

    if (m_socket)
    {
      m_socket->Close ();
    }
  }

  /* My Send Packet
  * Function to schedule the packet send
  */
  void
  TcpSender::MySendPacket (void)
  {
    // Check on the packet list
    if(!m_pktDelayList.empty()){
      double pktDelay = m_pktDelayList.front();
      m_pktDelayList.pop_front();
      if (m_running)
      {
        m_sendEvent = Simulator::Schedule (Seconds(pktDelay), &TcpSender::SendNow, this);
      }
    }
  }

  /* Send Now
  * Function to actually send the packets
  */
  void
  TcpSender::SendNow()
  {
    // Read the packet dimension
    if(m_packetsSent < pktDimensionVector.size()){
      double pktDim = pktDimensionVector[m_packetsSent];

      // Create packet
      Ptr<Packet> packet = Create<Packet>((uint8_t*) pktContentVector[m_packetsSent].str().c_str(), pktDim);
      Time rightNow = Simulator::Now() - m_start;
      m_packetsSent++;

      Address addr;
      m_socket->GetSockName(addr);
      InetSocketAddress iaddr = InetSocketAddress::ConvertFrom (addr);

      NS_LOG_INFO ("[S] #" << m_packetsSent << " - '"
        << pktContentVector[m_packetsSent-1].str().c_str()
        << "' - SENT from Client with IP "
        << iaddr.GetIpv4 () << ":" << iaddr.GetPort () 
        << " at: "
        << rightNow.GetSeconds()
        << "s"
        << " - Dimension: "
        << pktDim
        );

      // Send
      m_socket->Send(packet);
      MySendPacket();
      //NS_LOG_INFO("Packet sent");
    }
  }

  /* Handle Read Feedback
  * Function to handle the read of the feedbacks coming back from the server
  */
  void
  TcpSender::HandleReadFB (Ptr<Socket> socket)
  {
    NS_LOG_FUNCTION(this << socket);
    NS_LOG_INFO("Received feedback");

    // Buffered read
    while(( m_readReturn = socket->Recv(m_recvBuffer, (uint32_t) m_readReturn , SKT_CONTROL_FLAGS)) >= 0 )
    {
      if (m_readReturn == 0)
      {
        // CASE 0: Recv returns 0 if there is nothing to be read
        m_readReturn = FB_SIZE;
        break;
      }
      else
      {
        if((m_readReturn != 0) && (m_readReturn != m_toComplete)){
          // CASE 1: PARTIAL read.
          m_readReturn = m_toComplete = FB_SIZE - m_readReturn;
          break;
        }
        else if( (m_readReturn != 0) && (m_readReturn == m_toComplete)){
          // CASE 2: Read COMPLETED. Update counters and move on
          m_readReturn = m_toComplete = FB_SIZE;

          Time rightNow = Simulator::Now() - m_startTime;
          char charToPrint[FB_SIZE];
          memcpy(charToPrint, m_recvBuffer, FB_SIZE);

          // Log print
          NS_LOG_INFO( "[F]<- #" << m_FBReceived
                      << " - Feedback RECEIVED from Client at: "
                      << rightNow.GetSeconds()
                      << " of size: "
                      << m_readReturn
          );
        }
      }
    } // end while
  } 

  void
  TcpSender::HandleReadFB2 (Ptr<Socket> socket)
  {
    //NS_LOG_INFO("Received feedback prima");
    NS_LOG_FUNCTION(this << socket);
    //NS_LOG_INFO("Received feedback dopo");

    Ptr<Packet> packet;
    Address from;
    Address localAddress;
    while (packet = socket->RecvFrom(from)) {
      uint8_t *buffer = new uint8_t[packet->GetSize ()];
      packet->CopyData(buffer, packet->GetSize ());
      std::string packetData = std::string((char*)buffer);
      Time rightNow = Simulator::Now() - m_startTime;
      m_FBReceived++;
      InetSocketAddress iaddr = InetSocketAddress::ConvertFrom (from);

      // Log print
      NS_LOG_INFO( "[F]<- " << " Feedback '"
        << packetData
        << "' RECEIVED from Client with IP " 
        << iaddr.GetIpv4 () << ":" << iaddr.GetPort ()  
        << " at: "
        << rightNow.GetSeconds()
        << " of size: "
        << packet->GetSize()
      );

    } // end while
  }

} // namespace ns3