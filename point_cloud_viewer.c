#include "raylib.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    FILE* f = fopen("chunk.bin", "rb");
    if (!f) {
        printf("chunk.bin not found - run fetch_chunk.exe first\n");
        return 1;
    }

    int32_t dims[3];
    fread(dims, sizeof(int32_t), 3, f);
    int dimZ = dims[0], dimY = dims[1], dimX = dims[2];

    size_t total = (size_t)dimZ * dimY * dimX;
    float* data = malloc(sizeof(float) * total);
    fread(data, sizeof(float), total, f);
    fclose(f);

    float minVal = data[0], maxVal = data[0];
    for (size_t i = 0; i < total; i++) {
        if (data[i] < minVal) minVal = data[i];
        if (data[i] > maxVal) maxVal = data[i];
    }
    float threshold = minVal + (maxVal - minVal) * 0.5f;

    const int stride = 8;
    int count = 0;
    for (int z = 0; z < dimZ; z += stride)
        for (int y = 0; y < dimY; y += stride)
            for (int x = 0; x < dimX; x += stride)
                if (data[z * dimY * dimX + y * dimX + x] > threshold)
                    count++;

    Vector3* points = malloc(sizeof(Vector3) * count);
    Color* colors = malloc(sizeof(Color) * count);

    float scale = 0.05f;
    int idx = 0;
    for (int z = 0; z < dimZ; z += stride) {
        for (int y = 0; y < dimY; y += stride) {
            for (int x = 0; x < dimX; x += stride) {
                float val = data[z * dimY * dimX + y * dimX + x];
                if (val > threshold) {
                    points[idx].x = (x - dimX / 2) * scale;
                    points[idx].y = (z - dimZ / 2) * scale;
                    points[idx].z = (y - dimY / 2) * scale;

                    float t = (val - minVal) / (maxVal - minVal);
                    if (t < 0) t = 0;
                    if (t > 1) t = 1;
                    unsigned char gray = (unsigned char)(t * 255);
                    colors[idx] = (Color){ gray, gray, gray, 255 };
                    idx++;
                }
            }
        }
    }

    InitWindow(1000, 700, "Scroll Point Cloud");
    DisableCursor();

    Camera3D camera = { 0 };
    camera.position = (Vector3){ 20.0f, 20.0f, 20.0f };
    camera.target   = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.up       = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy     = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_TAB)) {
            if (IsCursorHidden()) EnableCursor();
            else DisableCursor();
        }

        if (IsCursorHidden()) {
            UpdateCamera(&camera, CAMERA_FREE);
        }

        BeginDrawing();
            ClearBackground(RAYWHITE);
            BeginMode3D(camera);
                for (int i = 0; i < count; i++) {
                    DrawPoint3D(points[i], colors[i]);
                }
                DrawGrid(10, 1.0f);
            EndMode3D();
            DrawFPS(10, 10);
            DrawText(TextFormat("Points: %d", count), 10, 30, 20, DARKGRAY);
        EndDrawing();
    }

    CloseWindow();
    free(points);
    free(colors);
    free(data);
    return 0;
}