/*
 * (c) Jaakko Karjanlahti
 * 
 * Published under MIT licence (See "LICENCE")
 * 
 * This code is written to demonstrate the basic functionality
 * of a simple and unoptimized triangle rasterizer.
 * 
 * Many steps are cut off and simplified to make everything
 * more understandable.
 */

#include <stdio.h> // FILE, fopen, fwrite, fclose, printf
#include <string.h> // memcpy, memset
#include <stdlib.h> // malloc, free, EXIT_SUCCESS, EXIT_FAILURE
#include <stdint.h> // uint32_t, uint8_t

#define WIDTH  512
#define HEIGHT 512

// Simple types for triangle edge and xy-coordinate.
typedef struct edge_s {
    float a, b, c;
} edge_t;

typedef struct vec2_s {
    float x, y;
} vec2_t;

// Creates a new edge from two vector positions.
edge_t init_edge(vec2_t v0, vec2_t v1);

// Calculates the distance between the given point and edge.
float test_edge(edge_t edge, vec2_t point);

// Saves rgb888 pixel array as a bmp-file.
int save_pixels_as_bmp(const char* path, uint8_t* pixels, int w, int h);

int main() {
    // Allocate the pixel array and clear it with a black color.
    uint8_t* pixels = (uint8_t*) malloc(WIDTH*HEIGHT*3);
    memset(pixels, 0x00, WIDTH*HEIGHT*3);
    
    // Declare the 3 corner points of a triangle.
    vec2_t v0 = { WIDTH/2.0f-0.5f , 0.5f             };
    vec2_t v1 = { 0.5f            , HEIGHT-0.5f      };
    vec2_t v2 = { WIDTH-0.5f      , HEIGHT/2.0f-0.5f };

    // Calculate the edges between each point.
    edge_t edge0 = init_edge(v2, v1);
    edge_t edge1 = init_edge(v0, v2);
    edge_t edge2 = init_edge(v1, v0);

    // Sum of the edge c-terms do give us the area of the triangle.
    float area = edge0.c + edge1.c + edge2.c;
    
    // Loop through the pixel array.
    for (unsigned y=0; y<HEIGHT; y++) {
        for (unsigned x=0; x<WIDTH; x++) {
            
            // Current pixel location. (+0.5 to get center)
            vec2_t point = { x+0.5f, y+0.5f };
            
            // Calculate the distance to the point from each edge.
            float d0 = test_edge(edge0, point);
            float d1 = test_edge(edge1, point);
            float d2 = test_edge(edge2, point);

            // The point is inside the triangle when it's distance
            // from every three edges is positive.
            if (d0 > 0.0f && d1 > 0.0f && d2 > 0.0f) {

                // We can now divide the distances with the area of the triangle
                // to get 'weights' that can be used to interpolate between point
                // attributes such as color.
                float i = d0 / area;
                float j = d1 / area;
                float k = d2 / area;

                // Here we are using the 'weights' to interpolate between colors.
                pixels[(y*WIDTH+x)*3+0] = (uint8_t)(i * 255.0f);
                pixels[(y*WIDTH+x)*3+1] = (uint8_t)(j * 255.0f);
                pixels[(y*WIDTH+x)*3+2] = (uint8_t)(k * 255.0f);
            }
        }
    }

    // Save the pixel array as bmp so we can look at the resulting image.
    int res = save_pixels_as_bmp("result.bmp", pixels, WIDTH, HEIGHT);

    free(pixels); // And last but not least, free the allocated memory and exit.
    return res;
}

// Only the 'functionality' of the edge elements is explained.
// If intrested you can search yourself for more specific information about edge functions.

edge_t init_edge(vec2_t v0, vec2_t v1) {
    edge_t edge;

    // a and b will tell us the 'angle' of the edge.
    edge.a = v0.y - v1.y;
    edge.b = v1.x - v0.x;

    // c is used for the area and as a 'offset'
    edge.c = v0.x * v1.y - v0.y * v1.x;

    return edge;
}

float test_edge(edge_t edge, vec2_t point) {
    return edge.a * point.x + edge.b * point.y + edge.c;
}

// Short implementation of a bmp writer. This is out of the context and
// only used to save the resulting image, so not much documentation is given.
// This is only capable of saving rgb888 format pixel data.
int save_pixels_as_bmp(const char* path, uint8_t* pixels, int w, int h) {
    const int padding = (4 - (w*3) % 4) % 4;
    const int size = 54 + (w*3+padding)*h;
    const uint8_t header[54] = { // File info- and DIB-header.
        'B','M',
        (uint8_t)(size),(uint8_t)(size >> 8),(uint8_t)(size >> 16),(uint8_t)(size >> 24),
        0,0,0,0, 54,0,0,0, 40,0,0,0,
        (uint8_t)(w),(uint8_t)(w >> 8),(uint8_t)(w >> 16),(uint8_t)(w >> 24),
        (uint8_t)(h),(uint8_t)(h >> 8),(uint8_t)(h >> 16),(uint8_t)(h >> 24),
        1,0,24,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0
    };
    uint8_t* bytes = (uint8_t*) malloc(size); // Allocate byte buffer.
    uint8_t* ptr = bytes;
    memcpy(ptr, header, 54); // Save header to buffer.
    ptr += 54;
    // Save the pixel data to buffer in right order and padding.
    for (int y=h-1; y>=0; y--) {
        for (int x=0; x<w; x++)
            for (int i=2; i>=0; i--, ptr++)
                *ptr = pixels[(y*w+x)*3+i];
        for (int i=0; i<padding; i++, ptr++)
            *ptr = 0x00;
    }
    // Open file as binary, write the buffer bytes and close the file.
    FILE* file = fopen(path, "wb");
    if (file == NULL) {
        printf("Failed to open file '%s'\n", path);
        free(bytes);
        return EXIT_FAILURE;
    }
    fwrite(bytes, 1, size, file);
    fclose(file);
    free(bytes); // Free memory and exit.
    return EXIT_SUCCESS;
}
