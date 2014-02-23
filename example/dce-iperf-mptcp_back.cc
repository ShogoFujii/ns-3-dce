#include <iostream>
#include <cstdlib>
#include <map>
#include <string.h>
#include <math.h>
#include <sstream>

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

void setPos(Ptr<Node> n, int x, int y, int z);
void iperf(multimap<int, string> &ip_set, double start, double end, NodeContainer host);
void socketTraffic(multimap<int, string> &ip_set, uint32_t byte, double start, double end, NodeContainer nodes);
void makebox(int *box, int size);
void shuffle(int *box, int size);
void map_insert(multimap<int, string> &adrs_set, int *box, int size, double ratio);
int host_detect(char *adrs);
string double2string(double d);
string int2string(int d);
char* string2char(string s);

//--script--//
void setPos(Ptr<Node> n, int x, int y, int z) {
	Ptr < ConstantPositionMobilityModel > loc = CreateObject<
			ConstantPositionMobilityModel>();
	n->AggregateObject(loc);
	Vector locVec2(x, y, z);
	loc->SetPosition(locVec2);
}

void makebox(int *box, int size){
	int i;
	for(i=0; i < size; i++){
		box[i]=i;
	}
}

void shuffle(int *box, int size){
	int i;
	makebox(box, size);
	//srand((unsigned) time(NULL));
	for(i=0; i < size; i++){
		int r = rand() % size;
		int m = box[i];
		box[i] = box[r];
		box[r] = m;
	}
}

void init_rand(){
	srand((unsigned) time(NULL));
}

bool random_slot(double ratio){
	//cout << rand() % 100 << endl;
	if (rand() % 100 < ratio * 100){
		return true;
	}
	return false;
}

void map_insert(multimap<int, string> &adrs_set, int *box, int size, double ratio){
	int i;
	ostringstream ip;
	for(i=0; i < size; i++){
		ip.str("");
		if (random_slot(ratio) == true){
			if(i != box[i]){
				ip << "10." << box[i] / 4 + 1  << "."  << box[i] << ".2" ;
				adrs_set.insert(map<int, string>::value_type(i, ip.str()));
			}
		}
	}
}
void show_map(multimap<int, string> &adrs_set){
	multimap<int, string>::iterator it = adrs_set.begin();
	while(it != adrs_set.end()){
		cout << "From host" << (*it).first << ": [Dest_IP]" << (*it).second << endl;
		++it;
	}
	cout << "Total patterns : " << (unsigned int)adrs_set.size() << endl;
}

int host_detect(char *adrs){
	//tha case th secound place of address from the end is node id//
	const char delm[] = ".";
	char *tok;
	int host;

	tok = strtok(adrs, delm);
	int j = 0;
	while(tok != NULL){
		if(j == 2){
			host = atoi(tok);
			break;
		}
		tok = strtok(NULL, delm);
		j++;
	}
	return host;
}

string double2string(double d){
	string rt;
	stringstream ss;
	ss << d;
	ss >> rt;
	return rt;
}

string int2string(int d){
	string rt;
	stringstream ss;
	ss << d;
	ss >> rt;
	return rt;
}

char* string2char(string s){
	int len = s.length();
	char* c = new char[len+1];
	memcpy(c, s.c_str(), len+1);
	return c;
}

void iperf(multimap<int, string> &ip_set, double start, double end, NodeContainer host){
	int tasks = ip_set.size();

	DceApplicationHelper dce[tasks];
	ApplicationContainer apps[tasks];

	multimap<int, string>::iterator it = ip_set.begin();
	int i = 0;
	int time = (int) end - start;
	time = time +1;
	while(it != ip_set.end()){
		dce[i].SetStackSize(1 << 20);

		// Launch iperf client 送る側
		dce[i].SetBinary("iperf");
		dce[i].ResetArguments();
		dce[i].ResetEnvironment();
		dce[i].AddArgument("-c");
		dce[i].AddArgument((*it).second); // dist address
		dce[i].AddArgument("-i");
		dce[i].AddArgument("1");
		dce[i].AddArgument("--time");
		dce[i].AddArgument("1");
		cout << "-c " << (*it).second << " --time " << int2string(time) << " host:" << (*it).first << " start: " << start << " end: " << end << endl;
		apps[i] = dce[i].Install(host.Get((*it).first)); //送る側
		apps[i].Start(Seconds(start));
		apps[i].Stop(Seconds(end));

		// Launch iperf server 受ける側
		dce[i].SetBinary("iperf");
		dce[i].ResetArguments();
		dce[i].ResetEnvironment();
		dce[i].AddArgument("-s");
		dce[i].AddArgument("-P");
		dce[i].AddArgument("1");
		int ser = host_detect(string2char((*it).second));
		apps[i] = dce[i].Install(host.Get(ser));
		cout << "client:" << ser << endl;
		++it;
		i++;
	}

	for(i=0;i<tasks;i++){
		apps[i].Start(Seconds(start));
	}
	//apps[0].Start(Seconds(1));
}

