#pragma once
#include "GInclude.h"
class SegmentInfo
{
public:
	int clusterIdx;
	int segmentIdx;
	std::vector<int> frames;
	std::vector<int> vecIndices;
	Matrix<double, Dynamic, Dynamic> laplacianMatrix;
	Matrix<double, Dynamic, Dynamic> eigenVetors;
	SegmentInfo()
	{
	}
};