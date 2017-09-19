/*
 * uav.h
 *
 *  Created on: Jul 19, 2016
 *      Author: farooq
 */

#ifndef UAV_CONTAINER_H_
#define UAV_CONTAINER_H_

#include "ns3/node-container.h"
#include "ns3/uav.h"


using namespace std;

namespace ns3{

/**
 * \ingroup mobility
 * \brief UAV mobility model.
 *
 */

class UavContainer {

public:
	UavContainer();
     void Create (uint32_t n);
     Ptr<Uav> Get (uint32_t i) const;
	virtual ~UavContainer();

private:
  std::vector<Ptr<Uav> > m_nodes;
};

}

#endif /* UAV_CONTAINER_H_ */
