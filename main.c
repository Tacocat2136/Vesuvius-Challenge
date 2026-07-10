#define VESUVIUS_IMPL
#include "vesuvius-c.h"

int main() {
    // pick a region in the scroll to visualize
    int vol_start[3] = {3072, 3072, 3072};
    int chunk_dims[3] = {128, 512, 512};

    // initialize the volume
    volume* scroll_vol = vs_vol_new(
        "./54keV_7.91um_Scroll1A.zarr/0/",
        "https://dl.ash2txt.org/full-scrolls/Scroll1/PHercParis4.volpkg/volumes_zarr_standardized/54keV_7.91um_Scroll1A.zarr/0/");

    // get the scroll data, reading from cache and downloading if necessary
    chunk* scroll_chunk = vs_vol_get_chunk(scroll_vol, vol_start, chunk_dims);

    // extract a slice from the volume
    slice* myslice = vs_slice_extract(scroll_chunk, 0);

    // write slice image to file
    vs_bmp_write("xy_slice.bmp", myslice);

    return 0;
}