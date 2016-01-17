// Ref.: https://github.com/prideout/par/blob/master/test/test_msquares.c
#include <stdio.h>

extern "C" {
#undef __cplusplus
#include "lodepng.h"
#define __cplusplus
}

#define PAR_MSQUARES_IMPLEMENTATION
#include "par_msquares.h"

#define CELLSIZE 8 /*32*/
#define INSIDE_COLOR 0x5B9CD7
#define OUTSIDE_COLOR 0xFFFFFFFF

static void svg_begin(FILE* svgfile)
{
    fputs(
        "<svg viewBox='-.1 -.1 1.2 1.2' width='500px' height='500px' "
        "version='1.1' "
        "xmlns='http://www.w3.org/2000/svg'>\n"
        "<g transform='translate(0 1) scale(1 -1)'>", svgfile);
}

static void svg_write_path(FILE* svgfile, par_msquares_mesh const* mesh, uint32_t color, float fill, float stroke)
{
    par_msquares_boundary* polygon = par_msquares_extract_boundary(mesh);
    fprintf(svgfile, "<path\n"
        " fill='#%6.6x'\n"
        " stroke='#%6.6x'\n"
        " stroke-width='0.005'\n"
        " fill-opacity='%f'\n"
        " stroke-opacity='%f'\n"
        " d='", color, color, fill, stroke);
    for (int c = 0; c < polygon->nchains; c++) {
        float const* chain = polygon->chains[c];
        int length = (int) polygon->lengths[c];
        uint16_t first = 1;
        for (uint16_t s = 0; s < length; s++) {
            fprintf(svgfile, "%c%f,%f", first ? 'M' : 'L', chain[s * 2],
                chain[s * 2 + 1]);
            first = 0;
        }
        fputs("Z", svgfile);
    }
    fputs("'\n/>", svgfile);
    par_msquares_free_boundary(polygon);
}

int main(int argc, char* argv[])
{
    unsigned char* pixels;
    unsigned img_width = 0;
    unsigned img_height = 0;
    lodepng_decode_file(&pixels, &img_width, &img_height, "./res/twitter_bird.png", LCT_RGBA, 8);

    // Flip pixels
    {
        unsigned char* pixels_rev = (unsigned char*)malloc(sizeof(unsigned char) * img_width * img_height * 4);
        for (int r = img_height - 1; r >= 0; --r ) {
            for (int c = img_width - 1; c >= 0; --c ) {
                unsigned int offset_org = 4 * (r * img_width + c);
                unsigned int offset_rev = 4 * ((img_height - 1 - r) * img_width + (img_width - 1 - c));
                pixels_rev[offset_rev + 0] = pixels[offset_org + 0];
                pixels_rev[offset_rev + 1] = pixels[offset_org + 1];
                pixels_rev[offset_rev + 2] = pixels[offset_org + 2];
                pixels_rev[offset_rev + 3] = pixels[offset_org + 3];
            }
        }
        free(pixels);
        pixels = pixels_rev;
    }

    int flags = 0; // PAR_MSQUARES_SIMPLIFY;
    par_msquares_meshlist* mlist = par_msquares_color(pixels, img_width, img_height, CELLSIZE, OUTSIDE_COLOR, 4, flags);
    par_msquares_mesh const* mesh = par_msquares_get_mesh(mlist, 0);

    // Output mesh as SVG
    FILE* svgfile = fopen("./build/twitter_bird.svg", "wt");
    svg_begin(svgfile);
    svg_write_path(svgfile, mesh, INSIDE_COLOR, 0.5, 1.0);
    fputs("</g>\n</svg>", svgfile);
    fclose(svgfile);

    // Output mesh as OBJ
    if (argc > 1 && strcmp(argv[1], "-obj") == 0) {
        FILE* objfile = fopen("./build/twitter_bird.obj", "wt");
        float* pt = mesh->points;
        for (int i = 0; i < mesh->npoints; i++) {
            float z = mesh->dim > 2 ? pt[2] : 0;
            fprintf(objfile, "v %f %f %f\n", pt[0], pt[1], z);
            pt += mesh->dim;
        }
        uint16_t* index = mesh->triangles;
        for (int i = 0; i < mesh->ntriangles; i++) {
            int a = 1 + *index++;
            int b = 1 + *index++;
            int c = 1 + *index++;
            fprintf(objfile, "f %d/%d %d/%d %d/%d\n", a, a, b, b, c, c);
        }
        fclose(objfile);
    }

    par_msquares_free(mlist);

    free(pixels);

    return 0;
}
