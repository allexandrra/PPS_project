#ifndef NS3_BGP_CLIENT_H
#define NS3_BGP_CLIENT_H
#include "ns3/socket.h"
#include "ns3/application.h"
#include "TCP-common.h"

using namespace ns3;

namespace ns3 {
    class BGPClient : public TCPCommon {

        public:

            /**
             * @brief Constructor for BGPClient
             **/
            BGPClient ();

            /**
             * @brief Destructor for BGPClient
             **/
            virtual ~BGPClient();

            /**
             * @brief Method to setup and initialize the BGPClient
             * @param socket Socket to be used by the BGPClient
             * @param address Address of the BGPClient
             * @param startTime Time at which the BGPClient should start
             * @param stopTime Time at which the BGPClient should stop
             **/
            void Setup(Ptr<Socket> socket, Address address, Time startTime, Time stopTime);

            /**
             * @brief Method to add a single packet rapresented by a stringstream to the queue of messages to send
             * @param msg Message to be added to the queue
             * @param atTime Time at which the message should be sent
             **/
            void AddPacketsToQueue(std::stringstream& msg, Time atTime);

            /**
             * @brief Method to add a periodically the KeepAlive packet to the queue of messages to send
             * @param events Vector of the schedule events (that rappresent a message added to the queue)
             **/
            void AddPacketsToQueuePeriodically(std::vector<EventId> events);

             /**
             * @brief Method to add a periodically the TrustRate packet to the queue of messages to send
             * @param events Vector of the schedule events (that rappresent a message added to the queue)
             **/
            void exchangeTrust(std::vector<EventId> events);

            /**
             * @brief Method to return the socket used by the BGPClient
             * @return Socket used by the BGPClient
             **/
            Ptr<Socket> get_socket(void) const;

        private:

            /**
             * @brief Method to start the BGPClient
             **/
            virtual void StartApplication(void);

            /**
             * @brief Method to stop the BGPClient
             **/
            virtual void StopApplication(void);

            /**
             * @brief Callback used to handle a successful connection
             * @param socket Socket used by the BGPClient for the connection
             **/
            void ConnectionSucceeded(Ptr<Socket>);

            /**
             * @brief Callback used to handle a failed connection
             * @param socket Socket used by the BGPClient for the connection
             **/
            void ConnectionFailed(Ptr<Socket>);

            /**
             * @brief Method to send the packets in the queue
             **/
            void SendMyPackets();

            /**
             * @brief Callback to handle the read of a packet from the socket with specific implementation for BGPClient
             * @param socket Socket used by the BGPClient for the connection
             **/
            void HandleRead (Ptr<Socket> socket);


            /**
             * @brief Socket used by the BGPClient
            */
            Ptr<Socket>         m_socket;

            /**
             * @brief Address of the BGPClient peeer (server) to connect to
            */
            Address             m_peer;

            /**
             * @brief Flag to check if the BGPClient is running
            */
            bool                m_running;

            /**
             * @brief Time at which the BGPClient should start
            */
            Time                m_start;

            /**
             * @brief Time at which the BGPClient should stop
            */
            Time                m_stop;

            /**
             * @brief Event ID of the last packet sent
            */
            EventId             m_sendEvent;

            /**
             * @brief List of the packets that still need to be sent
            */
            std::list<Ptr<Packet>>   m_pktList;

            /**
             * @brief List of the times at which the packets should be sent
            */
            std::list<Time>     m_pktDelayList;

    };
}
#endif