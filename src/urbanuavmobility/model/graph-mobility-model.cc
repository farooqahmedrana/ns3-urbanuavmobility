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
#include "graph-mobility-model.h"
#include "ns3/config.h"
#include "ns3/test.h"
#include "ns3/node-list.h"
#include <sstream>
#include <cstdlib>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("GraphMobilityModel");

NS_OBJECT_ENSURE_REGISTERED (GraphMobilityModel);

float GraphMobilityModel::PAUSE_TIME = 2.0;

TypeId
GraphMobilityModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::GraphMobilityModel")
    .SetParent<WaypointMobilityModel> ()
    .SetGroupName ("Mobility")
    .AddConstructor<GraphMobilityModel> ()
  ;

  return tid;
}

GraphMobilityModel::GraphMobilityModel ()
{

}

GraphMobilityModel::GraphMobilityModel (char* file)
{

     graph.load(file);
     walk();
}

GraphMobilityModel::GraphMobilityModel (char* file,double maxSpeed)
{

     this->maxSpeed = maxSpeed; // m/s

     cout << "loading graph" << endl;      
     graph.load(file);
     baseNode = Vector(graph.getRoot()->getX(),graph.getRoot()->getY(),0);
     currentNode = baseNode;
     nextNode = baseNode;
     currentNodeId = "base";
     nextNodeId = "base";

     cout << "initiating walk" << endl;
     walk();
}

GraphMobilityModel::GraphMobilityModel (char* file,double maxSpeed,float baseX,float baseY)
: baseNode(baseX,baseY,0)
{
     this->maxSpeed = maxSpeed; // m/s

     currentNode = baseNode;
     nextNode = baseNode;
     currentNodeId = "base";
     nextNodeId = "base";

     cout << "loading graph" << endl;      
     graph.load(file);
     graph.setRoot(graph.findNearest(baseX,baseY));
     cout << "initiating walk" << endl;
     walk();
}

int GraphMobilityModel::GetId(){
     Ptr<Node> node = GetObject<Node> ();
     if (node != NULL){
          return node->GetId();
     }

     return -1;
}

double GraphMobilityModel::GetSpeed(){
     return maxSpeed;
}

Vector GraphMobilityModel::DoGetPosition (void) const{

     Time delta = Simulator::Now() - lastUpdate;
     double factor = 0;
     if (travelDelay.GetSeconds() > 0){
          factor = delta.GetSeconds() / travelDelay.GetSeconds();
     }

     Vector diff;
     diff.x = (nextNode.x - currentNode.x) * factor;
     diff.y = (nextNode.y - currentNode.y) * factor;
     diff.z = (nextNode.z - currentNode.z) * factor;

     Vector position = currentNode;
     position.x += diff.x;
     position.y += diff.y;
     position.z += diff.z;
     return position;
}

void GraphMobilityModel::DoSetPosition (const Vector &position) { }

Vector GraphMobilityModel::DoGetVelocity (void) const
{
  return Vector();
}

void GraphMobilityModel::onReached(){
     graph.markEdge(currentNodeId,nextNodeId);
     currentNodeId = nextNodeId;
}

void GraphMobilityModel::reached(){
     cout << GetId() <<  " reached at " << Simulator::Now() << endl;
     currentNode = nextNode;
     onReached();
     pause();
}

void GraphMobilityModel::pause(){
     cout << GetId() << " paused at " << Simulator::Now() << endl;
     Time pauseTime = Seconds(PAUSE_TIME);
     lastUpdate = Simulator::Now();
     lastScheduledEvent = Simulator::Schedule(pauseTime, &GraphMobilityModel::resume, this);          
}

void GraphMobilityModel::resume(){
     cout << GetId() << " resumed at " << Simulator::Now() << endl;
     walk();          
}

void GraphMobilityModel::walk(){
     graph.stepWalk(this);
}

void GraphMobilityModel::accept(GraphNode* node){


     Vector point(node->getX(),node->getY(),0.0);
     nextNode = point;
     nextNodeId = node->getId();

     cout << GetId() << ";current: (" << currentNode.x << "," << currentNode.y << ");"
          << "next: (" << nextNode.x << "," << nextNode.y << ")" << endl;

     travelDelay = Seconds (CalculateDistance (currentNode, nextNode) / GetSpeed());
     lastUpdate = Simulator::Now();

     lastScheduledEvent = Simulator::Schedule(travelDelay, &GraphMobilityModel::reached, this);

}

void GraphMobilityModel::printResults(){
     cout << getResults() << endl;
}

string GraphMobilityModel::getResults(){
     stringstream result;
     result << "result of " << GetId() << ":";
     result << graph.stats();     

     return result.str();
}

GraphMobilityModel::~GraphMobilityModel ()
{

}

} // namespace ns3

