#include <iostream>
#include <cstdlib>
#include <map>
#include <string.h>
#include <math.h>
#include <sstream>
#include <vector>
#include <algorithm>

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
void map_insert(multimap<int, string> &adrs_set, int *box, int size, double ratio, int path_num);
int host_detect(char *adrs);
string double2string(double d);
string int2string(int d);
char* string2char(string s);
string fix_adrs(char *adrs, int path);

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
	int pre_box[size];
	//srand((unsigned) time(NULL));
	memcpy(pre_box, box, sizeof(int) *size);
	int count=0;
	while(count != size){
		count = 0;
		for(i=0; i < size; i++){
			int r = rand() % size;
			if(pre_box[i]/4 != box[r]/4 and pre_box[i] != box[r]){
				int m = box[i];
				box[i] = box[r];
				box[r] = m;
				count++;
			}
		}
	}
}
void shuffle2(int *box, int size){
	int i;
	int pre_box[size];
	memcpy(pre_box, box, sizeof(int) *size);
	int count=0;
	while(count != size){
		count = 0;
		for(i=0; i < size; i++){
			int r = rand() % size;
			if(pre_box[i]/4 != box[r]/4 and pre_box[i] != box[r]){
				//cout << "pre_box[i] : " << pre_box[i] << endl;
				int m = box[i];
				box[i] = box[r];
				box[r] = m;
				count++;
			}
		}
	}
}
void randomize_box(int *box, int box_size,  multimap<int, string> &adrs_set){
	int size = box_size;
	string tmp;
	int pre_box[size];
	memcpy(pre_box, box, sizeof(int) *size);
	shuffle2(box, size);
	for(int i=0; i < size; i++){
		tmp = "10." + int2string(box[i]/4 + 1) + "." + int2string(box[i]) + ".2";
		adrs_set.insert(map<int, string>::value_type(pre_box[i], tmp));
	}
}

void init_rand(){
	srand((unsigned) time(NULL));
}
void randomize_box2(int *box, int box_size,  map<int, string> &adrs_set){
	int size = box_size;
	string tmp;
	int pre_box[size];
	memcpy(pre_box, box, sizeof(int) *size);
	shuffle2(box, size);
	for(int i=0; i < size; i++){
		tmp = "10." + int2string(box[i]/4 + 1) + "." + int2string(box[i]) + ".2";
		adrs_set.insert(map<int, string>::value_type(pre_box[i], tmp));
	}
}


bool random_slot(double ratio){
	//cout << rand() % 100 << endl;
	if (rand() % 100 < ratio * 100){
		return true;
	}
	return false;
}

