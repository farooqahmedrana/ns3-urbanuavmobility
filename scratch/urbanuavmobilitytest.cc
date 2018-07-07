/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018 Computer Science Department, FAST-NU, Lahore.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Farooq Ahmed <farooq.ahmad@nu.edu.pk>
 */
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <map>
#include <vector>
#include <cstdlib>
#include <ctime>

#include <libxml/parser.h>
#include "ns3/core-module.h"
#include "ns3/mobility-module.h"
#include "ns3/urbanuavmobility-module.h"
#include "ns3/ns2-mobility-helper.h"
#include "ns3/netanim-module.h"

#include "ns3/point-to-point-module.h"
#include "ns3/wifi-module.h"
#include "ns3/propagation-module.h"
#include "ns3/trace-helper.h"

using namespace ns3;
using namespace std;

struct SimulationProperties {

  string traceFile;
  string graphFile;
  string outputFile;
  string fleetFile;

  int    nodeNum;
  int    nodeSize;
  double duration;

  double voltage;  // parrot : 11.1
  double power;      // parrot : 15
  int capacity;     // custom : 200mA , other possibilities : 4480mAh

  int maxSpeed;
  int uavNum;
  int uavSize;
  vector<std::string> uavGraph;

  double mobilityInterval;

  double ascendSpeed;
  double descendSpeed;
  double communicationRange;
  string selectionStrategy;
};

int checkSimulationArguments(int,char*[],SimulationProperties* properties);
map<std::string,std::string> parse(std::string);
void createTraffic(SimulationProperties* properties);
void createUavs(SimulationProperties* properties);
void parseFleet(SimulationProperties* properties);
void parseTeam(xmlNodePtr node,SimulationProperties* properties);
Ptr<Channel> getCommunicationChannel(SimulationProperties* properties);
Ptr<Base> createBase(SimulationProperties* properties,double,double,string,Ptr<Channel>);
Ptr<Uav> createUav(SimulationProperties* properties,string,string,Ptr<Channel>);
void createUav(Ptr<Node>,SimulationProperties* properties,string);
void printResults();

vector<int> baseNums;
vector<int> uavNums;

// Example to use ns2 traces file in ns3
int main (int argc, char *argv[])
{
  srand(time(0));
  SimulationProperties properties;

  // Enable logging from the ns2 helper
  LogComponentEnable ("Ns2MobilityHelper",LOG_LEVEL_DEBUG);

  // Parse command line attribute
  if (checkSimulationArguments(argc,argv,&properties) ) {

    createTraffic(&properties);
//    createUavs(&properties);
    parseFleet(&properties);

    Simulator::Stop (Seconds (properties.duration));

    AnimationInterface anim (properties.outputFile);
    anim.SetMobilityPollInterval(Seconds(properties.mobilityInterval));
    anim.EnablePacketMetadata (true);
    anim.SkipPacketTracing();
/*    for(int i=0; i < properties.nodeNum; i++){
       anim.UpdateNodeSize(i,properties.nodeSize,properties.nodeSize);
    }
    for(int i=properties.nodeNum; i < (properties.nodeNum + properties.uavNum); i++){
       anim.UpdateNodeSize(i,properties.uavSize,properties.uavSize);
    }
*/

    for(int i=0; i < (int) uavNums.size(); i++){
         anim.UpdateNodeSize(uavNums[i],properties.uavSize,properties.uavSize);
    }

    Simulator::Run ();
    printResults();
    Simulator::Destroy ();
  }

  return 0;
}

