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
#ifndef BASE_H_
#define BASE_H_

#include "ns3/node.h"
#include "ns3/applications-module.h"
#include "ns3/network-module.h"
#include "uav-application.h"
#include <string>

using namespace std;

namespace ns3{

/**
 * \ingroup mobility
 * \brief UAV Base station.
 *
 */

class Base : public Node {

     ApplicationContainer app;

public:
     static TypeId GetTypeId (void);
     Base();
	Base(double x,double y); 
     void setup(Ptr<Channel> channel,string ip);
     void send(string ip);    
	virtual ~Base();
};

}

#endif /* BASE_H_ */
