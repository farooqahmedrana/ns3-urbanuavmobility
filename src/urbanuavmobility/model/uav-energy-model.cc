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

UavEnergyModel::UavEnergyModel(Ptr<Node> node,double voltage,int capacity){
  this->voltage = voltage;
  this->capacity = capacity;
  this->SetNode(node);

  energySource = CreateObject<BasicEnergySource> ();
  energySource->SetSupplyVoltage(this->voltage);
  energySource->SetInitialEnergy(GetEnergy(voltage,capacity));

  energySource->SetNode(GetNode());
  energySource->AppendDeviceEnergyModel(this);
  energySource->SetAttribute("BasicEnergyHighBatteryThreshold", DoubleValue(1));
  this->SetEnergySource(energySource);
 
}

double UavEnergyModel::GetEnergy(double voltage,int capacity){
  return capacity * voltage;
}


double UavEnergyModel::GetCurrent(double energy,double distance,double speed){
  double time = distance / speed;
  return energy / (voltage * time);
}

Ptr<Node> UavEnergyModel::GetMobileNode(){
  return energySource->GetNode();
}

Ptr<UavEnergyModel> UavEnergyModel::clone(){
  Ptr<UavEnergyModel> model = CreateObject<UavEnergyModel>(GetMobileNode(),this->voltage,this->capacity);
  return model;
}

void UavEnergyModel::start(){
  cout << "start energy supply" << endl;
  this->SetCurrentA(energyForHover(0)/(voltage));
  cout << "remaining energy:" << this->energySource->GetRemainingEnergy() << endl;
}

void UavEnergyModel::stop(){
  cout << "stop energy supply" << endl;
  this->SetCurrentA(0);
  cout << "remaining energy:" << this->energySource->GetRemainingEnergy() << endl;
}

void UavEnergyModel::ascend(double altitude,double speed){
  cout << "energy to ascend" << endl;
  this->SetCurrentA(GetCurrent(energyForAscend(altitude,speed),altitude,speed));
}

void UavEnergyModel::descend(double altitude,double speed){
  cout << "energy to descend" << endl;
  this->SetCurrentA(GetCurrent(energyForDescend(altitude,speed),altitude,speed));
  cout << "remaining energy:" << this->energySource->GetRemainingEnergy() << endl;
}

void UavEnergyModel::move(double altitude,double speed){
  cout << "energy to move" << endl;
  this->SetCurrentA(GetCurrent(energyForMove(altitude,speed),1,speed));
}

void UavEnergyModel::hover(double altitude){
  cout << "energy to hover" << endl;
  this->SetCurrentA(energyForHover(altitude)/(voltage));
}

UavEnergyModel::~UavEnergyModel ()
{
  NS_LOG_FUNCTION (this);
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

double UavEnergyModel::energyForAscend(double height,double speed){
     return 250 * height / speed;     
     //return 1;
}

double UavEnergyModel::energyForDescend(double height,double speed){
     return 210 * height / speed;
     //return 1;
}

double UavEnergyModel::energyForMove(double height,double speed){
     return getPower(speed)/speed;
//     return 1;
}

double UavEnergyModel::energyForHover(double height){
     return 220;
     //return 1;
}

double UavEnergyModel::getAvailableEnergy(){
     return voltage*capacity;
}

double UavEnergyModel::range(double altfly,double altobs,double spasc,double spdes,double spmov){
     return distance(getAvailableEnergy() - energyForAscend(altobs,spasc) - energyForDescend(altobs,spdes),altfly,spmov) / 2;
}

double UavEnergyModel::distance(double energy,double height,double speed){
     return energy / energyForMove(height,speed);
}

void UavEnergyModel::SetLowBatteryThreshold(double altfly,double altobs,double spasc,double spdes,double spmov){
  double threshold = range(altfly,altobs,spasc,spdes,spmov) * energyForMove(altfly,spmov); 
  double thresholdRatio = threshold / energySource->GetInitialEnergy();
  energySource->SetAttribute("BasicEnergyLowBatteryThreshold", DoubleValue( thresholdRatio ));
}

bool UavEnergyModel::isLow(double dist,double altfly,double spmov,double spdes,double obsTime){
  double energyLeft = energySource->GetRemainingEnergy() - energyForDescend(altfly,spdes) - energyForHover(altfly) * obsTime;
  if(distance(energyLeft,altfly,spmov) < dist){
     cout << "LOW ENERGY: " << energyLeft << endl;
     return true;
  }

  return false;
}

double UavEnergyModel::getPower(double speed){
     const int size = 12;
     double measuredSpeed[size] = {0.1,1.8,2.8,5,6,6.8,8,9,9.8,13,15};
     double measuredPower[size] = {220,220,220,210,206,206,212,214,220,214,280,310};
     return interpolate(speed,measuredSpeed,measuredPower,size,5);
}

double UavEnergyModel::interpolate(double x, double xi[], double yi[],  
              int isize, int npoints) 
{ 
     double lambda[isize]; 
     double y; 
     int j, is, il; 
     // check order of interpolation 
     if (npoints > isize) npoints = isize;     
     // if x is ouside the xi[] interval  
     if (x <= xi[0])       return y = yi[0]; 
     if (x >= xi[isize-1]) return y = yi[isize-1];     
     // loop to find j so that x[j-1] < x < x[j] 
     j = 0; 
     while (j <= isize-1) 
     { 
          if (xi[j] >= x) break; 
          j = j + 1; 
     } 
     // shift j to correspond to (npoint-1)th interpolation 
     j = j - npoints/2; 
     // if j is ouside of the range [0, ... isize-1] 
     if (j < 0) j=0; 
     if (j+npoints-1 > isize-1 ) j=isize-npoints; 
     y = 0.0; 
     for (is = j; is <= j+npoints-1; is = is+1) 
     { 
          lambda[is] = 1.0; 
          for (il = j; il <= j+ npoints-1; il = il + 1) 
          { 
               if(il != is) lambda[is] = lambda[is]* 
                                             (x-xi[il])/(xi[is]-xi[il]); 
          } 
          y = y + yi[is]*lambda[is]; 
     } 
     return y; 
} 

} // namespace ns3
