//
// Created by Jack Barrett on 2024-08-15.
//

#include <iostream>
#include <fstream>
#include <sstream>
#include <random>

typedef double f64;

#define EARTH_RADIUS 6372.8
#define NUM_CLUSTERS 10

static f64 Square(f64 A) {
    f64 Result = (A*A);
    return Result;
}

static f64 RadiansFromDegrees(f64 Degrees) {
    f64 Result = 0.01745329251994329577f * Degrees;
    return Result;
}

// NOTE(casey): EarthRadius is generally expected to be 6372.8
static f64 ReferenceHaversine(f64 X0, f64 Y0, f64 X1, f64 Y1) {
    /* NOTE(casey): This is not meant to be a "good" way to calculate the Haversine distance.
       Instead, it attempts to follow, as closely as possible, the formula used in the real-world
       question on which these homework exercises are loosely based.
    */

    f64 lat1 = Y0;
    f64 lat2 = Y1;
    f64 lon1 = X0;
    f64 lon2 = X1;

    f64 dLat = RadiansFromDegrees(lat2 - lat1);
    f64 dLon = RadiansFromDegrees(lon2 - lon1);
    lat1 = RadiansFromDegrees(lat1);
    lat2 = RadiansFromDegrees(lat2);

    f64 a = Square(sin(dLat/2.0)) + cos(lat1)*cos(lat2)*Square(sin(dLon/2));
    f64 c = 2.0*asin(sqrt(a));

    f64 Result = EARTH_RADIUS * c;

    return Result;
}

double uniform_point_distribution(int seed, int num_points) {
    std::stringstream filename;
    filename << "uniform-haversine-" << seed << "-" << num_points;
    std::ofstream outf{filename.str() + ".json"};

    outf << "{\"pairs\":[\n";

    std::ofstream distances(filename.str() + ".f64", std::ios::binary);
    double sum_of_distances {0};

    std::mt19937 gen(seed);
    std::uniform_real_distribution<> xs(-180, 180);
    std::uniform_real_distribution<> ys(-90, 90);

    for (int i {1}; i <= num_points; i++) {
        double x0 {xs(gen)};
        double x1 {xs(gen)};
        double y0 {ys(gen)};
        double y1 {ys(gen)};

        double distance = ReferenceHaversine(x0, y0, x1, y1);
        sum_of_distances += distance;
        distances.write((char*) &distance, sizeof(double));
        outf << "{\"x0\": " << x0 << ", \"y0\": " << y0 << ",\n";
        outf << "\"x1\": " << x1 << ", \"y1\": " << y1 << "}";
        if (i < num_points) {
            outf << ",\n";
        } else {
            outf << "\n";
        }
    }

    outf << "]}\n";
    return sum_of_distances / num_points;
}

void update_cluster_distributions(std::uniform_real_distribution<> xs, std::uniform_real_distribution<> ys,
                                  std::uniform_real_distribution<>& cluster_xs,
                                  std::uniform_real_distribution<>& cluster_ys,
                                  std::mt19937 gen) {
    double xd1 {xs(gen)};
    double xd2 {xs(gen)};
    double yd1 {ys(gen)};
    double yd2 {ys(gen)};
    cluster_xs = std::uniform_real_distribution<> (std::min(xd1, xd2), std::max(xd1, xd2));
    cluster_ys = std::uniform_real_distribution<> (std::min(yd1, yd2), std::max(yd1, yd2));
}

double clustered_point_distribution(int seed, int num_points) {
    std::stringstream filename;
    filename << "clustered-haversine-" << seed << "-" << num_points;
    std::ofstream outf{filename.str() + ".json"};

    outf << "{\"pairs\":[\n";

    std::ofstream distances(filename.str() + ".f64", std::ios::binary);
    double sum_of_distances {0};

    std::mt19937 gen(seed);
    std::uniform_real_distribution<> xs(-180, 180);
    std::uniform_real_distribution<> ys(-90, 90);
    std::uniform_real_distribution<> cluster_xs;
    std::uniform_real_distribution<> cluster_ys;
    update_cluster_distributions(xs, ys, cluster_xs, cluster_ys, gen);

    int count_per_cluster {num_points / NUM_CLUSTERS};
    for (int i {1}; i <= num_points; i++) {
        if (i % count_per_cluster == 0) {
            update_cluster_distributions(xs, ys, cluster_xs, cluster_ys, gen);
        }
        double x0 {cluster_xs(gen)};
        double x1 {cluster_xs(gen)};
        double y0 {cluster_ys(gen)};
        double y1 {cluster_ys(gen)};

        double distance = ReferenceHaversine(x0, y0, x1, y1);
        sum_of_distances += distance;
        distances.write((char*) &distance, sizeof(double));
        outf << "{\"x0\": " << x0 << ", \"y0\": " << y0 << ",\n";
        outf << "\"x1\": " << x1 << ", \"y1\": " << y1 << "}";
        if (i < num_points) {
            outf << ",\n";
        } else {
            outf << "\n";
        }
    }

    outf << "]}\n";
    return sum_of_distances / num_points;
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cout << "Usage: [uniform/cluster] [seed] [number of points]\n";
        return 1;
    }

    std::string mode{argv[1]};
    if (mode != "uniform" && mode != "cluster") return 1;

    std::stringstream convert{argv[2]};
    int seed {};
    if (!(convert >> seed)) return 1;

    convert.str(argv[3]);
    convert.clear();
    int num_points {};
    if (!(convert >> num_points)) return 1;
    if (num_points < 10) {
        std::cout << "For the number of points, enter an integer greater than 9.";
        return 1;
    }

    double mean_distance {};
    if (mode == "uniform") {
        mean_distance = uniform_point_distribution(seed, num_points);
    } else {
        mean_distance = clustered_point_distribution(seed, num_points);
    }

    std::cout << "Mode: " + mode + "\n";
    std::cout << "Seed: " + std::to_string(seed) + "\n";
    std::cout << "Number of points: " + std::to_string(num_points) + "\n";
    std::cout << "Expected sum: " + std::to_string(mean_distance) + "\n";
    return 0;
}
