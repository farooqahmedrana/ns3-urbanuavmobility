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
#ifndef UAV_RANDOM_WAYPOINT_H
#define UAV_RANDOM_WAYPOINT_H

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
 * \brief Uav-Random-Waypoint mobility model.
 *
 */

class UavRandomWaypointMobilityModel : public MobilityModel
{
protected:
  Graph graph;
  vector<Vector> trajectory;
  Ptr<UavEnergyModel> energyModel;

  ConstantVelocityHelper m_helper; //!< helper for velocity computations
  Ptr<PositionAllocator> m_position; //!< pointer to position allocator
  Ptr<RandomVariableStream> m_speed; //!< random variable to generate speeds
  Ptr<RandomVariableStream> m_pause; //!< random variable to generate pauses
  EventId m_event; //!< event ID of next scheduled event

  bool halted ;

  virtual void DoInitialize (void); 

  /**
   * Get next position, begin moving towards it, schedule future pause event
   */
  void BeginWalk (void);
  /**
   * Begin current pause event, schedule future walk event
   */
  void DoInitializePrivate (void);
  virtual Vector DoGetPosition (void) const;
  virtual void DoSetPosition (const Vector &position);
  virtual Vector DoGetVelocity (void) const;

  void halt();

  void recordTrajectory();

public:

  UavRandomWaypointMobilityModel();
  UavRandomWaypointMobilityModel(string graphFile,Ptr<UavEnergyModel> energyModel);
  static TypeId GetTypeId (void);
  void printTrajectory();
  void printCoverage(float,float);
};

} // namespace ns3

#endif /* UAV_RANDOM_WAYPOINT_H */

