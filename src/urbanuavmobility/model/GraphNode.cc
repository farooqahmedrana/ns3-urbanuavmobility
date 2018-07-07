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
#include "GraphNode.h"
#include "Graph.h"
#include "Visitor.h"
#include <iostream>
#include <cstdlib>
#include <cmath>
#include <sstream>

using namespace std;

namespace ns3{


GraphNode* RandomSelection::select(vector<GraphNode*>& nodes){
	int randomNumber = rand() % (int) nodes.size();
	return nodes[randomNumber];
}

LeastVisitedEdgesSelection::LeastVisitedEdgesSelection(string f,Graph* g){
     from = f;
     graph = g;
}

GraphNode* LeastVisitedEdgesSelection::select(vector<GraphNode*>& nodes){
     vector<GraphNode*> equalNodes;
     equalNodes.push_back(nodes[0]);

     for (int i=1; i < (int) nodes.size(); i++){
          if (graph->getEdgeVisitCount(from,nodes[i]->getId()) < 
               graph->getEdgeVisitCount(from,equalNodes[0]->getId()) ){
               equalNodes.clear();
               equalNodes.push_back(nodes[i]);
          }
          else if (graph->getEdgeVisitCount(from,nodes[i]->getId()) ==
               graph->getEdgeVisitCount(from,equalNodes[0]->getId()) ){
               equalNodes.push_back(nodes[i]);          
          }
     }

     int randomNumber = rand() % (int) equalNodes.size();
	return equalNodes[randomNumber];
}

GraphNode::GraphNode(string i,double x,double y,SelectionStrategy* s) {
	this->id = i;
	this->x = x;
	this->y = y;
     strategy = s;
}

void GraphNode::print(){
	cout << id << ":" << x <<	"," << y << ":" << nodes.size() << endl;
}

void GraphNode::addNode(GraphNode* node){
	nodes.push_back(node);

}

void GraphNode::walk(int steps,int current,Visitor* visitor){

	if(current == steps){
		return;
	}

	visitor->accept(this);
	current++;
	next()->walk(steps,current,visitor);
}

GraphNode* GraphNode::next(){
     if(nodes.size() == 0){
		return this;
	}

     return strategy->select(nodes);
}

string GraphNode::getId(){
	return id;
}

double GraphNode::getX(){
	return x;
}

double GraphNode::getY(){
	return y;
}

double GraphNode::distance(double x,double y){
	double xd = this->x - x;
	double yd = this->y - y;
	return sqrt(xd*xd + yd*yd);
}

GraphNode::~GraphNode() {
     delete strategy;
}

}
