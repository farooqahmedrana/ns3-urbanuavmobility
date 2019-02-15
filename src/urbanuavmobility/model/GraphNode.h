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
#ifndef GRAPHNODE_H_
#define GRAPHNODE_H_

#include <vector>
#include <string>
#include "ns3/vector.h"
#include "Util.h"

using namespace std;

namespace ns3{

class Visitor;
class GraphNode;
class Graph;

class SelectionStrategy {
public:
     virtual GraphNode* select(vector<GraphNode*>&) = 0;

};

class RandomSelection : public SelectionStrategy {
public:
     virtual GraphNode* select(vector<GraphNode*>&);
};

class LeastVisitedEdgesSelection : public SelectionStrategy {
private:
     string from;
     Graph* graph;
public:
     LeastVisitedEdgesSelection(string from,Graph* g);
     virtual GraphNode* select(vector<GraphNode*>&);
};

class GraphNode {
private:
	string id;
	double x;
	double y;
	vector<GraphNode*> nodes;
     SelectionStrategy* strategy;

public:
	GraphNode(string i,double x,double y,SelectionStrategy* s);
	void addNode(GraphNode* node);
	void print();
	GraphNode* next();
	void walk(int steps,int current, Visitor* visitor);	
     string getId();
	double getX();
	double getY();
	double distance(double x,double y);
     bool lineEdgesIntersect(Vector p1,Vector p2);
     bool pointExists(float x,float y);
     bool cellExists(Region& cell);
	virtual ~GraphNode();
};

}

#endif /* GRAPHNODE_H_ */
