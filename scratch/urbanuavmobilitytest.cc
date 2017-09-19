/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2017 Computer Science Department, FAST-NU, Lahore.
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
#include <sstream>
#include <map>
#include <vector>

#include <libxml/parser.h>
#include "ns3/core-module.h"
#include "ns3/mobility-module.h"
#include "ns3/urbanuavmobility-module.h"
#include "ns3/ns2-mobility-helper.h"
#include "ns3/netanim-module.h"

using namespace ns3;

struct SimulationProperties {

  std::string traceFile;
  std::string graphFile;
  std::string outputFile;

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

};

int checkSimulationArguments(int,char*[],SimulationProperties* properties);
map<std::string,std::string> parse(std::string);
void createTraffic(SimulationProperties* properties);
void createUavs(SimulationProperties* properties);
void createUav(Ptr<Node>,SimulationProperties* properties,std::string);
void printResults();


// Example to use ns2 traces file in ns3
int main (int argc, char *argv[])
{
  SimulationProperties properties;

  // Enable logging from the ns2 helper
  LogComponentEnable ("Ns2MobilityHelper",LOG_LEVEL_DEBUG);

  // Parse command line attribute
  if (checkSimulationArguments(argc,argv,&properties) ) {

    createTraffic(&properties);
    createUavs(&properties);

    Simulator::Stop (Seconds (properties.duration));

    AnimationInterface anim (properties.outputFile);
    anim.SetMobilityPollInterval(Seconds(properties.mobilityInterval));
    for(int i=0; i < properties.nodeNum; i++){
       anim.UpdateNodeSize(i,properties.nodeSize,properties.nodeSize);
    }
    for(int i=properties.nodeNum; i < (properties.nodeNum + properties.uavNum); i++){
       anim.UpdateNodeSize(i,properties.uavSize,properties.uavSize);
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

  if (properties->traceFile.empty () || properties->nodeNum <= 0 || properties->duration <= 0 || properties->graphFile.empty ()) {    
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

void createTraffic(SimulationProperties* properties){
    Ns2MobilityHelper ns2 = Ns2MobilityHelper (properties->traceFile);
    NodeContainer stas;
    stas.Create (properties->nodeNum);
    ns2.Install ();
}

void createUavs(SimulationProperties* properties){

    UavContainer uavs;
    uavs.Create(properties->uavNum);

    for (int i=0; i < properties->uavNum; i++) {
       createUav(uavs.Get(i),properties,properties->uavGraph[i]);
    }

}

void createUav(Ptr<Node> uav,SimulationProperties* properties,std::string graphFile) {
     Ptr<UavEnergyModel> energyModel = CreateObject<UavEnergyModel>(uav,properties->voltage,properties->power,properties->capacity);
     Ptr<GraphMobilityModel> mobilityModel = CreateObject<UavMobilityModel>((char*)graphFile.c_str(),properties->maxSpeed,energyModel);
     uav->AggregateObject(mobilityModel);
}

void printResults(){
     int nodesListSize = (int) NodeList::GetNNodes(); 
     for (int i=0 ; i < nodesListSize; i++){
          Ptr<Node> node = NodeList::GetNode(i);
          Ptr<GraphMobilityModel> mobilityModel = node->GetObject<GraphMobilityModel>();
          if (mobilityModel != NULL){
               mobilityModel->printResults();
          }       
     }
}    