int checkSimulationArguments(int argc,char* argv[],SimulationProperties* properties){
  std::string settingsFile;
  CommandLine cmd;
  cmd.AddValue("settings","Settings file",settingsFile);
  cmd.Parse(argc,argv);
  map<std::string,std::string> settings = parse(settingsFile);

  properties->traceFile = settings["traceFile"];
  properties->graphFile = settings["graphFile"];
  properties->outputFile = settings["outputFile"];
  properties->fleetFile = settings["fleetFile"];

  properties->duration = atof ( (const char*) settings["duration"].c_str() );
  properties->nodeNum = atoi ( (const char*) settings["nodeNum"].c_str() );
  properties->nodeSize = atoi ( (const char*) settings["nodeSize"].c_str() );

  properties->voltage = atof ( (const char*) settings["voltage"].c_str() );
  properties->power = atof ( (const char*) settings["power"].c_str() );
  properties->capacity = atoi ( (const char*) settings["capacity"].c_str() );

  properties->maxSpeed = atoi ( (const char*) settings["maxSpeed"].c_str() );
  properties->uavNum = atoi ( (const char*) settings["uavNum"].c_str() );
  properties->uavSize = atoi ( (const char*) settings["uavSize"].c_str() );

  for (int i=0; i < properties->uavNum; i++) {
    std::stringstream nameGraph;
    nameGraph << "uav" << i << "Graph";
    properties->uavGraph.push_back(settings[nameGraph.str()]);
  }

  GraphMobilityModel::PAUSE_TIME = atof ( (const char*) settings["pauseTime"].c_str() );
  UavMobilityModel::BATTERY_REPLACE_TIME = atof ( (const char*) settings["batteryReplaceTime"].c_str() );
  UavMobilityModel::CAMERA_WINDOW_WIDTH = atoi ( (const char*) settings["cameraWidth"].c_str() );
  UavMobilityModel::CAMERA_WINDOW_HEIGHT = atoi ( (const char*) settings["cameraHeight"].c_str() );

  properties->mobilityInterval = atof ( (const char*) settings["mobilityInterval"].c_str() );

  properties->ascendSpeed = atoi ( (const char*) settings["ascendSpeed"].c_str() );
  properties->descendSpeed = atoi ( (const char*) settings["descendSpeed"].c_str() );
  properties->communicationRange = atof ( (const char*) settings["communicationRange"].c_str() );

  UavMobilityModel::ALT_FLY = atoi ( (const char*) settings["flyAltitude"].c_str() );
  UavMobilityModel::ALT_OBS = atoi ( (const char*) settings["observationAltitude"].c_str() );  

  properties->selectionStrategy = settings["selectionStrategy"];


  if (properties->traceFile.empty () || properties->fleetFile.empty ()) {    
    std::cout << "settings not proper" << endl;
    return 0;
  }

  return 1;
}

map<std::string,std::string> parse(std::string settingsFile){

     map<std::string,std::string> properties;
  	xmlDocPtr doc;
	xmlNodePtr cur;

	doc = xmlParseFile((const char*)settingsFile.c_str());

	if (doc == NULL ) {
		cout << "Not parsed" << endl;
		return properties;
	}

	cur = xmlDocGetRootElement(doc);


	if (cur == NULL) {
		cout << "Empty document" << endl;
		xmlFreeDoc(doc);
		return properties;
	}

	if (xmlStrcmp(cur->name, (const xmlChar *) "settings") != 0) {
		cout << "document of the wrong type, root node != settings" << endl;
		xmlFreeDoc(doc);
		return properties;
	}
	else {
		xmlNodePtr node = cur->xmlChildrenNode;

		while (node != NULL){

			if (xmlStrcmp(node->name, (const xmlChar *) "property") == 0) {

                    std::string name( (const char*) xmlGetProp(node,(const xmlChar*) "name") );
                    std::string value( (const char*) xmlGetProp(node,(const xmlChar*) "value") );
                    properties[name] = value;
			}

			node = node->next;
		}

		xmlFreeDoc(doc);
	}

  return properties;

}

void parseFleet(SimulationProperties* properties){
     xmlDocPtr doc;
	xmlNodePtr cur;

	doc = xmlParseFile((const char*) properties->fleetFile.c_str());

	if (doc == NULL ) {
		cout << "Fleet file Not parsed" << endl;
		return ;
	}

	cur = xmlDocGetRootElement(doc);

	if (cur == NULL) {
		cout << "Empty document" << endl;
		xmlFreeDoc(doc);
	}

	if (xmlStrcmp(cur->name, (const xmlChar *) "fleet") != 0) {
		cout << "document of the wrong type, root node != fleet" << endl;
		xmlFreeDoc(doc);
	}
	else {
		xmlNodePtr node = cur->xmlChildrenNode;

		while (node != NULL){

			if (xmlStrcmp(node->name, (const xmlChar *) "team") == 0) {
                    parseTeam(node,properties);
			}

			node = node->next;
		}

		xmlFreeDoc(doc);
	}
}

void parseTeam(xmlNodePtr root,SimulationProperties* properties){
     Ptr<Channel> channel = getCommunicationChannel(properties);
     Ptr<Base> base;
     Ptr<Uav> uav;
     string uavIp;

     xmlNodePtr node = root->xmlChildrenNode;
	while (node != NULL){
		if (xmlStrcmp(node->name, (const xmlChar *) "base") == 0) {
			double baseX = atof ((const char*) xmlGetProp(node,(const xmlChar*) "x"));
			double baseY = atof ((const char*) xmlGetProp(node,(const xmlChar*) "y"));
			string baseIp = string ((const char*) xmlGetProp(node,(const xmlChar*) "ip"));

               base = createBase(properties,baseX,baseY,baseIp,channel);
               baseNums.push_back(base->GetId());
		}

          if (xmlStrcmp(node->name, (const xmlChar *) "uav") == 0) {
			string graph = string ((const char*) xmlGetProp(node,(const xmlChar*) "graph"));
			uavIp = string ((const char*) xmlGetProp(node,(const xmlChar*) "ip"));

               uav = createUav(properties,graph,uavIp,channel);
               uavNums.push_back(uav->GetId());
		}

		node = node->next;
	}

     //Simulator::Schedule(Seconds(100), &Base::send, base, uavIp );
}

