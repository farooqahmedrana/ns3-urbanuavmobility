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
#ifndef UAV_MOBILITY_MODEL_H
#define UAV_MOBILITY_MODEL_H

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
#include <vector>
#include "Visitor.h"
#include "Graph.h"
#include "GraphNode.h"
#include "graph-mobility-model.h"

using namespace std;

namespace ns3 {

/**
 * \ingroup mobility
 * \brief UAV mobility model.
 *
 */
enum Mode {patrolling = 1, monitoring};

class UavMobilityModel : public GraphMobilityModel
{

private:
  bool init;
  Ptr<UavEnergyModel> energyModel;
  Ptr<Node> node;
  double altitude;
  bool lowEnergyMode;
  int numberOfRecharges;
  Mode mode;
  Vector monitoringDestination;
  double ascendSpeed;
  double descendSpeed;

  vector<Vector> trajectory;


  void monitorTraffic();
  void checkReturningToBase();
  void ascend(void (ns3::UavMobilityModel::* next) ());
  void ascend(double alt,void (ns3::UavMobilityModel::* next) ());
  void descend(void (ns3::UavMobilityModel::* next) ());
  void descend(double alt,void (ns3::UavMobilityModel::* next) ());
  void move();
  void observe();
  void switchBattery();
  void onBatterySwitched();
  void onDescentAfterObserve();
  void moveToMonitoringDestination();

  void recordTrajectory();

protected:
  virtual void reached();
  virtual void resume();
  virtual void returnToBase();
  virtual void baseReached();
  virtual void activateLowEnergyMode();
  virtual string getResults();
 
public:

  static float BATTERY_REPLACE_TIME;
  static int CAMERA_WINDOW_WIDTH;
  static int CAMERA_WINDOW_HEIGHT;
  static double ALT_FLY;
  static double ALT_OBS;

  static TypeId GetTypeId (void);
  UavMobilityModel();
  UavMobilityModel(char* file,double maxSpeed,double ascSpeed,double descSpeed,Ptr<UavEnergyModel> energyModel,string selectionStrategy = "random");
  void start();
  void go();
  void stop();
  void setMode(Mode m);
  void setMonitoringDestination(Vector& dest);
  Ptr<UavEnergyModel> getEnergyModel();
  virtual void accept(GraphNode* node);
  string getEdgesVisitCount();
  void printTrajectory();
  void printCoverage(float,float);
  void mergeMobilityData(string);
  
  virtual ~UavMobilityModel ();

};

} // namespace ns3

#endif /* UAV_MOBILITY_MODEL_H */

