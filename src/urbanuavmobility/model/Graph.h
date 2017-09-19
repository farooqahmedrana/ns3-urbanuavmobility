/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2017 Computer Science Department, FAST-NU, Lahore.
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
#ifndef GRAPH_H_
#define GRAPH_H_

#include "GraphNode.h"
#include "Visitor.h"
#include <libxml/parser.h>
#include <map>
#include <string>

using namespace std;

namespace ns3{

class Graph {
private:
	map<string,GraphNode*> nodes;
     map<string,int> edgesVisitCount;
	GraphNode* rootNode;

	void parseNodes(xmlNodePtr);
	void parseEdges(xmlNodePtr);
	void parseNode(xmlNodePtr);
	void parseEdge(xmlNodePtr);

     string getEdgeId(string from,string to);

	GraphNode* stepWalkNode;

public:
	Graph();
	void load(char* file);
	void print();
	void walk(int steps, Visitor* visitor);
	void stepWalk(Visitor* visitor);
	void setRoot(GraphNode* node);
	GraphNode* getRoot();
	GraphNode* findNearest(double x,double y);
     void markEdge(string from,string to);

     string stats();
	virtual ~Graph();
};

}

#endif /* GRAPH_H_ */

