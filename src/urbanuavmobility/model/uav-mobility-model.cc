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
#include <limits>
#include "ns3/abort.h"
#include "ns3/simulator.h"
#include "ns3/uinteger.h"
#include "ns3/log.h"
#include "ns3/boolean.h"
#include "uav-mobility-model.h"
#include "ns3/config.h"
#include "ns3/test.h"
#include "ns3/node-list.h"
#include "ns3/system-thread.h"

#include <cstdlib>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("UavMobilityModel");

NS_OBJECT_ENSURE_REGISTERED (UavMobilityModel);

float UavMobilityModel::BATTERY_REPLACE_TIME = 5.0;
int UavMobilityModel::CAMERA_WINDOW_WIDTH = 10;
int UavMobilityModel::CAMERA_WINDOW_HEIGHT = 20;


TypeId
UavMobilityModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::UavMobilityModel")
    .SetParent<WaypointMobilityModel> ()
    .SetGroupName ("Mobility")
    .AddConstructor<UavMobilityModel> ()
  ;

  return tid;
}

UavMobilityModel::UavMobilityModel(){
  lowEnergyMode = false;
  numberOfRecharges = 0;
}

UavMobilityModel::UavMobilityModel(char* file,double maxSpeed,Ptr<UavEnergyModel> energyModel)
: GraphMobilityModel(file,maxSpeed)
{
     this->energyModel = energyModel;
     this->energyModel->SetEnergyDepletionCallback(MakeCallback(&UavMobilityModel::returnToBase,this));
     roundTrips = 1;
     maxDistance = energyModel->GetFlyTime() * maxSpeed;
     maxRoundTripDistance = maxDistance / 2;
     range = maxRoundTripDistance / roundTrips;
     this->energyModel->SetLowBatteryThreshold(range,maxSpeed);
     numberOfRecharges = 0;
}

UavMobilityModel::UavMobilityModel (char* file,double maxSpeed,Ptr<UavEnergyModel> energyModel,float baseX,float baseY)
: GraphMobilityModel(file,maxSpeed,baseX,baseY)
{
     this->energyModel = energyModel;
     this->energyModel->SetEnergyDepletionCallback(MakeCallback(&UavMobilityModel::activateLowEnergyMode,this));
     roundTrips = 1;
     maxDistance = energyModel->GetFlyTime() * maxSpeed;
     maxRoundTripDistance = maxDistance / 2;
     range = maxRoundTripDistance / roundTrips;
     this->energyModel->SetLowBatteryThreshold(range,maxSpeed);
     numberOfRecharges = 0;
}

void UavMobilityModel::activateLowEnergyMode(){
     lowEnergyMode = true;
     cout << "Activating low energy mode for " << GetId() << endl;     
     checkReturningToBase();
}

void UavMobilityModel::checkReturningToBase(){
     if (lowEnergyMode == true){
          Vector currentPosition = DoGetPosition();
          double nextDistance = CalculateDistance (currentNode, nextNode);
          double nextBaseDistance = CalculateDistance (nextNode, baseNode);
          if ( (nextDistance+nextBaseDistance) / GetSpeed()  > energyModel->GetRemainingTime() ){
               returnToBase();
          }
     }
}

void UavMobilityModel::returnToBase(){
     cout << GetId() << " returning to base at " << Simulator::Now() << endl;
     lastScheduledEvent.Cancel();

     currentNode = DoGetPosition(); // the intermediate position that node has reached so far
     nextNode = baseNode;

     cout << "current: (" << currentNode.x << "," << currentNode.y << ");"
          << "next: (" << nextNode.x << "," << nextNode.y << ")" << endl;

     travelDelay = Seconds (CalculateDistance (currentNode, nextNode) / GetSpeed());
     lastUpdate = Simulator::Now();
     lastScheduledEvent = Simulator::Schedule(travelDelay, &UavMobilityModel::baseReached, this);
}

void UavMobilityModel::baseReached(){
     cout << GetId() << " reached base at " << Simulator::Now() << endl;
     currentNode = baseNode;
     nextNode = baseNode;

     this->energyModel = this->energyModel->clone();
     this->energyModel->SetEnergyDepletionCallback(MakeCallback(&UavMobilityModel::activateLowEnergyMode,this));
     this->energyModel->SetLowBatteryThreshold(range,maxSpeed);

     numberOfRecharges++;

     Time pauseTime = Seconds(BATTERY_REPLACE_TIME);
     lowEnergyMode = false;
     lastUpdate = Simulator::Now();
     lastScheduledEvent = Simulator::Schedule(pauseTime, &UavMobilityModel::resume, this);
}

void UavMobilityModel::onReached() {
     GraphMobilityModel::onReached();
     monitorTraffic();
     checkReturningToBase();
}

void UavMobilityModel::accept(GraphNode* node){
     GraphMobilityModel::accept(node);
     checkReturningToBase();

     Simulator::Schedule(Seconds(0.25*travelDelay.GetSeconds()), &UavMobilityModel::monitorTraffic, this);
     Simulator::Schedule(Seconds(0.50*travelDelay.GetSeconds()), &UavMobilityModel::monitorTraffic, this);
     Simulator::Schedule(Seconds(0.75*travelDelay.GetSeconds()), &UavMobilityModel::monitorTraffic, this);
}

void UavMobilityModel::monitorTraffic(){
     Vector currentPosition = DoGetPosition();
     int windowWidth = CAMERA_WINDOW_WIDTH;     // todo: update using camera Field of View and orientation parameters
     int windowHeight = CAMERA_WINDOW_HEIGHT;

     float windowTop = currentPosition.y - windowHeight;
     float windowBottom = currentPosition.y + windowHeight;
     float windowLeft = currentPosition.x - windowWidth;
     float windowRight = currentPosition.x + windowWidth;
     
     int numNodesInWindow = 0;
     int nodesListSize = (int) NodeList::GetNNodes(); 
     for (int i=0 ; i < nodesListSize; i++){
          Ptr<Node> node = NodeList::GetNode(i);
          Ptr<MobilityModel> mobilityModel = node->GetObject<MobilityModel>();
          Vector position = mobilityModel->GetPosition();
        
          if (position.x >= windowLeft && position.x <= windowRight &&
               position.y >= windowTop && position.y <= windowBottom){

               numNodesInWindow++;
          }
     }

     cout << "window (top,left,right,bottom): (" << windowTop << "," << windowLeft << "," << windowRight << "," << windowBottom << ")" << endl;
     cout << "Traffic detected by " << GetId() << " at " << Simulator::Now() << " :" << numNodesInWindow << endl;
}

string UavMobilityModel::getResults(){
     stringstream result;
     result << GraphMobilityModel::getResults();
     result << "number of recharges:" << numberOfRecharges;     

     return result.str();
}

UavMobilityModel::~UavMobilityModel ()
{

}

} // namespace ns3

