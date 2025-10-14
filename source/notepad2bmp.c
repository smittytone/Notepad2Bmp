/*
    notepad2bmp

    Copyright © 2025 Tony Smith. All rights reserved.

    Version 0.4.0

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
#include <stdbool.h>
#include <getopt.h>


/*
    CONSTANTS
*/
#define RAW_DATA_SIZE                           4096
#define UNSCALED_WIDTH                          480
#define UNSCALED_HEIGHT                         64
#define UNSCALED_DATA_SIZE                      (UNSCALED_WIDTH * UNSCALED_HEIGHT)
#define SCALE_FACTOR                            3
#define SCALED_WIDTH                            (UNSCALED_WIDTH * SCALE_FACTOR)
#define SCALED_HEIGHT                           (UNSCALED_HEIGHT * SCALE_FACTOR)
#define SCALED_DATA_SIZE                        (SCALED_WIDTH * SCALED_HEIGHT)

#define BMP_V1_HEADER_DATA_SIZE                 62
#define BMP_V5_HEADER_DATA_SIZE                 146

#define BMP_HEADER_FILE_SIZE_INDEX              2
#define DIB_V5_HEADER_WIDTH_INDEX               4
#define DIB_V5_HEADER_HEIGHT_INDEX              8
#define DIB_V5_HEADER_BITS_PER_PIXEL_INDEX      14
#define DIB_V5_HEADER_DATA_SIZE_INDEX           20
#define DIB_V5_HEADER_H_RESOLUTION_INDEX        24
#define DIB_V5_HEADER_V_RESOLUTION_INDEX        28

#define ERROR_NONE                              0
#define ERROR_OPEN_SOURCE_FILE                  1
#define ERROR_OPEN_BMP_FILE                     2


/*
    FORWARD DECLARATIONS
*/
void output_header_bytes(const char* data, FILE *outfile, unsigned int byteCount);
void scale(char* source, char* destination);
int  convert(char* inpath, char* outpath, bool do_scale);
void show_error(int error_code, char* info);
void show_help(void);


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

/*
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
    0x6C,0x00,0x00,0x00,        // HEADER SIZE (108 bytes)
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
*/

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

// This CLT contains two colours: white and black
// (plus an optional 'LCD green' alternative for white)
const char BMP_CLT[8] = {
    //0x70,0x9D,0xA8,0x00,      // LCD GREEN IN BGRA
    0xFF,0xFF,0xFF,0x00,        // WHITE IN BGRA
    0x00,0x00,0x00,0x00         // BLACK IN BGRA
};


/*
    MAIN ROUTINE
*/
int main (int argc, char *argv[] ) {

    // FROM 0.4.0
    // Replace many original vars
    char*       source_path = NULL;
    char*       target_path = NULL;
    static int  do_scale = 1;
    int         option_index = 0;
    int         short_option = -1;
    bool        do_free_target_path = false;
    // Prevent error reporting by `getopt_long()`
                opterr = 0;
    // Define long options
    static struct option long_options[] = {
        {"rawsize", no_argument, &do_scale, 0},
        {"help", no_argument, NULL, 'h'},
        {0, 0, NULL, 0}
    };

    // Insufficient or too many args? Print help
    if (argc < 2 || argc > 4 ) {
        show_help();
        exit(0);
    }

    // Process args
    while (1) {
        short_option = getopt_long(argc, argv, "rh", long_options, &option_index);
        if (short_option == -1) break;
        switch(short_option) {
            case 'r':
                do_scale = 0;
            break;
            case 'h':
                show_help();
                exit(0);
            break;
            case '?':
                printf("[ERROR] Unknown option '%c'\n", optopt);
                exit(1);
        }
    }

    // Process positional args, ie. the file paths
    // TODO Update to handle multiple paths
    int arg_count = 0;
    if (optind < argc) {
        while (optind < argc) {
            if (arg_count == 0) {
                source_path = argv[optind++];
                arg_count++;
            } else if (arg_count == 1) {
                target_path = argv[optind++];
            }
        }
    } else {
        printf("[ERROR] Missing path to source screenshot\n");
        exit(1);
    }

    // Check the target path
    if (target_path != NULL) {
        // FROM 0.3.0
        // Make sure the supplied destination file name ends in '.bmp'
        if (strstr(target_path, ".bmp") == NULL) {
            // NOTE Above call succeeds on first `.bmp` found, so we'll currently
            //      not come here on files ending in, say, `.bmp.xxx`. We should
            //      really check that the file ENDS in `.bmp`.

            // It does not end in '.bmp' so add it
            int target_len = strlen(target_path);
            char* tmp_target_path = calloc(target_len + 5, sizeof(char));
            do_free_target_path = true;
            strcpy(tmp_target_path, target_path);
            strcpy(&tmp_target_path[target_len], ".bmp");
            target_path = tmp_target_path;
        }
    } else {
        // FROM 0.3.0
        // Use the source file as the basis for the destination file name
        // if no destination file name is provided.
        // Determine the length of the source filename minus any extension
        int length = 0;
        const char* result = strchr(source_path, '.');
        if (result != NULL) {
            length = result - source_path;
        } else {
            length = strlen(source_path);
        }

        // Allocate zeroed memory for the name and write in the source
        // name and then append the standard file extension
        target_path = calloc(length + 5, sizeof(char));
        do_free_target_path = true;
        strncpy(target_path, source_path, length);
        strcpy(&target_path[0] + length, ".bmp");
    }

    // FROM 0.4.0
    // Use the `convert()` function
    int error = convert(source_path, target_path, do_scale);
    if (error != 0) {
        show_error(error, error == ERROR_OPEN_SOURCE_FILE ? source_path : target_path);
    }

    // Free the generated-path memory
    if (do_free_target_path) free(target_path);

    // Exit with state
    exit(error);
}


