/*
    notepad2bmp

    Copyright © 2025 Tony Smith. All rights reserved.

    Version 0.2.0

    MIT License
    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>


/*
    CONSTANTS
*/
#define RAW_DATA_SIZE 4096
#define UNSCALED_WIDTH 480
#define UNSCALED_HEIGHT 64
#define UNSCALED_DATA_SIZE (UNSCALED_WIDTH * UNSCALED_HEIGHT)
#define SCALE_FACTOR 3
#define SCALED_WIDTH (UNSCALED_WIDTH * SCALE_FACTOR)
#define SCALED_HEIGHT (UNSCALED_HEIGHT * SCALE_FACTOR)
#define SCALED_DATA_SIZE (SCALED_WIDTH * SCALED_HEIGHT)

#define BMP_V1_HEADER_DATA_SIZE 62
#define BMP_V5_HEADER_DATA_SIZE 146

#define BMP_HEADER_FILE_SIZE_INDEX 2
#define DIB_V5_HEADER_WIDTH_INDEX 4
#define DIB_V5_HEADER_HEIGHT_INDEX 8
#define DIB_V5_HEADER_BITS_PER_PIXEL_INDEX 14
#define DIB_V5_HEADER_DATA_SIZE_INDEX 20
#define DIB_V5_HEADER_H_RESOLUTION_INDEX 24
#define DIB_V5_HEADER_V_RESOLUTION_INDEX 28


/*
    FORWARD DECLARATIONS
*/
void output_header_bytes(const char* data, FILE *output, unsigned int count);
void scale(char* data, char* dest);


/*
    BMP HEADER DATA
    These are stock values for the size and type of file we are generating. They comprise:

    1. BMP general header
    2. Device-independent bitmap header (multiple versions included; version 5 used)
    3. Colour look-up table

    For more on the BMP format see https://en.wikipedia.org/wiki/BMP_file_format
*/
char BMP_HEADER[14] = {
    0x42,0x4D,                  // TYPE (BM)
    0x92,0x10,0x00,0x00,        // FILE SIZE (4096+14+124+8)
    0x00,0x00,0x00,0x00,        // RESERVED
    0x92,0x00,0x00,0x00         // Offset to the pixel data (14+40+8)
};

char DIB_V1_HEADER[40] = {
    0x28,0x00,0x00,0x00,        // HEADER SIZE (40 bytes)
    0xE0,0x01,0x00,0x00,        // IMAGE WIDTH (480)
    0x40,0x00,0x00,0x00,        // IMAGE HEIGHT (64)
    0x01,0x00,                  // COLOUR PLANES (1)
    0x01,0x00,                  // BITS PER PIXEL (1)
    0x00,0x00,0x00,0x00,        // COMPRESSION (0)
    0x00,0x10,0x00,0x00,        // DATA SIZE (4096)
    0x13,0x0B,0x00,0x00,        // HORIZONTAL RESOLUTION in DOTS PER METRE (calculated from 72dpi)
    0x13,0x0B,0x00,0x00,        // VERTICAL RESOLUTION in DOTS PER METRE (calculated from 72dpi)
    0x02,0x00,0x00,0x00,        // NO. COLOURS IN THE PALETTE
    0x00,0x00,0x00,0x00         // IMPORTANT COLOURS (0 = ALL)
};

char DIB_V4_HEADER[108] = {
    0x7C,0x00,0x00,0x00,        // HEADER SIZE (108 bytes)
    0xE0,0x01,0x00,0x00,        // IMAGE WIDTH (480)
    0x40,0x00,0x00,0x00,        // IMAGE HEIGHT (64)
    0x01,0x00,                  // COLOUR PLANES (1)
    0x01,0x00,                  // BITS PER PIXEL (1)
    0x00,0x00,0x00,0x00,        // COMPRESSION (0)
    0x00,0x10,0x00,0x00,        // DATA SIZE (4096)
    0x13,0x0B,0x00,0x00,        // HORIZONTAL RESOLUTION in DOTS PER METRE (calculated from 72dpi)
    0x13,0x0B,0x00,0x00,        // VERTICAL RESOLUTION in DOTS PER METRE (calculated from 72dpi)
    0x02,0x00,0x00,0x00,        // NO. COLOURS IN THE PALETTE
    0x00,0x00,0x00,0x00,        // IMPORTANT COLOURS (0 = ALL)
    0x00,0x00,0x00,0x00,        // R MASK
    0x00,0x00,0x00,0x00,        // G MASK
    0x00,0x00,0x00,0x00,        // B MASK
    0x00,0x00,0x00,0x00,        // A MASK
    0x42,0x47,0x52,0x73,        // COLOUR SPACE TYPE (sRGB)
    0x00,0x00,0x00,0x00,        // ENDPOINTS
    0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,        // R GAMMA
    0x00,0x00,0x00,0x00,        // G GAMMA
    0x00,0x00,0x00,0x00,        // B GAMMA
};

