#ifndef NS3_BGP_SERVER_H
#define NS3_BGP_SERVER_H
#include "ns3/socket.h"
#include "ns3/application.h"
#include "TCP-common.h"

using namespace ns3;

namespace ns3 {
    class BGPServer : public TCPCommon {
    
        public:

            /**
             * @brief Constructor for BGPServer
             **/
            BGPServer();

            /**
             * @brief Destructor for BGPServer
             **/
            virtual ~BGPServer();

            /**
             * @brief Method to return the TypeId of the BGPServer
             * @return TypeId of the BGPServer
             **/            
            static TypeId GetTypeId (void);

            /**
             * @brief Method to setup and initialize the BGPServer
             * @param serverPort Port on which the BGPServer should listen
             * @param startTime Time at which the BGPServer should start
             **/
            void Setup(uint16_t serverPort, Time startTime);

            /**
             * @brief Method to return the socket used by the BGPServer
             * @return Socket used by the BGPServer
             **/
            Ptr<Socket> GetSocket(void) const;

        private:

            /**
             * @brief Method to start the BGPServer
             **/
            virtual void StartApplication();

            /**
             * @brief Method to stop the BGPServer
             **/
            virtual void StopApplication();

            /**
             * @brief Method to dispose the BGPServer
             **/
            void DoDispose();

            /**
             * @brief Callback to handle the closing of a connection by a peer
             * @param socket Socket of the client
             **/
            void HandlePeerClose (Ptr<Socket> socket);

            /**
             * @brief Callback to handle the error of a connection
             * @param socket Socket of the client
             **/
            void HandlePeerError (Ptr<Socket> socket);

            /**
             * @brief Callback to handle the accept of a connection
             * @param socket Socket of the client
             * @param from Address of the client
             **/
            void HandleAccept (Ptr<Socket> socket, const Address& from);

            /**
             * @brief Callback to handle a successful closing of a connection
             * @param socket Socket of the client
             **/
            void HandleSuccessClose(Ptr<Socket> socket);

            /**
             * @brief Callback to handle the read of a message from a client
             * @param socket Socket of the client
             **/
            void HandleRead (Ptr<Socket> socket);

            /**
             * Socket used by the BGPServer
            */
            Ptr<Socket>             m_socket;

            /**
             * List of the connected sockets of the clients
            */
            std::list<Ptr<Socket> > m_socketList;

            /**
             * TypeId of the BGPServer
            */
            TypeId                  m_tid;

            /**
             * Time at which the BGPServer should start
            */
            Time                    m_startTime;

            /**
             * Port on which the BGPServer should listen
            */
            uint16_t                m_port;

    };
}
#endif