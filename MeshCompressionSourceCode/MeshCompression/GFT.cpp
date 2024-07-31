#pragma once
#include <numeric> 
#include "GInclude.h"
#include "SegmentInfo.h"
#include "CommonVecOperation.h"
#include "GFT.h"

#include<iostream>
#include<string.h>
#include<fstream>
#include<sstream>
#include<vector>
#include <iomanip>
#include <ppl.h>

using namespace Concurrency;

GFT::GFT(VectorX<VectorX<Vector3d>> vertices, std::vector<std::vector<int>> faces, std::vector<SegmentInfo> segmentInfo, int numCluster, int numSeqments)
{
	this->numFrames = (int)vertices.size();
	this->numVertices = (int)vertices[0].size();
	this->vertices = vertices;
	this->faces = faces;
	this->segmentInfo = segmentInfo;
	this->numSeqments = numSeqments;
	this->numCluster = numCluster;
	this->segmentInfo = segmentInfo;
}

void GFT::makeSegmentEigenVetors()
{

	int numSegs = (int)segmentInfo.size();


	//for (int s = 0;s< numSegs;s++)
	parallel_for(int(0), numSegs, [&](int s)
		{

			auto segment = segmentInfo[s];
			int numVertices = (int)segment.vecIndices.size();
			int numFaces = (int)faces.size();

			Matrix<double, Dynamic, Dynamic> adjMat;
			adjMat.resize(numVertices, numVertices);
			adjMat.setConstant(0);

			for (int f = 0; f < numFaces; f++)
			{
				std::vector<int> face = faces[f];
				for (int i = 0; i < face.size(); i++) {

					int v1 = face[i];  // Vertex indices start from 1 in obj
					int v2 = face[(i + 1) % face.size()];

					int idxV1 = binarySearch(segment.vecIndices, 0, numVertices - 1, v1 - 1);
					int idxV2 = binarySearch(segment.vecIndices, 0, numVertices - 1, v2 - 1);
					if (idxV1 != -1 && idxV2 != -1)
					{
						adjMat(idxV1, idxV2) = 1;
						adjMat(idxV2, idxV1) = 1;
					}
				}
			}

			VectorX<double> adjSumRow(numVertices);

			for (int i = 0; i < numVertices; i++)
			{
				adjSumRow(i) = adjMat.row(i).sum();
			}
			//LaplacianMatrix =DegreeMatrix - AdjacencyMatrix
			Eigen::MatrixXd degMat = adjSumRow.asDiagonal();
			Eigen::MatrixXd lapMat = degMat - adjMat;
			segment.laplacianMatrix = (lapMat);

			segmentInfo[s].eigenVetors = Matrix<double, Dynamic, Dynamic>(computeEigenVectors(lapMat));

			std::cout << "Cluster:" << segment.clusterIdx << "   Segment:" << s << "   Number of vertices:" << numVertices << std::endl;
			//std::cout <<(segmentInfo[s].eigenVetors * segmentInfo[s].eigenVetors.transpose()).diagonal() << "\n";
		});

}
std::vector<std::vector<std::vector<double>>> GFT::getReconstrucedMesh(double compressionFactorPerecent)
{
#ifdef DEBUG_LOG
	//std::stringstream ss;
	//string filename = "eigen.txt";
	//std::ofstream ofile(filename.c_str());
	//if (!ofile.is_open())
	//{
	//	throw "ERROR in save_obj_file method: Could not careat file.";
	//}
#endif



	//Initialze compressed mesh frames 
	std::vector<std::vector<std::vector<double>>> compressedMeshFrames(numFrames);
	for (int f = 0; f < numFrames; f++)
	{
		compressedMeshFrames[f] = std::vector<std::vector<double>>(numVertices);
		for (int v = 0; v < numVertices; v++)
		{
			compressedMeshFrames[f][v] = std::vector<double>(3);
			compressedMeshFrames[f][v][0] = 0;
			compressedMeshFrames[f][v][1] = 0;
			compressedMeshFrames[f][v][2] = 0;
		}
	}

	//For each segment 
	for (int s = 0; s < segmentInfo.size(); s++)
	{
		auto segment = segmentInfo[s];
		int numSegVertices = (int)segment.vecIndices.size(); //frames of cluster which this segment  belongs to  
		auto frames = segmentInfo[s].frames;

		for (int fIdx = 0; fIdx < frames.size(); fIdx++)
		{
			int frameIdx = frames[fIdx];
			//Make separates vectors of  x, y, z values of vectors of the segment
			VectorXd x(numSegVertices);
			VectorXd y(numSegVertices);
			VectorXd z(numSegVertices);
			x.setZero();
			y.setZero();
			z.setZero();

			for (int v = 0; v < numSegVertices; v++)
			{
				int meshVIdx = segment.vecIndices[v];  //vertex index in the mesh
				x[v] = vertices[frameIdx][meshVIdx][0];
				y[v] = vertices[frameIdx][meshVIdx][1];
				z[v] = vertices[frameIdx][meshVIdx][2];
			}

			//Eigen::MatrixXd eigenVetors = segment.eigenVetors.transpose()*segment.eigenVetors;
			Eigen::MatrixXd eigenVetors = segment.eigenVetors;

			//Because for each segment we have different number of verttice it is better to use percentage of coefficient instead of numbers
			int k = int(numSegVertices * (compressionFactorPerecent / 100.0));

			k = k < numSegVertices ? k : numSegVertices;


			// Firts we compute GFT of vertices
			x = eigenVetors.transpose() * x;
			y = eigenVetors.transpose() * y;
			z = eigenVetors.transpose() * z;


			//Select coefficients by the magnitude of (x,y,z) coefficient vector 
			std::vector<double> mag(numSegVertices);
			for (int v = 0; v < numSegVertices; v++)
			{
				mag[v] = sqrtl(x[v] * x[v] + y[v] * y[v] + z[v] * z[v]);
			}
			std::vector<int> sortedIdx = sort_indexes<double>(mag);
			for (int b = 0; b < k; b++)
			{
				x[sortedIdx[b]] = 0;
				y[sortedIdx[b]] = 0;
				z[sortedIdx[b]] = 0;
			}

			/*Select coefficients based individual values of x,y,z
			std::vector<double> vx(numSegVertices);
			std::vector<double> vy(numSegVertices);
			std::vector<double> vz(numSegVertices);
			for (int v = 0; v < numSegVertices; v++)
			{
				vx[v] = x[v];
				vy[v] = y[v];
				vz[v] = z[v];
			}
			std::vector<int> sortedIdxX = sort_indexes<double>(vx);
			std::vector<int> sortedIdxY = sort_indexes<double>(vy);
			std::vector<int> sortedIdxZ = sort_indexes<double>(vz);
			std::cout << "k=\n" << k ;

			for (int b = 0; b < k; b++)
			{
				x[sortedIdxX[b]] = 0;
				y[sortedIdxY[b]] = 0;
				z[sortedIdxZ[b]] = 0;
			}*/

			//Finally do an inverse GFT to get the compressed singals
			x = eigenVetors * x;
			y = eigenVetors * y;
			z = eigenVetors * z;


#ifdef DEBUG_LOG
			/*ss << "#SEG=" << s << std::endl;
			VectorXd dig(eigenVetors.diagonal());
			for (int u1 = 0; u1 < numSegVertices; u1++)
			{
				ss << dig(u1) << ",";
			}
			ss << std::endl;*/
			//std::cout<<"seg=\n" << s << "eigenVetorsI\n" << eigenVetors.diagonal();
#endif 

			//Store reconstructed vertices 

			for (int v = 0; v < numSegVertices; v++)
			{
				int meshVIdx = segment.vecIndices[v];  //vertex index in the mesh
				compressedMeshFrames[frameIdx][meshVIdx][0] = x[v];
				compressedMeshFrames[frameIdx][meshVIdx][1] = y[v];
				compressedMeshFrames[frameIdx][meshVIdx][2] = z[v];
			}

		}
	}

#ifdef  DEBUG_LOG
	//ofile << ss.str();
	//ofile.close();
#endif 

	return compressedMeshFrames;
}
Matrix<double, Dynamic, Dynamic> GFT::computeEigenVectors(Matrix<double, Dynamic, Dynamic> matrix)
{
	Eigen::EigenSolver<Eigen::MatrixXd> solver(matrix);

	// Get the eigenvalues
	Eigen::VectorXd eigenvalues = solver.eigenvalues().real();
	// Get the eigenvectors
	Eigen::MatrixXd eigenvectors = solver.eigenvectors().real();

	// Sort eigenvectors based on the magnitude of eigenvalues
	std::vector<int> indices(eigenvalues.size());
	std::iota(indices.begin(), indices.end(), 0);  // Initialize indices

	std::sort(indices.begin(), indices.end(), [&eigenvalues](int i, int j) {
		return std::abs(eigenvalues[i]) > std::abs(eigenvalues[j]);
		});

	// Apply the sorting to eigenvectors
	Eigen::MatrixXd sorted_eigenvectors = eigenvectors.col(indices[0]);
	for (int i = 1; i < eigenvectors.cols(); ++i) {
		sorted_eigenvectors.conservativeResize(Eigen::NoChange, sorted_eigenvectors.cols() + 1);
		sorted_eigenvectors.col(i) = eigenvectors.col(indices[i]);
	}

	return sorted_eigenvectors;
}
