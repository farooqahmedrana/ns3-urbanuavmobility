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
#include "uav-random-waypoint.h"
#include "ns3/config.h"
#include "ns3/test.h"
#include "ns3/node-list.h"
#include <sstream>
#include <cstdlib>
#include <time.h>

#include <cmath>
#include "ns3/simulator.h"
#include "ns3/random-variable-stream.h"
#include "ns3/pointer.h"
#include "ns3/string.h"
#include "ns3/position-allocator.h"
#include "ns3/rng-seed-manager.h"


namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (UavRandomWaypointMobilityModel);

TypeId
UavRandomWaypointMobilityModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::UavRandomWaypointMobilityModel")
    .SetParent<MobilityModel> ()
    .SetGroupName ("Mobility")
    .AddConstructor<UavRandomWaypointMobilityModel> ()
    .AddAttribute ("Speed",
                   "A random variable used to pick the speed of a random waypoint model.",
                   StringValue ("ns3::UniformRandomVariable[Min=0.3|Max=0.7]"),
                   MakePointerAccessor (&UavRandomWaypointMobilityModel::m_speed),
                   MakePointerChecker<RandomVariableStream> ())
    .AddAttribute ("Pause",
                   "A random variable used to pick the pause of a random waypoint model.",
                   StringValue ("ns3::ConstantRandomVariable[Constant=2.0]"),
                   MakePointerAccessor (&UavRandomWaypointMobilityModel::m_pause),
                   MakePointerChecker<RandomVariableStream> ())
    .AddAttribute ("PositionAllocator",
                   "The position model used to pick a destination point.",
                   PointerValue (),
                   MakePointerAccessor (&UavRandomWaypointMobilityModel::m_position),
                   MakePointerChecker<PositionAllocator> ());

  return tid;
}

UavRandomWaypointMobilityModel::UavRandomWaypointMobilityModel(){

}

UavRandomWaypointMobilityModel::UavRandomWaypointMobilityModel(string graphFile,  Ptr<UavEnergyModel> energyModel)
{
     RngSeedManager::SetSeed(time(NULL));
     graph.load((char*)graphFile.c_str());
     this->energyModel = energyModel;
     energyModel->SetEnergyDepletionCallback(MakeCallback(&UavRandomWaypointMobilityModel::halt,this));
     energyModel->SetLowBatteryThreshold(0);

     halted = false;
}

void
UavRandomWaypointMobilityModel::halt ()
{
     halted = true;
     cout << "halt time:" << Simulator::Now() << endl;
}


void
UavRandomWaypointMobilityModel::BeginWalk (void)
{
  if(halted)
     return;

  m_helper.Update ();
  Vector m_current = m_helper.GetCurrentPosition ();
  NS_ASSERT_MSG (m_position, "No position allocator added before using this model");
  Vector destination = m_position->GetNext ();
  double speed = m_speed->GetValue ();
  energyModel->move(speed,speed);

  double dx = (destination.x - m_current.x);
  double dy = (destination.y - m_current.y);
  double dz = (destination.z - m_current.z);
  double k = speed / std::sqrt (dx*dx + dy*dy + dz*dz);

  m_helper.SetVelocity (Vector (k*dx, k*dy, k*dz));
  m_helper.Unpause ();
  Time travelDelay = Seconds (CalculateDistance (destination, m_current) / speed);
  m_event.Cancel ();
  m_event = Simulator::Schedule (travelDelay,
                                 &UavRandomWaypointMobilityModel::DoInitializePrivate, this);
  NotifyCourseChange ();
}

void
UavRandomWaypointMobilityModel::DoInitialize (void)
{
  DoInitializePrivate ();
  MobilityModel::DoInitialize ();
  recordTrajectory ();
}

void
UavRandomWaypointMobilityModel::recordTrajectory ()
{
  m_helper.Update();
  trajectory.push_back(m_helper.GetCurrentPosition());
  Simulator::Schedule (Seconds(1),
                                 &UavRandomWaypointMobilityModel::recordTrajectory, this);
}


void
UavRandomWaypointMobilityModel::DoInitializePrivate (void)
{
  if(halted)
     return;

  m_helper.Update ();
  m_helper.Pause ();
  Time pause = Seconds (m_pause->GetValue ());
  m_event = Simulator::Schedule (pause, &UavRandomWaypointMobilityModel::BeginWalk, this);
  NotifyCourseChange ();
}

Vector
UavRandomWaypointMobilityModel::DoGetPosition (void) const
{
  if (! halted){
     m_helper.Update ();
  }
  return m_helper.GetCurrentPosition ();
}

void 
UavRandomWaypointMobilityModel::DoSetPosition (const Vector &position)
{
  m_helper.SetPosition (position);
  Simulator::Remove (m_event);
  m_event = Simulator::ScheduleNow (&UavRandomWaypointMobilityModel::DoInitializePrivate, this);
}

Vector
UavRandomWaypointMobilityModel::DoGetVelocity (void) const
{
  return m_helper.GetVelocity ();
}

void
UavRandomWaypointMobilityModel::printTrajectory()
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
UavRandomWaypointMobilityModel::printCoverage(float cellwidth,float celllength)
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

