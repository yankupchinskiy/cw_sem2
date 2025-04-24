#include <png.h>
#include <iostream>
#include <getopt.h>
#include <zlib.h>
#include <string.h>
#include <cstdlib>
#include <string>

struct Line
{
    long start_x;
    long start_y= -1;

    long end_x;
    long end_y;

    long color_r;
    long color_g;
    long color_b;

    int thickness;
};

struct Mirror
{
    long top_x;
    long top_y;

    long bottom_x;
    long bottom_y;

    char axis;
};


struct Pentagram
{
    long top_x;
    long top_y;

    long bottom_x;
    long bottom_y;

    char axis;
};
int main(int argc, char *argv[]) {
    double thickness = 1.0;
    std::string color = "0.0.0";


    unsigned long start_x = -1, start_y = -1, end_x = -1, end_y = -1;
    char axis = 'n';
    unsigned long leftup_x = -1, leftup_y = -1, rightdown_x = -1, rightdown_y = -1;
    unsigned long center_x = -1, center_y = -1, radius = -1;

 
        static struct option long_options[] = {
        {"help",        no_argument,       0, 'h'},
        {"input",       required_argument, 0, 'i'},
        {"line",        no_argument,       0,  1},
        {"mirror",      no_argument,       0,  2},
        {"pentagram",   no_argument,       0,  3},
        {"thickness",   required_argument, 0, 't'},
        {"color",       required_argument, 0, 'c'},
        {"start",       required_argument, 0,  4},
        {"end",         required_argument, 0,  5},
        {"axis",        required_argument, 0,  6},
        {"left_up",     required_argument, 0,  7},
        {"right_down",  required_argument, 0,  8},
        {"center",      required_argument, 0,  9},
        {"radius",      required_argument, 0, 10},
        {0, 0, 0, 0}
    };


    int long_index = -1;
    int opt;
    long r = -1,g = -1,b = -1;

    bool line = false, mirror = false, pentagram = false;

    while ((opt = getopt_long(argc, argv, "hi:t:c:", long_options, &long_index)) != -1) {
        switch (opt) {
            case 'h':
                std::cout << "Usage: --line, --mirror, --pentagram with their params\n";
                return (0);

            case 'i':
                break;

            case 't':
                thickness = atof(optarg);
                if (thickness <= 0) {
                    std::cerr << "Ошибка: толщина должна быть больше 0\n";
                    return EXIT_FAILURE;
                }
                break;

            case 'c':
                if ((sscanf(optarg, "%ld.%ld.%ld", &r, &g, &b) != 3) || (r > 255) || (g > 255) || (b > 255)) {
                    std::cerr << "Ошибка в --color (ожидалось R.G.B)\n"; return EXIT_FAILURE;
                }
                break;

            case 1:
                line = true;
                break;

            case 2:  
                mirror = true; 
                break;

            case 3:  
                pentagram = true; 
                break;

            case 4: 
            if (sscanf(optarg, "%lu.%lu", &start_x, &start_y) != 2) {
                std::cerr << "Ошибка в --start\n"; return EXIT_FAILURE;
            }
                break;

            case 5: 
            if (sscanf(optarg, "%lu.%lu", &end_x, &end_y) != 2) {
                std::cerr << "Ошибка в --end\n"; return EXIT_FAILURE;
            }
                break;
            
            case 6: axis = optarg[0]; 
                break;
            
            case 7:
                if (sscanf(optarg, "%lu.%lu", &leftup_x, &leftup_y) != 2) {
                    std::cerr << "Ошибка в --left_up\n"; return EXIT_FAILURE;
                }
                break;
            
            case 8:
                if (sscanf(optarg, "%lu.%lu", &rightdown_x, &rightdown_y) != 2) {
                    std::cerr << "Ошибка в --right_down\n"; return EXIT_FAILURE;
                }
                break;
            
            case 9:
                if (sscanf(optarg, "%lu.%lu", &center_x, &center_y) != 2) {
                    std::cerr << "Ошибка в --center\n"; return EXIT_FAILURE;
                }
                break;
            
            case 10: radius = atof(optarg);
                     if (radius <= 0) {
                         std::cerr << "Ошибка: радиус должен быть больше 0\n";
                         return EXIT_FAILURE;
                     }
                     break;
            default:
            std::cout<<"Не было введено ни одного корректного ключа, для помощи --help"<<std::endl;
            exit(EXIT_FAILURE);
                break;
        }
    }

    if (!(line || mirror || pentagram)) {
        std::cerr << "Ошибка: не указано ни одной операции (--line, --mirror, --pentagram)\n";
        return EXIT_FAILURE;
    }

    if(line){
        Line ln = {
            (long)start_x, (long)start_y,
            (long)end_x, (long)end_y,
            r, g, b,
            (int)thickness
        };
        if (ln.start_x == -1||ln.start_y == -1||ln.end_x == -1||
            ln.end_y == -1||ln.color_r == -1||ln.color_g == -1||ln.color_b == -1){
                std::cout <<"как минимум один из необходимых программе флагов не был передан, для помощи --help"<<std::endl;
        }
    }

    if(mirror){
        Mirror mr = {
            (long)leftup_x, (long)leftup_y,
            (long)rightdown_x, (long)rightdown_y,
            (char)axis
        };
        if (mr.top_y ==-1 || mr.top_y ==-1 || mr.bottom_x ==-1 || mr.bottom_y == -1  || mr.axis != 'n'){
                std::cout <<"как минимум один из необходимых программе флагов не был передан, для помощи --help"<<std::endl;
        }
    }


    FILE* opened_file = fopen("test.png", "rb");
    if (!opened_file) {
        std::cerr << "file opening failed\n";
        return EXIT_FAILURE;
    }

    png_byte fst_eight[8];
    if (fread(fst_eight, 1, 8, opened_file) != 8 || png_sig_cmp(fst_eight, 0, 8)) {
        std::cerr << "Неверный PNG-файл\n";
        fclose(opened_file);
        return EXIT_FAILURE;
    }

    std::cout << "PNG file verified successfully\n";

    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) return EXIT_FAILURE;

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        png_destroy_read_struct(&png_ptr, NULL, NULL);
        return EXIT_FAILURE;
    }

    if (setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
        fclose(opened_file);
        return EXIT_FAILURE;
    }

    png_init_io(png_ptr, opened_file);
    png_set_sig_bytes(png_ptr, 8);
    png_read_info(png_ptr, info_ptr);

    png_uint_32 width, height;
    int bit_depth, color_type;
    png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, nullptr, nullptr, nullptr);

    png_read_update_info(png_ptr, info_ptr);
    int rowbytes = png_get_rowbytes(png_ptr, info_ptr);
    png_bytep* row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * height);

    for (int y = 0; y < height; y++) {
        row_pointers[y] = (png_byte*)malloc(rowbytes);
    }

    png_read_image(png_ptr, row_pointers);


    png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
    fclose(opened_file);
}