char DIB_V5_HEADER[124] = {
    0x7C,0x00,0x00,0x00,        // HEADER SIZE (124 bytes)
    0xE0,0x01,0x00,0x00,        // IMAGE WIDTH (480)
    0x40,0x00,0x00,0x00,        // IMAGE HEIGHT (64)
    0x01,0x00,                  // COLOUR PLANES (1)
    0x01,0x00,                  // BITS PER PIXEL (1)
    0x00,0x00,0x00,0x00,        // COMPRESSION (0)
    0x00,0x10,0x00,0x00,        // DATA SIZE (4096)
    0x13,0x0B,0x00,0x00,        // HORIZONTAL RESOLUTION in DOTS PER METRE (calculated from 72dpi)
    0x13,0x0B,0x00,0x00,        // VERTICAL RESOLUTION in DOTS PER METRE (calculated from 72dpi)
    0x02,0x00,0x00,0x00,        // NO. COLOURS IN THE PALETTE
    0x00,0x00,0x00,0x00,        // IMPORTANT COLOURS (0 = ALL)
    0x00,0x00,0x00,0x00,        // R MASK
    0x00,0x00,0x00,0x00,        // G MASK
    0x00,0x00,0x00,0x00,        // B MASK
    0x00,0x00,0x00,0x00,        // A MASK
    0x42,0x47,0x52,0x73,        // COLOUR SPACE TYPE (sRGB)
    0x00,0x00,0x00,0x00,        // ENDPOINTS
    0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,        // R GAMMA
    0x00,0x00,0x00,0x00,        // G GAMMA
    0x00,0x00,0x00,0x00,        // B GAMMA
    0x00,0x00,0x00,0x00,        // INTENT
    0x00,0x00,0x00,0x00,        // PROFILE DATA
    0x00,0x00,0x00,0x00,        // PROFILE SIZE
    0x00,0x00,0x00,0x00         // RESERVED
};

const char BMP_CLT[8] = {
    //0x70,0x9D,0xA8,0x00,      // LCD GREEN IN BGRA
    0xFF,0xFF,0xFF,0x00,        // WHITE IN BGRA
    0x00,0x00,0x00,0x00         // BLACK IN BGRA
};


/*
    MAIN ROUTINE
*/
int main (int argc, char *argv[] ) {

    FILE *source_file, *bmp_file;
    int byte;
    char data[RAW_DATA_SIZE] = {0};
    // FROM 0.2.0
    char scaled[SCALED_DATA_SIZE] = {0};
    char do_scale = 1;

    // Insufficient args? Print help
    if (argc < 3 || argc > 4 ) {
        printf("Usage: notepad2bmp {source filename} {output filename}\n");
        exit(0);
    }

    // FROM 0.2.0
    // Add `-r/--rawsize` switch to disable upscaling
    if (argc == 4) {
        for (unsigned int i = 1 ; i < 4 ; ++i) {
            if (strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "--rawsize") == 0) {
                do_scale = 0;
                break;
            }
        }
    }

    // Open the source file if we can
    source_file = fopen (argv[1], "rb");
    if (source_file == NULL) {
        printf("[ERROR] File %s not found.\n" , argv[1]);
        exit (1);
    }

    // Open the destination file if we can
    bmp_file = fopen (argv[2], "wb");
    if (bmp_file == NULL) {
        printf("[ERROR] Cannot create file %s.\n" , argv[2]);
        exit(1);
    }

    // Read in the Amstrad screen grab data
    for (unsigned int row = 0 ; row < 64 ; ++row) {
        for (int col = 0 ; col < 60 ; ++col) {
            data[row * 64 + col] = fgetc(source_file);
        }

        // Ignore the four NC100 padding bytes per row
        fgetc(source_file);
        fgetc(source_file);
        fgetc(source_file);
        fgetc(source_file);
    }

    // Close the source file
    fclose(source_file);

    if (do_scale == 1) {
        // Change BMP header values to match the scaled image
        unsigned int size = SCALED_DATA_SIZE + BMP_V5_HEADER_DATA_SIZE;
        BMP_HEADER[BMP_HEADER_FILE_SIZE_INDEX]     = (char)(size & 0xFF);;
        BMP_HEADER[BMP_HEADER_FILE_SIZE_INDEX + 1] = (char)((size & 0xFF00) >> 8);;
        BMP_HEADER[BMP_HEADER_FILE_SIZE_INDEX + 2] = (char)((size & 0xFF0000) >> 16);;

        DIB_V5_HEADER[DIB_V5_HEADER_WIDTH_INDEX]      = (char)(SCALED_WIDTH & 0xFF);
        DIB_V5_HEADER[DIB_V5_HEADER_WIDTH_INDEX + 1]  = (char)((SCALED_WIDTH & 0xFF00) >> 8);
        DIB_V5_HEADER[DIB_V5_HEADER_HEIGHT_INDEX]     = (char)SCALED_HEIGHT;

        DIB_V5_HEADER[DIB_V5_HEADER_BITS_PER_PIXEL_INDEX] = 8;

        size = SCALED_DATA_SIZE;
        DIB_V5_HEADER[DIB_V5_HEADER_DATA_SIZE_INDEX]     = (char)(size & 0xFF);
        DIB_V5_HEADER[DIB_V5_HEADER_DATA_SIZE_INDEX + 1] = (char)((size & 0xFF00) >> 8);
        DIB_V5_HEADER[DIB_V5_HEADER_DATA_SIZE_INDEX + 2] = (char)((size & 0xFF0000) >> 16);

        DIB_V5_HEADER[DIB_V5_HEADER_H_RESOLUTION_INDEX]     = 0x38;
        DIB_V5_HEADER[DIB_V5_HEADER_H_RESOLUTION_INDEX + 1] = 0x21;
        DIB_V5_HEADER[DIB_V5_HEADER_V_RESOLUTION_INDEX]     = 0x38;
        DIB_V5_HEADER[DIB_V5_HEADER_V_RESOLUTION_INDEX + 1] = 0x21;

    }

    // Write out the BMP headers in order
    output_header_bytes(BMP_HEADER, bmp_file, 14);
    output_header_bytes(DIB_V5_HEADER, bmp_file, 124);
    output_header_bytes(BMP_CLT, bmp_file, 8);

    if (do_scale == 1) {
        // Upscale the image using nearest neighbour mode
        scale(data, scaled);

        // Write out the scaled data
        for (unsigned int i = 0 ; i < SCALED_DATA_SIZE ; ++i) {
            fputc(scaled[i], bmp_file);
        }
    } else {
        // Write out the raw data
        for (unsigned int row = 0 ; row < 64 ; ++row) {
            // Exclude the padding rows
            for (unsigned int col = 0 ; col < 60 ; ++col) {
                // Read in the byte from the bottom rather than the
                // top of the array, as BMP reverses Amstrad's row order
                byte = data[(63 - row) * 64 + col];
                fputc(byte, bmp_file);
            }
        }
    }

    // Close the file now we're done
    fclose(bmp_file);
}


