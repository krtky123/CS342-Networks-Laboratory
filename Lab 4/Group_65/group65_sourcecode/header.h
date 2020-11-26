/* Network topology

// This diagram shows a Dumbbell topology with two routers R1 and R2. 
// Each router( R1 and R2) is connected to 3 hosts.
// R1 is connected to H1,H2,H3 and R2 is connected to H4,H5,H6. 
// The link between the routers (R1 and R2) is 10 Mb/s, 100 ms.
// All other links are 100 Mb/s, 10 ms.


  (H1)n0                                n5(H4)
       \ 100 Mb/s, 10ms                /
        \          10Mb/s, 100ms      /   
(H2)n1---n3(R1)------------------n4(R2)----n6(H5)
        /                             \
       /                               \  
  (H3)n2                                n7(H6)

*/

/*
Tasks:
 1. Increase the rate of one UDP flow up to 100 Mbps and observe its impact on the throughput of the TCP flows and the other UDP flow.
 2. Vary the buffer size in the range of 10 packets to 800 packets and repeat the above 
    experiments to find out the impact of buffer size on the fair share of bandwidth.
*/

#include <fstream>
#include <string>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/point-to-point-layout-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/gnuplot.h"

#define NUM 1024

using namespace ns3;

static uint16_t sinkPort = 8080;
uint32_t start_time_sink = 0;
uint32_t start_time_apps = 0.5;

//Run time for the applications
uint32_t run_time = 10;

//Packet size 1.5 KB (1500 Bytes)
uint32_t packetSize = 1500;

uint32_t packetsToSend = 1000000;
uint32_t variable_rate = 20;

// Instantiating Datasets
Gnuplot2dDataset dataset_buffsize_fairness;
Gnuplot2dDataset dataset_buffsize_TCP;
Gnuplot2dDataset dataset_buffsize_UDP;

Gnuplot2dDataset dataset_Udprate_TCP;
Gnuplot2dDataset dataset_Udprate_UDP;
Gnuplot2dDataset dataset_Udprate_otherUDP;

class MyApp : public Application
{

  public:
    // Constructor Function for initialization purposes
    MyApp ():m_socket (0),
      m_peer (),
      m_packetSize (0),
      m_nPackets (0),
      m_dataRate (0),
      m_sendEvent (),
      m_running (false),
      m_packetsSent (0){}

    // Destructor Function  
    virtual ~MyApp()
    {
      m_socket = 0;
    }

    // Utility Functions
    void Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate)
    {
      m_socket = socket;
      m_peer = address;
      m_packetSize = packetSize;
      m_nPackets = nPackets;
      m_dataRate = dataRate;
    }

    // Public Function to change the data rate of the application
    void ChangeRate(DataRate newrate)
    {
      m_dataRate = newrate;
      return;  
    }

  private:

    virtual void StartApplication (void)
    {
      m_running = true;
      m_packetsSent = 0;
      m_socket->Bind ();
      m_socket->Connect (m_peer);
      SendPacket ();
    }

    virtual void StopApplication (void)
    {
      m_running = false;
      if(m_sendEvent.IsRunning ())
      {
        Simulator::Cancel (m_sendEvent);
      }

      if(m_socket)
      {
        m_socket->Close ();
      }
    }

    void ScheduleTx (void)
    {
      if(m_running)
      {
        Time tNext (Seconds (m_packetSize * 8 / static_cast<double> (m_dataRate.GetBitRate ())));
        m_sendEvent = Simulator::Schedule (tNext, &MyApp::SendPacket, this);
      }
    }

    void SendPacket (void)
    {
      Ptr<Packet> packet = Create<Packet> (m_packetSize);
      m_socket->Send (packet);

      if(++m_packetsSent < m_nPackets)
      {
        ScheduleTx ();
      }
    }

    // Data member declarations
    Ptr<Socket>     m_socket;
    Address         m_peer;
    uint32_t        m_packetSize;
    uint32_t        m_nPackets;
    DataRate        m_dataRate;
    EventId         m_sendEvent;
    bool            m_running;
    uint32_t        m_packetsSent;
};

