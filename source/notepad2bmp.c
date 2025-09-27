/*
    notepad2bmp

    Copyright © 2025 Tony Smith. All rights reserved.

    Version 0.1.0

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


/*
    FORWARD DECLARATIONS
*/
void output_header(const char* data, FILE *output, unsigned int count) ;


/*
    BMP HEADER DATA
    These are stock values for the size and type of file we are generating. They comprise:

    1. BMP general header
    2. Device-independent bitmap header
    3. Colour look-up table

    For more on the BMP format see https://en.wikipedia.org/wiki/BMP_file_format
*/
const char BMP_HEADER[14] = {
    0x42,0x4D,                  // TYPE (BM>
    0x3E,0x10,0x00,0x00,        // FILE SIZE (4096+14+40+8)
    0x00,0x00,0x00,0x00,
    0x3E,0x00,0x00,0x00         // Offset to the pixel data (14+40+8)
};

const char DIB_HEADER[40] = {
    0x28,0x00,0x00,0x00,        // HEADER SIZE (40 bytes)
    0xE0,0x01,0x00,0x00,        // IMAGE WIDTH (480)
    0x40,0x00,0x00,0x00,        // IMAGE HEIGHT (64)
    0x01,0x00,                  // COLOUR PLANES (1)
    0x01,0x00,                  // BITS PER PIXEL (1)
    0x00,0x00,0x00,0x00,
    0x00,0x10,0x00,0x00,        // DATA SIZE (4096)
    0x13,0x0B,0x00,0x00,        // HORIZONTAL RESOLUTION in DOTS PER METRE (calculated from 72dpi)
    0x13,0x0B,0x00,0x00,        // VERTICAL RESOLUTION in DOTS PER METRE (calculated from 72dpi)
    0x02,0x00,0x00,0x00,        // NO. COLOURS IN THE PALETTE
    0x00,0x00,0x00,0x00         // IMPORTANT COLOURS (0 = ALL)
};

const char BMP_CLT[8] = {
    //0x70,0x9D,0xA8,0x00,        // LCD GREEN IN BGRA
    0xFF,0xFF,0xFF,0x00,        // WHITE IN BGRA
    0x00,0x00,0x00,0x00         // BLACK IN BGRA
};


/*
    MAIN ROUTINE
*/
int main (int argc, char *argv[] ) {

    FILE *source_file, *bmp_file;
    int byte;
    char data[4096] = {0};
    char scaled[34560] = {0};

    // Insufficient args? Print help
    if (argc < 3 || argc > 3 ) {
        printf("Usage: notepad2bmp {source filename} {output filename}\n");
        exit(0);
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

    // Write out the 62-byte BMP headers
    output_header(BMP_HEADER, bmp_file, 14);
    output_header(DIB_HEADER, bmp_file, 40);
    output_header(BMP_CLT, bmp_file, 8);

    // Write out the rows: BMP
    for (int row = 0 ; row < 64 ; ++row) {
        // Exclude the padding rows
        for (int col = 0 ; col < 60 ; ++col) {
            // Read in the byte from the bottom rather than the
            // top of the array, as BMP reverses Amstrad's row order
            byte = data[(63 - row) * 64 + col];
            fputc(byte, bmp_file);
        }
    }

    fclose(bmp_file);
}


/*
    Write a block of bytes to a file.

    - Parameters:
        - data:   Pointer to a header data.
        - output: Pointer to open output file record.
        - count:  Number of bytes in the header data.
*/
void output_header(const char* data, FILE *output, unsigned int count) {

    for (unsigned int i = 0 ; i < count ; ++i) {
        fputc(data[i], output);
    }
}
