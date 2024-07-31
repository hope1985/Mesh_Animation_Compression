#include "GInclude.h"
#include "Kmeans.h"

class TemporalCluster
{
public:

	Matrix<double, Dynamic, Dynamic> temporalClusterDataMatrix;
	std::vector<std::vector<double>> temporalClusterDataStdVector;
	VectorX<VectorX<double>> torsionVec;
	VectorX<VectorX<double>> curvatureVec;
	int numCluster ;
	std::vector<int> clusterIndices;

	TemporalCluster(VectorX<VectorX<double>> curvatureVec, VectorX<VectorX<double>> torsionVec, int numCluster);
	void makeClusterMatrix();
	void makeClustersStdVector();
	void makeClusterData();
	void clustring();
};