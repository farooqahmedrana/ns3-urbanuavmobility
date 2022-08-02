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

#include "ns3/log.h"
#include "ns3/address.h"
#include "ns3/ipv4.h"
#include "ns3/ipv4-header.h"
#include "ns3/ipv4-interface-address.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/packet-socket-address.h"
#include "ns3/node.h"
#include "ns3/nstime.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/string.h"
#include "ns3/pointer.h"
#include "uav-application.h"
#include "uav-mode-header.h"
#include "uav-edges-header.h"
#include "uav.h"
#include <iostream>

using namespace std;

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("UavApplication");

NS_OBJECT_ENSURE_REGISTERED (UavApplication);

TypeId
UavApplication::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::UavApplication")
    .SetParent<Application> ()
    .SetGroupName("Applications")
    .AddConstructor<UavApplication> ()
    .AddAttribute ("Remote", "The address of the destination",
                   AddressValue (),
                   MakeAddressAccessor (&UavApplication::m_peer),
                   MakeAddressChecker ())
    .AddAttribute ("Protocol", "The type of protocol to use.",
                   TypeIdValue (UdpSocketFactory::GetTypeId ()),
                   MakeTypeIdAccessor (&UavApplication::m_tid),
                   MakeTypeIdChecker ())
  ;
  return tid;
}


UavApplication::UavApplication ()
  : m_socket_local (0),
     m_socket_remote (0),
     broadcast_source(0),
     broadcast_sink(0)
{
  NS_LOG_FUNCTION (this);
}

UavApplication::~UavApplication()
{
  NS_LOG_FUNCTION (this);
}


void
UavApplication::DoDispose (void)
{
  NS_LOG_FUNCTION (this);

  m_socket_local = 0;
  m_socket_remote = 0;
  broadcast_source = 0;
  broadcast_sink = 0;
  // chain up
  Application::DoDispose ();
}

// Application Methods
void UavApplication::StartApplication () // Called at time specified by Start
{
  NS_LOG_FUNCTION (this);

  // Create the socket if not already
  if (!m_socket_local)
    {

      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      m_socket_local = Socket::CreateSocket (GetNode (), tid);
      InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (),9);
      m_socket_local->Bind (local);
 

      m_socket_local->SetRecvCallback (MakeCallback (&UavApplication::HandleRead, this));
    }

  if (! m_socket_remote)
    {
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      m_socket_remote = Socket::CreateSocket (GetNode (), tid);
      if (Ipv4Address::IsMatchingType(m_peer))
        {
          m_socket_remote->Bind ();
          m_socket_remote->Connect (InetSocketAddress (Ipv4Address::ConvertFrom(m_peer),9));
        }
    }


   if (!broadcast_sink)
    {
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      broadcast_sink = Socket::CreateSocket (GetNode (), tid);
      InetSocketAddress localAddr = InetSocketAddress (GetNode()->GetObject<ns3::Ipv4>()->GetAddress(1,0).GetLocal(),9999);
      broadcast_sink->Bind(localAddr);
      broadcast_sink->SetRecvCallback (MakeCallback (&UavApplication::HandleBroadcastRead, this));
    } 

    if (!broadcast_source)
    {
      InetSocketAddress broadcastAddr = InetSocketAddress ("255.255.255.255",9999);
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      broadcast_source = Socket::CreateSocket (GetNode (), tid);
      broadcast_source->SetAllowBroadcast(true);
      broadcast_source->Connect (broadcastAddr);
    }


}

void UavApplication::StopApplication () // Called at time specified by Stop
{
  NS_LOG_FUNCTION (this);

  if(m_socket_local != 0)
    {
      m_socket_local->Close ();
    }
  else
    {
      NS_LOG_WARN ("UavApplication found null socket to close in StopApplication");
    }

if(m_socket_remote != 0)
    {
      m_socket_remote->Close ();
    }
  else
    {
      NS_LOG_WARN ("UavApplication found null socket to close in StopApplication");
    }

}

void UavApplication::BroadcastPacket ()
{
  NS_LOG_FUNCTION (this);


  UavEdgesHeader edgesHeader;
  edgesHeader.SetEdges(edgesInfo);
  Ptr<Packet> packet = Create<Packet> ();
  packet->AddHeader(edgesHeader);  

  broadcast_source->Send (packet);

  NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds ()
                   << "s uav application broadcast "
                   <<  packet->GetSize () << " bytes to "
                   << " port " << 9);

}


void UavApplication::SendPacket ()
{
  NS_LOG_FUNCTION (this);

  UavModeHeader header;
  UavEdgesHeader edgesHeader;
  header.SetMode(1);
  edgesHeader.SetEdges(edgesInfo);
  Ptr<Packet> packet = Create<Packet> ();
  packet->AddHeader(header);  
  packet->AddHeader(edgesHeader);  


  m_socket_remote->Send (packet);

  if (Ipv4Address::IsMatchingType (m_peer))
    {
      NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds ()
                   << "s uav application sent "
                   <<  packet->GetSize () << " bytes to "
                   << Ipv4Address::ConvertFrom(m_peer)
                   << " port " << /*InetSocketAddress::ConvertFrom (m_peer).GetPort ()*/9);
    }
 }


void UavApplication::HandleRead (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  Ptr<Packet> packet;
  Address from;
  while ((packet = socket->RecvFrom (from)))
    {
      if (packet->GetSize () > 0)
        {
          UavEdgesHeader edgesHeader;
          packet->RemoveHeader (edgesHeader);
          string edges = edgesHeader.GetEdges ();
          cout << "Edges : " << edges << endl;

          UavModeHeader modeHeader;
          packet->RemoveHeader (modeHeader);
          uint32_t mode = modeHeader.GetMode ();

          cout << "Mode : " << mode << endl;
//         updateMode(mode);
       }
    }


}

void UavApplication::HandleBroadcastRead (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  Ptr<Packet> packet;
  Address from;
  while ((packet = socket->RecvFrom (from)))
    {
      if (packet->GetSize () > 0)
        {
          UavEdgesHeader edgesHeader;
          packet->RemoveHeader (edgesHeader);
          string edges = edgesHeader.GetEdges ();
//          cout << "Edges : " << edges << endl;

          Ptr<Uav> uav = DynamicCast<Uav>(GetNode());
          if (uav != 0){
               uav->handleEdgesInfo(edges);
          }

       }
    }


}

void UavApplication::updateMode(uint32_t mode){
     currentMode = mode;
     if (currentMode == 0){
          // switch to patrolling mode
          Ptr<Uav> uav = DynamicCast<Uav>(GetNode());
          cout << "uav : " << uav << endl;
          if (uav != 0){
               cout << "setting patrolling mode" << endl;
               uav->setPatrollingMode();
          }
     }
     else if (currentMode == 1){
          // switch to monitoring mode
          Ptr<Uav> uav = DynamicCast<Uav>(GetNode());
          cout << "uav : " << uav << endl;
          if (uav != 0){
               cout << "setting monitoring mode" << endl;
               uav->setMonitoringMode();
          }
     }
}

void UavApplication::setEdges(string e){
     edgesInfo = e;
}



} // Namespace ns3
