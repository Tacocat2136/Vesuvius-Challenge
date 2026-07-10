#define MC_IMPLEM_ENABLE
#include "MC.h"

#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <fstream>
#include <vector>

int main(int argc, char** argv) {
    float thresholdFraction = 0.75f;
    if (argc > 1) {
        thresholdFraction = std::atof(argv[1]);
    }

    std::ifstream in("chunk.bin", std::ios::binary);
    if (!in) {
        printf("chunk.bin not found - run fetch_chunk.exe first\n");
        return 1;
    }

    int32_t dims[3]; // {z, y, x}, matches vesuvius-c's chunk->dims layout
    in.read(reinterpret_cast<char*>(dims), sizeof(dims));

    int dimZ = dims[0], dimY = dims[1], dimX = dims[2];
    size_t total = static_cast<size_t>(dimZ) * dimY * dimX;

    std::vector<float> data(total);
    in.read(reinterpret_cast<char*>(data.data()), total * sizeof(float));
    in.close();

    // Find min/max to pick a reasonable default threshold
    float minVal = data[0], maxVal = data[0];
    for (size_t i = 0; i < total; i++) {
        if (data[i] < minVal) minVal = data[i];
        if (data[i] > maxVal) maxVal = data[i];
    }
    float threshold = minVal + (maxVal - minVal) * thresholdFraction; // tune this

    // Build the scalar field: shift so the surface we want sits at the zero crossing.
    // Our data's flat layout (z*dimY*dimX + y*dimX + x, x fastest) already matches
    // what MC::marching_cube expects when called with (nx=dimX, ny=dimY, nz=dimZ).
    std::vector<MC::MC_FLOAT> field(total);
    for (size_t i = 0; i < total; i++) {
        field[i] = static_cast<MC::MC_FLOAT>(data[i] - threshold);
    }

    MC::mcMesh mesh;
    MC::marching_cube(field.data(), dimX, dimY, dimZ, mesh);

    printf("Generated mesh: %zu vertices, %zu triangles\n",
           mesh.vertices.size(), mesh.indices.size() / 3);

    // Write as an OBJ so we can open it in Blender/MeshLab
    std::ofstream out("chunk.obj");
    for (const auto& v : mesh.vertices) {
        out << "v " << v.x << " " << v.y << " " << v.z << "\n";
    }
    for (size_t i = 0; i + 2 < mesh.indices.size(); i += 3) {
        // OBJ face indices are 1-based
        out << "f " << (mesh.indices[i] + 1) << " "
                     << (mesh.indices[i + 1] + 1) << " "
                     << (mesh.indices[i + 2] + 1) << "\n";
    }
    out.close();

    printf("Wrote chunk.obj\n");
    return 0;
}