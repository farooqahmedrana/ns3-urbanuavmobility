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
double UavMobilityModel::ALT_FLY = 10;
double UavMobilityModel::ALT_OBS = 20;

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
  init = true;
  lowEnergyMode = false;
  numberOfRecharges = 0;
  mode = patrolling;
  monitoringDestination = baseNode;
}

UavMobilityModel::UavMobilityModel(char* file,double maxSpeed,double ascSpeed,double descSpeed,Ptr<UavEnergyModel> energyModel,string selectionStrategy)
: GraphMobilityModel(file,maxSpeed,selectionStrategy)
{
     init = true;
     this->energyModel = energyModel;
     this->energyModel->SetEnergyDepletionCallback(MakeCallback(&UavMobilityModel::activateLowEnergyMode,this));
     this->energyModel->SetLowBatteryThreshold(ALT_FLY,ALT_OBS,GetSpeed(),GetSpeed(),GetSpeed());
     numberOfRecharges = 0;
     mode = patrolling;
     monitoringDestination = baseNode;
     ascendSpeed = ascSpeed;
     descendSpeed = descSpeed;
}

void UavMobilityModel::activateLowEnergyMode(){
     lowEnergyMode = true;
     cout << "Activating low energy mode for " << GetId() << endl;     
     if(mode == monitoring && altitude > ALT_FLY){
          descend(ALT_FLY,&UavMobilityModel::returnToBase);
     }
     else checkReturningToBase();
}

void UavMobilityModel::checkReturningToBase(){
     if (lowEnergyMode == true){
          Vector currentPosition = DoGetPosition();
          double nextDistance = CalculateDistance (currentNode, nextNode);
          double nextBaseDistance = CalculateDistance (nextNode, baseNode);
          if ( energyModel->isLow(nextDistance + nextBaseDistance,ALT_FLY,GetSpeed(),GetSpeed(),GraphMobilityModel::PAUSE_TIME) ){
               returnToBase();
          }
     }
}

void UavMobilityModel::returnToBase(){
     energyModel->move(ALT_FLY,GetSpeed());
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

     descend(&UavMobilityModel::switchBattery);
}

void UavMobilityModel::switchBattery(){
     stop();

     this->energyModel = this->energyModel->clone();
     this->energyModel->SetEnergyDepletionCallback(MakeCallback(&UavMobilityModel::activateLowEnergyMode,this));
     this->energyModel->SetLowBatteryThreshold(ALT_FLY,ALT_OBS,ascendSpeed,descendSpeed,GetSpeed());

     numberOfRecharges++;

     Time pauseTime = Seconds(BATTERY_REPLACE_TIME);
     lowEnergyMode = false;
     lastUpdate = Simulator::Now();

     lastScheduledEvent = Simulator::Schedule(pauseTime, &UavMobilityModel::onBatterySwitched, this);
}

void UavMobilityModel::onBatterySwitched(){
     start();
     go();
}

