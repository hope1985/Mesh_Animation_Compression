#pragma once
#include "GInclude.h"
#include "SegmentInfo.h"


class SpatialSegmentation
{
	public:
		std::vector <std::vector<std::vector<double>>> spatialSegmentationData;
		double alpha;
		std::vector<int> clusterIndices;  //The cluster indices for each frame
		std::vector<std::vector<int>> clusterFrameIndices; //The list of frame indices for each cluster
		std::vector<SegmentInfo> seqments;
		VectorX<VectorX<Vector3d>> vertices;
		int numTemporalClusters;
		int numVertices;
		int numSegments;
		int numFrames;
		VectorX<VectorX<double>> torsionVec;
		VectorX<VectorX<double>> curvatureVec;

		SpatialSegmentation(int numSegments, double alpha, VectorX<VectorX<Vector3d>> vertices, 
			std::vector<int> clusterIndices, int numTemporalClusters, VectorX<VectorX<double>> curvatureVec, 
			VectorX<VectorX<double>> torsionVec);

		void makeSegmentStdVector();
		void doSegmentation();

};

