#pragma once
#include "GInclude.h"
#include "SegmentInfo.h"
#include "Kmeans.h"

class KGerror
{
private:

	
	double numarator = 0;     
	
	// ||V_org-E(V_org)|| which  V_org and  are 3NxF Matrix (N=Num of vertices, F=Number of frames) 
	// E(V_org) is average value of vertices for each frames (in fact all elements of each columns in the E(V_org) have the same values)
	// Therefore it is a fixed value that can be computed once
	double denominator = 0;   

	std::vector<std::vector<std::vector<double>>> originalVertices;
	int numFrames;
	int numVertices ;

	void computeDenominator();

public:

	KGerror();

	void SetOriginalVertices(std::vector<std::vector<std::vector<double>>> originalVertices);

	//|| V_org-V_rec||  V_org and V_rec are 3NxF Matrix (N=Num of vertices, F=Number of frames)   
	double ComputeKG_error(std::vector < std::vector<std::vector<double>>> recVertices);



};

