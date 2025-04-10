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

    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,NULL,NULL,NULL);
    if (!png_ptr){
        std::cout <<"png struct creation failed";
        return EXIT_FAILURE;
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);

    if (!info_ptr)
    {
       png_destroy_read_struct(&png_ptr, NULL, NULL);
       std::cout <<"png struct information failed";
       return EXIT_FAILURE;
    }

    if (setjmp(png_jmpbuf(png_ptr))) {
        std::cerr << "libpng error\n";
        png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
        fclose(opened_file);
        return EXIT_FAILURE;
    }

    png_init_io(png_ptr, opened_file);

    png_set_sig_bytes(png_ptr,8);
    
    png_read_info(png_ptr, info_ptr);

    fclose(opened_file);
    return EXIT_SUCCESS;
}
