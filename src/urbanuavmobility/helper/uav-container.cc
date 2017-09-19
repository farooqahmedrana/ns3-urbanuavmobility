/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 Phillip Sitbon
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
 * Author: Phillip Sitbon <phillip@sitbon.net>
 */

#include "uav-container.h"


namespace ns3 {

UavContainer::UavContainer(){

}

void 
UavContainer::Create (uint32_t n)
{
  for (uint32_t i = 0; i < n; i++)
    {
      m_nodes.push_back (CreateObject<Uav> ());
    }
}

Ptr<Uav> 
UavContainer::Get (uint32_t i) const
{
  return m_nodes[i];
}

UavContainer::~UavContainer(){

}



} // namespace ns3

