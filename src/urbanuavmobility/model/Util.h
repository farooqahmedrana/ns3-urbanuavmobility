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
#ifndef UTIL_H_
#define UTIL_H_

#include <vector>
#include "ns3/vector.h"

using namespace std;

namespace ns3{

/**
 * \ingroup mobility
 * \brief Utilities
 *
 */

class Region{
private:
	Vector topleft,bottomright;
	int color;
public:
	Region(float x1,float y1,float x2,float y2)
	:topleft(x1,y1,0),bottomright(x2,y2,0)
	{
		color = 0;
	}

	Vector getTopLeft(){
		return topleft;
	}

	Vector getTopRight(){
		return Vector(bottomright.x,topleft.y,0);
	}

	Vector getBottomLeft(){
		return Vector(topleft.x,bottomright.y,0);
	}

	Vector getBottomRight(){
		return bottomright;
	}

	void print(){
		cout << topleft.x << "," << topleft.y << "," << bottomright.x << "," << bottomright.y << endl;
	}

	void mark(int c){
		color = c;
	}

	int getColor(){
		return color;
	}

     bool hasPoint(Vector point){
          if (point.x >= topleft.x && point.y >= topleft.y &&
               point.x <= bottomright.x && point.y <= bottomright.y){

               return true;
          }
          return false;
     }

	vector<Region> decompose(float width,float length){
		vector<Region> cells;

		for(int i=topleft.x; i < bottomright.x; i+= width ){
			for(int j=topleft.y; j < bottomright.y; j+= length){
				cells.push_back(Region(i,j,i+width-1,j+length-1));
			}
		}

		return cells;
	}
};



class Util {

private:
     static bool onSegment(Vector p, Vector q, Vector r);
     static int orientation(Vector p, Vector q, Vector r);

public:
     static bool linesegmentsIntersect(Vector p1,Vector p2,Vector q1,Vector q2);
     static float min(vector<float> values);
     static float max(vector<float> values);

};

}

#endif /* UTIL_H_ */
