#include <png.h>
#include <iostream>
#include <getopt.h>
#include <zlib.h>
#include <string.h>
#include <cstdlib>

int main() {
    FILE* opened_file = fopen("test.png", "rb");
    if (!opened_file) {
        std::cerr << "file opening failed\n";
        return EXIT_FAILURE;
    }

    png_byte fst_eight[8];
    size_t read_bytes = fread(fst_eight, 1, 8, opened_file);
    if (read_bytes != 8) {
        std::cerr << "failed to read PNG signature\n";
        fclose(opened_file);
        return EXIT_FAILURE;
    }

    if (png_sig_cmp(fst_eight, 0, 8)) {
        std::cerr << "file is not a PNG\n";
        fclose(opened_file);
        return EXIT_FAILURE;
    }

    std::cout << "PNG file verified successfully\n";

    png_structp image = png_create_read_struct(PNG_LIBPNG_VER_STRING,NULL,NULL,NULL);


    fclose(opened_file);
    return EXIT_SUCCESS;
}
