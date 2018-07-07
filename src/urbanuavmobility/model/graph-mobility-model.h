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
#ifndef GRAPH_MOBILITY_MODEL_H
#define GRAPH_MOBILITY_MODEL_H

#include <stdint.h>
#include <deque>
#include "ns3/mobility-model.h"
#include "ns3/vector.h"
#include "ns3/waypoint.h"
#include "ns3/waypoint-mobility-model.h"
#include "ns3/uav-energy-model.h"
#include "ns3/event-id.h"
#include <libxml/parser.h>
#include <iostream>
#include <map>
#include "Visitor.h"
#include "Graph.h"
#include "GraphNode.h"

using namespace std;

namespace ns3 {

/**
 * \ingroup mobility
 * \brief Graph-based mobility model.
 *
 */
class GraphMobilityModel : public MobilityModel, Visitor
{
protected:
  xmlDocPtr doc;
  Graph graph; 
  double maxSpeed;
  Vector baseNode;
  Vector currentNode;
  Vector nextNode;
  string currentNodeId;
  string nextNodeId;
  Time lastUpdate;
  Time travelDelay;
  EventId lastScheduledEvent;

  int GetId();
  double GetSpeed();
  virtual void reached();
  virtual void onReached();
  virtual void pause();
  virtual void resume();
  virtual string getResults();
 
public:

  static TypeId GetTypeId (void);
  static float PAUSE_TIME;

  GraphMobilityModel ();
  GraphMobilityModel (char* file);
  GraphMobilityModel(char* file,double maxSpeed,string selectionStrategy = "random");
  GraphMobilityModel(char* file,double maxSpeed,float baseX,float baseY);
  void walk();

  virtual void accept(GraphNode* node);
  virtual Vector DoGetPosition (void) const;
  virtual void DoSetPosition (const Vector &position);
  virtual Vector DoGetVelocity (void) const;
  virtual void printResults();
  virtual ~GraphMobilityModel ();

};

} // namespace ns3

#endif /* GRAPH_MOBILITY_MODEL_H */