void UavMobilityModel::accept(GraphNode* node){
     energyModel->move(ALT_FLY,GetSpeed());
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

void UavMobilityModel::start(){
     energyModel->start();
     if(init){
          recordTrajectory();
     }
}

void UavMobilityModel::stop(){
     energyModel->stop();
}

void UavMobilityModel::go(){
     ascend(&UavMobilityModel::move);
}

void UavMobilityModel::ascend( void (ns3::UavMobilityModel::* next) () ){
     ascend(ALT_FLY,next);
}

void UavMobilityModel::ascend(double alt, void (ns3::UavMobilityModel::* next) () ){
     energyModel->ascend(alt,ascendSpeed);
     altitude = alt;
     cout << "ascending to altitude: " << altitude << endl;

     lastScheduledEvent.Cancel();
     travelDelay = Seconds (altitude / ascendSpeed);
     lastUpdate = Simulator::Now();
     lastScheduledEvent = Simulator::Schedule(travelDelay, next, this);
}

void UavMobilityModel::descend(void (ns3::UavMobilityModel::* next) ()){
     descend(0,next);
}

void UavMobilityModel::descend(double alt,void (ns3::UavMobilityModel::* next) ()){
     energyModel->descend(altitude,descendSpeed);  

     cout << "descending to altitude: " << alt << endl;

     lastScheduledEvent.Cancel();
     travelDelay = Seconds ( (altitude - alt) / descendSpeed );
     lastUpdate = Simulator::Now();
     lastScheduledEvent = Simulator::Schedule(travelDelay, next, this);

     altitude = alt;
}

void UavMobilityModel::move(){
     if(init){
          init = false;
          walk();
     }
     else{
          resume();
     }
}

void UavMobilityModel::reached(){
     onReached();
     ascend(ALT_OBS,&UavMobilityModel::observe);
}

void UavMobilityModel::onDescentAfterObserve(){
     GraphMobilityModel::resume();
}

void UavMobilityModel::resume(){
     if (altitude > ALT_FLY){
          descend(ALT_FLY,&UavMobilityModel::onDescentAfterObserve);
     }
     else{
          GraphMobilityModel::resume();
     }
     checkReturningToBase();
}

void UavMobilityModel::observe(){
     if(mode == patrolling){
          energyModel->hover(ALT_OBS);
          pause();
          monitorTraffic();
     }

     if(mode == monitoring){
          cout << "monitoring node (" << monitoringDestination.x << "," << monitoringDestination.y << ")" << endl;
          monitorTraffic();
          lastScheduledEvent = Simulator::Schedule(Seconds(GraphMobilityModel::PAUSE_TIME), &UavMobilityModel::observe, this);
          checkReturningToBase();
     }
}

Ptr<UavEnergyModel> UavMobilityModel::getEnergyModel(){
     return energyModel;
}

void UavMobilityModel::setMode(Mode m){
     mode = m;
     moveToMonitoringDestination();
}

void UavMobilityModel::setMonitoringDestination(Vector& dest){
     monitoringDestination = dest;     
     moveToMonitoringDestination();
}

void UavMobilityModel::moveToMonitoringDestination(){
     cout << "check moving to monitoring dest" << endl;
     if(mode == monitoring && 
          currentNode.x != monitoringDestination.x && currentNode.y != monitoringDestination.y){
     cout << "now moving to monitoring dest" << endl;
          lastScheduledEvent.Cancel();
          currentNode = DoGetPosition();
          nextNode = monitoringDestination;
          travelDelay = Seconds (CalculateDistance (currentNode, nextNode) / GetSpeed());
          lastUpdate = Simulator::Now();
          lastScheduledEvent = Simulator::Schedule(travelDelay, &UavMobilityModel::reached, this);
     }
}

void
UavMobilityModel::recordTrajectory ()
{

  trajectory.push_back(DoGetPosition());
  Simulator::Schedule (Seconds(1),
                                 &UavMobilityModel::recordTrajectory, this);
}


void
UavMobilityModel::printTrajectory()
{
     int count = 0;
     for(int i=0; i < (int) trajectory.size() - 1; i++){

          Vector p1(trajectory[i].x,trajectory[i].y,trajectory[i].z);
          Vector p2(trajectory[i+1].x,trajectory[i+1].y,trajectory[i+1].z);

          if(graph.lineEdgesIntersect(p1,p2)){
               count++;
          }

     }
     cout << "count:" << count << endl;
     cout << "trajectory:" << (trajectory.size() - 1) << endl;
     cout << "oncourse:" << ( 100.0 * count / (trajectory.size()-1) )  << "%" << endl;
}

void
UavMobilityModel::printCoverage(float cellwidth,float celllength)
{
     int totalrelevantcount = 0,relevantcovered = 0, irrelevantcovered = 0;

     vector<Region> cells = graph.decompose(cellwidth,celllength);
     for(int i=0; i < (int) cells.size(); i++){
          if(cells[i].getColor() == 1) {
               totalrelevantcount++;
          }
     }


     for(int i=0; i < (int) trajectory.size(); i++){
          for(int j=0; j < (int) cells.size(); j++){
               if(cells[j].hasPoint(trajectory[i])) {
                    if (cells[j].getColor() == 1)
                         cells[j].mark(2);
                    else if (cells[j].getColor() == 0)
                         cells[j].mark(3);
                    break;
               }
          }
     }

//     int count = 0;
     for(int i=0; i < (int) cells.size(); i++){
          if(cells[i].getColor() == 2) {
               relevantcovered++;
          }
          else if (cells[i].getColor() == 3){
               irrelevantcovered++;
          }
     }

     cout << "coverage:" << ( 100.0 * relevantcovered / totalrelevantcount )  << "%" << endl;
     cout << "deviation:" << ( 100.0 * irrelevantcovered / (relevantcovered+irrelevantcovered) )  << "%" << endl;

     for(int i=0; i < (int) cells.size(); i++){
          if(cells[i].getColor() == 1) {
               cells[i].print();
          }
     }

}



UavMobilityModel::~UavMobilityModel ()
{

}

} // namespace ns3

