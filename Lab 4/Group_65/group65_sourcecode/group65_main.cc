#include "header.h"

NS_LOG_COMPONENT_DEFINE ("Networks_Lab4");

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

int main (int argc, char *argv[])
{
	//The smallest measurable time interval is 1 ns
	Time::SetResolution (Time::NS);

	//Set the deafult configuration as TCP New Reno(mentioned in the problem statement)
	Config::SetDefault("ns3::TcpL4Protocol::SocketType", TypeIdValue (TcpNewReno::GetTypeId()));

	uint32_t bufferSize = 10 * packetSize;	

	// Set the title of the datasets
	dataset_buffsize_fairness.SetTitle("Fairness");
	dataset_buffsize_TCP.SetTitle("TCP Throughput");
	dataset_buffsize_UDP.SetTitle("UDP Throughput");
	dataset_Udprate_TCP.SetTitle("TCP Throughput");
	dataset_Udprate_UDP.SetTitle("UDP Throughput");
	dataset_Udprate_otherUDP.SetTitle("UDP Throughput of Other UDP Connection");
	
	// Set the style of the datasets
	dataset_buffsize_fairness.SetStyle(Gnuplot2dDataset::LINES_POINTS);
	dataset_buffsize_TCP.SetStyle(Gnuplot2dDataset::LINES_POINTS);
	dataset_buffsize_UDP.SetStyle(Gnuplot2dDataset::LINES_POINTS);
	dataset_Udprate_TCP.SetStyle(Gnuplot2dDataset::LINES_POINTS);
	dataset_Udprate_UDP.SetStyle(Gnuplot2dDataset::LINES_POINTS);
	dataset_Udprate_otherUDP.SetStyle(Gnuplot2dDataset::LINES_POINTS);

	// change buffersize after every iteration and caculate throughput
	for(;bufferSize<=800*packetSize;)
	{ 
		//Creating 8 nodes (one for each host and router.)
		NodeContainer nodes;
		nodes.Create(8);

		/*
		Create node containers for every link
		*/
		// Link H1-R1
		NodeContainer n0_3 = NodeContainer(nodes.Get(0), nodes.Get(3)); 
		// Link H2-R1
		NodeContainer n1_3 = NodeContainer(nodes.Get(1), nodes.Get(3)); 
		// Link H3-R1
		NodeContainer n2_3 = NodeContainer(nodes.Get(2), nodes.Get(3));
		// Link R1-R2 
		NodeContainer n3_4 = NodeContainer(nodes.Get(3), nodes.Get(4)); 
		// Link R2-H4
		NodeContainer n4_5 = NodeContainer(nodes.Get(4), nodes.Get(5));
		// Link R2-H5 
		NodeContainer n4_6 = NodeContainer(nodes.Get(4), nodes.Get(6));
		// Link R2-H6 
		NodeContainer n4_7 = NodeContainer(nodes.Get(4), nodes.Get(7)); 

		//Creating point to point channels between the nodes
		PointToPointHelper p2p;
		
		// installing the internet stack(protocols) on the nodes created above
		InternetStackHelper internet;
		internet.Install(nodes);

		// Reference : https://www.nsnam.org/docs/release/3.14/models/html/queue.html

		//Host to router links(H-R links)
		p2p.SetDeviceAttribute ("DataRate", StringValue("100Mbps"));
		p2p.SetChannelAttribute ("Delay", StringValue("10ms"));
		NetDeviceContainer d0_3 = p2p.Install(n0_3);
		NetDeviceContainer d1_3 = p2p.Install(n1_3);
		NetDeviceContainer d2_3 = p2p.Install(n2_3);
		NetDeviceContainer d4_5 = p2p.Install(n4_5);
		NetDeviceContainer d4_6 = p2p.Install(n4_6);
		NetDeviceContainer d4_7 = p2p.Install(n4_7);
		
		//Set the queueing configuration in the router to router link to DropTailQueue
		p2p.SetQueue ("ns3::DropTailQueue", "MaxSize", QueueSizeValue(QueueSize("85p")));
		p2p.SetDeviceAttribute ("DataRate", StringValue("10Mbps"));
		p2p.SetChannelAttribute ("Delay", StringValue("100ms"));
		NetDeviceContainer d3_4 = p2p.Install(n3_4);

		Ipv4AddressHelper ipv4;
		ipv4.SetBase ("10.1.1.0", "255.255.255.0");
		Ipv4InterfaceContainer i0_3 = ipv4.Assign(d0_3);
		ipv4.SetBase ("10.1.2.0", "255.255.255.0");
		Ipv4InterfaceContainer i1_3 = ipv4.Assign(d1_3);
		ipv4.SetBase ("10.1.3.0", "255.255.255.0");
		Ipv4InterfaceContainer i2_3 = ipv4.Assign(d2_3);
		ipv4.SetBase ("10.1.4.0", "255.255.255.0");
		Ipv4InterfaceContainer i3_4 = ipv4.Assign(d3_4);
		ipv4.SetBase ("10.1.5.0", "255.255.255.0");
		Ipv4InterfaceContainer i4_5 = ipv4.Assign(d4_5);
		ipv4.SetBase ("10.1.6.0", "255.255.255.0");
		Ipv4InterfaceContainer i4_6 = ipv4.Assign(d4_6);
		ipv4.SetBase ("10.1.7.0", "255.255.255.0");
		Ipv4InterfaceContainer i4_7 = ipv4.Assign(d4_7);

		Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

		// 4 TCP Reno Connections

		//TCP - H2(n1) to H5(n6)
		establishTCP(nodes, i4_6, 1, 6, bufferSize);

		//TCP - H2(n1) to H6(n7)
		establishTCP(nodes, i4_7, 1, 7, bufferSize);

		//TCP - H4(n5) to H6(n7)
		establishTCP(nodes, i4_7, 5, 7, bufferSize);

		//TCP - H4(n5) to H2(n1)
		establishUDP(nodes, i1_3, 5, 1, bufferSize);

		// 2 CBR traffic on UDP Flows

		FlowMonitorHelper flowmn;
		Ptr<FlowMonitor> fmonitor_ptr = flowmn.InstallAll();

		//UDP - H4(n5) to H3(n2)
		establishUDP(nodes, i2_3, 5, 2, bufferSize);

		//UDP - H1(n0) to H3(n2)
		Ptr<MyApp> udp_to_change = establishUDP(nodes, i2_3, 0, 2, bufferSize);

		//increasing the UDP data rate upto 100Mbps in discrete time steps for UDP between H1 and H3
		int boolvar=0;
		if(bufferSize==10*packetSize)
		{
			boolvar=1;
		}

		Simulator::Schedule(Seconds(2.0), &IncRate, udp_to_change, DataRate("30Mbps"), &flowmn, fmonitor_ptr, boolvar);
		Simulator::Schedule(Seconds(3.0), &IncRate, udp_to_change, DataRate("40Mbps"), &flowmn, fmonitor_ptr, boolvar);
		Simulator::Schedule(Seconds(4.0), &IncRate, udp_to_change, DataRate("50Mbps"), &flowmn, fmonitor_ptr, boolvar);
		Simulator::Schedule(Seconds(5.0), &IncRate, udp_to_change, DataRate("60Mbps"), &flowmn, fmonitor_ptr, boolvar);
		Simulator::Schedule(Seconds(6.0), &IncRate, udp_to_change, DataRate("70Mbps"), &flowmn, fmonitor_ptr, boolvar);
		Simulator::Schedule(Seconds(7.0), &IncRate, udp_to_change, DataRate("80Mbps"), &flowmn, fmonitor_ptr, boolvar);
		Simulator::Schedule(Seconds(8.0), &IncRate, udp_to_change, DataRate("90Mbps"), &flowmn, fmonitor_ptr, boolvar);
		Simulator::Schedule(Seconds(9.0), &IncRate, udp_to_change, DataRate("100Mbps"), &flowmn, fmonitor_ptr, boolvar);
		Simulator::Schedule(Seconds(10.0),&IncRate, udp_to_change, DataRate("100Mbps"), &flowmn, fmonitor_ptr, boolvar);


		NS_LOG_INFO ("Run Simulation");
		Simulator::Stop (Seconds(10.0));
		Simulator::Run ();

		fmonitor_ptr->CheckForLostPackets();

		//This is used to calculate the throughput at each second. 
		//Total throughput, throuput from TCP connections and throughput from UDP connections are calculated.
		Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmn.GetClassifier ());
		std::map<FlowId, FlowMonitor::FlowStats> fmStats = fmonitor_ptr->GetFlowStats ();

		double total_flow = 0;          //total throughput
	    double total_flow_square = 0;    //total squared throughput
	  	double total_TCP = 0;           //total throughtput in TCP conncetions
	    double total_UDP=0;             //total throughtput in UDP conncetions 

		for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = fmStats.begin (); i != fmStats.end (); ++i)
		{
			Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);

			//Calculate the throughput of the flow
			double throughPut = i->second.rxBytes * 8.0 / (i->second.timeLastRxPacket.GetSeconds() - i->second.timeFirstTxPacket.GetSeconds())/(1024*1024);
			total_flow += throughPut;
			total_flow_square += throughPut * throughPut;

            //TCP connections
			if(t.sourceAddress == "10.1.2.1" && t.destinationAddress == "10.1.6.2")
			{
				total_TCP += throughPut;
			}
			else if(t.sourceAddress == "10.1.2.1" && t.destinationAddress == "10.1.7.2")
			{
				total_TCP += throughPut;
			}
			else if(t.sourceAddress == "10.1.5.2" && t.destinationAddress == "10.1.7.2")
			{
				total_TCP += throughPut;
			}
			else if(t.sourceAddress == "10.1.5.2" && t.destinationAddress == "10.1.2.1" )
			{
				total_TCP += throughPut;
			}
			//UDP connections
			else if(t.sourceAddress == "10.1.5.2" && t.destinationAddress == "10.1.3.1" )
			{
				total_UDP += throughPut;
			}
			else if(t.sourceAddress == "10.1.1.1" && t.destinationAddress == "10.1.3.1" )
			{
				total_UDP += throughPut;
			}
		}
		//Calculate the fainess index from the throughput
		double FairnessIndex = (total_flow * total_flow)/ (6 * total_flow_square);

		//Adding to the dataset
		dataset_buffsize_fairness.Add (bufferSize/packetSize, FairnessIndex);
		dataset_buffsize_TCP.Add (bufferSize/packetSize, total_TCP);
		dataset_buffsize_UDP.Add (bufferSize/packetSize, total_UDP);
		std :: cout << "BufferSize: " << bufferSize/packetSize << "packets\t\t\t";
		std :: cout << "Fairness Index: " << (FairnessIndex*1000000)/1000000 << "\t\t\t";
		std :: cout << "TCP Throughput: " << total_TCP << "\t\t\t";
		std :: cout << "UDP Throughput: " << total_UDP << std :: endl;

		Simulator::Destroy ();

		if(bufferSize < 200*packetSize)
		{
			bufferSize+=30*packetSize;
		} 	
		else if(bufferSize == 220*packetSize)
		{
			bufferSize+=80*packetSize;
		}
		else 
		{
			bufferSize+=100*packetSize;
		}
	}

	// Initialize Plot file names
    std ::string fileNameWithNoExtension1 = "buffersize_fairness";
    std ::string graphicsFileName1 = fileNameWithNoExtension1 + ".png";
    std ::string plotFileName1 = fileNameWithNoExtension1 + ".plt";
    std ::string plotTitle1 = "Buffer Size vs Fairness plot";

    std ::string fileNameWithNoExtension2 = "buffersize_tcp";
    std ::string graphicsFileName2 = fileNameWithNoExtension2 + ".png";
    std ::string plotFileName2 = fileNameWithNoExtension2 + ".plt";
    std ::string plotTitle2 = "Buffer Size vs Throughput(TCP)";

    std ::string fileNameWithNoExtension3 = "buffersize_udp";
    std ::string graphicsFileName3 = fileNameWithNoExtension3 + ".png";
    std ::string plotFileName3 = fileNameWithNoExtension3 + ".plt";
    std ::string plotTitle3 = "Buffer Size vs Throughput(UDP)";

    std ::string fileNameWithNoExtension4 = "tcp_udprate";
    std ::string graphicsFileName4 = fileNameWithNoExtension4 + ".png";
    std ::string plotFileName4 = fileNameWithNoExtension4 + ".plt";
    std ::string plotTitle4 = "UDP Rate vs Throughput(TCP)";

    std ::string fileNameWithNoExtension5 = "udp_udprate";
    std ::string graphicsFileName5 = fileNameWithNoExtension5 + ".png";
    std ::string plotFileName5 = fileNameWithNoExtension5 + ".plt";
    std ::string plotTitle5 = "UDP Rate vs Throughput(UDP)";

    std ::string fileNameWithNoExtension6 = "otherudp_udprate";
    std ::string graphicsFileName6 = fileNameWithNoExtension6 + ".png";
    std ::string plotFileName6 = fileNameWithNoExtension6 + ".plt";
    std ::string plotTitle6 = "UDP Rate vs Throughput of other UDP connection";

    // Instantiate the plot and set its title.
    Gnuplot plot1(graphicsFileName1);
    Gnuplot plot2(graphicsFileName2);
    Gnuplot plot3(graphicsFileName3);
    Gnuplot plot4(graphicsFileName4);
    Gnuplot plot5(graphicsFileName5);
    Gnuplot plot6(graphicsFileName6);

    plot1.SetTitle(plotTitle1);
    plot2.SetTitle(plotTitle2);
    plot3.SetTitle(plotTitle3);
    plot4.SetTitle(plotTitle4);
    plot5.SetTitle(plotTitle5);
    plot6.SetTitle(plotTitle6);

    // Make the graphics file, which the plot file will create when it
    // is used with Gnuplot, be a PNG file.
    plot1.SetTerminal("png");
    plot2.SetTerminal("png");
    plot3.SetTerminal("png");
    plot4.SetTerminal("png");
    plot5.SetTerminal("png");
    plot6.SetTerminal("png");

    // Set the labels for each axis.
    plot1.SetLegend("Buffer Size(packets)", "Fairness");
    plot2.SetLegend("Buffer Size(packets)", "Throughput(in mbps)");
    plot3.SetLegend("Buffer Size(packets)", "Throughput(in mbps)");
    plot4.SetLegend("UDP Rate(Mbps)", "Throughput(Mbps)");
    plot5.SetLegend("UDP Rate(Mbps)", "Throughput(Mbps)");
    plot6.SetLegend("UDP Rate(Mbps)", "Throughput(Mbps)");

    // Set the x value ranges for each plots
    plot1.AppendExtra("set xrange [0:800]");
    plot2.AppendExtra("set xrange [0:800]");
    plot3.AppendExtra("set xrange [0:800]");
    plot4.AppendExtra("set xrange [10:100]");
    plot5.AppendExtra("set xrange [10:100]");
    plot6.AppendExtra("set xrange [10:100]");


    // Add the dataset_buffsize_fairness to the plot.
    plot1.AddDataset(dataset_buffsize_fairness);
    plot2.AddDataset(dataset_buffsize_TCP);
    plot3.AddDataset(dataset_buffsize_UDP);
    plot4.AddDataset(dataset_Udprate_TCP);
    plot5.AddDataset(dataset_Udprate_UDP);
    plot6.AddDataset(dataset_Udprate_otherUDP);

    // Open the plot file for buffer size vs fairness
    std::ofstream plotFile1(plotFileName1.c_str());
    plot1.GenerateOutput(plotFile1);
    plotFile1.close();

    // Open the plot file for buffer size vs TCP throughput
    std::ofstream plotFile2(plotFileName2.c_str());
    plot2.GenerateOutput(plotFile2);
    plotFile2.close();

    // Open the plot file for buffer size vs UDP throughput
    std::ofstream plotFile3(plotFileName3.c_str());
    plot3.GenerateOutput(plotFile3);
    plotFile3.close();

    // Open the plot file for udp rate vs TCP throughput
    std::ofstream plotFile4(plotFileName4.c_str());
    plot4.GenerateOutput(plotFile4);
    plotFile4.close();

    // Open the plot file for udp rate vs UDP throughput
    std::ofstream plotFile5(plotFileName5.c_str());
    plot5.GenerateOutput(plotFile5);
    plotFile5.close();

    // Open the plot file for udp rate vs other UDP throughput
    std::ofstream plotFile6(plotFileName6.c_str());
    plot6.GenerateOutput(plotFile6);
    plotFile6.close();

	NS_LOG_INFO ("Completed");
}