void map_insert(multimap<int, string> &adrs_set, int *box, int size, double ratio, int path_num){
	int i;
	int subnet;
	ostringstream ip;
	for(i=0; i < size; i++){
		ip.str("");
		if (random_slot(ratio) == true){
			if(i != box[i]){
				subnet=box[i] / 4 + 1;
				if(path_num == 2){
					if(subnet == 2){
						subnet=1;
					}else if(subnet == 4){
						subnet=3;
					}
				}else if(path_num == 3){
					if(subnet == 4){
						subnet = 3;
					}
				}
				ip << "10." << subnet  << "."  << box[i] << ".2" ;
				adrs_set.insert(map<int, string>::value_type(i, ip.str()));
			}
		}
	}
}
void show_multimap(multimap<int, string> &adrs_set){
	multimap<int, string>::iterator it = adrs_set.begin();
	while(it != adrs_set.end()){
		cout << "From host" << (*it).first << ": [Dest_IP]" << (*it).second << endl;
		++it;
	}
	cout << "Total patterns : " << (unsigned int)adrs_set.size() << endl;
}
void show_map(map<int, string> &adrs_set){
	map<int, string>::iterator it = adrs_set.begin();
	while(it != adrs_set.end()){
		cout << "From host" << (*it).first << ": [Dest_IP]" << (*it).second << endl;
		++it;
	}
	cout << "Total patterns : " << (unsigned int)adrs_set.size() << endl;
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
string fix_adrs(char *adrs, int path){
	const char delm[] = ".";
	char *tok;
	int post;
	int host;
	string fix="";

	tok = strtok(adrs, delm);
	int j = 0;
	while(tok != NULL){
		if(j == 1){
			host = atoi(tok);
		}else if(j == 2){
			post = atoi(tok);
		}
		tok = strtok(NULL, delm);
		j++;
	}
	if(path == 2){
		if(host == 2){
			host = 1;
		}else if (host == 4){
			host = 3;
		}
	}else if (path == 3){
		if(host == 4){
			host = 3;
		}
	}
	fix=fix+"10."+int2string(host)+"."+int2string(post)+".2";
	return fix;
}
void map_slice(multimap<int, string> &adrs_set, map<int, string> &adrs_back, double ratio){
	int size = (int) adrs_set.size();
	int box[size];
	shuffle(box, size);
	int bound = (int) ((int) adrs_set.size()*ratio);
	map<int, string>::iterator it;
	for(int k=0;k < bound;k++){
		it = adrs_set.find(box[k]);
		adrs_back.insert(map<int, string>::value_type(box[k], (*it).second));
	}
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
void socketTraffic2(map<int, string> &ip_set, uint32_t byte, double start, double end, NodeContainer nodes){
	uint32_t size = byte;
		if (byte != 0){
			size = byte * 1000 - 1502;
		}
	int tasks = ip_set.size();
	ApplicationContainer apps[tasks], sinkApps[tasks];
	map<int, string>::iterator it = ip_set.begin();
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

int* poisson_pros(int sec, int end_time, int freq, int *poisson_size){
	int total_queue=0, queue=0;
	int random;
	int *pre_set;
	pre_set = new int[5000];
	int k=0;
	srand((unsigned) time(NULL));
	while(sec < end_time){
		queue = 0;
		random = rand() % 100 + 1;
		if(random < freq){
			queue++;
		}
		if(queue != 0){
			pre_set[k]=sec;
			//cout << sec << "[msec]" << queue << endl;
			k++;
		}
		sec=sec+10;
		//cout << sec << endl;
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
	uint32_t nDir_name=1;
	uint32_t buffer_size = 524000; //524000 -> 512KB, 32800 -> 32KB, 128000 -> 125KB
	uint32_t buffer_size2 = 524000; //524000 -> 512KB
	uint32_t prtcl_bool = 1; // 0 -> TCP, 1 -> MPTCP
	uint32_t Path_num = 4;
	uint32_t interface = 1; // 0 -> Single-interface, 1 -> multi-interface
	CommandLine cmd;
	cmd.AddValue("nSize", "FatTree Size. Default 2", nSize);
	cmd.AddValue("nDir_name", "the place of pcap files", nDir_name);
	cmd.AddValue("buffer_size", "buffer_size", buffer_size);
	cmd.AddValue("prtcl_bool", "0 -> TCP, 1 -> MPTCP", prtcl_bool);
	cmd.AddValue("Path_num", "Path_num", Path_num);
	cmd.AddValue("interface", "interface", interface);
	cmd.Parse(argc, argv);
	uint32_t nCore = nSize * nSize;
	uint32_t nAggr = 2 * nSize * nSize;
	uint32_t nEdge = 2 * nSize * nSize;
	uint32_t nHost = nEdge * nSize;
	bool MPTCP_bool=true, interface_bool;
	uint32_t nPath = 2 * nSize * nSize; // using paths, max's as default
	if (prtcl_bool == 0){
		MPTCP_bool = false; // MPTCP's on(true)/off(false) switch
	}else if(prtcl_bool == 1){
		MPTCP_bool = true; // MPTCP's on(true)/off(false) switch
	}
	if (interface == 0){
		interface_bool = false; // multi_interfaces's on(true)/off(false) switch
	}else if(interface == 1){
		interface_bool = true; // multi_interfaces's on(true)/off(false) switch
	}

	init_rand();
	multimap<int, string> adrs_set, adrs_set2, test;
	map<int, string> adrs_back, adrs_short;
	int box[nHost];
	int size=0;
	double ratio = 1.0;
	for (int i=0; i < 1; i++){
		while(size != nHost){
			adrs_set.clear();
			size = 0;
			shuffle(box, nHost);
			map_insert(adrs_set, box, nHost, ratio, Path_num);
			size = (int) adrs_set.size();
		}
	}
	map_slice(adrs_set, adrs_back, (double) 1/3);
	show_map(adrs_back);
	multimap<int, string>::iterator it = adrs_back.begin();

	map<int, string> list, short_list;
	while(it != adrs_back.end()){
		list.insert(map<int, string>::value_type((*it).first, "long"));
		list.insert(map<int, string>::value_type(host_detect(string2char((*it).second)), "long"));
		++it;
	}
	cout << list.size() << endl;
	int short_size = (int) list.size();
	int num=0;
	int short_box[nHost-short_size];
	for (int k=0; k<nHost;k++){
		it=list.find(k);
		if(it == list.end()){
			short_list.insert(map<int, string>::value_type(k, "short"));
			short_box[num]=k;
			num++;
		}
	}
	randomize_box(short_box, sizeof short_box / sizeof short_box[0], adrs_set2);
	short_list.clear();
	//short_list.insert(map<int, string>::value_type(14, "short"));


	uint32_t kb = 70;
	double start = 1.0;
	double end = 4.0;
	int counter = 20;

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
	NetDeviceContainer devices_he[nEdge+4], devices_ea[nAggr], devices_ea2[nAggr], devices_ac[nAggr];
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
	pointToPoint_he.SetDeviceAttribute("DataRate", StringValue("10Mbps"));
	pointToPoint_ea.SetDeviceAttribute("DataRate", StringValue("20Mbps"));
	pointToPoint_ac.SetDeviceAttribute("DataRate", StringValue("40Mbps"));
/*
	pointToPoint_he.SetChannelAttribute("Delay", StringValue("1ms"));
	pointToPoint_ea.SetChannelAttribute("Delay", StringValue("1ms"));
	pointToPoint_ac.SetChannelAttribute("Delay", StringValue("1ms"));
*/

	pointToPoint_he.SetChannelAttribute("Delay", TimeValue(MicroSeconds(80)));
	pointToPoint_ea.SetChannelAttribute("Delay", TimeValue(MicroSeconds(80)));
	pointToPoint_ac.SetChannelAttribute("Delay", TimeValue(MicroSeconds(80)));



	cout << "nSize : " << nSize << endl;
	cout << "nCore : " << nCore << endl;
	cout << "nEdge : " << nEdge << endl;
	cout << "nAggr : " << nAggr << endl;
	cout << "nHost : " << nHost << endl;
	uint32_t i = 0, j = 0, k = 0;
	int ads=0;

	for (uint32_t i = 0; i < nEdge; i++) {
		//from host to edge
		for (uint32_t j = 0; j < nSize; j++) {
			k = i * nSize + j; //k:index for all nodes
			devices_he[i] = pointToPoint_he.Install(edge.Get(i), host.Get(k));
			if(MPTCP_bool == true){
				for(int num=1;num < Path_num;num++){
					if(short_list.find(k) != short_list.end()){
						cout << "break" << endl;
						break;
					}
					devices_he[i+num] = pointToPoint_he.Install(edge.Get(i), host.Get(k));
				}
				//For Multipath TCP
				for(uint32_t l = 0; l < Path_num; l++) {
					//if_he[i] = address_he[l].Assign(devices_he[i]);
					//cout << "l : " << l << endl
					if(Path_num==2){
						ads=l*2;
					}else{
						ads=l;
					}
					if(short_list.find(k) != short_list.end() and Path_num!=2){
						if(Path_num==2){
							if(i/2==1) ads=0;
							else if(i/2==3) ads=2;
						}else if(Path_num==3){
							if(i/2==3) ads=2;
							else ads=i/2;
						}else{
							ads=i/2;
						}
					}
					if_he[i+l] = address_he[ads].Assign(devices_he[i+l]);
					//if_he[i+l] = address_he[l*2].Assign(devices_he[i+l]);
					if(short_list.find(k) == short_list.end()){
						address_he[ads].NewNetwork();
					}
					//address_he[l*2].NewNetwork();
					//setup ip route
					////configure of host
					cmd_oss.str("");
					cmd_oss << "rule add from " << if_he[i+l].GetAddress(1, 0) << " table " << l+1;
					cout <<"host" << k << "  :  " << cmd_oss.str() << endl;
					LinuxStackHelper::RunIp(host.Get(k), Seconds(0.1), cmd_oss.str().c_str());
					cmd_oss.str("");
					cmd_oss << "route add 10." << ads + 1 <<"." << k << ".0/24 scope link" << " table " << l+1;
					cout <<"host" << k << "  :  " << cmd_oss.str() << endl;
					LinuxStackHelper::RunIp(host.Get(k), Seconds(0.1), cmd_oss.str().c_str());
					cmd_oss.str("");
					cmd_oss << "route add default via " << if_he[i+l].GetAddress(0, 0) << " table " << l+1;
					cout <<"host" << k << "  :  " << cmd_oss.str() << endl;
					LinuxStackHelper::RunIp(host.Get(k), Seconds(0.1), cmd_oss.str().c_str());
/*
					cmd_oss.str("");
					//cmd_oss << "route add default via " << if_he[i].GetAddress(0, l);
					cmd_oss << "route add 10." << l + 1 <<".0.0/16 via "  << if_he[i+l].GetAddress(0, 0);
					//cmd_oss << "route add 10." << l*2 + 1 <<".0.0/16 via "  << if_he[i+l].GetAddress(0, 0);
					cout <<"host" << k << "  :  " << cmd_oss.str() << endl;
					LinuxStackHelper::RunIp(host.Get(k), Seconds(0.1), cmd_oss.str().c_str());
*/
					////configure of edge
					cmd_oss.str("");
					//cmd_oss << "route add 10." << l + 1 <<"."<< k <<".0/24 via " << if_he[i].GetAddress(0, l);
					cmd_oss << "route add 10." << ads + 1 <<"."<< k <<".0/24 via " << if_he[i+l].GetAddress(1, 0);
					//cmd_oss << "route add 10." << l*2 + 1 <<"."<< k <<".0/24 via " << if_he[i+l].GetAddress(1, 0);
					cout <<"edge" << i << "  :  " << cmd_oss.str() << endl;
					LinuxStackHelper::RunIp(edge.Get(i), Seconds(0.2), cmd_oss.str().c_str());
					if(short_list.find(k) != short_list.end()){
						cout << "break" << endl;
						for(int num=0 ; num < nCore ; num++){
							address_he[num].NewNetwork();
						}
						break;
					}
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
			if(MPTCP_bool == true){
				if(short_list.find(k) != short_list.end()){
					cmd_oss.str("");
					cmd_oss << "route add default via 10."<< i/2 + 1 <<"." << k <<".1";
					cout << "host" << k << "  :  "<< cmd_oss.str() << endl;
					LinuxStackHelper::RunIp (host.Get (k), Seconds (0.1), cmd_oss.str().c_str());
				}else{
					cmd_oss.str("");
					cmd_oss << "route add default via 10.1." << k <<".1";
					cout << "host" << k << "  :  "<< cmd_oss.str() << endl;
					LinuxStackHelper::RunIp (host.Get (k), Seconds (0.1), cmd_oss.str().c_str());
				}
			}
		}
		cout << endl;
		//from edge to aggr
		if (i % 2 == 0) {
			cout << "Path_num : " << Path_num << endl;
			devices_ea[i] = pointToPoint_ea.Install(aggr.Get(i), edge.Get(i));
			if_ea[i] = address_ea.Assign(devices_ea[i]);
			if(Path_num > 2 and MPTCP_bool == true){
				devices_ea2[i] = pointToPoint_ea.Install(aggr.Get(i), edge.Get(i));
				if_ea2[i] = address_ea2.Assign(devices_ea2[i]);
			}else{
				if_ea2[i] = address_ea2.Assign(devices_ea[i]);
			}
			address_ea.NewNetwork();
			address_ea2.NewNetwork();
			devices_ea[i+1] = pointToPoint_ea.Install(aggr.Get(i),edge.Get(i + 1));
			if_ea[i+1] = address_ea.Assign(devices_ea[i+1]);
			if(Path_num > 2 and MPTCP_bool == true){
				devices_ea2[i+1] = pointToPoint_ea.Install(aggr.Get(i),edge.Get(i + 1));
				if_ea2[i+1] = address_ea2.Assign(devices_ea2[i+1]);
			}else{
				if_ea2[i+1] = address_ea2.Assign(devices_ea[i+1]);
			}
			address_ea.NewNetwork();
			address_ea2.NewNetwork();

			for (uint32_t j = 0; j < nHost; j++) {
				if(i == j / nSize){
					//setup ip route
					////configure of aggr
					cmd_oss.str("");
					cmd_oss << "route add 10.1." << j << ".0/24 via " << if_ea[i].GetAddress(1, 0);
					cout <<"aggr" << i << "  :  " << cmd_oss.str() << endl;
					LinuxStackHelper::RunIp(aggr.Get(i), Seconds(0.2), cmd_oss.str().c_str());
					cmd_oss.str("");
					if(Path_num > 2 and MPTCP_bool == true){
						cmd_oss << "route add 10.2." << j << ".0/24 via " << if_ea2[i].GetAddress(1, 0);
					}else{
						cmd_oss << "route add 10.2." << j << ".0/24 via " << if_ea2[i].GetAddress(0, 1);
					}
					cout <<"aggr" << i << "  :  " << cmd_oss.str() << endl;
					//cout << "debug : " << if_ea2[i].GetAddress(0, 1) << endl;
					LinuxStackHelper::RunIp(aggr.Get(i), Seconds(0.2), cmd_oss.str().c_str());
				}else{
					////configure of edge
					cmd_oss.str("");
					cmd_oss << "route add 10.1." << j << ".0/24 via " << if_ea[i].GetAddress(0, 0);
					cout <<"edge" << i << "  :  " << cmd_oss.str() << endl;
					LinuxStackHelper::RunIp(edge.Get(i), Seconds(0.2), cmd_oss.str().c_str());
					cmd_oss.str("");
					if(Path_num > 2 and MPTCP_bool == true){
						cmd_oss << "route add 10.2." << j << ".0/24 via " << if_ea2[i].GetAddress(0, 0);
					}else{
						cmd_oss << "route add 10.2." << j << ".0/24 via " << if_ea2[i].GetAddress(1, 1);
					}
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
					cmd_oss << "route add 10.1." << j << ".0/24 via " << if_ea[i+1].GetAddress(1, 0);
					cout <<"aggr" << i << "  :  " << cmd_oss.str() << endl;
					LinuxStackHelper::RunIp(aggr.Get(i), Seconds(0.2), cmd_oss.str().c_str());
					cmd_oss.str("");
					if(Path_num > 2 and MPTCP_bool == true){
						cmd_oss << "route add 10.2." << j << ".0/24 via " << if_ea2[i+1].GetAddress(1, 0);
					}else{
						cmd_oss << "route add 10.2." << j << ".0/24 via " << if_ea2[i+1].GetAddress(1, 1);
					}
					cout <<"aggr" << i << "  :  " << cmd_oss.str() << endl;
					LinuxStackHelper::RunIp(aggr.Get(i), Seconds(0.2), cmd_oss.str().c_str());
				}else{
					////configure of edge
					cmd_oss.str("");
					cmd_oss << "route add 10.1." << j << ".0/24 via " << if_ea[i+1].GetAddress(0, 0);
					cout <<"edge" << i + 1 << "  :  " << cmd_oss.str() << endl;
					LinuxStackHelper::RunIp(edge.Get(i+1), Seconds(0.2), cmd_oss.str().c_str());
					cmd_oss.str("");
					if(Path_num > 2 and MPTCP_bool == true){
						cmd_oss << "route add 10.2." << j << ".0/24 via " << if_ea2[i+1].GetAddress(0, 0);
					}else{
						cmd_oss << "route add 10.2." << j << ".0/24 via " << if_ea2[i+1].GetAddress(0, 1);
					}

					cout <<"edge" << i + 1 << "  :  " << cmd_oss.str() << endl;
					LinuxStackHelper::RunIp(edge.Get(i+1), Seconds(0.2), cmd_oss.str().c_str());
				}
			}

			cout << endl;

		} else {
			devices_ea[i - 1] = pointToPoint_ea.Install(aggr.Get(i), edge.Get(i - 1));

			if_ea[i - 1] = address_ea.Assign(devices_ea[i - 1]);
			if(Path_num > 3 and MPTCP_bool == true){
				devices_ea2[i - 1] = pointToPoint_ea.Install(aggr.Get(i), edge.Get(i - 1));
				if_ea2[i - 1] = address_ea2.Assign(devices_ea2[i - 1]);
			}else{
				if_ea2[i - 1] = address_ea2.Assign(devices_ea[i - 1]);
			}
			address_ea.NewNetwork();
			address_ea2.NewNetwork();
			devices_ea[i] = pointToPoint_ea.Install(aggr.Get(i),edge.Get(i));
			if_ea[i] = address_ea.Assign(devices_ea[i]);
			if(Path_num > 3 and MPTCP_bool == true){
				devices_ea2[i] = pointToPoint_ea.Install(aggr.Get(i),edge.Get(i));
				if_ea2[i] = address_ea2.Assign(devices_ea2[i]);
			}else{
				if_ea2[i] = address_ea2.Assign(devices_ea[i]);
			}
			address_ea.NewNetwork();
			address_ea2.NewNetwork();

			for (uint32_t j = 0; j < nHost; j++) {
				if(i - 1 == j / nSize){
					//setup ip route
					////configure of aggr
					cmd_oss.str("");
					cmd_oss << "route add 10.3." << j << ".0/24 via " << if_ea[i-1].GetAddress(1, 0);
					cout <<"aggr" << i << "  :  " << cmd_oss.str() << endl;
					LinuxStackHelper::RunIp(aggr.Get(i), Seconds(0.2), cmd_oss.str().c_str());
					cmd_oss.str("");
					if(Path_num > 3 and MPTCP_bool == true){
						cmd_oss << "route add 10.4." << j << ".0/24 via " << if_ea2[i-1].GetAddress(1, 0);
					}else{
						cmd_oss << "route add 10.4." << j << ".0/24 via " << if_ea2[i-1].GetAddress(1, 1);
					}
					cout <<"aggr" << i << "  :  " << cmd_oss.str() << endl;
					LinuxStackHelper::RunIp(aggr.Get(i), Seconds(0.2), cmd_oss.str().c_str());
				}else{
					////configure of edge
					cmd_oss.str("");
					cmd_oss << "route add 10.3." << j << ".0/24 via " << if_ea[i-1].GetAddress(0, 0);
					cout <<"edge" << i - 1 << "  :  " << cmd_oss.str() << endl;
					LinuxStackHelper::RunIp(edge.Get(i - 1), Seconds(0.2), cmd_oss.str().c_str());
					cmd_oss.str("");
					if(Path_num > 3 and MPTCP_bool == true){
						cmd_oss << "route add 10.4." << j << ".0/24 via " << if_ea2[i-1].GetAddress(0, 0);
					}else{
						cmd_oss << "route add 10.4." << j << ".0/24 via " << if_ea2[i-1].GetAddress(0, 1);
					}
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
					cmd_oss << "route add 10.3." << j << ".0/24 via " << if_ea[i].GetAddress(1, 0);
					cout <<"aggr" << i << "  :  " << cmd_oss.str() << endl;
					LinuxStackHelper::RunIp(aggr.Get(i), Seconds(0.2), cmd_oss.str().c_str());
					cmd_oss.str("");
					if(Path_num > 3 and MPTCP_bool == true){
						cmd_oss << "route add 10.4." << j << ".0/24 via " << if_ea2[i].GetAddress(1, 0);
					}else{
						cmd_oss << "route add 10.4." << j << ".0/24 via " << if_ea2[i].GetAddress(1, 1);
					}
					cout <<"aggr" << i << "  :  " << cmd_oss.str() << endl;
					LinuxStackHelper::RunIp(aggr.Get(i), Seconds(0.2), cmd_oss.str().c_str());
				}else{
					////configure of edge
					cmd_oss.str("");
					cmd_oss << "route add 10.3." << j << ".0/24 via " << if_ea[i].GetAddress(0, 0);
					cout <<"edge" << i << "  :  " << cmd_oss.str() << endl;
					LinuxStackHelper::RunIp(edge.Get(i), Seconds(0.2), cmd_oss.str().c_str());
					cmd_oss.str("");
					if(Path_num > 3 and MPTCP_bool == true){
						cmd_oss << "route add 10.4." << j << ".0/24 via " << if_ea2[i].GetAddress(0, 0);
					}else{
						cmd_oss << "route add 10.4." << j << ".0/24 via " << if_ea2[i].GetAddress(0, 1);
					}
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
	  // default route
	/*
	  LinuxStackHelper::RunIp (host.Get (0), Seconds (0.1), "route add default via 10.1.0.1");
	  cout << "node0 : " << "route add default via 10.1.0.1 " << endl;
	  LinuxStackHelper::RunIp (host.Get (1), Seconds (0.1), "route add default via 10.1.1.1");
	  cout << "node1 : " << "route add default via 10.1.1.1 " << endl;
	  */
	// debug
	stack.SysctlSet(host, ".net.mptcp.mptcp_debug", "1");

	string mem_1 = int2string(buffer_size);
	string mem_2 = int2string(4096) + " " + int2string(buffer_size) + " " + int2string(907374);

	string mem_sw_1 = int2string(buffer_size2);
	string mem_sw_2 = int2string(4096) + " " + int2string(buffer_size2) + " " + int2string(907374);
/*
	string mem_1 = int2string(buffer_size);
	string mem_2 = int2string(buffer_size) + " " + int2string(buffer_size) + " " + int2string(buffer_size);

	string mem_sw_1 = int2string(buffer_size2);
	string mem_sw_2 = int2string(buffer_size2) + " " + int2string(buffer_size2) + " " + int2string(buffer_size2);
*/
	cout << "mem_1 : " << mem_1 << endl;
	cout << "mem_2 : " << mem_2 << endl;
/*
	stack.SysctlSet (host, ".net.ipv4.tcp_rmem", "4096 157000 907374");
	stack.SysctlSet (host, ".net.ipv4.tcp_wmem", "4096 157000 903744");
	stack.SysctlSet (host, ".net.core.rmem_max", "907374");
	stack.SysctlSet (host, ".net.core.wmem_max", "907374");

	stack.SysctlSet (edge, ".net.ipv4.tcp_rmem", "4096 157000 907374");
	stack.SysctlSet (edge, ".net.ipv4.tcp_wmem", "4096 157000 903744");
	stack.SysctlSet (edge, ".net.core.rmem_max", "907374");
	stack.SysctlSet (edge, ".net.core.wmem_max", "907374");

	stack.SysctlSet (aggr, ".net.ipv4.tcp_rmem", "4096 157000 907374");
	stack.SysctlSet (aggr, ".net.ipv4.tcp_wmem", "4096 157000 903744");
	stack.SysctlSet (aggr, ".net.core.rmem_max", "907374");
	stack.SysctlSet (aggr, ".net.core.wmem_max", "907374");

	stack.SysctlSet (core, ".net.ipv4.tcp_rmem", "4096 157000 907374");
	stack.SysctlSet (core, ".net.ipv4.tcp_wmem", "4096 157000 903744");
	stack.SysctlSet (core, ".net.core.rmem_max", "907374");
	stack.SysctlSet (core, ".net.core.wmem_max", "907374");
*/
/*
	stack.SysctlSet (host, ".net.ipv4.tcp_rmem", "4096 87380 907374");
	stack.SysctlSet (host, ".net.ipv4.tcp_wmem", "4096 16384 903744");
	stack.SysctlSet (host, ".net.core.rmem_max", "907374");
	stack.SysctlSet (host, ".net.core.wmem_max", "907374");

	stack.SysctlSet (edge, ".net.ipv4.tcp_rmem", "4096 87380 907374");
	stack.SysctlSet (edge, ".net.ipv4.tcp_wmem", "4096 16384 903744");
	stack.SysctlSet (edge, ".net.core.rmem_max", "907374");
	stack.SysctlSet (edge, ".net.core.wmem_max", "907374");

	stack.SysctlSet (aggr, ".net.ipv4.tcp_rmem", "4096 87380 907374");
	stack.SysctlSet (aggr, ".net.ipv4.tcp_wmem", "4096 16384 903744");
	stack.SysctlSet (aggr, ".net.core.rmem_max", "907374");
	stack.SysctlSet (aggr, ".net.core.wmem_max", "907374");

	stack.SysctlSet (core, ".net.ipv4.tcp_rmem", "4096 87380 907374");
	stack.SysctlSet (core, ".net.ipv4.tcp_wmem", "4096 16384 903744");
	stack.SysctlSet (core, ".net.core.rmem_max", "907374");
	stack.SysctlSet (core, ".net.core.wmem_max", "907374");
*/

	stack.SysctlSet (host, ".net.ipv4.tcp_rmem", mem_2);
	stack.SysctlSet (host, ".net.ipv4.tcp_wmem", mem_2);
	stack.SysctlSet (host, ".net.core.rmem_max", mem_1);
	stack.SysctlSet (host, ".net.core.wmem_max", mem_1);

	stack.SysctlSet (edge, ".net.ipv4.tcp_rmem", mem_sw_2);
	stack.SysctlSet (edge, ".net.ipv4.tcp_wmem", mem_sw_2);
	stack.SysctlSet (edge, ".net.core.rmem_max", mem_sw_1);
	stack.SysctlSet (edge, ".net.core.wmem_max", mem_sw_1);

	stack.SysctlSet (aggr, ".net.ipv4.tcp_rmem", mem_sw_2);
	stack.SysctlSet (aggr, ".net.ipv4.tcp_wmem", mem_sw_2);
	stack.SysctlSet (aggr, ".net.core.rmem_max", mem_sw_1);
	stack.SysctlSet (aggr, ".net.core.wmem_max", mem_sw_1);

	stack.SysctlSet (core, ".net.ipv4.tcp_rmem", mem_sw_2);
	stack.SysctlSet (core, ".net.ipv4.tcp_wmem", mem_sw_2);
	stack.SysctlSet (core, ".net.core.rmem_max", mem_sw_1);
	stack.SysctlSet (core, ".net.core.wmem_max", mem_sw_1);


	//adrs_set.insert(map<int, string>::value_type(0, "10.3.1.2"));
	//adrs_set.insert(map<int, string>::value_type(1, "10.3.4.2"));
	//adrs_set2.insert(map<int, string>::value_type(2, "10.3.0.2"));
	//adrs_set2.insert(map<int, string>::value_type(3, "10.1.15.2"));
/*
	adrs_set2.insert(map<int, string>::value_type(4, "10.3.11.2"));
	//adrs_set2.insert(map<int, string>::value_type(5, "10.1.7.2"));
	adrs_set2.insert(map<int, string>::value_type(6, "10.3.3.2"));
	adrs_set2.insert(map<int, string>::value_type(7, "10.1.13.2"));

	adrs_set2.insert(map<int, string>::value_type(8, "10.1.2.2"));
	//adrs_set2.insert(map<int, string>::value_type(9, "10.3.8.2"));
	adrs_set2.insert(map<int, string>::value_type(10, "10.1.14.2"));
	//adrs_set2.insert(map<int, string>::value_type(11, "10.3.9.2"));

	adrs_set2.insert(map<int, string>::value_type(12, "10.1.6.2"));
	adrs_set2.insert(map<int, string>::value_type(13, "10.3.5.2"));
	//adrs_set2.insert(map<int, string>::value_type(14, "10.1.12.2"));
	adrs_set2.insert(map<int, string>::value_type(15, "10.3.10.2"));
*/

	//adrs_set2.insert(map<int, string>::value_type(0, "10.1.1.2"));
	//adrs_set2.insert(map<int, string>::value_type(1, "10.2.4.2"));
	//adrs_set2.insert(map<int, string>::value_type(0, "10.1.0.2"));
	//adrs_set2.insert(map<int, string>::value_type(0, "10.4.15.2"));
	/*
	if(MPTCP_bool == false){
		adrs_set2.insert(map<int, string>::value_type(0, "10.2.4.2"));
		adrs_set2.insert(map<int, string>::value_type(0, "10.2.5.2"));
		adrs_set2.insert(map<int, string>::value_type(0, "10.2.6.2"));
		adrs_set2.insert(map<int, string>::value_type(0, "10.2.7.2"));

		adrs_set.insert(map<int, string>::value_type(1, "10.2.4.2"));
		adrs_set.insert(map<int, string>::value_type(1, "10.2.5.2"));
		adrs_set.insert(map<int, string>::value_type(1, "10.2.6.2"));
		adrs_set.insert(map<int, string>::value_type(1, "10.2.7.2"));
	}else{
		adrs_set2.insert(map<int, string>::value_type(0, "10.1.4.2"));
		adrs_set2.insert(map<int, string>::value_type(0, "10.3.5.2"));
		adrs_set2.insert(map<int, string>::value_type(0, "10.1.6.2"));
		adrs_set2.insert(map<int, string>::value_type(0, "10.3.7.2"));

		adrs_set.insert(map<int, string>::value_type(1, "10.1.4.2"));
		adrs_set.insert(map<int, string>::value_type(1, "10.3.5.2"));
		adrs_set.insert(map<int, string>::value_type(1, "10.1.6.2"));
		adrs_set.insert(map<int, string>::value_type(1, "10.3.7.2"));
	}
	*/


	/*adrs_set.insert(map<int, string>::value_type(2, "10.2.4.2"));
	adrs_set.insert(map<int, string>::value_type(2, "10.2.5.2"));
	adrs_set.insert(map<int, string>::value_type(2, "10.2.6.2"));
	adrs_set.insert(map<int, string>::value_type(2, "10.2.7.2"));*/

	//adrs_set.insert(map<int, string>::value_type(1, "10.2.7.2"));
	/*adrs_set2.insert(map<int, string>::value_type(0, "10.4.13.2"));

	adrs_set2.insert(map<int, string>::value_type(0, "10.1.2.2"));
	adrs_set2.insert(map<int, string>::value_type(0, "10.3.8.2"));
	adrs_set2.insert(map<int, string>::value_type(0, "10.4.14.2"));
	adrs_set2.insert(map<int, string>::value_type(0, "10.3.9.2"));

	adrs_set2.insert(map<int, string>::value_type(0, "10.2.6.2"));
	adrs_set2.insert(map<int, string>::value_type(0, "10.2.5.2"));
	adrs_set2.insert(map<int, string>::value_type(0, "10.4.12.2"));
	adrs_set2.insert(map<int, string>::value_type(0, "10.3.10.2"));
*/
	//adrs_set2.insert(map<int, string>::value_type(1, "10.1.0.2"));
	adrs_set2.insert(map<int, string>::value_type(0, "10.1.3.2"));
	//adrs_set.insert(map<int, string>::value_type(0, "10.2.4.2"));
	//adrs_set.insert(map<int, string>::value_type(3, "10.4.15.2"));
	//adrs_set.insert(map<int, string>::value_type(5, "10.3.7.2"));
	//test.insert(map<int, string>::value_type(0, "10.2.4.2"));



	cout << "--Display traffic lists --" << endl;
	//show_map(list);
	//show_map(short_list);
	//show_map(adrs_back);
	show_multimap(adrs_set2);

	int Path_num2 = Path_num;
	if(MPTCP_bool == false){
		Path_num2=1;
	}
	int poisson_size;
	int *poisson_set = poisson_pros(1000, 3000, 5, &poisson_size);
	adrs_set.clear();
	init_rand();
	for(int i=0;i < poisson_size;i++){

		shuffle(box, nHost);
		map_insert(adrs_set, box, nHost, 1.0, Path_num2);
		//cout << i << " : " << (double) poisson_set[i] / 1000 << endl;
		//randomize_box(short_box, sizeof short_box / sizeof short_box[0], adrs_set2);
		//randomize_box(short_box, sizeof short_box / sizeof short_box[0], adrs_set2);
		//socketTraffic(adrs_set, kb, (double) poisson_set[i] / 1000, end, host);
		//show_multimap(adrs_set);
		adrs_set.clear();
	}
	adrs_set2.clear();
	adrs_set2.insert(map<int, string>::value_type(0, "10.1.7.2"));
	//map_insert(adrs_set, box, nHost, 0.5, Path_num2);
	socketTraffic(adrs_set2, kb, 1.5, end, host);
	//socketTraffic(adrs_set, kb, 2.0, end, host);
	//socketTraffic(adrs_set2, kb, 2.5, end, host);
	//socketTraffic(adrs_set2, kb, 3.0, end, host);

	//shuffle(box, nHost);
	//show_map(adrs_set2);
	//map_insert(adrs_set2, box, nHost, ratio, Path_num);
	//cout << i << " : " << (double) poisson_set[i] / 1000 << endl;
	//socketTraffic(adrs_set, 70, 1.5, end, host);
	//socketTraffic2(adrs_back, 0, start, end, host);
	//socketTraffic(adrs_set2, 70, 1.5, end, host);
	//show_map(adrs_set2);
	//adrs_set.clear();

	//socketTraffic(adrs_set2, 0, start, end, host);
	//socketTraffic(adrs_set2, kb, 1.7, end, host);
	string pcap_place = "./pcap/sub2/he/" + int2string(nDir_name) + "/iperf-he";
	cout << pcap_place << endl;
	cout << "buffer_size : " << buffer_size << endl;
	pointToPoint_he.EnablePcapAll(pcap_place, false);
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
		cout << "Multipath TCP : Path " << Path_num << endl;
	}else{
		cout << "Single-path TCP" << endl;
	}
	Simulator::Stop(Seconds(end));
	AnimationInterface anim("./xml/mptcp_test.xml");
	Simulator::Run();
	Simulator::Destroy();

	return 0;
}


