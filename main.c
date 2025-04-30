#include <png.h>
#include <stdio.h>
#include <getopt.h>
#include <zlib.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

struct Png{
    int width, height;
    png_byte color_type;
    png_byte bit_depth;
    png_structp png_ptr;
    png_infop info_ptr;
    int number_of_passes;
    png_bytep *row_pointers;
   }; 

typedef struct Line
{
    long start_x;
    long start_y;

    long end_x;
    long end_y;

    long color_r;
    long color_g;
    long color_b;

    int thickness;
} Line;

typedef struct Mirror
{
    long top_x;
    long top_y;

    long bottom_x;
    long bottom_y;

    char axis;
} Mirror;


typedef struct Pentagram
{
    long center_x;
    long center_y;

    int radius;

    long color_r;
    long color_g;
    long color_b;

    int thickness;

} Pentagram;

void set_pixel(png_bytep* row_pointers, int x, int y, png_byte r, png_byte g, png_byte b) {
    if (x < 0 || y < 0 || !row_pointers || !row_pointers[y]) return;
    png_bytep row = row_pointers[y];
    png_bytep ptr = &(row[x * 3]);
    ptr[0] = r;
    ptr[1] = g;
    ptr[2] = b;
}


int read_png(const char *filename, png_bytep **row_pointers, png_uint_32 *width, png_uint_32 *height, int *bit_depth, int *color_type) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        fprintf(stderr, "Ошибка открытия файла на чтение: %s\n", filename);
        return -1;
    }

    png_byte header[8];
    if (fread(header, 1, 8, fp) != 8 || png_sig_cmp(header, 0, 8)) {
        fprintf(stderr, "Неверный PNG-файл\n");
        fclose(fp);
        return -1;
    }

    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) {
        fclose(fp);
        return -1;
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        png_destroy_read_struct(&png_ptr, NULL, NULL);
        fclose(fp);
        return -1;
    }

    if (setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        fclose(fp);
        return -1;
    }

    png_init_io(png_ptr, fp);
    png_set_sig_bytes(png_ptr, 8);
    png_read_info(png_ptr, info_ptr);

    png_get_IHDR(png_ptr, info_ptr, width, height, bit_depth, color_type, NULL, NULL, NULL);

    int rowbytes = png_get_rowbytes(png_ptr, info_ptr);
    *row_pointers = (png_bytep *)malloc(sizeof(png_bytep) * (*height));
    if (!(*row_pointers)) {
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        fclose(fp);
        return -1;
    }

    for (png_uint_32 y = 0; y < *height; y++) {
        (*row_pointers)[y] = (png_byte *)malloc(rowbytes);
        if (!(*row_pointers)[y]) {
            for (png_uint_32 i = 0; i < y; i++) {
                free((*row_pointers)[i]);
            }
            free(*row_pointers);
            png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
            fclose(fp);
            return -1;
        }
    }

    png_read_image(png_ptr, *row_pointers);

    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    fclose(fp);

    return 0;
}

int write_png(const char *filename, png_bytep *row_pointers, png_uint_32 width, png_uint_32 height, int bit_depth, int color_type) {
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        fprintf(stderr, "Ошибка открытия файла на запись: %s\n", filename);
        return -1;
    }

    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) {
        fclose(fp);
        return -1;
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        png_destroy_write_struct(&png_ptr, NULL);
        fclose(fp);
        return -1;
    }

    if (setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_write_struct(&png_ptr, &info_ptr);
        fclose(fp);
        return -1;
    }

    png_init_io(png_ptr, fp);

    png_set_IHDR(
        png_ptr, info_ptr,
        width, height,
        bit_depth, color_type,
        PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_DEFAULT,
        PNG_FILTER_TYPE_DEFAULT
    );

    png_write_info(png_ptr, info_ptr);
    png_write_image(png_ptr, row_pointers);
    png_write_end(png_ptr, NULL);

    png_destroy_write_struct(&png_ptr, &info_ptr);
    fclose(fp);

    return 0;
}

void draw_line(png_bytep *row_pointers, Line line){
    const int deltax = abs(line.end_x - line.start_x);
    const int deltay = abs(line.end_y - line.start_y);
    const int signX = line.start_x < line.end_x ? 1 : -1;
    const int signY = line.start_y < line.end_y ? 1 : -1;
    int error = deltax - deltay;
    set_pixel(row_pointers,line.end_x,line.end_y,line.color_r, line.color_g, line.color_b);
    while(line.start_x != line.end_x || line.start_y != line.end_y){
        set_pixel(row_pointers,line.start_x, line.start_y,line.color_r, line.color_g, line.color_b);
        int error2 = error * 2;
    
        if(error2 > -deltay){
            error -= deltay;
            line.start_x -= signX;
        }

        if(error2 < deltax){
            error += deltax;
            line.start_y -= signY;
        }
    }
    }



