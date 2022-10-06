#ifndef NS3_TEST_RECEIVER_H
#define NS3_TEST_RECEIVER_H
#include "ns3/socket.h"
#include "ns3/application.h"

using namespace ns3;

namespace ns3 {
class TcpReceiver : public Application {

	public:
		TcpReceiver ();
		virtual ~TcpReceiver ();
                void Setup(uint16_t serverPort, Time startTime);

	private:
        virtual void StartApplication();
        virtual void StopApplication();
        uint32_t GetTotalRx();
        void DoDispose();
        void HandleRead (Ptr<Socket> socket);
        void HandleRead2 (Ptr<Socket> socket);
        void HandlePeerClose (Ptr<Socket> socket);
        void HandlePeerError (Ptr<Socket> socket);
        void HandleAccept (Ptr<Socket> socket, const Address& from);
        void HandleSuccessClose(Ptr<Socket> socket);
        // FeedBack handling
        void MyFBSend();
        void SendNowFB();
        void SetFill (char* fill);

        Ptr<Socket>             m_socket;
        uint32_t                m_totalRx;
        std::list<Ptr<Socket> > m_socketList;
        TypeId                  m_tid;
        uint32_t                m_packetsReceived;
        Time                    m_startTime;
        uint8_t*                m_recvBuffer;
        std::ostringstream      m_timeStringContainer;
        int32_t                 m_readReturn;
        int32_t                 m_toComplete;
        uint32_t                m_dataSize;
        uint8_t*                m_data;
        uint16_t                m_port;
        EventId                 m_sendEvent;
        uint32_t                m_FBSent;

        // Vector
        std::vector<uint32_t>   pktDimensionVector;
	};
}
#endif