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
#ifndef UAV_ENERGY_MODEL_H
#define UAV_ENERGY_MODEL_H

#include "ns3/simple-device-energy-model.h"
#include "ns3/basic-energy-source.h"
#include "ns3/nstime.h"
#include "ns3/event-id.h"
#include "ns3/traced-value.h"


namespace ns3 {

/**
 * \ingroup energy
 * \brief A UAV energy model.
 *
 */
class UavEnergyModel : public SimpleDeviceEnergyModel
{

public:
  static TypeId GetTypeId (void);
  UavEnergyModel ();
  UavEnergyModel (Ptr<Node> node,double voltage,int capacity);
  virtual ~UavEnergyModel ();

  void SetEnergyDepletionCallback (Callback<void> callback);
  void SetEnergyRechargedCallback (Callback<void> callback);
  virtual void HandleEnergyDepletion (void);
  virtual void HandleEnergyRecharged (void);


  double GetEnergy(double voltage,int capacity);
  double GetCurrent(double energy,double distance,double speed);
  void SetLowBatteryThreshold(double altfly,double altobs,double spasc,double spdes,double spmov);
  bool isLow(double dist,double altfly,double spmov,double spdes,double obsTime);
  Ptr<Node> GetMobileNode();
  Ptr<UavEnergyModel> clone();

  void start();
  void stop();
  void ascend(double distance,double speed);
  void descend(double distance,double speed);
  void move(double altitude,double speed);
  void hover(double altitude);

protected:
  double voltage;
  int capacity;
  Ptr<BasicEnergySource> energySource;

  virtual double energyForAscend(double,double);
  virtual double energyForDescend(double,double);
  virtual double energyForMove(double,double);
  virtual double energyForHover(double);

  double getAvailableEnergy();
  double range(double,double,double,double,double);
  double distance(double,double,double);

  double getPower(double);

private:

  double interpolate(double x, double xi[], double yi[],int isize, int npoints);

  Callback<void> m_energyDepletionCallback;
  Callback<void> m_energyRechargedCallback;

};

} // namespace ns3

#endif /* UAV_ENERGY_MODEL_H */