int main(int argc, char *argv[]) {

    int thickness = -1;
    long start_x = -1, start_y = -1, end_x = -1, end_y = -1;
    char axis = 'n';
    unsigned long leftup_x = -1, leftup_y = -1, rightdown_x = -1, rightdown_y = -1;
    unsigned long center_x = -1, center_y = -1, radius = -1;
    int long_index = -1;
    int opt;
    long r = -1,g = -1,b = -1;
    bool line = false, mirror = false, pentagram = false;
 
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


    while ((opt = getopt_long(argc, argv, "hi:t:c:", long_options, &long_index)) != -1) {
        switch (opt) {
            case 'h':
                fprintf(stderr,"Usage: --line, --mirror, --pentagram with their params\n");
                return (0);

            case 'i':
                break;

            case 't':
                thickness = atoi(optarg);
                if (thickness <= 0) {
                    fprintf(stderr,"Ошибка: толщина должна быть больше 0\n");
                    return EXIT_FAILURE;
                }
                break;

            case 'c':
                if ((sscanf(optarg, "%ld.%ld.%ld", &r, &g, &b) != 3) ||(r < 0 || g < 0 || b < 0 || r > 255 || g > 255 || b > 255)) 
{
                    fprintf(stderr,"Ошибка в --color (ожидалось R.G.B)\n"); return EXIT_FAILURE;
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
                fprintf(stderr,"Ошибка в --start\n"); return EXIT_FAILURE;
            }
                break;

            case 5: 
            if (sscanf(optarg, "%lu.%lu", &end_x, &end_y) != 2) {
                fprintf(stderr,"Ошибка в --end\n"); return EXIT_FAILURE;
            }
                break;
            
            case 6: axis = optarg[0]; 
                break;
            
            case 7:
                if (sscanf(optarg, "%lu.%lu", &leftup_x, &leftup_y) != 2) {
                    fprintf(stderr,"Ошибка в --left_up\n"); return EXIT_FAILURE;
                }
                break;
            
            case 8:
                if (sscanf(optarg, "%lu.%lu", &rightdown_x, &rightdown_y) != 2) {
                    fprintf(stderr,"Ошибка в --right_down\n"); return EXIT_FAILURE;
                }
                break;
            
            case 9:
                if (sscanf(optarg, "%lu.%lu", &center_x, &center_y) != 2) {
                    fprintf(stderr,"Ошибка в --center\n"); return EXIT_FAILURE;
                }
                break;
            
            case 10: radius = atoi(optarg);
                     if (radius <= 0) {
                         fprintf(stderr,"Ошибка: радиус должен быть больше 0\n");
                         return EXIT_FAILURE;
                     }
                     break;
            default:
                fprintf(stderr,"Не было введено ни одного корректного ключа, для помощи --help");
                exit(EXIT_FAILURE);
                break;
        }
    }

    if (!(line || mirror || pentagram)) {
        fprintf(stderr,"Ошибка: не указано ни одной операции (--line, --mirror, --pentagram)\n");
        return EXIT_FAILURE;
    }

    if ((line + mirror + pentagram) != 1 ){
        fprintf(stderr, "было введено более одного флага функции\n");
        exit (EXIT_FAILURE);
    }
    Line ln;
    Mirror mr;
    Pentagram pg;

    if(line){
        
        ln.start_x = start_x;
        ln.start_y = start_y;
        ln.end_x = end_x;
        ln.end_y = end_y;        
        ln.color_r = r,ln.color_g = g,ln.color_b = b;
        ln.thickness = thickness;


        if (ln.start_x == -1||ln.start_y == -1||ln.end_x == -1||
            ln.end_y == -1||ln.color_r == -1||ln.color_g == -1||ln.color_b == -1||ln.thickness ==-1){
                fprintf(stderr, "как минимум один из необходимых программе флагов не был передан, для помощи --help");
                exit (EXIT_FAILURE);
     
        }
    }

    if (mirror) {
        mr.top_x = leftup_x;
        mr.top_y = leftup_y;
        mr.bottom_x = rightdown_x;
        mr.bottom_y = rightdown_y;
        mr.axis = axis;
    
        if (mr.top_x == -1 || mr.top_y == -1 || mr.bottom_x == -1 || mr.bottom_y == -1 || (mr.axis != 'x' && mr.axis != 'y')) {
            fprintf(stderr, "как минимум один из необходимых программе флагов не был передан, для помощи --help");
            exit(EXIT_FAILURE);
        }
    }
    

    if(pentagram){

            pg.center_x =center_x, pg.center_y =center_y;
            pg.radius = radius;
            pg.color_r = r,pg.color_g = g,pg.color_b = b;
            pg.thickness = thickness;
        if (pg.center_x == -1||pg.center_y == -1||pg.color_r == -1||pg.color_g == -1||pg.color_b == -1|| pg.thickness == -1 || pg.radius == -1){
                fprintf(stderr,"как минимум один из необходимых программе флагов не был передан, для помощи --help");
                exit (EXIT_FAILURE);
        }
    }


    if(optind < argc){
        char* filename = argv[optind];
    }
    

    png_bytep *row_pointers;
    png_uint_32 width, height;
    int bit_depth, color_type;

    if (read_png(argv[optind], &row_pointers, &width, &height, &bit_depth, &color_type) != 0) {
        return EXIT_FAILURE;
    }

    if(line){
        draw_line(row_pointers,ln);
    }


    if (write_png("output.png", row_pointers, width, height, bit_depth, color_type) != 0) {
        return EXIT_FAILURE;
    }
    
         // Освобождаем память
         for (int y = 0; y < height; y++) {
             free(row_pointers[y]);
         }
         free(row_pointers);
     
         return 0;
     }

