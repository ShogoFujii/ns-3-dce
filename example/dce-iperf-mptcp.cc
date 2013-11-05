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
  Ipv4InterfaceContainer if_he[nEdge], if_ea[nAggr], if_ac[nAggr];
  std::ostringstream cmd_oss;
  address_he.SetBase ("10.1.0.0", "255.255.255.0");
  address_ea.SetBase ("10.2.0.0", "255.255.255.0");
  address_ac.SetBase ("10.3.0.0", "255.255.255.0");

  pointToPoint_he.SetDeviceAttribute ("DataRate", StringValue ("250Mbps"));
  pointToPoint_he.SetChannelAttribute ("Delay", StringValue ("10ms"));

  pointToPoint_ea.SetDeviceAttribute ("DataRate", StringValue ("500Mbps"));
  pointToPoint_ea.SetChannelAttribute ("Delay", StringValue ("10ms"));

  pointToPoint_ac.SetDeviceAttribute ("DataRate", StringValue ("1Gbps"));
  pointToPoint_ac.SetChannelAttribute ("Delay", StringValue ("10ms"));

  cout << "nSize : " << nSize << endl;
  cout << "nEdge : " << nEdge << endl;
  uint32_t i = 0, j = 0, k = 0;
  for (uint32_t i = 0;i < nEdge; i++){
	//from host to edge
	for (uint32_t j = 0; j < nSize; j++){
	   k = i * nSize + j;
	   
	   devices_he[i] = pointToPoint_he.Install(edge.Get (i), host.Get (k));
	}
	//from edge to aggr
	if(i % 2 ==0){
	   devices_ea[i] = pointToPoint_ea.Install(aggr.Get (i), edge.Get (i));
	   devices_ea[i] = pointToPoint_ea.Install(aggr.Get (i), edge.Get (i+1));
	}
	else{
	   devices_ea[i] = pointToPoint_ea.Install(aggr.Get (i), edge.Get (i-1));
	   devices_ea[i] = pointToPoint_ea.Install(aggr.Get (i), edge.Get (i));
	}
	//from aggr to core
	for (uint32_t l = 0; l < nCore; l++){
	   devices_ac[i] = pointToPoint_ac.Install(core.Get (l), aggr.Get (i));
	}
	if_he[i] = address_he.Assign (devices_he[i]);
	if_ea[i] = address_ea.Assign (devices_ea[i]);
	if_ac[i] = address_ac.Assign (devices_ac[i]);	
  }




  AnimationInterface anim ("anim.xml");

  for (uint32_t i = 0;i < nHost; i++){
	anim.SetConstantPosition (host.Get(i), i*10, 100);
  }
  for (uint32_t i = 0;i < nEdge; i++){
	anim.SetConstantPosition (edge.Get(i), i*40+15, 200);
	anim.SetConstantPosition (aggr.Get(i), i*40+15, 300);
  }
  for (uint32_t i = 0;i < nCore; i++){
	anim.SetConstantPosition (core.Get(i), i*80+35, 500);
  }

  Simulator::Stop (Seconds (200.0));
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
