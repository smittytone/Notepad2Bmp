/*
    notepad2pcx

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
    BMP HEADER DATA
    Stock PCX header prepped for the size and type of image we are converting
*/
const char PCX_HEADER [128] = {
    0x0A,0x05,0x01,0x01,0x00,0x00,0x00,0x00,0xDF,0x01,0x3f,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x01,0x3C,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};


/*
    MAIN ROUTINE
*/
int main (int argc, char *argv[] ) {
    FILE *source_file, *pcx_file;

    // Insufficient args? Print help
    if (argc < 3 || argc > 3 ) {
        printf("Usage: notepad2pcx {source filename} {output filename}\n");
        exit(0);
    }

    source_file = fopen (argv[1], "rb");
    if (source_file == NULL) {
        printf("[ERROR] File %s not found.\n" , argv[1]);
        exit (1);
    }

    pcx_file = fopen (argv[2], "wb");
    if (pcx_file == NULL) {
        printf("[ERROR] Cannot create file %s.\n" , argv[2]);
        exit(1);
    }

    // Write out the 128-byte PCX header
    for (int i = 0 ; i < 128 ; ++i) {
        fputc(PCX_HEADER[i], pcx_file);
    }

    // Write out the rows: PCX
    for (int row = 0 ; row < 64 ; ++row) {
        for (int col = 0 ; col < 60 ; ++col) {
            // Read in the byte...
            char byte = fgetc(source_file);

            // ...and write it out
            fputc(0xC1, pcx_file);
            fputc(byte, pcx_file);
        }

        // Four padding bytes, which we can ignore for PCX
        fgetc(source_file);
        fgetc(source_file);
        fgetc(source_file);
        fgetc(source_file);
    }

    fclose(source_file);
    fclose(pcx_file);
}
