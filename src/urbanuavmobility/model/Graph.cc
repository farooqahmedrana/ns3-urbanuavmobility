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
#include "Graph.h"
#include <iostream>
#include <cstdlib>
#include <time.h>
#include <sstream>

using namespace std;

namespace ns3{


Graph::Graph() {
     rootNode = NULL;
	stepWalkNode = NULL;
     selectionStrategy = "random";
}

Graph::Graph(string strategy) {
     rootNode = NULL;
	stepWalkNode = NULL;
     selectionStrategy = strategy;
}

SelectionStrategy* Graph::getSelectionStrategy(string nodeId){
     srand(time(NULL));
     if(selectionStrategy.compare("leastvisited") == 0){
          return new LeastVisitedEdgesSelection(nodeId,this);
     }

     return new RandomSelection();
}

void Graph::load(char* file){
	xmlDocPtr doc;
	xmlNodePtr cur;

	doc = xmlParseFile(file);

	if (doc == NULL ) {
		cout << "Not parsed" << endl;
		return;
	}

	cur = xmlDocGetRootElement(doc);


	if (cur == NULL) {
		cout << "Empty document" << endl;
		xmlFreeDoc(doc);
		return;
	}



	if (xmlStrcmp(cur->name, (const xmlChar *) "graph") != 0) {
		cout << "document of the wrong type, root node != graph" << endl;
		xmlFreeDoc(doc);
		return;
	}
	else {
		xmlNodePtr node = cur->xmlChildrenNode;

		while (node != NULL){

			if (xmlStrcmp(node->name, (const xmlChar *) "nodes") == 0) {
				parseNodes(node);
			}

			if (xmlStrcmp(node->name, (const xmlChar *) "edges") == 0) {
				parseEdges(node);
			}

			node = node->next;
		}

		xmlFreeDoc(doc);
	}

}

void Graph::parseNodes(xmlNodePtr root){
	cout << "parsing nodes" << endl;
	xmlNodePtr node = root->xmlChildrenNode;
	while (node != NULL){
		if (xmlStrcmp(node->name, (const xmlChar *) "node") == 0) {
			parseNode(node);
		}
		node = node->next;
	}

     if (rootNode == NULL)
     	rootNode = nodes.begin()->second;
}

void Graph::parseEdges(xmlNodePtr root){
	cout << "parsing edges" << endl;
	xmlNodePtr edge = root->xmlChildrenNode;

	while (edge != NULL){
		if (xmlStrcmp(edge->name, (const xmlChar *) "edge") == 0) {
			parseEdge(edge);
		}
		edge = edge->next;
	}
}

void Graph::parseNode(xmlNodePtr node){
	string id ((const char*) xmlGetProp(node,(const xmlChar*) "id"));
	double x = atof((const char*) xmlGetProp(node,(const xmlChar*) "x"));
	double y = atof((const char*)  xmlGetProp(node,(const xmlChar*) "y"));

	//nodes[id] = new GraphNode(id,x,y,new RandomSelection());
     nodes[id] = new GraphNode(id,x,y,getSelectionStrategy(id));

     if (xmlGetProp(node,(const xmlChar*) "type") == NULL) return;

     string type ((const char*) xmlGetProp(node,(const xmlChar*) "type"));
     if ( type.compare("base") == 0 ){
          rootNode = nodes[id];
     }
}

void Graph::parseEdge(xmlNodePtr node){

	string from((const char*) xmlGetProp(node,(const xmlChar*) "from"));
	string to((const char*) xmlGetProp(node,(const xmlChar*) "to"));

	if(nodes.count(from) > 0 && nodes.count(to) > 0){
		nodes[from]->addNode(nodes[to]);
          edgesVisitCount[getEdgeId(from,to)] = 0;
	}

}

string Graph::getEdgeId(string from,string to){
     stringstream id;
     id << from << "->" << to;
     return id.str();
}

void Graph::print(){
	map<string,GraphNode*>::iterator iter;
	for(iter = nodes.begin(); iter != nodes.end(); iter++){
		iter->second->print();
	}
}

void Graph::walk(int steps,Visitor* visitor){
	rootNode->walk(steps,0,visitor);
}

void Graph::stepWalk(Visitor* visitor){
	if(stepWalkNode == NULL){
		stepWalkNode = rootNode;
	}
	else {
		stepWalkNode = stepWalkNode->next();
	}
	stepWalkNode->print();
	visitor->accept(stepWalkNode);
}

void Graph::setRoot(GraphNode* node){
	if (node != NULL){
		rootNode = node;
	}
}

GraphNode* Graph::getRoot(){
	return rootNode;
}

GraphNode* Graph::findNearest(double x,double y){
	GraphNode* node = rootNode;

	map<string,GraphNode*>::iterator iter;
	for(iter = nodes.begin(); iter != nodes.end(); iter++){
		if (iter->second->distance(x,y) < node->distance(x,y)){
			node = iter->second;
		}
	}

	return node;
}

void Graph::markEdge(string from,string to, double time){
     string edgeId = getEdgeId(from,to);
     if (edgesVisitCount.count(edgeId) != 0){
          edgesVisitCount[edgeId]++;
          edgesVisitTime[edgeId].push_back(time);     
     }

     string edgeIdReverse = getEdgeId(to,from);
     if (edgesVisitCount.count(edgeIdReverse) != 0){
          edgesVisitCount[edgeIdReverse]++;     
          edgesVisitTime[edgeId].push_back(time);
     }
}

int Graph::getEdgeVisitCount(string from,string to){
     return edgesVisitCount[getEdgeId(from,to)];
}

double Graph::getAverageIdleness(){
     double totalIdleTime = 0;     
     double totalVisits = 0;
     double lastIdleTime;

     for(map<string,vector<double> >::iterator iter = edgesVisitTime.begin(); iter != edgesVisitTime.end(); iter++){
          vector<double>& edgeVisitTimes = iter->second;

          lastIdleTime = 0;
          for(int i=0; i < (int) edgeVisitTimes.size(); i++){
               totalIdleTime += edgeVisitTimes[i] - lastIdleTime;
               lastIdleTime = edgeVisitTimes[i];
          }

          totalVisits += edgeVisitTimes.size();
     }

     if(totalVisits == 0) return 0;
     return totalIdleTime / totalVisits;
}

double Graph::getWorstIdleness(){
     double idleTime;
     double lastIdleTime;
     double worstIdleTime = 0;

     for(map<string,vector<double> >::iterator iter = edgesVisitTime.begin(); iter != edgesVisitTime.end(); iter++){
          vector<double>& edgeVisitTimes = iter->second;

          lastIdleTime = 0;
          for(int i=0; i < (int) edgeVisitTimes.size(); i++){
               idleTime = edgeVisitTimes[i] - lastIdleTime;

               if (idleTime > worstIdleTime){
                    worstIdleTime = idleTime;
               }

               lastIdleTime = edgeVisitTimes[i];
          }
     }

     return worstIdleTime;
}

string Graph::stats(){
     stringstream stats;

     int totalVisitCount = 0;
     int unvisitedEdges = 0;
     for(map<string,int>::iterator iter = edgesVisitCount.begin(); iter != edgesVisitCount.end(); iter++){
          totalVisitCount += iter->second;
          if (iter->second == 0){
               unvisitedEdges++;
          }
     }

     float visitsPerEdges = ((float) totalVisitCount) / edgesVisitCount.size();
     stats << "total visits:" << totalVisitCount << ";" ;
     stats << "visits per edges:" << visitsPerEdges << ";" ;
     stats << "unvisited edges:" << unvisitedEdges << ";" ;
     stats << "total edges:" << edgesVisitCount.size() << ";" ;
     stats << "coverage:" << ((float) (edgesVisitCount.size() - unvisitedEdges) / edgesVisitCount.size()) * 100 << ";";
     stats << "average idleness:" << getAverageIdleness() << ";";
     stats << "worst idleness:" << getWorstIdleness() << ";";
     return stats.str();
}

bool Graph::lineEdgesIntersect(Vector p1,Vector p2){
	for(map<string,GraphNode*>::iterator iter = nodes.begin(); iter != nodes.end(); iter++){
		GraphNode* node = iter->second;
          if (node->lineEdgesIntersect(p1,p2)){
               return true;
          }
	}

	return false;
}

bool Graph::pointExists(float x,float y){
	for(map<string,GraphNode*>::iterator iter = nodes.begin(); iter != nodes.end(); iter++){
		GraphNode* node = iter->second;
          if (node->pointExists(x,y)){
               return true;
          }
	}

	return false;
}

Region Graph::spanningArea(){
	vector<float> x;
	vector<float> y;
	for(map<string,GraphNode*>::iterator iter = nodes.begin(); iter != nodes.end(); iter++){
		x.push_back(iter->second->getX());
		y.push_back(iter->second->getY());
	}

	return Region(Util::min(x),Util::min(y),Util::max(x),Util::max(y));
}

vector<Region> Graph::decompose(float w,float l){
     vector<Region> markedCells;
	vector<Region> cells = spanningArea().decompose(w,l);

	for(int j=0; j < (int) cells.size(); j++){
		for(map<string,GraphNode*>::iterator iter = nodes.begin(); iter != nodes.end(); iter++){
               GraphNode* node = iter->second;
               if(cells[j].getColor() == 0 &&
                     node->cellExists(cells[j]) ) {

                    cells[j].mark(1);
//                    markedCells.push_back(cells[j]);
               }
		}
	}

//	return markedCells;
     return cells;
}



Graph::~Graph() {
	// TODO Auto-generated destructor stub
}

}
