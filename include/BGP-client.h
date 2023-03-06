#ifndef NS3_BGP_CLIENT_H
#define NS3_BGP_CLIENT_H
#include "ns3/socket.h"
#include "ns3/application.h"
#include "TCP-common.h"

using namespace ns3;

namespace ns3 {
    class BGPClient : public TCPCommon {

        public:
            BGPClient ();
            virtual ~BGPClient();
            void Setup (Ptr<Socket> socket, Address address, Time startTime, Time stopTime);
            void AddPacketsToQueue(std::stringstream& msg, Time atTime);
            void AddPacketsToQueuePeriodically(std::vector<EventId> events);
            Ptr<Socket> GetSocket(void) const;

        private:
            virtual void StartApplication(void);
            virtual void StopApplication(void);
            void ConnectionSucceeded(Ptr<Socket>);
            void ConnectionFailed(Ptr<Socket>);
            void SendMyPackets();
            void HandleRead (Ptr<Socket> socket);

            Ptr<Socket>         m_socket;
            Address             m_peer;
            bool                m_running;
            Time                m_start;
            Time                m_stop;
            EventId             m_sendEvent;
            std::list<Ptr<Packet>>   m_pktList;
            std::list<Time>     m_pktDelayList;

    };
}
#endif