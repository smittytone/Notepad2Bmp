# Release Notes

- 0.4.0
    - Migrate arg parsing to `getopt_long()`.
    - Separate out conversion code into a function.
- 0.3.0
    - Auto generate output filename from source filename when not included.
    - Add .bmp to output filename if not included.
    - Switch code to C99 bools.
- 0.2.0
    - Add automatic 3x image upscaling, which can be disabled with the new --rawsize flag.
    - Switch the the most recent version (5) of the BMP DIB header format.
- 0.1.0
    - Initial release.
