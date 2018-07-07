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

#include "base.h"
#include "ns3/mobility-module.h"
#include "ns3/log.h"

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/wifi-module.h"
#include "ns3/applications-module.h"

#include "ns3/trace-helper.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("Base");

NS_OBJECT_ENSURE_REGISTERED (Base);


TypeId
Base::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::Base")
    .SetParent<Node> ()
    .SetGroupName ("Mobility")
    .AddConstructor<Base> ()
  ;

  return tid;
}

Base::Base() {

}

Base::Base(double x,double y){
     Ptr<MobilityModel> mobility = CreateObject<ConstantPositionMobilityModel>();
     mobility->SetPosition(Vector(x,y,0));
     AggregateObject(mobility);  
}

void Base::setup(Ptr<Channel> channel,string ip){
/*
     Ptr<PointToPointNetDevice> device = CreateObject<PointToPointNetDevice>();
     device->SetAddress (Mac48Address::Allocate ());  
     AddDevice(device);
     ObjectFactory queueFactory;
     queueFactory.SetTypeId ("ns3::DropTailQueue");
     Ptr<Queue> queue = queueFactory.Create<Queue>();
     device->SetQueue (queue);
*/

     YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
     phy.SetChannel (DynamicCast<YansWifiChannel>(channel));
     WifiHelper wifi = WifiHelper::Default ();
     wifi.SetRemoteStationManager ("ns3::AarfWifiManager");
     NqosWifiMacHelper mac = NqosWifiMacHelper::Default ();
     Ssid ssid = Ssid ("ns-3-ssid");
     mac.SetType ("ns3::ApWifiMac",
                  "Ssid", SsidValue (ssid));
     NetDeviceContainer apDevices;
     apDevices = wifi.Install(phy, mac, this);
     Ptr<NetDevice> device = apDevices.Get(0);

     InternetStackHelper stack;
     stack.Install(this);

     Ptr<Ipv4> ipv4 = this->GetObject<Ipv4> ();
     int32_t interface = ipv4->GetInterfaceForDevice (device);
     if (interface == -1)
     {
          interface = ipv4->AddInterface (device);
     }

      Ipv4InterfaceAddress ipv4Addr = Ipv4InterfaceAddress (Ipv4Address(ip.c_str()), Ipv4Mask("255.255.255.0"));
      ipv4->AddAddress (interface, ipv4Addr);
      ipv4->SetMetric (interface, 1);
      ipv4->SetUp (interface);

     cout << "base created" << endl;

}

void Base::send(string ip){
     cout << "starting echo app" << endl;
/*
     UdpEchoClientHelper echoClient (Ipv4Address("10.0.0.2"), 9);
     echoClient.SetAttribute ("MaxPackets", UintegerValue (10));
     echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
     echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

     app = echoClient.Install(this);
     app.Start(Seconds(1));
*/

     Ptr<UavApplication> app = CreateObject<UavApplication>();
     app->SetAttribute("Remote",AddressValue(Ipv4Address(ip.c_str())));
     AddApplication(app);
     app->SetStartTime(Seconds(0));
     Simulator::Schedule (Seconds(2), &UavApplication::SendPacket, app);
}

Base::~Base(){

}


} // namespace ns3

