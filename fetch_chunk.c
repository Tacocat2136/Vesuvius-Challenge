#define VESUVIUS_IMPL
#include "vesuvius-c.h"
#include <stdio.h>
#include <string.h>

int main(int argc, char** argv) {
    int preprocess = 1; // pass "raw" as an argument to skip preprocessing for comparison
    if (argc > 1 && strcmp(argv[1], "raw") == 0) {
        preprocess = 0;
    }

    volume* scroll_vol = vs_vol_new(
        "./54keV_7.91um_Scroll1A.zarr/0/",
        "https://dl.ash2txt.org/full-scrolls/Scroll1/PHercParis4.volpkg/volumes_zarr_standardized/54keV_7.91um_Scroll1A.zarr/0/");

    s32 vol_start[3]  = {3072, 3072, 3072};
    s32 chunk_dims[3] = {128, 512, 512};

    chunk* scroll_chunk = vs_vol_get_chunk(scroll_vol, vol_start, chunk_dims);
    chunk* output_chunk = scroll_chunk;

    chunk* normalized = NULL;
    chunk* sharpened = NULL;

    if (preprocess) {
        normalized = vs_normalize_chunk(scroll_chunk);
        sharpened = vs_unsharp_mask_3d(normalized, 1.5f, 3); // amount, kernel_size - tune these
        output_chunk = sharpened;
        printf("Applied normalize + unsharp mask\n");
    } else {
        printf("Writing raw density data (no preprocessing)\n");
    }

    FILE* f = fopen("chunk.bin", "wb");
    if (!f) {
        printf("Failed to open output file\n");
        return 1;
    }

    fwrite(output_chunk->dims, sizeof(int32_t), 3, f);
    size_t total = (size_t)output_chunk->dims[0] * output_chunk->dims[1] * output_chunk->dims[2];
    fwrite(output_chunk->data, sizeof(float), total, f);
    fclose(f);

    printf("Wrote chunk.bin: %d x %d x %d\n", output_chunk->dims[0], output_chunk->dims[1], output_chunk->dims[2]);

    vs_chunk_free(scroll_chunk);
    if (normalized) vs_chunk_free(normalized);
    if (sharpened) vs_chunk_free(sharpened);

    return 0;
}