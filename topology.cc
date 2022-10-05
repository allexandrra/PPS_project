/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

//
// Network topology: Topology 1 image on Drive
// All links are point-to-point links 
// To run the simulation: ./ns3 run scratch/PPS_project/Topology.cc
// To view the logs: export NS_LOG=Topology=info

#include <iostream>
#include <fstream>
#include <string>
#include <cassert>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/ipv4-global-routing-helper.h"

#include "tcp-receiver.h"
#include "tcp-sender.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("Topology");

int main(int argc, char *argv[])
{

  Time::SetResolution(Time::NS);

  // Create the 10 nodes for the topology
  NS_LOG_INFO("Create nodes");
  NodeContainer c;
  c.Create(11);
  NodeContainer as1as2 = NodeContainer(c.Get(0), c.Get(1));
  NodeContainer as2as3 = NodeContainer(c.Get(1), c.Get(2));
  NodeContainer as3as4 = NodeContainer(c.Get(2), c.Get(3));
  NodeContainer as4as5 = NodeContainer(c.Get(3), c.Get(4));
  NodeContainer as5as6 = NodeContainer(c.Get(4), c.Get(5));
  NodeContainer as2as7 = NodeContainer(c.Get(1), c.Get(6));
  NodeContainer as7as5 = NodeContainer(c.Get(6), c.Get(4));
  NodeContainer as2as8 = NodeContainer(c.Get(1), c.Get(7));
  NodeContainer as8as9 = NodeContainer(c.Get(7), c.Get(8));
  NodeContainer as9as10 = NodeContainer(c.Get(8), c.Get(9));
  NodeContainer as10as6 = NodeContainer(c.Get(9), c.Get(5));


  InternetStackHelper internet;
  internet.Install(c);

  // Create the 11 peer-to-peer channels without any IP addressing information
  NS_LOG_INFO("Create channels");
  PointToPointHelper p2p;
  p2p.SetDeviceAttribute("DataRate", StringValue("1024Mbps"));
  p2p.SetChannelAttribute("Delay", StringValue("2ms"));

  NetDeviceContainer d1d2 = p2p.Install(as1as2);
  NetDeviceContainer d2d3 = p2p.Install(as2as3);
  NetDeviceContainer d3d4 = p2p.Install(as3as4);
  NetDeviceContainer d4d5 = p2p.Install(as4as5);
  NetDeviceContainer d5d6 = p2p.Install(as5as6);
  NetDeviceContainer d2d7 = p2p.Install(as2as7);
  NetDeviceContainer d7d5 = p2p.Install(as7as5);
  NetDeviceContainer d2d8 = p2p.Install(as2as8);
  NetDeviceContainer d8d9 = p2p.Install(as8as9);
  NetDeviceContainer d9d10 = p2p.Install(as9as10);
  NetDeviceContainer d10d6 = p2p.Install(as10as6);

  // Add IP addresses to the channels
  NS_LOG_INFO("Assign IP Addresses");
  Ipv4AddressHelper ipv4;
  
  ipv4.SetBase("160.0.0.0", "255.255.255.252");
  Ipv4InterfaceContainer i1i2 = ipv4.Assign(d1d2);
  ipv4.SetBase("160.0.0.4", "255.255.255.252");
  Ipv4InterfaceContainer i2i3 = ipv4.Assign(d2d3);
  ipv4.SetBase("160.0.0.12", "255.255.255.252");
  Ipv4InterfaceContainer i3i4 = ipv4.Assign(d3d4);
  ipv4.SetBase("160.0.0.20", "255.255.255.252");
  Ipv4InterfaceContainer i4i5 = ipv4.Assign(d4d5);
  ipv4.SetBase("160.0.0.24", "255.255.255.252"); 
  Ipv4InterfaceContainer i5i6 = ipv4.Assign(d5d6);
  ipv4.SetBase("160.0.0.8", "255.255.255.252"); 
  Ipv4InterfaceContainer i2i7 = ipv4.Assign(d2d7);
  ipv4.SetBase("160.0.0.16", "255.255.255.252"); 
  Ipv4InterfaceContainer i7i5 = ipv4.Assign(d7d5);
  ipv4.SetBase("160.0.0.40", "255.255.255.252"); 
  Ipv4InterfaceContainer i2i8 = ipv4.Assign(d2d8);
  ipv4.SetBase("160.0.0.36", "255.255.255.252"); 
  Ipv4InterfaceContainer i8i9 = ipv4.Assign(d8d9);
  ipv4.SetBase("160.0.0.32", "255.255.255.252"); 
  Ipv4InterfaceContainer i9i10 = ipv4.Assign(d9d10);
  ipv4.SetBase("160.0.0.28", "255.255.255.252"); 
  Ipv4InterfaceContainer i10i6 = ipv4.Assign(d10d6);

  NS_LOG_INFO("IP address of server: " << i1i2.GetAddress(0));
  NS_LOG_INFO("IP address of client: " << i1i2.GetAddress(1));

  // Create router nodes, initialize routing database and set up the routing tables in the nodes
  Ipv4GlobalRoutingHelper::PopulateRoutingTables();
  // Enable packet printing
  Packet::EnablePrinting (); 

  LogComponentEnable("TcpReceiver", LOG_LEVEL_INFO);
  LogComponentEnable("TcpSender", LOG_LEVEL_INFO);

  // Settings
  uint16_t serverPort   = 179;
  // Set timings
  Time startClient    = Seconds(1.);
  Time stopClient     = Seconds(300.);
  Time startServer    = Seconds(0.);
  Time stopServer     = Seconds(300.0);
  Time stopSimulation = Seconds(310.0);  // giving some more time than the server stop

  // Applications
  // Tcp RECEIVER -> now on as1 which is directly linked (p2p) with as2
  Address receiverAddress (InetSocketAddress (i1i2.GetAddress(0), serverPort));
  Ptr<TcpReceiver> receiverApp = CreateObject<TcpReceiver>();
  receiverApp->Setup(serverPort, startClient);

  // Install and load the server
  as1as2.Get(0)->AddApplication(receiverApp);
  receiverApp->SetStartTime(startServer);
  receiverApp->SetStopTime(stopServer);

  // Tcp SENDER -> now on the fas2
  Ptr<Socket> ns3TcpSocket = Socket::CreateSocket (as1as2.Get(1), TcpSocketFactory::GetTypeId ());
  Ptr<TcpSender> senderApp = CreateObject<TcpSender> ();
  senderApp->Setup (ns3TcpSocket, receiverAddress, startClient, stopClient);

  // Install and load the client
  as1as2.Get(1)->AddApplication (senderApp);
  senderApp->SetStartTime (startClient);
  senderApp->SetStopTime (stopClient);

  Simulator::Stop (stopSimulation);
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;

}