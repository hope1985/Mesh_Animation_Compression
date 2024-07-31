#pragma once
#include "GInclude.h"
#include "SegmentInfo.h"
#include "Kmeans.h"
#include "SpatialSegmentation.h"


SpatialSegmentation::SpatialSegmentation(int numSegments, double alpha, VectorX<VectorX<Vector3d>> vertices, std::vector<int> clusterIndices, int numTemporalClusters, VectorX<VectorX<double>> curvatureVec, VectorX<VectorX<double>> torsionVec)
{
	this->torsionVec = torsionVec;
	this->curvatureVec = curvatureVec;
	this->alpha = alpha;
	this->numSegments = numSegments;
	this->vertices = vertices;
	this->clusterIndices = clusterIndices;
	this->numTemporalClusters = numTemporalClusters;
	//this->temporalClusterDataStdMatrix = temporalClusterMatrix;

	this->numVertices = (int)vertices[0].size();

	clusterFrameIndices = std::vector<std::vector<int>>(numTemporalClusters);
	for (int i = 0; i < numTemporalClusters; i++)
		clusterFrameIndices[i] = std::vector<int>();

	this->numFrames = (int)(this->clusterIndices.size()); //or this->vertices.size()
	for (int i = 0; i < numFrames; i++)
		clusterFrameIndices[clusterIndices[i]].push_back(i);

	spatialSegmentationData = vector <std::vector<std::vector<double>>>();

}
	
void SpatialSegmentation::makeSegmentStdVector()
{
	double alpha2 = (1 - alpha) / 2.0;


	for (int c = 0; c < clusterFrameIndices.size(); c++)
	{

		std::vector<int> frames = clusterFrameIndices[c];

		VectorX<Vector3d>  avgVertices(numVertices);
		for (int i = 0; i < numVertices; i++)
			avgVertices[i].setZero();


		VectorX<double>  avgVerticesX(numVertices);
		VectorX<double>  avgVerticesY(numVertices);
		VectorX<double>  avgVerticesZ(numVertices);
		for (int i = 0; i < numVertices; i++)
		{
			avgVerticesX.setZero();
			avgVerticesY.setZero();
			avgVerticesZ.setZero();
		}

		Matrix<double, Dynamic, Dynamic> clusteredFrames;
		clusteredFrames.resize(numVertices, frames.size() * 2);

		for (int f = 0; f < frames.size(); f++)
		{
			int frameIdx = frames[f];
			//segmentsFramesIdx.push_back(frameIdx);

			avgVertices += vertices[frameIdx];
			for (int v = 0; v < numVertices; v++)
			{

				clusteredFrames(v, 2 * f) = curvatureVec[f][v];
				clusteredFrames(v, 2 * f + 1) = torsionVec[f][v];
			}
		}

		//Matrix<double, Dynamic, Dynamic> clusteredFramesT = clusteredFrames.transpose();

		std::vector<std::vector<double>>  clusteredFramesVector(clusteredFrames.rows());
		for (int i = 0; i < clusteredFramesVector.size(); i++)
		{
			clusteredFramesVector[i] = vector<double>(clusteredFrames.cols() + 3);
			//Normalization
			for (int j = 0; j < clusteredFramesVector[i].size() - 3; j++)
			{
				double max = clusteredFrames.col(j).maxCoeff();
				double min = clusteredFrames.col(j).minCoeff();
				clusteredFramesVector[i][j + 3] = alpha2 * ((clusteredFrames(i, j) - min) / (min + max));
			}
		}

		for (int j = 0; j < numVertices; j++)
		{
			avgVertices[j] = avgVertices[j] / frames.size();
			avgVerticesX[j] = avgVertices[j].x();
			avgVerticesY[j] = avgVertices[j].y();
			avgVerticesZ[j] = avgVertices[j].z();
		}
		double maxX = avgVerticesX.maxCoeff();
		double maxY = avgVerticesY.maxCoeff();
		double maxZ = avgVerticesZ.maxCoeff();
		double minX = avgVerticesX.minCoeff();
		double minY = avgVerticesY.minCoeff();
		double minZ = avgVerticesZ.minCoeff();

		avgVerticesX = (avgVerticesX.array() - minX) / (minX + maxX);
		avgVerticesY = (avgVerticesY.array() - minY) / (minY + maxY);
		avgVerticesZ = (avgVerticesZ.array() - minZ) / (minZ + maxZ);


		for (int v = 0; v < clusteredFramesVector.size(); v++)
		{
			clusteredFramesVector[v][0] = alpha * avgVerticesX[v];
			clusteredFramesVector[v][1] = alpha * avgVerticesY[v];
			clusteredFramesVector[v][2] = alpha * avgVerticesZ[v];
		}
		spatialSegmentationData.push_back(clusteredFramesVector);
	}
}

void SpatialSegmentation::SpatialSegmentation::doSegmentation()
{

	seqments = std::vector<SegmentInfo>(spatialSegmentationData.size() * numSegments);
	KMeans km = KMeans(numSegments);
	for (int clusterId = 0; clusterId < spatialSegmentationData.size(); clusterId++)
	{
		km.fit(spatialSegmentationData[clusterId]);
		std::vector<int> verticesSeqmentIds = km.cluster(spatialSegmentationData[clusterId]);

		auto seqmentvertexIds = std::vector<std::vector<int>>(numSegments);
		for (int i = 0; i < numSegments; i++)
			seqmentvertexIds[i] = std::vector<int>();

		int numVertice = (int)verticesSeqmentIds.size(); //The len is the number of all vertices
		for (int i = 0; i < numVertice; i++)
			seqmentvertexIds[verticesSeqmentIds[i]].push_back(i);

		for (int segId = 0; segId < numSegments; segId++)
		{
			SegmentInfo segmentItem;
			segmentItem.segmentIdx = segId;
			segmentItem.frames = clusterFrameIndices[clusterId];
			segmentItem.clusterIdx = clusterId;
			segmentItem.vecIndices = seqmentvertexIds[segId];
			seqments[clusterId * numSegments + segId] = segmentItem;
		}
	}
}