//This function is used to increase the data rate of an application
// We are increasing the rate of second UDP Flow i.e. between 10.1.1.1 and 10.1.3.1
void IncRate (Ptr<MyApp> app, DataRate newrate, FlowMonitorHelper *flowMonitorHelp, Ptr<FlowMonitor> flowMonitor, int boolvar)
{
  app->ChangeRate(newrate);
  if(boolvar==1)
  {
  	Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowMonitorHelp->GetClassifier ());
  	std::map<FlowId, FlowMonitor::FlowStats> fmStats = flowMonitor->GetFlowStats ();

  	double total_flow = 0;          //total throughput
    double total_flow_square = 0;    //total squared throughput
  	double total_TCP = 0;           //total throughtput in TCP conncetions
    double total_UDP=0;             //total throughtput in UDP conncetions 
    double totOtherUDP=0;        //total throughtput in other UDP conncetions
  	double TCP[4];               // 4 elements, one for each TCP connection
  	double UDP[2];	             // 2 elements, one for each UDP connection

  	for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = fmStats.begin (); i != fmStats.end (); ++i)
  	{
  		Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);

  		//Calculating the throughput
  		double throughPut = i->second.rxBytes * 8.0 / (i->second.timeLastRxPacket.GetSeconds() - i->second.timeFirstTxPacket.GetSeconds())/(NUM*NUM);
  		total_flow += throughPut;
  		total_flow_square += throughPut * throughPut;

  		// Calculating throughput from TCP connections.
  		if(t.sourceAddress == "10.1.2.1" and t.destinationAddress == "10.1.6.2") //h2-h5
  		{
  		    total_TCP += throughPut;
  		    TCP[0] += throughPut;
  		}
  		else if(t.sourceAddress == "10.1.2.1" and t.destinationAddress == "10.1.7.2") //h2-h6
  		{
  		  total_TCP += throughPut;
  		  TCP[1] += throughPut;
  		}
  		else if(t.sourceAddress == "10.1.5.2" and t.destinationAddress == "10.1.7.2")  //h4-h6
  		{
  	  		total_TCP += throughPut;
  		    TCP[2] += throughPut;
  		}
  		else if(t.sourceAddress == "10.1.5.2" and t.destinationAddress == "10.1.2.1" ) //h4-h2
  		{
  			total_TCP += throughPut;
  		  	TCP[3] += throughPut;
  		}
  		//Calculating throughput from UDP connections
  		else if(t.sourceAddress == "10.1.5.2" and t.destinationAddress == "10.1.3.1" ) //h4-h3
  		{
  			total_UDP += throughPut;
  			totOtherUDP += throughPut;
  		  	UDP[0] += throughPut;
  		}
  		else if(t.sourceAddress == "10.1.1.1" and t.destinationAddress == "10.1.3.1" ) //h1-h3
  		{
  			total_UDP += throughPut;
  		  	UDP[1] += throughPut;
  		}
	  }

  	if(total_TCP!=0)
  	 dataset_Udprate_TCP.Add(variable_rate, total_TCP);

  	if(total_UDP!=0)
  	 dataset_Udprate_UDP.Add(variable_rate, total_UDP);

  	if(totOtherUDP!=0)
  	 dataset_Udprate_otherUDP.Add(variable_rate, totOtherUDP); 

  	std::cout << "UDP Data Rate: " << variable_rate << "Mbps\t\t\t";
  	std::cout << "TCP Throughput: " << total_TCP << "\t\t\t";
  	std::cout << "UDP Throughput: " << total_UDP << "\t\t\t";
	  std::cout << "Other UDP Throughput: " << totOtherUDP << std :: endl;
  }

  variable_rate+=10;    //to increase the data rate for the next call

}

//To establish TCP connection between trace source and trace sink and returns the tcp agent
Ptr<MyApp> establishTCP(NodeContainer &nodes, Ipv4InterfaceContainer &intface, int source, int sink, int bsize)
{
  sinkPort = sinkPort+1;
  Address sinkAddress(InetSocketAddress(intface.GetAddress(1),sinkPort));
  PacketSinkHelper pckSink("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), sinkPort));

  //Sink application using packet sink helper which consumes all packets from port1
  ApplicationContainer sinkApps = pckSink.Install(nodes.Get(sink));
  sinkApps.Start(Seconds(start_time_sink));
  sinkApps.Stop(Seconds(run_time));

  // TCP socket creation
  Ptr<Socket> tcpSocket = Socket::CreateSocket(nodes.Get (source), TcpSocketFactory::GetTypeId());
  tcpSocket->SetAttribute("SndBufSize", ns3::UintegerValue(bsize));
  tcpSocket->SetAttribute("RcvBufSize", ns3::UintegerValue(bsize));

  // Sender application creation
  Ptr<MyApp> tcp_agent = CreateObject<MyApp> ();
  tcp_agent->Setup(tcpSocket, sinkAddress, packetSize, packetsToSend, DataRate ("20Mbps"));
  nodes.Get(source)->AddApplication(tcp_agent);
  tcp_agent->SetStartTime(Seconds(start_time_apps));
  tcp_agent->SetStopTime(Seconds(run_time));	
  
  return tcp_agent;
}

//Function for establishing UDP connection between trace source and trace sink and it returns the UDP agent
Ptr<MyApp> establishUDP(NodeContainer &nodes, Ipv4InterfaceContainer &intface, int source, int sink, int bsize){
    
   //Assign a new port for every new connection    
  sinkPort=sinkPort+1;

  //Creating sink
  Address sinkAddress(InetSocketAddress(intface.GetAddress(0), sinkPort));
  PacketSinkHelper pckSink("ns3::UdpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), sinkPort));

  //Creating a sink application to consume all packets from the port above
  ApplicationContainer sinkApps = pckSink.Install(nodes.Get(sink));
  sinkApps.Start(Seconds(start_time_sink));
  sinkApps.Stop(Seconds(run_time));

  // socket creation
  Ptr<Socket> ns3UdpSocket = Socket::CreateSocket(nodes.Get(source), UdpSocketFactory::GetTypeId());
  ns3UdpSocket->SetAttribute("RcvBufSize",  ns3::UintegerValue(bsize));

  // Sender application creation
  Ptr<MyApp> udp_agent = CreateObject<MyApp> ();
  udp_agent->Setup(ns3UdpSocket, sinkAddress, packetSize, packetsToSend, DataRate("20Mbps"));
  nodes.Get (source)->AddApplication (udp_agent);
  udp_agent->SetStartTime(Seconds(start_time_apps));
  udp_agent->SetStopTime(Seconds(run_time));

  return udp_agent;
}