void socketTraffic(multimap<int, string> &ip_set, uint32_t byte, double start, double end, NodeContainer nodes){
	uint32_t size = byte;
	if (byte != 0){
		size = byte * 1000 - 1502;
	}
	int tasks = ip_set.size();
	ApplicationContainer apps[tasks], sinkApps[tasks];
	multimap<int, string>::iterator it = ip_set.begin();
	string sock_factory = "ns3::LinuxTcpSocketFactory";
	int i = 0;
	while(it != ip_set.end()){
		//BulkSendHelper bulk[i];
		//cout << "IP : " << (*it).second << endl;
		BulkSendHelper bulk = BulkSendHelper (sock_factory, InetSocketAddress (string2char((*it).second), 50000));
		bulk.SetAttribute ("MaxBytes", UintegerValue (size));
		apps[0] = bulk.Install (nodes.Get((*it).first));

		apps[0].Start(Seconds(start));
		apps[0].Stop(Seconds(end));

		PacketSinkHelper sink = PacketSinkHelper (sock_factory, InetSocketAddress (Ipv4Address::GetAny (), 50000));
		int ser = host_detect(string2char((*it).second));
		sinkApps[0] = sink.Install (nodes.Get(ser));
		sinkApps[0].Start (Seconds (start));
		sinkApps[0].Stop (Seconds (end));
		++it;
		i++;
	}

}

int main (int argc, char *argv[])
{
  uint32_t nRtrs = 4;
  CommandLine cmd;
  cmd.AddValue ("nRtrs", "Number of routers. Default 2", nRtrs);
  cmd.Parse (argc, argv);

  NodeContainer nodes, routers;
  nodes.Create (2);
  routers.Create (nRtrs);

  DceManagerHelper dceManager;
  dceManager.SetTaskManagerAttribute ("FiberManagerType",
                                      StringValue ("UcontextFiberManager"));

  dceManager.SetNetworkStack ("ns3::LinuxSocketFdFactory",
                              "Library", StringValue ("liblinux.so"));
  LinuxStackHelper stack;
  stack.Install (nodes);
  stack.Install (routers);

  dceManager.Install (nodes);
  dceManager.Install (routers);

  PointToPointHelper pointToPoint;
  NetDeviceContainer devices1, devices2;
  Ipv4AddressHelper address1, address2;
  std::ostringstream cmd_oss;
  address1.SetBase ("10.1.0.0", "255.255.255.0");
  address2.SetBase ("10.2.0.0", "255.255.255.0");

  for (uint32_t i = 0; i < nRtrs; i++)
    {
      // Left link
	  cout << endl;
	  cout << "Left link" <<endl;
      pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("1Mbps"));
      //pointToPoint.SetChannelAttribute ("Delay", StringValue ("1ms"));
      pointToPoint.SetChannelAttribute("Delay", TimeValue(MicroSeconds(10)));
      devices1 = pointToPoint.Install (nodes.Get (0), routers.Get (i));
      // Assign ip addresses
      Ipv4InterfaceContainer if1 = address1.Assign (devices1);
      address1.NewNetwork ();
      // setup ip routes
      cmd_oss.str ("");
      cmd_oss <<"rule add from " << if1.GetAddress (0, 0) << " table " << (i+1);
      cout << "node0 : " <<  cmd_oss.str() << endl;
      LinuxStackHelper::RunIp (nodes.Get (0), Seconds (0.1), cmd_oss.str ().c_str ());
      cmd_oss.str ("");
      cmd_oss <<"route add 10.1." << i << ".0/24 scope link table " << (i+1);
      cout << "node0 : " <<  cmd_oss.str() << endl;
      LinuxStackHelper::RunIp (nodes.Get (0), Seconds (0.1), cmd_oss.str ().c_str ());
      cmd_oss.str ("");
      cmd_oss << "route add default via " << if1.GetAddress (1, 0) <<" table " << (i+1);
      cout << "node0 : " << cmd_oss.str() << endl;
      LinuxStackHelper::RunIp (nodes.Get (0), Seconds (0.1), cmd_oss.str ().c_str ());
      cmd_oss.str ("");
      cmd_oss << "route add 10.1.0.0/16 via " << if1.GetAddress (1, 0);
      cout << "router" << i << " : "<< cmd_oss.str() << endl;
      LinuxStackHelper::RunIp (routers.Get (i), Seconds (0.2), cmd_oss.str ().c_str ());

      // Right link
      cout << "Right link" <<endl;
      devices2 = pointToPoint.Install (nodes.Get (1), routers.Get (i));
      // Assign ip addresses
      Ipv4InterfaceContainer if2 = address2.Assign (devices2);
      address2.NewNetwork ();
      // setup ip routes
      cmd_oss.str ("");
      cmd_oss << "rule add from " << if2.GetAddress (0, 0) << " table " << (i+1);
      cout << "node1 : " << cmd_oss.str() << endl;
      LinuxStackHelper::RunIp (nodes.Get (1), Seconds (0.1), cmd_oss.str ().c_str ());
      cmd_oss.str ("");
      cmd_oss << "route add 10.2." << i << ".0/24 scope link table " << (i+1);
      cout <<  "node1 : " << cmd_oss.str() << endl;
      LinuxStackHelper::RunIp (nodes.Get (1), Seconds (0.1), cmd_oss.str ().c_str ());
      cmd_oss.str ("");
      cmd_oss << "route add default via " << if2.GetAddress (1, 0) << " table " << (i+1);
      cout << "node1 : " << cmd_oss.str() << endl;
      LinuxStackHelper::RunIp (nodes.Get (1), Seconds (0.1), cmd_oss.str ().c_str ());
      cmd_oss.str ("");
      cmd_oss << "route add 10.2.0.0/16 via " << if2.GetAddress (1, 0);
      cout << "router" << i << " : "<< cmd_oss.str() << endl;
      LinuxStackHelper::RunIp (routers.Get (i), Seconds (0.2), cmd_oss.str ().c_str ());

      setPos (routers.Get (i), 50, i * 20, 0);
    }

  // default route

  LinuxStackHelper::RunIp (nodes.Get (0), Seconds (0.1), "route add default via 10.1.0.2");
  cout << "node0 : " << "route add default via 10.1.0.2 " << endl;
  LinuxStackHelper::RunIp (nodes.Get (1), Seconds (0.1), "route add default via 10.2.0.2");
  cout << "node1 : "<< "route add default via 10.2.0.2 " << endl;

  // Schedule Up/Down (XXX: didn't work...)
  LinuxStackHelper::RunIp (nodes.Get (1), Seconds (1.0), "link set dev sim0 multipath off");
  LinuxStackHelper::RunIp (nodes.Get (1), Seconds (15.0), "link set dev sim0 multipath on");
  LinuxStackHelper::RunIp (nodes.Get (1), Seconds (30.0), "link set dev sim0 multipath off");


  // debug
  stack.SysctlSet (nodes, ".net.mptcp.mptcp_debug", "1");
