#ifndef NS3_TCP_COMMON_H
#define NS3_TCP_COMMON_H

#include "ns3/socket.h"
#include "ns3/application.h"

namespace ns3 {

	// Forward declaration used to avoid circular dependencies
	class Router;

	/**
	 * @brief Class that contains common methods for each TCP applications (both BGPClient and BGPServer will inherit from this class)
	 */
	class TCPCommon : public Application {
		public:

			/**
			 * @brief Constructor for TCPCommon
			 **/
			TCPCommon();

			/**
			 * @brief Destructor for TCPCommon
			 **/
			virtual ~TCPCommon();
			
			/**
			 * @brief Method to send a packet through a socket, simulating a TCP connection
			 * @param socket Socket to be used to send the packet
			 * @param packet Packet to be sent
			*/
			void Send(Ptr<Socket> socket, Ptr<Packet> packet);

			/**
			 * @brief Method to handle the read of a packet from a socket, simulating a TCP connection
			 * @param socket Socket from which the packet should be read
			 * @return String containing the packet read
			*/
			std::string HandleRead(Ptr<Socket> socket);

			/**
			 * @brief Method to dispose the TCPCommon application
			*/
			void DoDispose();

			/**
			 * @brief Method to set the router where the application is installed
			 * @param router Router reference where the application is installed
			*/
			void SetRouter(Router *router);

			/**
			 * @brief Method to return the router where the application is installed
			 * @return Router reference where the application is installed
			*/
			Router *GetRouter();

			/**
			 * @brief Method to convert a binary bitstream that represents an IP address to dotted notation string
			 * @param ipBinary IP address in binary bitstream
			 * @return IP address in dotted notation string
			*/
			std::string binaryToDottedNotation(std::string ipBinary);

		protected:

			/**
			 * @brief Reference to the router where the application is installed
			*/
			Router *m_router;

	};
}
#endif