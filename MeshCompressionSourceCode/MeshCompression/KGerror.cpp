#pragma once
#include "GInclude.h"
#include "KGerror.h"



KGerror::KGerror()
{

}

void KGerror::SetOriginalVertices(std::vector<std::vector<std::vector<double>>> originalVertices)
{
	this->originalVertices = originalVertices;
	this->numFrames = (int)originalVertices.size();
	this->numVertices = (int)originalVertices[0].size();
	computeDenominator();
}

void KGerror::computeDenominator()
{
	//reset denominator value
	denominator = 0;

	std::vector<double> avgVerticesPerframe(numFrames * 3);

	//Compute average values of vertices for each frame
	for (int f = 0; f < numFrames; f++)
	{
		for (int v = 0; v < numVertices; v++)
		{
			avgVerticesPerframe[3 * f] += originalVertices[f][v][0];
			avgVerticesPerframe[3 * f + 1] += originalVertices[f][v][1];
			avgVerticesPerframe[3 * f + 2] += originalVertices[f][v][2];
		}
		avgVerticesPerframe[3 * f] = avgVerticesPerframe[3 * f] / numVertices;
		avgVerticesPerframe[3 * f + 1] = avgVerticesPerframe[3 * f + 1] / numVertices;
		avgVerticesPerframe[3 * f + 2] = avgVerticesPerframe[3 * f + 2] / numVertices;
	}

	for (int f = 0; f < numFrames; f++)
	{
		//Average value of vertices for each frame
		double avgX = avgVerticesPerframe[3 * f];
		double avgY = avgVerticesPerframe[3 * f + 1];
		double avgZ = avgVerticesPerframe[3 * f + 2];

		for (int v = 0; v < numVertices; v++)
		{
			denominator += powl((originalVertices[f][v][0] - avgX), 2);
			denominator += powl((originalVertices[f][v][1] - avgY), 2);
			denominator += powl((originalVertices[f][v][2] - avgZ), 2);
		}
	}

	denominator = sqrt(denominator);
}


//|| V_org-V_rec||  V_org and V_rec are 3NxF Matrix (N=Num of vertices, F=Number of frames)   
double KGerror::ComputeKG_error(std::vector < std::vector<std::vector<double>>> recVertices)
{
	double numarator = 0;

	for (int f = 0; f < numFrames; f++)
	{
		for (int v = 0; v < numVertices; v++)
		{
			numarator += powl((originalVertices[f][v][0] - recVertices[f][v][0]), 2);
			numarator += powl((originalVertices[f][v][1] - recVertices[f][v][1]), 2);
			numarator += powl((originalVertices[f][v][2] - recVertices[f][v][2]), 2);
		}
	}

	double kg_error = 100 * (sqrt(numarator) / denominator);
	return kg_error;
}



