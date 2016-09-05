#include <iostream>
#include <fstream>
#include <string>
#include <cassert>

#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/netanim-module.h"
using namespace ns3;

int main (int argc, char *argv[])
{	

	// zadajemo azuriranje tabela rutiranja posle svake izmene stanja linka:
	Config::SetDefault ("ns3::Ipv4GlobalRouting::RespondToInterfaceEvents",
	BooleanValue (true));
	
	
	// pravimo cvorove:
	
	NodeContainer c;
	c.Create (6);
	
	// instaliramo stek protokola na cvorovima:	
	
	InternetStackHelper internet;
	internet.Install (c);
	
	NodeContainer n0n1 = NodeContainer (c.Get (0), c.Get (1));
	NodeContainer n0n2 = NodeContainer (c.Get (0), c.Get (2));
	NodeContainer n0n4 = NodeContainer (c.Get (0), c.Get (4));
	NodeContainer n1n3 = NodeContainer (c.Get (1), c.Get (3));
	NodeContainer n2n4 = NodeContainer (c.Get (2), c.Get (4));
	NodeContainer n3n4 = NodeContainer (c.Get (3), c.Get (4));
	NodeContainer n4n5 = NodeContainer (c.Get (4), c.Get (5));
	
	

	


	// pravimo linkove:
	PointToPointHelper p2p;

	p2p.SetDeviceAttribute ("DataRate", StringValue ("10Mbps"));
	p2p.SetChannelAttribute ("Delay", StringValue ("4ms"));
	
	//instaliranje linka izmedju cvorova
	NetDeviceContainer d0d1 = p2p.Install (n0n1);
	NetDeviceContainer d0d2 = p2p.Install (n0n2);
	p2p.SetChannelAttribute ("Delay", StringValue ("3ms"));

	NetDeviceContainer d0d4 = p2p.Install (n0n4);
	
	p2p.SetChannelAttribute ("Delay", StringValue ("2ms"));

	NetDeviceContainer d1d3 = p2p.Install (n1n3);
	NetDeviceContainer d2d4 = p2p.Install (n2n4);
	NetDeviceContainer d3d4 = p2p.Install (n3n4);

	p2p.SetChannelAttribute ("Delay", StringValue ("0ms"));
	NetDeviceContainer d4d5 = p2p.Install (n4n5);
	

	// dodeljujemo IP adrese:
	
	Ipv4AddressHelper ipv4;

	ipv4.SetBase ("164.70.1.0", "255.255.255.0");
	Ipv4InterfaceContainer i0i1 = ipv4.Assign (d0d1);
	ipv4.SetBase ("164.70.2.0", "255.255.255.0");
	Ipv4InterfaceContainer i0i2 =ipv4.Assign (d0d2);
	ipv4.SetBase ("164.70.3.0", "255.255.255.0");
	Ipv4InterfaceContainer i0i4 =ipv4.Assign (d0d4);
	ipv4.SetBase ("164.70.4.0", "255.255.255.0");
	Ipv4InterfaceContainer i1i3 =ipv4.Assign (d1d3);
	ipv4.SetBase ("164.70.5.0", "255.255.255.0");
	Ipv4InterfaceContainer i2i4 = ipv4.Assign (d2d4);
	ipv4.SetBase ("164.70.6.0", "255.255.255.0");
	Ipv4InterfaceContainer i3i4 = ipv4.Assign (d3d4);
	ipv4.SetBase ("164.70.7.0", "255.255.255.0");
	Ipv4InterfaceContainer i4i5 = ipv4.Assign (d4d5);


	// zadajemo metrike linkova
	// (pretpostavljena vrednost je 1):

	i0i1.SetMetric (0, 4);
	i0i1.SetMetric (1, 4);
	i0i2.SetMetric (0, 2);
	i0i2.SetMetric (1, 2);
	i0i4.SetMetric (0, 6);
	i0i4.SetMetric (1, 6);
	i1i3.SetMetric (0, 2);
	i1i3.SetMetric (1, 2);
	i3i4.SetMetric (0, 3);
	i3i4.SetMetric (1, 3);

	// pravimo tabele rutiranja:
	

	Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
	

	// pravimo izvor saobracaja
	// i instaliramo ga na cvor 0:

	uint16_t port = 4001;
	OnOffHelper onoff ("ns3::TcpSocketFactory",InetSocketAddress (i4i5.GetAddress (1), port));
	onoff.SetAttribute ("OnTime",  StringValue("ns3::ConstantRandomVariable[Constant=100.0]"));
	onoff.SetAttribute ("OffTime",  StringValue("ns3::ConstantRandomVariable[Constant=0.0]"));
	onoff.SetAttribute ("DataRate", StringValue ("100kbps"));
	onoff.SetAttribute ("PacketSize", UintegerValue (150));

	ApplicationContainer apps = onoff.Install (c.Get (0));
	
	apps.Start (Seconds (1.0));
	apps.Stop (Seconds (5.0));

	// pravimo prijemnik saobracaja
	// i instaliramo ga na cvor 4:

	PacketSinkHelper sink ("ns3::TcpSocketFactory",Address (InetSocketAddress(Ipv4Address::GetAny (), port)));

	apps = sink.Install (c.Get (5));
	apps.Start (Seconds (1.0));
	apps.Stop (Seconds (5.0));

	// zadajemo snimanje izvestaja:
	
	AsciiTraceHelper ascii;
	Ptr<OutputStreamWrapper> stream = ascii.CreateFileStream ("dinamicko.tr");
	p2p.EnableAsciiAll (stream);
	internet.EnableAsciiIpv4All (stream);

	p2p.EnablePcapAll ("dinamicko");

	// postavljamo pointere:

	Ptr<Node> n0 = c.Get (0);
	Ptr<Ipv4> ipv4n0 = n0->GetObject<Ipv4> ();
	Ptr<Node> n2 = c.Get (2);
	Ptr<Ipv4> ipv4n2 = n2->GetObject<Ipv4> ();


	// u t = 2 s iskljucujemo link n2-n4:

	Simulator::Schedule (Seconds (2), &Ipv4::SetDown, ipv4n2, 2);

	// u t = 3 s iskljucujemo link n0-n4:

	Simulator::Schedule (Seconds (3), &Ipv4::SetDown, ipv4n0, 3);

 	// u t = 4 s ukljucujemo link n1-n2:
	
	Simulator::Schedule (Seconds (4), &Ipv4::SetUp, ipv4n2, 2);

	// snimamo tabele rutiranja:

	Ipv4GlobalRoutingHelper g;
	Ptr<OutputStreamWrapper> routingStream =Create<OutputStreamWrapper> ("dinamicko.routes", std::ios::out);

	g.PrintRoutingTableAllAt (Seconds (1), routingStream);
	g.PrintRoutingTableAllAt (Seconds (2), routingStream);
	g.PrintRoutingTableAllAt (Seconds (3), routingStream);
	g.PrintRoutingTableAllAt (Seconds (4), routingStream);
	g.PrintRoutingTableAllAt (Seconds (5), routingStream);

	//NetAnim
	std::string animFile = "dini.xml";
	AnimationInterface anim(animFile);
	// postavljamo potrebne pointere
	Ptr<Node> n1 = c.Get(1);
	Ptr<Node> n3 = c.Get(3);
	Ptr<Node> n4 = c.Get(4);
	Ptr<Node> n5 = c.Get(5);
	// zadajemo poziciju cvora n0 u NetAnim animaciji
	anim.SetConstantPosition( n0, 100, 100);
	anim.SetConstantPosition( n1, 200, 100);
	anim.SetConstantPosition( n2, 100, 200);
	anim.SetConstantPosition( n3, 200, 200);
	anim.SetConstantPosition( n4, 300, 150);
	anim.SetConstantPosition( n5, 300, 200);
	
	
	// pokre¢emo simulaciju
	// i po njenom zavrsetku uni²tavamo sve napravljene objekte:

	Simulator::Run ();
	Simulator::Destroy ();
}
