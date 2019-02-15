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
#ifndef UAV_SRCM_H
#define UAV_SRCM_H

#include <stdint.h>
#include <deque>
#include "ns3/mobility-model.h"
#include "ns3/vector.h"
#include "ns3/waypoint.h"
#include "ns3/random-waypoint-mobility-model.h"
#include "ns3/uav-energy-model.h"
#include "ns3/event-id.h"
#include <libxml/parser.h>
#include <iostream>
#include <map>
#include <vector>
#include "Visitor.h"
#include "Graph.h"
#include "GraphNode.h"


#include "ns3/constant-velocity-helper.h"
#include "ns3/position-allocator.h"
#include "ns3/ptr.h"
#include "ns3/random-variable-stream.h"


using namespace std;

namespace ns3 {

/**
 * \ingroup mobility
 * \brief Uav-SRCM mobility model.
 *
 */

class UavSrcmMobilityModel : public MobilityModel
{
protected:
  Graph graph;
  vector<Vector> trajectory;
  Ptr<UavEnergyModel> energyModel;

  Vector base;
  Vector current;
  Vector next;

  double range;
  double radius;
  double speed;
  int theta;

  int direction;  // 0 clock-wise, 1 counter-clock-wise
  Time lastUpdate;
  Time travelDelay;
  EventId lastScheduledEvent;

  void linear();
  void circular();
  void halt();

  virtual Vector DoGetPosition (void) const;
  virtual void DoSetPosition (const Vector &position);
  virtual Vector DoGetVelocity (void) const;

  void recordTrajectory();

public:

  UavSrcmMobilityModel();
  UavSrcmMobilityModel(string graphFile,Ptr<UavEnergyModel> energyModel,double range,double speed);
  static TypeId GetTypeId (void);
  void printTrajectory();
  void printCoverage(float,float);
};

} // namespace ns3

#endif /* UAV_SRCM_H */