/*
    Write a block of bytes to a file.

    - Parameters:
        - data:      Pointer to a header data.
        - outfile:   Pointer to open output file record.
        - byteCount: Number of bytes in the header data.
*/
void output_header_bytes(const char* data, FILE* outfile, unsigned int byteCount) {

    for (unsigned int i = 0 ; i < byteCount ; ++i) {
        fputc(data[i], outfile);
    }
}


/*
    Upscale pixel data.
    Currently we scale only by a factor of 3 (72dpi to 216dpi).

    FROM 0.2.0

    - Parameters:
        - data: Pointer to the raw bit per pixel data.
        - dest: Pointer to the scaled byte per pixel data.
*/
void scale(char* source, char* target) {

    // Buffer for the bit-per-pixel to byte-per-pixel conversion
    char tmp[UNSCALED_DATA_SIZE] = {0};

    // Convert each byte of the source into eight bytes, with one byte
    // per pixel in place of one byte per eight pixels.
    char* ptr = tmp;
    for (int row = 0 ; row < 64 ; ++row) {
        // Ignore the four-byte alignment padding in the source
        for (int col = 0 ; col < 60 ; ++col) {
            char byte = source[(63 - row) * 64 + col];
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
    char* destinationPtr = target;
    ptr = tmp;

    for (unsigned int i = 0 ; i < UNSCALED_DATA_SIZE ; ++i) {
        char byte = *ptr;
        ptr++;

        if (i % UNSCALED_WIDTH == 0) {
            // On a new row, so move the pointer on three whole rows
            destinationPtr = target + (((i / UNSCALED_WIDTH) * SCALE_FACTOR) + 1) * SCALED_WIDTH;
        } else {
            destinationPtr += 3;
        }

        // Fill in the neighbours in the destination buffer
        for (unsigned int j = 0; j < 9 ; ++j) {
            char* neighbourPtr = destinationPtr + delta[j];
            if (neighbourPtr < target + SCALED_DATA_SIZE) {
                *neighbourPtr = byte;
            }
        }
    }
}


/*
    Convert a single screenshot file to BMP.

    FROM 0.4.0

    - Parameters:
        - inpath:   Pointer to the path to the source file.
        - outpath:  Pointer to the path to the destination file.
        - do_scale: Should the image be scaled too?

    - Returns: 0 on success or an error value.
 */
int convert(char* inpath, char* outpath, bool do_scale) {

    char original[RAW_DATA_SIZE] = {0};
    char scaled[SCALED_DATA_SIZE] = {0};
    FILE* infile = NULL;
    FILE* outfile = NULL;

    // Read in the Amstrad screen grab data
    infile = fopen (inpath, "rb");
    if (infile == NULL) return ERROR_OPEN_SOURCE_FILE;

    for (unsigned int row = 0 ; row < 64 ; ++row) {
        for (int col = 0 ; col < 64 ; ++col) {
            // Ignore the four NC100 padding bytes per row
            int byte = fgetc(infile);
            if (col < 60) original[row * 64 + col] = byte;
        }
    }

    // Close the source file
    fclose(infile);

    // Open for output
    outfile = fopen (outpath, "wb");
    if (outfile == NULL) return ERROR_OPEN_BMP_FILE;

    if (do_scale) {
        // Change standard BMP header values to match the scaled image
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
    output_header_bytes(BMP_HEADER, outfile, 14);
    output_header_bytes(DIB_V5_HEADER, outfile, 124);
    output_header_bytes(BMP_CLT, outfile, 8);

    if (do_scale) {
        // Upscale the image using nearest neighbour mode
        scale(original, scaled);

        // Write out the scaled data
        for (unsigned int i = 0 ; i < SCALED_DATA_SIZE ; ++i) {
            fputc(scaled[i], outfile);
        }
    } else {
        // Write out the raw data
        for (unsigned int row = 0 ; row < 64 ; ++row) {
            // Exclude the padding rows
            for (unsigned int col = 0 ; col < 60 ; ++col) {
                // Read in the byte from the bottom rather than the
                // top of the array, as BMP reverses Amstrad's row order
                uint8_t byte = original[(63 - row) * 64 + col];
                fputc(byte, outfile);
            }
        }
    }

    // Close the file now we're done
    fclose(outfile);

    return ERROR_NONE;
}


/*
    Display an error message.

    FROM 0.4.0

    - Parameters:
        - error_code: The error to report.
        - info:       Pointer to extra message data
*/
void show_error(int error_code, char* info) {

    switch(error_code) {
        case ERROR_OPEN_SOURCE_FILE:
            printf("ERROR] Could not open Amstrad screenshot file %s\n", info);
            break;
        case ERROR_OPEN_BMP_FILE:
            printf("ERROR] Could not create BMP file %s\n", info);
            break;
        default:
            printf("ERROR] Unknown.\n");
    }
}


/*
    Display help info.

    FROM 0.4.0
*/
void show_help(void) {

    printf("notepad2bmp 0.4.0\n");
    printf("Copyright © 2025, Tony Smith (@smittytone). Source code available under the MIT licence.\n\n");
    printf("Usage: notepad2bmp {source filename} [output filename] [-r/--rawsize]\n\n");
    printf("Notes: If no output filename is provided, the name of the source file is used.\n");
    printf("       If no output filename extension is provided, .bmp is added.\n");
}
