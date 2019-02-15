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

#include "ns3/log.h"
#include "ns3/simulator.h"
#include "uav-manhattan.h"
#include <sstream>
#include <cstdlib>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("UavManhattanMobilityModel");

NS_OBJECT_ENSURE_REGISTERED (UavManhattanMobilityModel);

TypeId
UavManhattanMobilityModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::UavManhattanMobilityModel")
    .SetParent<GraphMobilityModel> ()
    .SetGroupName ("Mobility")
    .AddConstructor<UavManhattanMobilityModel> ()
  ;

  return tid;
}

UavManhattanMobilityModel::UavManhattanMobilityModel ()
{

}

UavManhattanMobilityModel::UavManhattanMobilityModel (char* file,Ptr<UavEnergyModel> energy,double maxSpeed,string selectionStrategy)
:GraphMobilityModel(file,maxSpeed,selectionStrategy)
{
     energyModel = energy;     
     energyModel->SetEnergyDepletionCallback(MakeCallback(&UavManhattanMobilityModel::halt,this));
     energyModel->SetLowBatteryThreshold(0);
     move();
     recordTrajectory();
}

void
UavManhattanMobilityModel::move ()
{
     energyModel->move(GetSpeed(),GetSpeed());
     walk();
}


void
UavManhattanMobilityModel::halt ()
{
     lastScheduledEvent.Cancel();
     currentNode = DoGetPosition();
     nextNode = currentNode;
     cout << "halt time:" << Simulator::Now() << endl;
}

void
UavManhattanMobilityModel::pause (){
     energyModel->hover(GetSpeed());
     GraphMobilityModel::pause();
}

void
UavManhattanMobilityModel::resume (){
     energyModel->move(GetSpeed(),GetSpeed());
     GraphMobilityModel::resume();
}

void
UavManhattanMobilityModel::recordTrajectory ()
{
  trajectory.push_back(DoGetPosition());
  Simulator::Schedule (Seconds(1),
                                 &UavManhattanMobilityModel::recordTrajectory, this);
}

void
UavManhattanMobilityModel::printTrajectory()
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
UavManhattanMobilityModel::printCoverage(float cellwidth,float celllength)
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

