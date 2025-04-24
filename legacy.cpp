#include <png.h>
#include <iostream>
#include <getopt.h>
#include <zlib.h>
#include <string.h>
#include <cstdlib>

int main(int argc, char *argv[]){
    double thickness = 1.0;
    std::string color = "0.0.0";
    bool do_line = false, do_mirror = false, do_pentagram = false;
    double start_x = 0, start_y = 0, end_x = 0, end_y = 0;
    char axis = 'x';
    double left = 0, up = 0, right = 0, down = 0;
    double center_x = 0, center_y = 0, radius = 0;
    auto parse_point = [](const char* s, double& x, double& y) {
        sscanf(s, "%lf.%lf", &x, &y);
    };
    
    static struct option long_options[] = {
        {"help",        no_argument,       0, 'h'},
        {"input",       required_argument, 0, 'i'},
    
        // Операции
        {"line",        no_argument,       0,  1},
        {"mirror",      no_argument,       0,  2},
        {"pentagram",   no_argument,       0,  3},
    
        // Общие
        {"thickness",   required_argument, 0, 't'},
        {"color",       required_argument, 0, 'c'},
    
        // Line
        {"start",       required_argument, 0,  4},
        {"end",         required_argument, 0,  5},
    
        // Mirror
        {"axis",        required_argument, 0,  6},
        {"left_up",     required_argument, 0,  7},
        {"right_down",  required_argument, 0,  8},
    
        // Pentagram
        {"center",      required_argument, 0,  9},
        {"radius",      required_argument, 0, 10},
    
        {0, 0, 0, 0}
    };
    
    int long_index = 0;
    int opt;
    while ((opt = getopt_long(argc, argv, "hi:t:c:", long_options, &long_index)) != -1) {
        switch (opt) {
            case 'h':
                std::cout << "Usage: --line, --mirror, --pentagram with their params\n";
                exit(0);
            case 'i':
                // можно использовать optarg как имя файла, если нужно
                break;
            case 't':
                thickness = atof(optarg);
                break;
            case 'c':
                color = optarg;
                break;
            case 1: do_line = true; break;
            case 2: do_mirror = true; break;
            case 3: do_pentagram = true; break;
            case 4: parse_point(optarg, start_x, start_y); break;
            case 5: parse_point(optarg, end_x, end_y); break;
            case 6: axis = optarg[0]; break;
            case 7: parse_point(optarg, left, up); break;
            case 8: parse_point(optarg, right, down); break;
            case 9: parse_point(optarg, center_x, center_y); break;
            case 10: radius = atof(optarg); break;
            default: break;
        }
    }

    if (thickness <= 0) {
        std::cerr << "Ошибка: толщина должна быть положительным числом\n";
        return EXIT_FAILURE;
    }

    int r, g, b;
if (sscanf(color.c_str(), "%d.%d.%d", &r, &g, &b) != 3 ||
    r < 0 || r > 255 || g < 0 || g > 255 || b < 0 || b > 255) {
    std::cerr << "Ошибка: цвет должен быть в формате rrr.ggg.bbb с диапазоном 0–255\n";
    return EXIT_FAILURE;
}


    

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

    png_uint_32 width, height;
    int bit_depth, color_type;
    png_get_IHDR(
        png_ptr, info_ptr,
        &width, &height,
        &bit_depth, &color_type,
        nullptr, nullptr, nullptr
    );
    
    png_read_update_info(png_ptr, info_ptr);

    int rowbytes = png_get_rowbytes(png_ptr, info_ptr);
    png_bytep* row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * height);
    
    for (int y = 0; y < height; y++) {
        row_pointers[y] = (png_byte*)malloc(rowbytes);
    }

    if (do_line) {
        std::cout << "[LINE] From (" << start_x << ", " << start_y << ") to ("
                  << end_x << ", " << end_y << ") Color: " << color
                  << " Thickness: " << thickness << "\n";
    }
    
    if (do_mirror) {
        std::cout << "[MIRROR] Axis: " << axis << ", Rect: ("
                  << left << ", " << up << ") to (" << right << ", " << down << ")\n";
    }
    
    if (do_pentagram) {
        std::cout << "[PENTAGRAM] Center: (" << center_x << ", " << center_y
                  << ") Radius: " << radius << " Color: " << color
                  << " Thickness: " << thickness << "\n";
    }
    
    
    png_read_image(png_ptr, row_pointers);
    

    png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
    fclose(opened_file);
}
