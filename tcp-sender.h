#ifndef NS3_TCP_SENDER_H
#define NS3_TCP_SENDER_H
#include "ns3/socket.h"
#include "ns3/application.h"

#define FB_SIZE       8

using namespace ns3;

namespace ns3 {
class TcpSender : public Application {

	public:
		TcpSender ();
        virtual ~TcpSender();
        void Setup (
            Ptr<Socket> socket,
            Address address,
            Time startTime,
            Time stopTime
        );


	private:
		virtual void StartApplication (void);
        virtual void StopApplication (void);
        void MySendPacket (void);
        void SendNow(void);
        void ConnectionSucceeded(Ptr<Socket>);
        void ConnectionFailed(Ptr<Socket>);
        void HandleReadFB(Ptr<Socket> socket);
        void HandleReadFB2(Ptr<Socket> socket);

        Ptr<Socket>         m_socket;
        Address             m_peer;
        EventId             m_sendEvent;
        bool                m_running;
        uint32_t            m_packetsSent;
        Time                m_start;
        Time                m_stop;

        // Structures
        std::list<double>   m_pktDelayList;
        std::vector<uint32_t>   pktDimensionVector;
        std::vector<std::ostringstream> pktContentVector;

        // Handle feedbacks
        int32_t             m_readReturn;
        int32_t             m_toComplete;
        uint8_t             m_recvBuffer[FB_SIZE];
        uint32_t            m_FBReceived;

	};
}
#endif