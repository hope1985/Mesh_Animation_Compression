#include "GInclude.h"

class MotionStrength
{
public:
	VectorX<VectorX<double>> torsionVec;
	VectorX<VectorX<double>> curvatureVec;

	VectorX<VectorX<Vector3d>> vertices;
	VectorX<VectorX<Vector3d>> vecD1;
	VectorX<VectorX<Vector3d>> vecD2;
	VectorX<VectorX<Vector3d>> vecD3;


	MotionStrength(VectorX<VectorX<Vector3d>> vertices);

	void computeDerivations();
	void computeTorsions();
	void computeCurvatures();
private:

	VectorX<VectorX<double>> computeTorsionVector();
	VectorX<VectorX<double>> computeCurvatureVector();

};

