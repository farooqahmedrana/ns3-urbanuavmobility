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
#include "uav-srcm.h"
#include "ns3/config.h"
#include "ns3/test.h"
#include "ns3/node-list.h"
#include <sstream>
#include <cstdlib>

#include <cmath>
#include "ns3/simulator.h"
#include "ns3/random-variable-stream.h"
#include "ns3/pointer.h"
#include "ns3/string.h"
#include "ns3/position-allocator.h"


namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (UavSrcmMobilityModel);

TypeId
UavSrcmMobilityModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::UavSrcmMobilityModel")
    .SetParent<MobilityModel> ()
    .SetGroupName ("Mobility")
    .AddConstructor<UavSrcmMobilityModel> ();

  return tid;
}

UavSrcmMobilityModel::UavSrcmMobilityModel(){

}

UavSrcmMobilityModel::UavSrcmMobilityModel(string graphFile,Ptr<UavEnergyModel> energyModel,double range,double speed)
{

     graph.load((char*)graphFile.c_str());
     base = Vector(graph.getRoot()->getX(),graph.getRoot()->getY(),0);
     current = base;
     this->range = range;
     this->speed = speed;
     next = base;

     this->energyModel = energyModel;
     energyModel->SetEnergyDepletionCallback(MakeCallback(&UavSrcmMobilityModel::halt,this));
     energyModel->SetLowBatteryThreshold(0);
     energyModel->move(speed,speed);

     recordTrajectory();
     linear();
}

void
UavSrcmMobilityModel::linear(){

     theta = 0;

     this->radius = (rand() % (int) range);
     next.x = base.x + radius;
     
     travelDelay = Seconds (CalculateDistance (current, next) / speed);
     lastUpdate = Simulator::Now();

     lastScheduledEvent = Simulator::Schedule(travelDelay, &UavSrcmMobilityModel::circular, this);
}

void
UavSrcmMobilityModel::circular(){
     current = next;
     if(theta == 360){
          lastScheduledEvent.Cancel();
          linear();
     }
     else{
                    
          theta += 10;
          next.x = base.x + (radius * cos(theta * 3.1415 / 180));
          next.y = base.y + (radius * sin(theta * 3.1415 / 180));

          travelDelay = Seconds (CalculateDistance (current, next) / speed);
          lastUpdate = Simulator::Now();
          lastScheduledEvent = Simulator::Schedule(travelDelay, &UavSrcmMobilityModel::circular, this);
     }
}

void
UavSrcmMobilityModel::halt ()
{
     current = DoGetPosition();
     next = current;
     lastScheduledEvent.Cancel();
     cout << "halt time:" << Simulator::Now() << endl;
}


void
UavSrcmMobilityModel::recordTrajectory ()
{

  trajectory.push_back(DoGetPosition());
  Simulator::Schedule (Seconds(1),
                                 &UavSrcmMobilityModel::recordTrajectory, this);
}



Vector
UavSrcmMobilityModel::DoGetPosition (void) const
{
     Time delta = Simulator::Now() - lastUpdate;
     double factor = 0;
     if (travelDelay.GetSeconds() > 0){
          factor = delta.GetSeconds() / travelDelay.GetSeconds();
     }

     Vector diff;
     diff.x = (next.x - current.x) * factor;
     diff.y = (next.y - current.y) * factor;
     diff.z = (next.z - current.z) * factor;

     Vector position = current;
     position.x += diff.x;
     position.y += diff.y;
     position.z += diff.z;
     return position;
}

void 
UavSrcmMobilityModel::DoSetPosition (const Vector &position)
{

}

Vector
UavSrcmMobilityModel::DoGetVelocity (void) const
{
  return Vector();
}

void
UavSrcmMobilityModel::printTrajectory()
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
UavSrcmMobilityModel::printCoverage(float cellwidth,float celllength)
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



} // namespace ns3

