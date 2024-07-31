#pragma once
#include <numeric> 
#include "GInclude.h"
#include "SegmentInfo.h"
#include<string.h>
#include<vector>


class GFT
{
public:
	int numCluster;
	int numSeqments;
	int numVertices;
	int numFrames;
	std::vector<std::vector<int>> faces;
	std::vector<SegmentInfo> segmentInfo;
	
	VectorX<VectorX<Vector3d>> vertices;

	GFT(VectorX<VectorX<Vector3d>> vertices, std::vector<std::vector<int>> faces, std::vector<SegmentInfo> segmentInfo, int numCluster, int numSeqments);
	void makeSegmentEigenVetors();
	std::vector<std::vector<std::vector<double>>> getReconstrucedMesh(double compressionFactorPerecent);
private:
	Matrix<double, Dynamic, Dynamic> computeEigenVectors(Matrix<double, Dynamic, Dynamic> matrix);
};