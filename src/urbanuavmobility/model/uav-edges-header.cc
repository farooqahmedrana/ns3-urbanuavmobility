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

#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/header.h"
#include "ns3/simulator.h"
#include "uav-edges-header.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("UavEdgesHeader");

NS_OBJECT_ENSURE_REGISTERED (UavEdgesHeader);

UavEdgesHeader::UavEdgesHeader ()
{
  NS_LOG_FUNCTION (this);
}

void
UavEdgesHeader::SetEdges (string e)
{
     edges = e;
}

string
UavEdgesHeader::GetEdges (void) const
{
  NS_LOG_FUNCTION (this);
  return edges;
}

TypeId
UavEdgesHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::UavEdgesHeader")
    .SetParent<Header> ()
    .SetGroupName("Applications")
    .AddConstructor<UavEdgesHeader> ()
  ;
  return tid;
}

TypeId
UavEdgesHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

void
UavEdgesHeader::Print (std::ostream &os) const
{
  NS_LOG_FUNCTION (this << &os);
  os << "(edges:" << edges << ")";
}

uint32_t
UavEdgesHeader::GetSerializedSize (void) const
{
  NS_LOG_FUNCTION (this);
  return edges.size();
}

void
UavEdgesHeader::Serialize (Buffer::Iterator start) const
{
  NS_LOG_FUNCTION (this << &start);
  Buffer::Iterator iter = start;
  const char* str = edges.c_str();
  for(unsigned int i=0; i < edges.length(); i++){
    iter.WriteU8(str[i]);
  }
}

uint32_t
UavEdgesHeader::Deserialize (Buffer::Iterator start)
{
  NS_LOG_FUNCTION (this << &start);
  Buffer::Iterator iter = start;

  edges.clear();
  while (!iter.IsEnd()) {
    edges.push_back(iter.ReadU8());
  }

  return GetSerializedSize ();
}

} // namespace ns3