/*
  stack.SysctlSet (nodes, ".net.ipv4.tcp_rmem", "5000000 5000000 5000000");
  stack.SysctlSet (nodes, ".net.ipv4.tcp_wmem", "5000000 5000000 5000000");
  stack.SysctlSet (nodes, ".net.core.rmem_max", "5000000");
  stack.SysctlSet (nodes, ".net.core.wmem_max", "5000000");
*/

  stack.SysctlSet (nodes, ".net.ipv4.tcp_rmem", "300000 300000 300000");
  stack.SysctlSet (nodes, ".net.ipv4.tcp_wmem", "300000 300000 300000");
  stack.SysctlSet (nodes, ".net.core.rmem_max", "300000");
  stack.SysctlSet (nodes, ".net.core.wmem_max", "300000");

  //Dell PC
/*
  stack.SysctlSet (nodes, ".net.ipv4.tcp_rmem", "163840 163840 163840");
  stack.SysctlSet (nodes, ".net.ipv4.tcp_wmem", "163840 163840 163840");
  stack.SysctlSet (nodes, ".net.core.rmem_max", "163840");
  stack.SysctlSet (nodes, ".net.core.wmem_max", "163840");
*/
  /*
  //catalyst 6500 シリーズWS-X6516-GE-TX
  stack.SysctlSet (routers, ".net.ipv4.tcp_rmem", "512000 512000 512000");
  stack.SysctlSet (routers, ".net.ipv4.tcp_wmem", "512000 512000 512000");
  stack.SysctlSet (routers, ".net.core.rmem_max", "512000");
  stack.SysctlSet (routers, ".net.core.wmem_max", "512000");
*/
/*
  multimap<int, string> adrs_set;
  uint32_t kb = 70;
  double start = 1.0;
  double end = 5.0;
  adrs_set.insert(map<int, string>::value_type(0, "10.2.1.1"));
  show_map(adrs_set);
  socketTraffic(adrs_set, 0, start, end, nodes);
*/

 // socketTraffic(adrs_set, 80, start, end, nodes);

  DceApplicationHelper dce;
  ApplicationContainer apps;

  dce.SetStackSize (1 << 20);

  // Launch iperf client on node 0
  dce.SetBinary ("iperf");
  dce.ResetArguments ();
  dce.ResetEnvironment ();
  dce.AddArgument ("-c");
  dce.AddArgument ("10.2.0.1");
  dce.AddArgument ("-i");
  dce.AddArgument ("9");
  dce.AddArgument ("--time");
  dce.AddArgument ("10");

  apps = dce.Install (nodes.Get (0));
  apps.Start (Seconds (5.0));
  apps.Stop (Seconds (15));

  // Launch iperf server on node 1
  dce.SetBinary ("iperf");
  dce.ResetArguments ();
  dce.ResetEnvironment ();
  dce.AddArgument ("-s");
  dce.AddArgument ("-P");
  dce.AddArgument ("1");
  apps = dce.Install (nodes.Get (1));

  apps.Start (Seconds (4));



  pointToPoint.EnablePcapAll ("./pcap/back/iperf-mptcp", false);
  setPos (nodes.Get (0), 0, 20 * (nRtrs - 1) / 2, 0);
  setPos (nodes.Get (1), 100, 20 * (nRtrs - 1) / 2, 0);

  Simulator::Stop (Seconds (15.0));
  AnimationInterface anim("./xml/mptcp_back.xml");
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
