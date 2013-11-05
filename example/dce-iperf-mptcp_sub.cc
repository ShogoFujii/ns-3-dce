#include <iostream>

#include "ns3/network-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/dce-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/netanim-module.h"
#include "ns3/constant-position-mobility-model.h"

using namespace ns3;
using namespace std;

void setPos (Ptr<Node> n, int x, int y, int z)
{
  Ptr<ConstantPositionMobilityModel> loc = CreateObject<ConstantPositionMobilityModel> ();
  n->AggregateObject (loc);
  Vector locVec2 (x, y, z);
  loc->SetPosition (locVec2);
}

int main (int argc, char *argv[])
{ 
  uint32_t nSize = 4;
  CommandLine cmd;
  cmd.AddValue ("nSize", "FatTree Size. Default 4", nSize);
  cmd.Parse (argc, argv);
  uint32_t nCore = nSize*nSize;
  uint32_t nAggr = 2*nSize*nSize;
  uint32_t nEdge = 2*nSize*nSize;
  uint32_t nHost = nEdge*nSize;
  /* 
  for(uint32_t i=0;i<nCore;i++){
   
  }*/
  NodeContainer core, aggr, edge, host;
  core.Create (nCore);
  aggr.Create (nAggr);
  edge.Create (nEdge);
  host.Create (nHost);

  DceManagerHelper dceManager;
  dceManager.SetTaskManagerAttribute ("FiberManagerType",
                                      StringValue ("UcontextFiberManager"));

  dceManager.SetNetworkStack ("ns3::LinuxSocketFdFactory",
                              "Library", StringValue ("liblinux.so"));
  LinuxStackHelper stack;
  stack.Install (core);
  stack.Install (aggr);
  stack.Install (edge);
  stack.Install (host);

  dceManager.Install (core);
  dceManager.Install (aggr);
  dceManager.Install (edge);
  dceManager.Install (host);

  PointToPointHelper pointToPoint_he, pointToPoint_ea, pointToPoint_ac;
  NetDeviceContainer devices_he[nEdge], devices_ea[nAggr], devices_ac[nAggr];
  Ipv4AddressHelper address_he, address_ea, address_ac;
  Ipv4InterfaceContainer if_he[nAggr], if_ea[nAggr], if_ac[nAggr];
  std::ostringstream cmd_oss;
  address_he.SetBase ("10.1.0.0", "255.255.255.0");
  address_ea.SetBase ("10.3.0.0", "255.255.255.0");
  address_ac.SetBase ("10.5.0.0", "255.255.255.0");

  pointToPoint_he.SetDeviceAttribute ("DataRate", StringValue ("250Mbps"));
  pointToPoint_he.SetChannelAttribute ("Delay", StringValue ("10ms"));

  pointToPoint_ea.SetDeviceAttribute ("DataRate", StringValue ("500Mbps"));
  pointToPoint_ea.SetChannelAttribute ("Delay", StringValue ("10ms"));

  pointToPoint_ac.SetDeviceAttribute ("DataRate", StringValue ("1Gbps"));
  pointToPoint_ac.SetChannelAttribute ("Delay", StringValue ("10ms"));

  cout << "nSize : " << nSize << endl;
  cout << "nCore : " << nCore << endl;
  cout << "nEdge : " << nEdge << endl;
  cout << "nAggr : " << nAggr << endl;
  cout << "nHost : " << nHost << endl;
  uint32_t i = 0, j = 0, k = 0;
/*
  for (uint32_t i = 0;i < nEdge; i++){
	//from host to edge
	for (uint32_t j = 0; j < nSize; j++){
	   k = i * nSize + j;
	   devices_he[i] = pointToPoint_he.Install(edge.Get (i), host.Get (k));
	   if_he[i] = address_he.Assign (devices_he[i]);
	   address_he.NewNetwork();
           //setup ip route
	   ////configure of host
           cmd_oss.str ("");
           cmd_oss << "rule add from " << if_he[i].GetAddress (1, 0) << " table " << 1;
           LinuxStackHelper::RunIp (host.Get (k), Seconds (0.1), cmd_oss.str ().c_str ());
           cmd_oss.str ("");
           cmd_oss << "route add 10.1." << k << ".0/24 dev sim" << 0 << " scope link table " << 1;
	   LinuxStackHelper::RunIp (host.Get (k), Seconds (0.1), cmd_oss.str ().c_str ());
	   cmd_oss.str ("");
           cmd_oss << "route add default via " << if_he[i].GetAddress (0, 0) << " dev sim" << 0 << " table " << 1;
           LinuxStackHelper::RunIp (host.Get (k), Seconds (0.1), cmd_oss.str ().c_str ());
           ////configure of edge
           cmd_oss.str ("");
           cmd_oss << "rule add from " << if_he[i].GetAddress (0, 0) << " table " << (j+1);
           LinuxStackHelper::RunIp (edge.Get (i), Seconds (0.2), cmd_oss.str ().c_str ());
           cmd_oss.str ("");
           cmd_oss << "route add 10.1." << k << ".0/24 dev sim" << j << " scope link table " << (j+1);
	   LinuxStackHelper::RunIp (edge.Get (i), Seconds (0.2), cmd_oss.str ().c_str ());
	   cmd_oss.str ("");
           cmd_oss << "route add default via " << if_he[i].GetAddress (0, 0) << " dev sim" << j << " table " << (j+1);
           LinuxStackHelper::RunIp (edge.Get (i), Seconds (0.2), cmd_oss.str ().c_str ());
	}
	//from edge to aggr
	if(i % 2 ==0){
	   devices_ea[i] = pointToPoint_ea.Install(aggr.Get (i), edge.Get (i));
	   if_ea[i] = address_ea.Assign (devices_ea[i]);
	   address_ea.NewNetwork();
	   //setup ip route 
	   ////configure of aggr
	   cmd_oss.str ("");
           cmd_oss << "rule add from " << if_ea[i].GetAddress (1, 0) << " table " << 1;
           LinuxStackHelper::RunIp (aggr.Get (i), Seconds (0.2), cmd_oss.str ().c_str ());
           cmd_oss.str ("");
           cmd_oss << "route add 10.3." << 2 * i << ".0/24 dev sim" << 0 << " scope link table " << 1;
	   LinuxStackHelper::RunIp (aggr.Get (i), Seconds (0.2), cmd_oss.str ().c_str ());
	   cmd_oss.str ("");
           cmd_oss << "route add default via " << if_he[i].GetAddress (0, 0) << " dev sim" << 0 << " table " << 1;
           LinuxStackHelper::RunIp (aggr.Get (i), Seconds (0.2), cmd_oss.str ().c_str ());

	   devices_ea[i] = pointToPoint_ea.Install(aggr.Get (i), edge.Get (i+1));
	   ////configure of edge
	   cmd_oss.str ("");
           cmd_oss << "rule add from " << if_ea[i].GetAddress (0, 0) << " table " << nSize+1;
           LinuxStackHelper::RunIp (edge.Get (i), Seconds (0.2), cmd_oss.str ().c_str ());
           cmd_oss.str ("");
           cmd_oss << "route add 10.3." << 2 * i << ".0/24 dev sim" << nSize << " scope link table " << nSize+1;;
	   LinuxStackHelper::RunIp (edge.Get (i), Seconds (0.2), cmd_oss.str ().c_str ());
	   cmd_oss.str ("");
           cmd_oss << "route add default via " << if_ea[i].GetAddress (1, 0) << " dev sim" << nSize << " table " << nSize+1;
           LinuxStackHelper::RunIp (edge.Get (i), Seconds (0.2), cmd_oss.str ().c_str ());

	   devices_ea[i] = pointToPoint_ea.Install(aggr.Get (i), edge.Get (i+1));
	   if_ea[i] = address_ea.Assign (devices_ea[i]);
	   address_ea.NewNetwork();

	   //setup ip route 
	   ////configure of aggr
	   cmd_oss.str ("");
           cmd_oss << "rule add from " << if_ea[i].GetAddress (1, 0) << " table " << 2;
           LinuxStackHelper::RunIp (aggr.Get (i), Seconds (0.2), cmd_oss.str ().c_str ());
           cmd_oss.str ("");
           cmd_oss << "route add 10.3." << 2 * i + 1 << ".0/24 dev sim" << 1 << " scope link table " << 2;
	   LinuxStackHelper::RunIp (aggr.Get (i), Seconds (0.2), cmd_oss.str ().c_str ());
	   cmd_oss.str ("");
           cmd_oss << "route add default via " << if_ea[i].GetAddress (0, 0) << " dev sim" << 1 << " table " << 2;
           LinuxStackHelper::RunIp (aggr.Get (i), Seconds (0.2), cmd_oss.str ().c_str ());
	   ////configure of edge
	   cmd_oss.str ("");
           cmd_oss << "rule add from " << if_ea[i].GetAddress (0, 0) << " table " << nSize+1;
           LinuxStackHelper::RunIp (edge.Get (i), Seconds (0.2), cmd_oss.str ().c_str ());
           cmd_oss.str ("");
           cmd_oss << "route add 10.3." << 2 * i + 1 << ".0/24 dev sim" << nSize << " scope link table " << nSize+1;;
	   LinuxStackHelper::RunIp (edge.Get (i), Seconds (0.2), cmd_oss.str ().c_str ());
	   cmd_oss.str ("");
           cmd_oss << "route add default via " << if_ea[i].GetAddress (1, 0) << " dev sim" << nSize << " table " << nSize+1;
           LinuxStackHelper::RunIp (edge.Get (i), Seconds (0.2), cmd_oss.str ().c_str ());
	}
	else{
	   devices_ea[i] = pointToPoint_ea.Install(aggr.Get (i), edge.Get (i-1));
	   if_ea[i] = address_ea.Assign (devices_ea[i]);


	   devices_ea[i] = pointToPoint_ea.Install(aggr.Get (i), edge.Get (i));
	   if_ea[i] = address_ea.Assign (devices_ea[i]);
	}
	//from aggr to core
	for (uint32_t l = 0; l < nCore; l++){
	   devices_ac[i] = pointToPoint_ac.Install(core.Get (l), aggr.Get (i));
	   if_ac[i] = address_ac.Assign (devices_ac[i]);
	}

	//setup ip routes
	cmd_oss.str ("");
	//cmd_oss << "rule add from " << if1.GetAddress (0, 0) << " table " << (i+1);
  }*/

   

  for (uint32_t i = 0;i < nHost; i++){
	setPos (host.Get (i), i*10, 0, 0);
  }
  for (uint32_t i = 0;i < nEdge; i++){
	setPos (edge.Get (i), (i*40)+15, 100, 0);
	setPos (aggr.Get (i), (i*40)+15, 200, 0);
  }
  for (uint32_t i = 0;i < nCore; i++){
	setPos (core.Get (i), (i*80)+35, 300, 0);
  }

  Simulator::Stop (Seconds (200.0));
  AnimationInterface anim ("./xml/mptcp_sub.xml");
  Simulator::Run ();
  Simulator::Destroy ();
 
  return 0;
}
