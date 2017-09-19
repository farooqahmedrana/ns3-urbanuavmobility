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
#include "ns3/simulator.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/energy-source.h"
#include "uav-energy-model.h"
#include "ns3/log.h"
#include <iostream>

using namespace std;

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("UavEnergyModel");

NS_OBJECT_ENSURE_REGISTERED (UavEnergyModel);

TypeId
UavEnergyModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::UavEnergyModel")
    .SetParent<SimpleDeviceEnergyModel> ()
    .SetGroupName ("Energy")    
  ;
  return tid;
}

UavEnergyModel::UavEnergyModel ()
{ }

UavEnergyModel::UavEnergyModel(Ptr<Node> node,double voltage,double power,int capacity,int motors){
  this->voltage = voltage;
  this->power = power;
  this->capacity = capacity;
  this->motors = motors;
  this->SetNode(node);

  energySource = CreateObject<BasicEnergySource> ();
  energySource->SetSupplyVoltage(this->voltage);
  energySource->SetInitialEnergy(GetEnergy(voltage,capacity));

  energySource->SetNode(GetNode());
  energySource->AppendDeviceEnergyModel(this);
  energySource->SetAttribute("BasicEnergyHighBatteryThreshold", DoubleValue(1));
  this->SetEnergySource(energySource);
 
  this->SetCurrentA(GetCurrent(voltage,power));
}

double UavEnergyModel::GetEnergy(double voltage,int capacity){
  return capacity * voltage;
}

double UavEnergyModel::GetCurrent(double voltage,double power){
  return (power * this->motors) / voltage;
}

Ptr<Node> UavEnergyModel::GetMobileNode(){
  return energySource->GetNode();
}

Ptr<UavEnergyModel> UavEnergyModel::clone(){
  Ptr<UavEnergyModel> model = CreateObject<UavEnergyModel>(GetMobileNode(),this->voltage,this->power,this->capacity,this->motors);
  return model;
}

UavEnergyModel::~UavEnergyModel ()
{
  NS_LOG_FUNCTION (this);
}

double UavEnergyModel::GetFlyTime(){
  return energySource->GetInitialEnergy() / (power * motors);
}

double UavEnergyModel::GetRemainingTime(){
  return energySource->GetRemainingEnergy() / (power * motors);
}

void UavEnergyModel::SetLowBatteryThreshold(double range,double speed){
  double time = range / speed;
  double energy = GetEnergy(voltage,GetCurrent(voltage,power) * time);
  energySource->SetAttribute("BasicEnergyLowBatteryThreshold", DoubleValue( (energy / energySource->GetInitialEnergy()) ));
}



void
UavEnergyModel::SetEnergyDepletionCallback (
  Callback<void> callback)
{
  NS_LOG_FUNCTION (this);
  if (callback.IsNull ())
    {
      NS_LOG_DEBUG ("UavEnergyModel:Setting NULL energy depletion callback!");
    }
  m_energyDepletionCallback = callback;
}

void
UavEnergyModel::SetEnergyRechargedCallback (
  Callback<void> callback)
{
  NS_LOG_FUNCTION (this);
  if (callback.IsNull ())
    {
      NS_LOG_DEBUG ("UavEnergyModel:Setting NULL energy recharged callback!");
    }
  m_energyRechargedCallback = callback;
}

void
UavEnergyModel::HandleEnergyDepletion (void)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG ("UavEnergyModel:Energy is depleted!");
  cout << "UavEnergyModel:Energy is depleted! @" << Simulator::Now() << endl;
  cout << "Remaining Energy : " << energySource->GetRemainingEnergy() << endl;

  // invoke energy depletion callback, if set.
  if (!m_energyDepletionCallback.IsNull ())
    {
      Callback<void> temp = m_energyDepletionCallback;
      m_energyDepletionCallback = MakeNullCallback<void>();
      temp ();
    }
}

void
UavEnergyModel::HandleEnergyRecharged (void)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG ("UavEnergyModel:Energy is recharged!");
  // invoke energy recharged callback, if set.
  cout << "UavEnergyModel:Energy is recharged! @" << Simulator::Now() << endl;
  if (!m_energyRechargedCallback.IsNull ())
    {
      m_energyRechargedCallback ();
    }
}



} // namespace ns3