void createTraffic(SimulationProperties* properties){
    Ns2MobilityHelper ns2 = Ns2MobilityHelper (properties->traceFile);
    NodeContainer stas;
    stas.Create (properties->nodeNum);
    ns2.Install ();
}

Ptr<Base> createBase(SimulationProperties* properties,double x,double y,string ip,Ptr<Channel> channel){
     Ptr<Base> base = CreateObject<Base>(x,y);
     base->setup(channel,ip);     
     return base;
}

Ptr<Channel> getCommunicationChannel(SimulationProperties* properties){

    YansWifiChannelHelper channelHelper = YansWifiChannelHelper::Default ();
    Ptr<YansWifiChannel> wifiChannel  = channelHelper.Create();
    Ptr<RangePropagationLossModel> loss = CreateObject<RangePropagationLossModel>();
    loss->SetAttribute("MaxRange",DoubleValue(properties->communicationRange));
    wifiChannel->SetPropagationLossModel(loss);
    return wifiChannel;
}

Ptr<Uav> createUav(SimulationProperties* properties,string graph,string ip,Ptr<Channel> channel){
    UavContainer uavs;
    uavs.Create(1);

    createUav(uavs.Get(0),properties,graph);
    uavs.Get(0)->setup(channel,ip);
    uavs.Get(0)->startServer();
    uavs.Get(0)->launch();
    return uavs.Get(0);
}

void createUavs(SimulationProperties* properties){

    Ptr<Channel> channel = getCommunicationChannel(properties);

    UavContainer uavs;
    uavs.Create(properties->uavNum);
    string ips[] = {"10.0.0.1","10.0.0.2","10.0.0.3"};

    for (int i=0; i < properties->uavNum; i++) {
       createUav(uavs.Get(i),properties,properties->uavGraph[i]);
       uavs.Get(i)->setup(channel,ips[i+1]);
//       uavs.Get(i)->launch();
    }

     Ptr<Base> base = CreateObject<Base>(162.69,362.77);
     base->setup(channel,ips[0]);     

/*
     PointToPointHelper helper;
     AsciiTraceHelper trace;
     helper.EnableAsciiAll(trace.CreateFileStream("randomgraphmobilitytemp1.tr"));
 
    Ptr<PointToPointChannel> channel = CreateObject<PointToPointChannel>();
     channel->SetAttribute("Delay", StringValue("2ms"));

     //channel->Attach(DynamicCast<PointToPointNetDevice>(base->GetDevice(0)));
     //channel->Attach(DynamicCast<PointToPointNetDevice>(uavs.Get(0)->GetDevice(0)));

     Ptr<PointToPointNetDevice> device = DynamicCast<PointToPointNetDevice>(base->GetDevice(0));
     if (device != 0){
          cout << "attaching channel" << endl ;
          device->Attach(channel);     
     }

     device = DynamicCast<PointToPointNetDevice>(uavs.Get(0)->GetDevice(0));
     if (device != 0){
          cout << "attaching channel" << endl ;
          device->Attach(channel);     
     }
*/

     for (int i=0; i < properties->uavNum; i++) {
         uavs.Get(i)->startServer();
         uavs.Get(i)->launch();
    }

     Simulator::Schedule(Seconds(100), &Base::send, base, ips[1] );

//    Simulator::Schedule(Seconds(100), &Uav::setMonitoringMode, uavs.Get(0) );
//    Simulator::Schedule(Seconds(150), &Uav::setPatrollingMode, uavs.Get(0) );
}

void createUav(Ptr<Node> uav,SimulationProperties* properties,std::string graphFile) {
     Ptr<UavEnergyModel> energyModel = CreateObject<UavEnergyModel>(uav,properties->voltage,properties->capacity);
     Ptr<UavMobilityModel> mobilityModel = CreateObject<UavMobilityModel>((char*)graphFile.c_str(),properties->maxSpeed,properties->ascendSpeed,properties->descendSpeed,energyModel,properties->selectionStrategy);
     uav->AggregateObject(mobilityModel);
}

void printResults(){
     int nodesListSize = (int) NodeList::GetNNodes(); 
     for (int i=0 ; i < nodesListSize; i++){
          Ptr<Node> node = NodeList::GetNode(i);
          Ptr<GraphMobilityModel> mobilityModel = node->GetObject<UavMobilityModel>();
          if (mobilityModel != NULL){
               mobilityModel->printResults();
          }       
     }
}    
