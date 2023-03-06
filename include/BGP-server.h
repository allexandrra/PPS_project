#ifndef NS3_BGP_SERVER_H
#define NS3_BGP_SERVER_H
#include "ns3/socket.h"
#include "ns3/application.h"
#include "TCP-common.h"

using namespace ns3;

namespace ns3 {
    class BGPServer : public TCPCommon {
    
        public:
            BGPServer();
            virtual ~BGPServer();
            static TypeId GetTypeId (void);
            void Setup(uint16_t serverPort, Time startTime);
            Ptr<Socket> GetSocket(void) const;

        private:
            virtual void StartApplication();
            virtual void StopApplication();
            void DoDispose();
            void HandlePeerClose (Ptr<Socket> socket);
            void HandlePeerError (Ptr<Socket> socket);
            void HandleAccept (Ptr<Socket> socket, const Address& from);
            void HandleSuccessClose(Ptr<Socket> socket);
            void HandleRead (Ptr<Socket> socket);

            Ptr<Socket>             m_socket;
            std::list<Ptr<Socket> > m_socketList;
            TypeId                  m_tid;
            Time                    m_startTime;
            uint16_t                m_port;

    };
}
#endif