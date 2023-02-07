#ifndef NS3_TCP_COMMON_H
#define NS3_TCP_COMMON_H

#include "ns3/socket.h"
#include "ns3/application.h"
//#include "Router.h"

namespace ns3 {

	class Router;

	class TCPCommon : public Application {
		public:
			TCPCommon();
			virtual ~TCPCommon();
			
			void Send(Ptr<Socket> socket, Ptr<Packet> packet);
			std::string HandleRead(Ptr<Socket> socket);
			void DoDispose();
			void SetRouter(Router *router);
			Router *GetRouter();

		protected:
			Router *m_router;

	};
}
#endif