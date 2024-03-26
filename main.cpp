#include <stdio.h>
#include <cstdlib>
#include "model.h"
#include "graph_drawer.h"
#include "shape.h"

const char* output_path = "out/output.tga";

int gwidth  = 500;
int gheight = 500;
int scale_ratio = 250;

static void draw_triangle(Triangle triangle, TGAImage& image, TGAColor color = white, bool fill = true, bool debug = false)
{
    line(triangle.get_point1().x, triangle.get_point1().y, triangle.get_point2().x, triangle.get_point2().y, image, color);
    line(triangle.get_point2().x, triangle.get_point2().y, triangle.get_point3().x, triangle.get_point3().y, image, color);
    line(triangle.get_point3().x, triangle.get_point3().y, triangle.get_point1().x, triangle.get_point1().y, image, color);

    if (fill) 
        fillif(std::bind(&Triangle::inside, &triangle, std::placeholders::_1), image, triangle.bounding_box(), color);

    if (debug)
        printf("\ntriangle (%d, %d), (%d, %d), (%d, %d) is drawed.",
            triangle.get_point1().x, triangle.get_point1().y, 
            triangle.get_point2().x, triangle.get_point2().y, 
            triangle.get_point3().x, triangle.get_point3().y);
}

static void draw_model(Model model, TGAImage& image, TGAColor linecolor = white, bool fill = true, bool debug = false)
{
    if (scale_ratio > gwidth / 2 || scale_ratio > gheight / 2)
        printf("warning: too large scale_ratio: %d", scale_ratio);

    const int face_num = model.nfaces();

    for (int facei = 0; facei < face_num; facei++)
    {
        std::vector<int> face = model.face(facei);
        Vec2i triangle_vertex[3];

        for (int veci = 0; veci < 3; veci++) // vec3f has three element (x, y, z)
        {
            Vec3f v0 = model.vert(face[veci]);
            Vec3f v1 = model.vert(face[(veci + 1) % 3]);

            // map coordinates
            int x0 = (v0.x + 1.) * scale_ratio;
            int y0 = (v0.y + 1.) * scale_ratio;
            int x1 = (v1.x + 1.) * scale_ratio;
            int y1 = (v1.y + 1.) * scale_ratio;

            line(x0, y0, x1, y1, image, linecolor);
            triangle_vertex[veci] = Vec2i(x0, y0);
        }

        draw_triangle(Triangle(triangle_vertex), image, random_color(), fill);

        if (debug)
            printf("\ndrawing model triangle: [%d / %d]", facei, face_num);
    }
}

int main(int argc, char** argv)
{
    TGAImage image(gwidth, gheight, TGAImage::RGB);

    /*Triangle triangle1(Vec2i(100, 100), Vec2i(150, 300), Vec2i(350, 350));
    Triangle triangle2(Vec2i(200, 450), Vec2i(450, 450), Vec2i(100, 350));
    Triangle triangle3(Vec2i(400, 250), Vec2i(200, 50) , Vec2i(250, 150));

    draw_triangle(triangle1, image, white, true);
    draw_triangle(triangle2, image, green, true);
    draw_triangle(triangle3, image, red  , false);*/

    Model model("resources/african_head.obj");

    draw_model(model, image, white, false, false);

    image.flip_vertically();
    image.write_tga_file(output_path);

    return 0;
}