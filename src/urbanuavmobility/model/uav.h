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
#ifndef UAV_H_
#define UAV_H_

#include "ns3/node.h"
#include "uav-mobility-model.h"
#include "ns3/network-module.h"
#include "uav-application.h"
#include <string>

using namespace std;

namespace ns3{

/**
 * \ingroup mobility
 * \brief UAV mobility model.
 *
 */

class Uav : public Node {

public:
     static TypeId GetTypeId (void);
	Uav();
     void setup(Ptr<Channel> channel,string ip);
     void startServer();
     void start();
     void launch();
     void stop();
     void setMonitoringMode();
     void setPatrollingMode();

	virtual ~Uav();
};

}

#endif /* UAV_H_ */
