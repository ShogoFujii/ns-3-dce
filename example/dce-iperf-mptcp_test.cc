#include <iostream>
#include <cstdlib>
#include <map>
#include <string.h>
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
//--declare--//
void setPos(Ptr<Node> n, int x, int y, int z);
void iperf(multimap<int, string> &ip_set, double start, double end, NodeContainer host);
void socketTraffic(multimap<int, string> &ip_set, uint32_t byte, double start, double end, NodeContainer nodes);
void makebox(int *box, int size);
void shuffle(int *box, int size);
void map_insert(multimap<int, string> &adrs_set, int *box, int size);
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

void map_insert(multimap<int, string> &adrs_set, int *box, int size){
	int i;
	ostringstream ip;
	for(i=0; i < size; i++){
		ip.str("");
		cout << "box" << i << ":" << box[i] << endl;
		ip << "10." << box[i] / 4 + 1  << "."  << box[i] << ".2" ;
		adrs_set.insert(map<int, string>::value_type(i, ip.str()));
	}
}

int host_detect(char *adrs){
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
		dce[i].AddArgument(int2string(time));
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
	uint32_t size = byte * 1000 - 1502;
	int tasks = ip_set.size();
	ApplicationContainer apps[tasks], sinkApps[tasks];
	//BulkSendHelper bulk;
	//PacketSinkHelper sink[tasks];
	multimap<int, string>::iterator it = ip_set.begin();	
	string sock_factory = "ns3::LinuxTcpSocketFactory";
	int i = 0;
	while(it != ip_set.end()){
		//BulkSendHelper bulk[i];
		cout << "IP : " << (*it).second << endl;
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

int* poisson_pros(int sec, int end_time, int freq, int *poisson_size){
	int total_queue=0, queue=0;
	int random;
	int *pre_set;
	pre_set = new int[5000];
	int k=0;
	srand((unsigned) time(NULL));
	cout << rand() % 100 + 1 << endl;
	while(sec < end_time){
		queue = 0;
		random = rand() % 100 + 1;
		if(random < freq){
			queue++;
		}
		if(queue != 0){
			pre_set[k]=sec;
			cout << sec << "[msec]" << queue << endl;
			k++;
		}

		sec++;
		total_queue += queue;
	}
	int *poisson_set;
	poisson_set = new int[k];
	for(int i=0;i < k;i++){
		poisson_set[i]=pre_set[i];
		cout << i << " : " << poisson_set[i] << endl;
	}
	*poisson_size = k;
	delete[] pre_set;
	return poisson_set;
}

// ---  main  ---
int main(int argc, char *argv[]) {
	uint32_t nSize = 2;
	CommandLine cmd;
	cmd.AddValue("nSize", "FatTree Size. Default 2", nSize);
	cmd.Parse(argc, argv);
	uint32_t nCore = nSize * nSize;
	uint32_t nAggr = 2 * nSize * nSize;
	uint32_t nEdge = 2 * nSize * nSize;
	uint32_t nHost = nEdge * nSize;

	uint32_t nPath = 2 * nSize * nSize; // using paths, max's as default
	bool MPTCP_bool = false; // MPTCP's on(true)/off(false) switch

	NodeContainer core, aggr, edge, host;
	core.Create(nCore);
	aggr.Create(nAggr);
	edge.Create(nEdge);
	host.Create(nHost);

	DceManagerHelper dceManager;
	dceManager.SetTaskManagerAttribute("FiberManagerType", StringValue("UcontextFiberManager"));
	dceManager.SetNetworkStack("ns3::LinuxSocketFdFactory", "Library", StringValue("liblinux.so"));
	LinuxStackHelper stack;
	stack.Install(core);
	stack.Install(aggr);
	stack.Install(edge);
	stack.Install(host);

	dceManager.Install(core);
	dceManager.Install(aggr);
	dceManager.Install(edge);
	dceManager.Install(host);

	PointToPointHelper pointToPoint_he, pointToPoint_ea, pointToPoint_ac;
	NetDeviceContainer devices_he[nEdge+4], devices_ea[nAggr], devices_ac[nAggr];
	Ipv4AddressHelper address_he[nPath+4], address_ea, address_ea2, address_ac;
	Ipv4InterfaceContainer if_he[nAggr+4], if_ea[nAggr], if_ea2[nAggr], if_ac[nAggr];
	std::ostringstream cmd_oss;
	//set IP Address
	for(uint32_t i = 0; i < nPath; i++) {
		cmd_oss.str("");
		cmd_oss << "10." << i+1 <<".0.0";
		address_he[i].SetBase(cmd_oss.str().c_str(), "255.255.255.0");
	}
	address_ea.SetBase("10.32.0.0", "255.255.255.0");
	address_ea2.SetBase("10.33.0.0", "255.255.255.0");
	address_ac.SetBase("10.35.0.0", "255.255.255.0");

	//configure path
	pointToPoint_he.SetDeviceAttribute("DataRate", StringValue("250Mbps"));
	pointToPoint_ea.SetDeviceAttribute("DataRate", StringValue("500Mbps"));
	pointToPoint_ac.SetDeviceAttribute("DataRate", StringValue("1Gbps"));
	
	pointToPoint_he.SetChannelAttribute("Delay", StringValue("1ms"));
	pointToPoint_ea.SetChannelAttribute("Delay", StringValue("1ms"));
	pointToPoint_ac.SetChannelAttribute("Delay", StringValue("1ms"));

	pointToPoint_he.SetChannelAttribute("Delay", TimeValue(MicroSeconds(250)));
	pointToPoint_ea.SetChannelAttribute("Delay", TimeValue(MicroSeconds(250)));
	pointToPoint_ac.SetChannelAttribute("Delay", TimeValue(MicroSeconds(250)));



	cout << "nSize : " << nSize << endl;
	cout << "nCore : " << nCore << endl;
	cout << "nEdge : " << nEdge << endl;
	cout << "nAggr : " << nAggr << endl;
	cout << "nHost : " << nHost << endl;
	uint32_t i = 0, j = 0, k = 0;

	for (uint32_t i = 0; i < nEdge; i++) {
		//from host to edge
		for (uint32_t j = 0; j < nSize; j++) {
			k = i * nSize + j; //k:index for all nodes
			devices_he[i] = pointToPoint_he.Install(edge.Get(i), host.Get(k));
/*			
			devices_he[i+1] = pointToPoint_he.Install(edge.Get(i), host.Get(k));
			devices_he[i+2] = pointToPoint_he.Install(edge.Get(i), host.Get(k));
			devices_he[i+3] = pointToPoint_he.Install(edge.Get(i), host.Get(k));
*/
			if(MPTCP_bool == true){
				//For Multipath TCP
				for(uint32_t l = 0; l < 4; l++) {
					if_he[i] = address_he[l].Assign(devices_he[i]);
					//if_he[i+l] = address_he[l].Assign(devices_he[i+l]);
					address_he[l].NewNetwork();

					//setup ip route
					////configure of host
					cmd_oss.str("");
					cmd_oss << "route add default via " << if_he[i].GetAddress(0, l);
					//cmd_oss << "route add default via " << if_he[i+l].GetAddress(0, 0);
					cout <<"host" << k << "  :  " << cmd_oss.str() << endl;
					LinuxStackHelper::RunIp(host.Get(k), Seconds(0.1), cmd_oss.str().c_str());

					////configure of edge
					cmd_oss.str("");
					cmd_oss << "route add 10." << l + 1 <<"."<< k <<".0/24 via " << if_he[i].GetAddress(0, l);
					//cmd_oss << "route add 10." << l + 1 <<"."<< k <<".0/24 via " << if_he[i+l].GetAddress(0, 0);
					cout <<"edge" << i << "  :  " << cmd_oss.str() << endl;
					LinuxStackHelper::RunIp(edge.Get(i), Seconds(0.2), cmd_oss.str().c_str());
				}
			}else{
				//For Sigle-path TCP
				if_he[i] = address_he[i/2].Assign(devices_he[i]); // belong network is decided by the pod number
				for(uint32_t l = 0; l < 4; l++) {
					address_he[l].NewNetwork();
				}
				cmd_oss.str("");
				//setup ip route
				////configure of host
				cmd_oss << "route add default via " << if_he[i].GetAddress(0, 0);
				cout <<"host" << k << "  :  " << cmd_oss.str() << endl;
				LinuxStackHelper::RunIp(host.Get(k), Seconds(0.1), cmd_oss.str().c_str());

				////configure of edge
				cmd_oss.str("");
				cmd_oss << "route add 10."<< i/2 + 1 <<"."<< k <<".0/24 via " << if_he[i].GetAddress(0, 0);
				cout <<"edge" << i << "  :  " << cmd_oss.str() << endl;
				LinuxStackHelper::RunIp(edge.Get(i), Seconds(0.2), cmd_oss.str().c_str());
			}


		}
		cout << endl;
		//from edge to aggr
		if (i % 2 == 0) {
			devices_ea[i] = pointToPoint_ea.Install(aggr.Get(i), edge.Get(i));
			if_ea[i] = address_ea.Assign(devices_ea[i]);
			if_ea2[i] = address_ea2.Assign(devices_ea[i]);
			address_ea.NewNetwork();
			address_ea2.NewNetwork();
			devices_ea[i+1] = pointToPoint_ea.Install(aggr.Get(i),edge.Get(i + 1));
			if_ea[i+1] = address_ea.Assign(devices_ea[i+1]);
			if_ea2[i+1] = address_ea2.Assign(devices_ea[i+1]);
			address_ea.NewNetwork();
			address_ea2.NewNetwork();

			for (uint32_t j = 0; j < nHost; j++) {
				if(i == j / nSize){
					//setup ip route
					////configure of aggr
					cmd_oss.str("");
					cmd_oss << "route add 10.1." << j << ".0/24 via " << if_ea[i].GetAddress(0, 0);
					cout <<"aggr" << i << "  :  " << cmd_oss.str() << endl;
					LinuxStackHelper::RunIp(aggr.Get(i), Seconds(0.2), cmd_oss.str().c_str());
					cmd_oss.str("");
					cmd_oss << "route add 10.2." << j << ".0/24 via " << if_ea2[i].GetAddress(0, 1);
					cout <<"aggr" << i << "  :  " << cmd_oss.str() << endl;
					cout << "debug : " << if_ea2[i].GetAddress(0, 1) << endl;
					LinuxStackHelper::RunIp(aggr.Get(i), Seconds(0.2), cmd_oss.str().c_str());
				}else{
					////configure of edge
					cmd_oss.str("");
					cmd_oss << "route add 10.1." << j << ".0/24 via " << if_ea[i].GetAddress(1, 0);
					cout <<"edge" << i << "  :  " << cmd_oss.str() << endl;
					LinuxStackHelper::RunIp(edge.Get(i), Seconds(0.2), cmd_oss.str().c_str());
					cmd_oss.str("");
					cmd_oss << "route add 10.2." << j << ".0/24 via " << if_ea2[i].GetAddress(1, 1);
					cout <<"edge" << i << "  :  " << cmd_oss.str() << endl;
					//cout << "debug : " << if_ea2[i].GetAddress(1, 1) << endl;
					LinuxStackHelper::RunIp(edge.Get(i), Seconds(0.2), cmd_oss.str().c_str());
				}
			}
			cout << endl;
			for (uint32_t j = 0; j < nHost; j++) {
				if(i + 1 == j / nSize){
					//setup ip route
					////configure of aggr
					cmd_oss.str("");
					cmd_oss << "route add 10.1." << j << ".0/24 via " << if_ea[i+1].GetAddress(0, 0);
					cout <<"aggr" << i << "  :  " << cmd_oss.str() << endl;
					LinuxStackHelper::RunIp(aggr.Get(i), Seconds(0.2), cmd_oss.str().c_str());
					cmd_oss.str("");
					cmd_oss << "route add 10.2." << j << ".0/24 via " << if_ea2[i+1].GetAddress(0, 1);
					cout <<"aggr" << i << "  :  " << cmd_oss.str() << endl;
					LinuxStackHelper::RunIp(aggr.Get(i), Seconds(0.2), cmd_oss.str().c_str());
				}else{
					////configure of edge
					cmd_oss.str("");
					cmd_oss << "route add 10.1." << j << ".0/24 via " << if_ea[i+1].GetAddress(1, 0);
					cout <<"edge" << i + 1 << "  :  " << cmd_oss.str() << endl;
					LinuxStackHelper::RunIp(edge.Get(i+1), Seconds(0.2), cmd_oss.str().c_str());
					cmd_oss.str("");
					cmd_oss << "route add 10.2." << j << ".0/24 via " << if_ea2[i+1].GetAddress(1, 1);
					cout <<"edge" << i + 1 << "  :  " << cmd_oss.str() << endl;
					LinuxStackHelper::RunIp(edge.Get(i+1), Seconds(0.2), cmd_oss.str().c_str());
				}
			}

			cout << endl;

		} else {
			devices_ea[i - 1] = pointToPoint_ea.Install(aggr.Get(i), edge.Get(i - 1));
			if_ea[i - 1] = address_ea.Assign(devices_ea[i - 1]);
			if_ea2[i - 1] = address_ea2.Assign(devices_ea[i - 1]);
			address_ea.NewNetwork();
			address_ea2.NewNetwork();
			devices_ea[i] = pointToPoint_ea.Install(aggr.Get(i),edge.Get(i));
			if_ea[i] = address_ea.Assign(devices_ea[i]);
			if_ea2[i] = address_ea2.Assign(devices_ea[i]);
			address_ea.NewNetwork();
			address_ea2.NewNetwork();

			for (uint32_t j = 0; j < nHost; j++) {
				if(i - 1 == j / nSize){
					//setup ip route
					////configure of aggr
					cmd_oss.str("");
					cmd_oss << "route add 10.3." << j << ".0/24 via " << if_ea[i-1].GetAddress(0, 0);
					cout <<"aggr" << i << "  :  " << cmd_oss.str() << endl;
					LinuxStackHelper::RunIp(aggr.Get(i), Seconds(0.2), cmd_oss.str().c_str());
					cmd_oss.str("");
					cmd_oss << "route add 10.4." << j << ".0/24 via " << if_ea2[i-1].GetAddress(0, 1);
					cout <<"aggr" << i << "  :  " << cmd_oss.str() << endl;
					LinuxStackHelper::RunIp(aggr.Get(i), Seconds(0.2), cmd_oss.str().c_str());
				}else{
					////configure of edge
					cmd_oss.str("");
					cmd_oss << "route add 10.3." << j << ".0/24 via " << if_ea[i-1].GetAddress(1, 0);
					cout <<"edge" << i - 1 << "  :  " << cmd_oss.str() << endl;
					LinuxStackHelper::RunIp(edge.Get(i - 1), Seconds(0.2), cmd_oss.str().c_str());
					cmd_oss.str("");
					cmd_oss << "route add 10.4." << j << ".0/24 via " << if_ea2[i-1].GetAddress(1, 1);
					cout <<"edge" << i - 1 << "  :  " << cmd_oss.str() << endl;
					LinuxStackHelper::RunIp(edge.Get(i - 1), Seconds(0.2), cmd_oss.str().c_str());

				}
			}

			cout << endl;
			for (uint32_t j = 0; j < nHost; j++) {
				if(i == j / nSize){
					//setup ip route
					////configure of aggr
					cmd_oss.str("");
					cmd_oss << "route add 10.3." << j << ".0/24 via " << if_ea[i].GetAddress(0, 0);
					cout <<"aggr" << i << "  :  " << cmd_oss.str() << endl;
					LinuxStackHelper::RunIp(aggr.Get(i), Seconds(0.2), cmd_oss.str().c_str());
					cmd_oss.str("");
					cmd_oss << "route add 10.4." << j << ".0/24 via " << if_ea2[i].GetAddress(0, 1);
					cout <<"aggr" << i << "  :  " << cmd_oss.str() << endl;
					LinuxStackHelper::RunIp(aggr.Get(i), Seconds(0.2), cmd_oss.str().c_str());
				}else{
					////configure of edge
					cmd_oss.str("");
					cmd_oss << "route add 10.3." << j << ".0/24 via " << if_ea[i].GetAddress(1, 0);
					cout <<"edge" << i << "  :  " << cmd_oss.str() << endl;
					LinuxStackHelper::RunIp(edge.Get(i), Seconds(0.2), cmd_oss.str().c_str());
					cmd_oss.str("");
					cmd_oss << "route add 10.4." << j << ".0/24 via " << if_ea2[i].GetAddress(1, 1);
					cout <<"edge" << i << "  :  " << cmd_oss.str() << endl;
					LinuxStackHelper::RunIp(edge.Get(i), Seconds(0.2), cmd_oss.str().c_str());
				}
			}
			cout << endl;

		}
		//from aggr to core
		for (uint32_t l = 0; l < nCore; l++) {
			if((l < 2 && i % 2 == 0) || (l >= 2 && i % 2 != 0)){
				devices_ac[i] = pointToPoint_ac.Install(aggr.Get(i), core.Get(l));
				if_ac[i] = address_ac.Assign(devices_ac[i]);
				address_ac.NewNetwork();
				for (uint32_t j = 0; j < nHost; j++) {
					//cout << i << " : " << j << endl;
					if(i / 2 == j / (nSize*2)){
						//setup ip route
						////configure of core
						cmd_oss.str("");
						cmd_oss << "route add 10." << l + 1 <<"." << j << ".0/24 via " << if_ac[i].GetAddress(0, 0);
						//cmd_oss << "route add 10." << i % 2 + 1 <<"." << j << ".0/24 via " << if_ac[i].GetAddress(0, 0);
						cout <<"core" << l << "  :  " << cmd_oss.str() << endl;
						LinuxStackHelper::RunIp(core.Get(l), Seconds(0.2), cmd_oss.str().c_str());
					}else{
						////configure of edge
						cmd_oss.str("");
						cmd_oss << "route add 10." << l + 1<<"." << j << ".0/24 via " << if_ac[i].GetAddress(1, 0);
						//cmd_oss << "route add 10." << i % 2 + 1 <<"." << j << ".0/24 via " << if_ac[i].GetAddress(1, 0);
						cout <<"aggr" << i << "  :  " << cmd_oss.str() << endl;
						LinuxStackHelper::RunIp(aggr.Get(i), Seconds(0.2), cmd_oss.str().c_str());
					}
				}
			}

			cout << endl;
		}
	}
	//LinuxStackHelper::RunIp (host.Get (0), Seconds (0.1), "rule show");
	//LinuxStackHelper::RunIp (edge.Get (1), Seconds (0.0), "link set dev sim0 multipath on");
	// debug
	stack.SysctlSet(host, ".net.mptcp.mptcp_debug", "1");
	multimap<int, string> adrs_set, adrs_set2;

	int box[nHost];
	//int box2[nHost];
	//shuffle(box, nHost);
	//shuffle(box2, nHost);
	//map_insert(adrs_set, box, nHost);
	//map_insert(adrs_set2, box2, nHost/2);
	
	
	adrs_set.insert(map<int, string>::value_type(0, "10.2.4.2"));
	adrs_set2.insert(map<int, string>::value_type(0, "10.2.4.2"));
	/*adrs_set.insert(map<int, string>::value_type(5, "10.1.1.2"));
	adrs_set.insert(map<int, string>::value_type(2, "10.2.6.2"));
	adrs_set.insert(map<int, string>::value_type(7, "10.1.3.2"));
	*/
	
	cout << "--Display traffic lists --" << endl;;
	//display map
	multimap<int, string>::iterator it = adrs_set.begin();
	while(it != adrs_set.end()){
		cout << "From host" << (*it).first << ": [Dest_IP]" << (*it).second << endl;
		int aa =host_detect(string2char((*it).second));
		++it;
	}
	cout << "Total patterns : " << (unsigned int)adrs_set.size() << endl;
	/*
	cout << "--Display traffic2 lists --" << endl;;
	//display map
	multimap<int, string>::iterator it2 = adrs_set2.begin();
	while(it2 != adrs_set2.end()){
		cout << "From host" << (*it2).first << ": [Dest_IP]" << (*it2).second << endl;
		int aa =host_detect(string2char((*it2).second));
		++it2;
	}
	cout << "Total patterns : " << (unsigned int)adrs_set2.size() << endl;
	*/
	/*
	socketTraffic(adrs_set2, 70, 1.5, end, host);
	socketTraffic(adrs_set2, 70, 2.0, end, host);
	socketTraffic(adrs_set2, 70, 2.3, end, host);
	*/
	
	uint32_t kb = 50;
	double start = 1.0;
	double end = 3.0;
	//socketTraffic(adrs_set, kb, start, end, host);
	//socketTraffic(adrs_set, kb, 2.0, end, host);
	int poisson_size;
	int *poisson_set = poisson_pros(1000, 3000, 2, &poisson_size);
	
	for(int i=0;i < poisson_size;i++){
		//cout << i << " : " << (double) poisson_set[i] / 1000 << endl;
		socketTraffic(adrs_set, kb, (double) poisson_set[i] / 1000, end, host);
	}
	socketTraffic(adrs_set, 0, start, end, host);
	pointToPoint_he.EnablePcapAll("./pcap/sub2/he/iperf-he", false);
	//pointToPoint_ea.EnablePcapAll("./pcap/sub2/ea/iperf-ea", false);
	//pointToPoint_ac.EnablePcapAll("./pcap/sub2/ac/iperf-ac", false);
	
	//Setting positions
	for (uint32_t i = 0; i < nHost; i++) {
		setPos(host.Get(i), i * 20 + 5, 0, 0);
	}
	for (uint32_t i = 0; i < nEdge; i++) {
		setPos(edge.Get(i), (i * 40) + 15, 100, 0);
		setPos(aggr.Get(i), (i * 40) + 15, 200, 0);
	}
	for (uint32_t i = 0; i < nCore; i++) {
		setPos(core.Get(i), (i * 80) + 35, 300, 0);
	}

	if(MPTCP_bool == true){
		cout << "Multipath TCP : Path " << nPath / 2<< endl;
	}else{
		cout << "Single-path TCP" << endl;
	}
	Simulator::Stop(Seconds(3.0));
	AnimationInterface anim("./xml/mptcp_test.xml");
	Simulator::Run();
	Simulator::Destroy();

	return 0;
}


