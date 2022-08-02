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

#ifndef UAV_EDGES_HEADER_H
#define UAV_EDGES_HEADER_H

#include "ns3/header.h"
#include "ns3/nstime.h"
#include <string>

using namespace std;

namespace ns3 {
/**
 * \ingroup mobility
 * \class UavModeHeader
 * \brief Packet header for UDP client/server application.
 *
 * The header contains mode code
 */
class UavEdgesHeader : public Header
{
public:
  UavEdgesHeader ();

  /**
   * \param seq the sequence number
   */
  void SetEdges (string );
  /**
   * \return the sequence number
   */
  string GetEdges (void) const;

  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  virtual TypeId GetInstanceTypeId (void) const;
  virtual void Print (std::ostream &os) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);

private:
  string edges; //!< Mode
};

} // namespace ns3

#endif /* UAV_EDGES_HEADER_H */
