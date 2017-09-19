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
#include "Graph.h"
#include <iostream>
#include <cstdlib>
#include <sstream>

using namespace std;

namespace ns3{

Graph::Graph() {
	stepWalkNode = NULL;
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

	nodes[id] = new GraphNode(id,x,y);
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

void Graph::markEdge(string from,string to){
     string edgeId = getEdgeId(from,to);
     if (edgesVisitCount.count(edgeId) != 0){
          edgesVisitCount[edgeId]++;     
     }

     string edgeIdReverse = getEdgeId(to,from);
     if (edgesVisitCount.count(edgeIdReverse) != 0){
          edgesVisitCount[edgeIdReverse]++;     
     }
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
     return stats.str();
}

Graph::~Graph() {
	// TODO Auto-generated destructor stub
}

}
