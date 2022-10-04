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

#include "test-app.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("Topology");

int 
main(int argc, char *argv[])
{

  Time::SetResolution(Time::NS);
  //LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
  //LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);


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


  // Create the UDP echo server to test the communication between 2 adjacent nodes
 // NS_LOG_INFO("Create UDP echo server");
//
 // UdpEchoServerHelper echoServer(9);
 // ApplicationContainer serverApps = echoServer.Install(c.Get(0));
 // serverApps.Start(Seconds(1.0));
 // serverApps.Stop(Seconds(10.0));


  //// Create the UDP echo client echo to test the communication between 2 adjacent nodes
  //NS_LOG_INFO("Create UDP echo client");
//
  //// Set information about the server on the client application
  //UdpEchoClientHelper echoClient(i1i2.GetAddress(0), 9);
  //echoClient.SetAttribute("MaxPackets", UintegerValue(1));
  //echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
  //echoClient.SetAttribute("PacketSize", UintegerValue(1024));
//
  //ApplicationContainer clientApps = echoClient.Install(c.Get(1));
  //clientApps.Start(Seconds(2.0));
  //clientApps.Stop(Seconds(10.0));
//


  // Run the simulation
  //Simulator::Run();
  //Simulator::Destroy();
  //return 0;

  Packet::EnablePrinting (); 

  //Create our Two UDP applications
  Ptr <TestApp> udp0 = CreateObject <TestApp> ();
  Ptr <TestApp> udp1 = CreateObject <TestApp> ();
  
  //Set the start & stop times
  udp0->SetStartTime (Seconds(0));
  udp0->SetStopTime (Seconds (10));
  udp1->SetStartTime (Seconds(0));
  udp1->SetStopTime (Seconds (10));
  
  //install one application at node 0, and the other at node 1
  c.Get(0)->AddApplication (udp0);
  c.Get(1)->AddApplication (udp1);
  
  //This is the IP address of node 1
  Ipv4Address dest_ip(i1i2.GetAddress(1));

  //Schedule an event to send a packet using udp0 targeting IP of node 0, and port 7777
  std::ostringstream msg; 
  msg << "Hello World!" << '\0';
  uint16_t packetSize = msg.str().length()+1;
  Ptr<Packet> packet1 = Create<Packet>((uint8_t*) msg.str().c_str(), packetSize);
  Simulator::Schedule(Seconds(1), &TestApp::SendPacket, udp0, packet1, dest_ip, 7777);
  
  //Another packet of size 800, targeting the same IP address, but a different port.
  Ptr <Packet> packet2 = Create <Packet> (800);
  Simulator::Schedule (Seconds (2), &TestApp::SendPacket, udp0, packet2, dest_ip, 9999);
    
  LogComponentEnable ("TestApp", LOG_LEVEL_INFO);

  Simulator::Stop(Seconds (10));
  Simulator::Run();
  Simulator::Destroy();

}
