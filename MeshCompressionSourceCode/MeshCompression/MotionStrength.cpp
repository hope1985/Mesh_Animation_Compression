#include "GInclude.h"
#include "CommonVecOperation.h"
#include "MotionStrength.h"


MotionStrength::MotionStrength(VectorX<VectorX<Vector3d>> vertices)
{
	this->vertices = vertices;
}
void MotionStrength::computeDerivations()
{

	vecD1 = computeDerivation(vertices);
	vecD2 = computeDerivation(vecD1);
	vecD3 = computeDerivation(vecD2);
}

VectorX<VectorX<double>> MotionStrength::computeTorsionVector()
{
	int numFrame = int(vecD1.size());
	int numVec = int(vecD1[0].size());
	VectorX<VectorX<double>> matTorsion(numFrame);


	for (int i = 0; i < numFrame; i++)
	{
		matTorsion[i] = VectorX<double>(numVec);
		for (int j = 0; j < numVec; j++)
		{
			matTorsion[i][j] = computeTorsionVec3d(vecD1[i][j], vecD2[i][j], vecD3[i][j]);
		}
	}
	return matTorsion;
}
VectorX<VectorX<double>>  MotionStrength::computeCurvatureVector()
{
	int numFrame = int(vecD1.size());
	int numVec = int(vecD1[0].size());
	VectorX<VectorX<double>> matCurvature(numFrame);

	for (int i = 0; i < numFrame; i++)
	{
		matCurvature[i]= VectorX<double>(numVec);
		for (int j = 0; j < numVec; j++)
		{
			matCurvature[i][j] = computeCurvatureVec3d(vecD1[i][j], vecD2[i][j]);
		}
	}

	return matCurvature;
}

void MotionStrength::computeTorsions()
{
	torsionVec = computeTorsionVector();
}
void MotionStrength::computeCurvatures()
{
	curvatureVec = computeCurvatureVector();
}