/*
    Write a block of bytes to a file.

    - Parameters:
        - data:   Pointer to a header data.
        - output: Pointer to open output file record.
        - count:  Number of bytes in the header data.
*/
void output_header_bytes(const char* data, FILE *output, unsigned int count) {

    for (unsigned int i = 0 ; i < count ; ++i) {
        fputc(data[i], output);
    }
}


/*
    Upscale pixel data.
    Currently we scale only by a factor of 3 (72dpi to 216dpi).

    - Parameters:
        - data: Pointer to the raw bit per pixel data.
        - dest: Pointer to the scaled byte per pixel data.
*/
void scale(char* data, char* dest) {

    // Buffer for the bit-per-pixel to byte-per-pixel conversion
    char tmp[UNSCALED_DATA_SIZE] = {0};

    // Convert each byte of the source into eight bytes, with one byte
    // per pixel in place of one byte per eight pixels.
    char* ptr = tmp;
    for (int row = 0 ; row < 64 ; ++row) {
        // Ignore the four-byte alignment padding in the source
        for (int col = 0 ; col < 60 ; ++col) {
            char byte = data[(63 - row) * 64 + col];
            for (int bit = 0 ; bit < 8 ; ++bit) {
                char pixel = (byte & (1 << (7 - bit))) >> (7 - bit);
                *ptr = pixel > 0 ? 0x01 : 0x00;
                ptr++;
            }
        }
    }

    // Delta values cover the eight bytes neighbouring the destination
    // byte mapped to a byte in the source buffer
    const int delta[9] = {-1441, -1440, -1439, -1, 0, 1, 1439, 1440, 1441};
    //char* destPtr = dest;
    ptr = tmp;
    int start = 0;

    for (unsigned int i = 0 ; i < UNSCALED_DATA_SIZE ; ++i) {
        char byte = *ptr;
        ptr++;

        if (i % UNSCALED_WIDTH == 0) {
            // On a new row, so move the pointer on three whole rows
            //destPtr = dest + (((i / UNSCALED_WIDTH) * SCALE_FACTOR) + 1) * SCALED_WIDTH;
            start = (((i / UNSCALED_WIDTH) * SCALE_FACTOR) + 1) * SCALED_WIDTH;
        } else {
            //destPtr += 3;
            start += 3;
        }

        // Fill in the neighbours in the destination buffer
        for (unsigned int j = 0; j < 9 ; ++j) {
            //char* neighbourPtr = destPtr + delta[j];
            int index = start + delta[j];
            //if (neighbourPtr < dest + SCALED_DATA_SIZE) {
            //    *neighbourPtr = byte;
            //}
            if (index < SCALED_DATA_SIZE) {
                dest[index] = byte;
            }
        }
    }
}