#include "ns3/log.h"
#include "test-app.h"
#include "ns3/ipv4-raw-socket-factory.h"
#include "ns3/ipv4-raw-socket-impl.h"
#include "ns3/simulator.h"
#include "ns3/csma-net-device.h"
#include "ns3/ethernet-header.h"
#include "ns3/arp-header.h"
#include "ns3/ipv4-header.h"
#include "ns3/tcp-header.h"

namespace ns3 {
    NS_LOG_COMPONENT_DEFINE("TestApp");
    NS_OBJECT_ENSURE_REGISTERED(TestApp);
    
    TypeId TestApp::GetTypeId() {
        static TypeId tid = TypeId("ns3::TestApp")
            .AddConstructor<TestApp>()
            .SetParent<Application>();
        return tid;
    }
    
    TypeId TestApp::GetInstanceTypeId() const {
        return TestApp::GetTypeId();
    }

    TestApp::TestApp(){
        m_port1 = 7777;
        m_port2 = 9999;
    }

    TestApp::~TestApp(){}

    void TestApp::SetupReceiveSocket(Ptr<Socket> socket, uint16_t port){
        InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), port);
        if (socket->Bind(local) == -1) {
            NS_FATAL_ERROR("Failed to bind socket");
        }  
    }

    void TestApp::StartApplication(){
        TypeId tid = TypeId::LookupByName("ns3::ipv4RawSocketFactory");
        m_recv_socket1 = Socket::CreateSocket(GetNode(), tid);
        m_recv_socket2 = Socket::CreateSocket(GetNode(), tid);

        SetupReceiveSocket(m_recv_socket1, m_port1);
        SetupReceiveSocket(m_recv_socket2, m_port2);

        m_recv_socket1->SetRecvCallback(MakeCallback(&TestApp::HandleReadOne, this));
        m_recv_socket2->SetRecvCallback(MakeCallback(&TestApp::HandleReadTwo, this));

        //Send Socket
        m_send_socket = Socket::CreateSocket(GetNode(), tid);
    }

    void TestApp::HandleReadOne(Ptr<Socket> socket) {
        NS_LOG_FUNCTION(this << socket);
        Ptr<Packet> packet;
        Address from;
        Address localAddress;
        while ((packet = socket->RecvFrom(from))) {
            uint8_t *buffer = new uint8_t[packet->GetSize ()];
            packet->CopyData(buffer, packet->GetSize ());
            std::string packetData = std::string((char*)buffer);
            NS_LOG_INFO("HandleReadTwo : Received a Packet of size: " << packet->GetSize() << " at time " << Now().GetSeconds() << " content: '" << packetData << "'");
        }
    }

    void TestApp::HandleReadTwo(Ptr<Socket> socket){
        NS_LOG_FUNCTION(this << socket);
        Ptr<Packet> packet;
        Address from;
        Address localAddress;
        while ((packet = socket->RecvFrom(from))) {
            uint8_t *buffer = new uint8_t[packet->GetSize ()];
            packet->CopyData(buffer, packet->GetSize ());
            std::string packetData = std::string((char*)buffer);
            NS_LOG_INFO("HandleReadTwo : Received a Packet of size: " << packet->GetSize() << " at time " << Now().GetSeconds() << " content: '" << packetData << "'");
        }
    }
        
    void TestApp::SendPacket(Ptr<Packet> packet, Ipv4Address destination, uint16_t port) {
        NS_LOG_FUNCTION (this << packet << destination << port);
        m_send_socket->Connect(InetSocketAddress(Ipv4Address::ConvertFrom(destination), port));
        m_send_socket->Send(packet);
        uint8_t *buffer = new uint8_t[packet->GetSize ()];
        packet->CopyData(buffer, packet->GetSize ());
        std::string packetData = std::string((char*)buffer);
        NS_LOG_INFO("Sent a Packet of size: " << packet->GetSize() << " at time " << Now().GetSeconds() << " with content '" << packetData << "'");
    }

} // namespace ns3