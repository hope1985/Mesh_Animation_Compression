#include "GInclude.h"
#include "Kmeans.h"
#include "TemporalCluster.h"



TemporalCluster::TemporalCluster(VectorX<VectorX<double>> curvatureVec, VectorX<VectorX<double>> torsionVec, int numCluster)
{
	this->curvatureVec = curvatureVec;
	this->torsionVec = torsionVec;
	this->numCluster = numCluster;
}

void TemporalCluster::makeClusterMatrix()
{
	int numFrame = (int)curvatureVec.size();
	int numVec = (int)curvatureVec[0].size();
	temporalClusterDataMatrix = Matrix<double, Dynamic, Dynamic>(numFrame, numVec * 2);
	for (int i = 0; i < numFrame; i++)
	{
		for (int j = 0; j < numVec; j++)
		{
			temporalClusterDataMatrix(i, 2 * j) = curvatureVec[i][j];
			temporalClusterDataMatrix(i, 2 * j + 1) = torsionVec[i][j];
		}
	}

}
void TemporalCluster::makeClustersStdVector()
{
	int numFrame = (int)curvatureVec.size();
	int numVec = (int)curvatureVec[0].size();
	temporalClusterDataStdVector = std::vector<std::vector<double>>(numFrame);

	for (int i = 0; i < numFrame; i++)
	{
		temporalClusterDataStdVector[i] = std::vector<double>(numVec * 2);
		for (int j = 0; j < numVec; j++)
		{
			temporalClusterDataStdVector[i][2 * j] = curvatureVec[i][j];
			temporalClusterDataStdVector[i][2 * j + 1] = torsionVec[i][j];
		}
	}
}

void TemporalCluster::makeClusterData()
{
	makeClusterMatrix();
	makeClustersStdVector();
}

void TemporalCluster::clustring()
{

	auto kmeans = KMeans(numCluster);
	kmeans.fit(temporalClusterDataStdVector);
	clusterIndices = kmeans.cluster(temporalClusterDataStdVector);
}

