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
#include "uav-mode-header.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("UavModeHeader");

NS_OBJECT_ENSURE_REGISTERED (UavModeHeader);

UavModeHeader::UavModeHeader ()
  : mode (0)
{
  NS_LOG_FUNCTION (this);
}

void
UavModeHeader::SetMode (uint32_t m)
{
  NS_LOG_FUNCTION (this << m);
  mode = m;
}

uint32_t
UavModeHeader::GetMode (void) const
{
  NS_LOG_FUNCTION (this);
  return mode;
}

TypeId
UavModeHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::UavModeHeader")
    .SetParent<Header> ()
    .SetGroupName("Applications")
    .AddConstructor<UavModeHeader> ()
  ;
  return tid;
}

TypeId
UavModeHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

void
UavModeHeader::Print (std::ostream &os) const
{
  NS_LOG_FUNCTION (this << &os);
  os << "(mode=" << mode << ")";
}

uint32_t
UavModeHeader::GetSerializedSize (void) const
{
  NS_LOG_FUNCTION (this);
  return 4;
}

void
UavModeHeader::Serialize (Buffer::Iterator start) const
{
  NS_LOG_FUNCTION (this << &start);
  Buffer::Iterator i = start;
  i.WriteHtonU32 (mode);
}

uint32_t
UavModeHeader::Deserialize (Buffer::Iterator start)
{
  NS_LOG_FUNCTION (this << &start);
  Buffer::Iterator i = start;
  mode = i.ReadNtohU32 ();

  return GetSerializedSize ();
}

} // namespace ns3
