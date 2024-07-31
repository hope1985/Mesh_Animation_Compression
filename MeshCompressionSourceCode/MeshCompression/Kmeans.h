#pragma once
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>    // std::random_shuffle

/**
 * @class KMeans
 * Implements the K-Means clustering algorithm.
 */
class KMeans {
private:
    int k; // Number of clusters
    std::vector<std::vector<double>> centroids; // Centroids of the clusters

public:
    /**
     * Constructs a KMeans object with the specified number of clusters.
     *
     * @param numClusters The number of clusters.
     */
    KMeans(int numClusters);

    /**
     * Fits the K-Means model to the given data.
     *
     * @param data The data points to be clustered.
     */
    void fit(const std::vector<std::vector<double>>& data);

    /**
     * Predicts the cluster for a given data point.
     *
     * @param point The data point to be predicted.
     * @return int The cluster index that the data point belongs to.
     */
    int predict(const std::vector<double>& point);

    std::vector<int> cluster(std::vector<std::vector<double>> data);

private:
    /**
     * Initializes the centroids randomly from the given data.
     *
     * @param data The data points to initialize centroids from.
     */
    void initializeCentroids(const std::vector<std::vector<double>>& data);

    /**
     * Assigns data points to the nearest centroid.
     *
     * @param data The data points to be assigned.
     * @param assignments The vector to store the assigned cluster indices.
     */
    void assignDataPoints(const std::vector<std::vector<double>>& data, std::vector<int>& assignments);

    /**
     * Updates the centroids based on the assigned data points.
     *
     * @param data The data points.
     * @param assignments The assigned cluster indices.
     * @return bool True if the centroids have converged, False otherwise.
     */
    bool updateCentroids(const std::vector<std::vector<double>>& data, const std::vector<int>& assignments);
    /**
     * Calculates the Euclidean distance between two points.
     *
     * @param point1 The first point.
     * @param point2 The second point.
     * @return double The Euclidean distance between the two points.
     */
    double distance(const std::vector<double>& point1, const std::vector<double>& point2);

    /**
     * Checks if two points have converged.
     *
     * @param point1 The first point.
     * @param point2 The second point.
     * @return bool True if the points have converged, False otherwise.
     */
    bool hasConverged(const std::vector<double>& point1, const std::vector<double>& point2);
};

