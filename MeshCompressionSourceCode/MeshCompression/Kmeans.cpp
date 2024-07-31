#pragma once
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>    // std::random_shuffle
#include "Kmeans.h"


/**
 * Constructs a KMeans object with the specified number of clusters.
 *
 * @param numClusters The number of clusters.
 */
KMeans::KMeans(int numClusters) {
	k = numClusters;
}

/**
 * Fits the K-Means model to the given data.
 *
 * @param data The data points to be clustered.
 */
void KMeans::fit(const std::vector<std::vector<double>>& data) {
	// Initialize centroids randomly
	initializeCentroids(data);

	// Assign data points to clusters
	std::vector<int> assignments(data.size());
	bool converged = false;

	while (!converged) {
		// Assign data points to the nearest centroid
		assignDataPoints(data, assignments);

		// Update centroids
		converged = updateCentroids(data, assignments);
	}
}

/**
 * Predicts the cluster for a given data point.
 *
 * @param point The data point to be predicted.
 * @return int The cluster index that the data point belongs to.
 */
int KMeans::predict(const std::vector<double>& point) {
	int clusterIndex = 0;
	double minDistance = distance(point, centroids[0]);

	for (int i = 1; i < k; i++) {
		double dist = distance(point, centroids[i]);
		if (dist < minDistance) {
			minDistance = dist;
			clusterIndex = i;
		}
	}

	return clusterIndex;
}

std::vector<int> KMeans::cluster(std::vector<std::vector<double>> data) {
	std::vector<int> clusters;
	for (const auto& point : data) {
		int clusterIndex = this->predict(point);
		clusters.push_back(clusterIndex);
	}
	return clusters;
}


/**
 * Initializes the centroids randomly from the given data.
 *
 * @param data The data points to initialize centroids from.
 */
void KMeans::initializeCentroids(const std::vector<std::vector<double>>& data) {
	centroids.clear();

	// Randomly select k data points as initial centroids
	std::vector<int> indices(data.size());
	for (int i = 0; i < data.size(); i++) {
		indices[i] = i;
	}

	std::random_shuffle(indices.begin(), indices.end());

	for (int i = 0; i < k; i++) {
		centroids.push_back(data[indices[i]]);
	}
}

/**
 * Assigns data points to the nearest centroid.
 *
 * @param data The data points to be assigned.
 * @param assignments The vector to store the assigned cluster indices.
 */
void KMeans::assignDataPoints(const std::vector<std::vector<double>>& data, std::vector<int>& assignments) {
	for (int i = 0; i < data.size(); i++) {
		int clusterIndex = predict(data[i]);
		assignments[i] = clusterIndex;
	}
}

/**
 * Updates the centroids based on the assigned data points.
 *
 * @param data The data points.
 * @param assignments The assigned cluster indices.
 * @return bool True if the centroids have converged, False otherwise.
 */
bool KMeans::updateCentroids(const std::vector<std::vector<double>>& data, const std::vector<int>& assignments) {
	std::vector<std::vector<double>> newCentroids(k, std::vector<double>(data[0].size(), 0.0));
	std::vector<int> clusterSizes(k, 0);

	// Calculate the sum of data points for each cluster
	for (int i = 0; i < data.size(); i++) {
		int clusterIndex = assignments[i];
		for (int j = 0; j < data[i].size(); j++) {
			newCentroids[clusterIndex][j] += data[i][j];
		}
		clusterSizes[clusterIndex]++;
	}

	// Divide the sum by the cluster size to get the new centroids
	bool converged = true;
	for (int i = 0; i < k; i++) {
		if (clusterSizes[i] > 0) {
			for (int j = 0; j < newCentroids[i].size(); j++) {
				newCentroids[i][j] /= clusterSizes[i];
			}
			// Check if the centroids have converged
			if (!hasConverged(centroids[i], newCentroids[i])) {
				converged = false;
			}
		}
	}

	centroids = newCentroids;
	return converged;
}

/**
 * Calculates the Euclidean distance between two points.
 *
 * @param point1 The first point.
 * @param point2 The second point.
 * @return double The Euclidean distance between the two points.
 */
double KMeans::distance(const std::vector<double>& point1, const std::vector<double>& point2) {
	double sum = 0.0;
	for (int i = 0; i < point1.size(); i++) {
		sum += pow(point1[i] - point2[i], 2);
	}
	return sqrt(sum);
}

/**
 * Checks if two points have converged.
 *
 * @param point1 The first point.
 * @param point2 The second point.
 * @return bool True if the points have converged, False otherwise.
 */
bool KMeans::hasConverged(const std::vector<double>& point1, const std::vector<double>& point2) {
	for (int i = 0; i < point1.size(); i++) {
		if (fabs(point1[i] - point2[i]) > 1e-6) {
			return false;
		}
	}
	return true;
}



//int main() {
//    // Example usage of KMeans class
//    {
//        std::vector<std::vector<double>> data = {
//            {2.0, 3.0},
//            {5.0, 6.0},
//            {1.0, 4.0},
//            {3.0, 1.0},
//            {6.0, 2.0},
//            {7.0, 5.0}
//        };
//
//        KMeans kmeans(2);
//        kmeans.fit(data);
//
//        std::cout << "Cluster assignments:\n";
//        for (const auto& point : data) {
//            int clusterIndex = kmeans.predict(point);
//            std::cout << "(" << point[0] << ", " << point[1] << ") -> Cluster " << clusterIndex << "\n";
//        }
//    }
//
//    return 0;
//}