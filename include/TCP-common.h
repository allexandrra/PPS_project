#ifndef NS3_TCP_COMMON_H
#define NS3_TCP_COMMON_H

#include "ns3/socket.h"
#include "ns3/application.h"

namespace ns3 {
	class TCPCommon : public Application {
		public:
			TCPCommon();
			virtual ~TCPCommon();
			
			void Send(Ptr<Socket> socket, Ptr<Packet> packet);
			void HandleRead(Ptr<Socket> socket);
			void DoDispose();		

	};
}
#endif