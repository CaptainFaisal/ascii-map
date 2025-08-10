#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <png.h>

// Function to read PNG file and return 2D array of RGB sums
int** read_png_sum_rgb(const char* filename, int* width, int* height) {
    FILE* fp = fopen(filename, "rb");
    if (!fp) {
        fprintf(stderr, "Error: Could not open file %s\n", filename);
        return NULL;
    }

    // Check if the file is a PNG
    png_byte header[8];
    fread(header, 1, 8, fp);
    if (png_sig_cmp(header, 0, 8)) {
        fprintf(stderr, "Error: %s is not a PNG file\n", filename);
        fclose(fp);
        return NULL;
    }

    // Initialize PNG structures
    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) {
        fprintf(stderr, "Error: png_create_read_struct failed\n");
        fclose(fp);
        return NULL;
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        fprintf(stderr, "Error: png_create_info_struct failed\n");
        png_destroy_read_struct(&png_ptr, NULL, NULL);
        fclose(fp);
        return NULL;
    }

    // Set error handling
    if (setjmp(png_jmpbuf(png_ptr))) {
        fprintf(stderr, "Error during PNG reading\n");
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        fclose(fp);
        return NULL;
    }

    // Initialize IO
    png_init_io(png_ptr, fp);
    png_set_sig_bytes(png_ptr, 8);

    // Read PNG info
    png_read_info(png_ptr, info_ptr);

    *width = png_get_image_width(png_ptr, info_ptr);
    *height = png_get_image_height(png_ptr, info_ptr);
    png_byte color_type = png_get_color_type(png_ptr, info_ptr);
    png_byte bit_depth = png_get_bit_depth(png_ptr, info_ptr);

    // Ensure we're dealing with RGB or RGBA images
    if (color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_palette_to_rgb(png_ptr);
    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
        png_set_expand_gray_1_2_4_to_8(png_ptr);
    if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
        png_set_tRNS_to_alpha(png_ptr);
    if (bit_depth == 16)
        png_set_strip_16(png_ptr);
    if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
        png_set_gray_to_rgb(png_ptr);

    // Update info after transformations
    png_read_update_info(png_ptr, info_ptr);

    // Allocate memory for row pointers
    png_bytep* row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * (*height));
    for (int y = 0; y < *height; y++) {
        row_pointers[y] = (png_byte*)malloc(png_get_rowbytes(png_ptr, info_ptr));
    }

    // Read the image
    png_read_image(png_ptr, row_pointers);

    // Allocate memory for our 2D array
    int** rgb_sum = (int**)malloc(sizeof(int*) * (*height));
    for (int y = 0; y < *height; y++) {
        rgb_sum[y] = (int*)malloc(sizeof(int) * (*width));
    }

    // Process each pixel
    int channels = png_get_channels(png_ptr, info_ptr);
    for (int y = 0; y < *height; y++) {
        png_bytep row = row_pointers[y];
        for (int x = 0; x < *width; x++) {
            png_bytep px = &(row[x * channels]);
            
            // Sum the RGB channels (ignore alpha if present)
            int sum = px[0] + px[1] + px[2];
            rgb_sum[y][x] = sum;
        }
    }

    // Clean up PNG resources
    for (int y = 0; y < *height; y++) {
        free(row_pointers[y]);
    }
    free(row_pointers);
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    fclose(fp);

    return rgb_sum;
}

// Function to free the 2D array
void free_rgb_sum(int** rgb_sum, int height) {
    for (int y = 0; y < height; y++) {
        free(rgb_sum[y]);
    }
    free(rgb_sum);
}

int main() {

    int width, height;
    int** rgb_sum = read_png_sum_rgb("map.png", &width, &height);
    if (!rgb_sum) {
        return 1;
    }

    int W = 10;
    int H = 20;
    int offsetX = W/2;
    int offsetY = H/2;
    printf("RGB Sum values (first 10x10 pixels):\n");
    for (int j = 0; j < floor(height/H); j++) {
        for (int i = 0; i < floor(width/W); i++) {
            int x = i*W;
            int y = j*H;
            if(rgb_sum[y+offsetY][x+offsetX] != 0){
                printf("*");
            }
            else {
                printf(" ");
            }
        }
        printf("\n");
    }
    printf("Width = %d, Height = %d", width, height);

    free_rgb_sum(rgb_sum, height);
    return 0;
}