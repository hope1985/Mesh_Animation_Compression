#pragma once 
#include "GInclude.h"

#include <numeric>      // std::iota
#include <algorithm>    // std::sort, std::stable_sort




static inline double sqrtVec3d(Vector3d v)
{
	return sqrtl((v[0] * v[0]) + (v[1] * v[1]) + (v[2] * v[2]));
}

static double computeCurvatureVec3d(Vector3d vD1, Vector3d vD2)
{
	if (vD1.isZero())
		return 0.0;
	else
	{
		Vector3d d1xd2 = vD1.cross(vD2);
		double numerator = sqrtVec3d(d1xd2);
		double denominator = powl(sqrtVec3d(vD1), 3);
		return numerator / denominator;
	}
}

static double  computeTorsionVec3d(Vector3d vD1, Vector3d vD2, Vector3d vD3)
{
	Vector3d d1xd2 = vD1.cross(vD2);
	if (d1xd2.isZero())
		return 0.0;
	else
	{
		double numerator = d1xd2.dot(vD3);
		double denominator = powl(sqrtVec3d(d1xd2), 2);
		return numerator / denominator;
	}
}


static VectorX<VectorX<Vector3d>> computeDerivation(VectorX<VectorX<Vector3d>> vecIn)
{
	int numFrame = int(vecIn.size());
	int numVec = int(vecIn[0].size());
	VectorX<VectorX<Vector3d>> vecDx(numFrame);
	for (int i = 0; i < numFrame; i++)
	{
		vecDx[i]= VectorX<Vector3d>(numVec);
		//for (int j = 0; j < numVec; j++)
		//	vecDx[i][j].setZero();
	}

	for (int i = 0; i < numFrame - 1; i++)
	{
		for (int j = 0; j < numVec; j++)
		{
			vecDx[i][j] = vecIn[i + 1][j] - vecIn[i][j];
		}
	}

	if (numFrame > 2)
	{
		for (int j = 0; j < numVec; j++)
		{
			vecDx[numFrame - 1][j] = (vecDx[numFrame - 2][j] + vecDx[numFrame - 3][j]) / 2.0;
		}
	}
	return vecDx;
}




// An iterative binary search function.
static int binarySearch(std::vector<int> data, int l, int r, int x)
{
	while (l <= r) {
		int m = l + (r - l) / 2;

		// Check if x is present at mid
		if (data[m] == x)
			return m;

		// If x greater, ignore left half
		if (data[m] < x)
			l = m + 1;

		// If x is smaller, ignore right half
		else
			r = m - 1;
	}

	// If we reach here, then element was not present
	return -1;
}


template <typename T>
static vector<int> sort_indexes(const vector<T>& v) {

	// initialize original index locations
	vector<int> idx(v.size());
	iota(idx.begin(), idx.end(), 0);

	// sort indexes based on comparing values in v
	// using std::stable_sort instead of std::sort
	// to avoid unnecessary index re-orderings
	// when v contains elements of equal values 
	stable_sort(idx.begin(), idx.end(),
		[&v](size_t i1, size_t i2) {return v[i1] < v[i2]; });

	return idx;
}
