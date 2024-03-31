#include <stdio.h>
#include <cstdlib>
#include "model.h"
#include "graph_drawer.h"
#include "shape.h"

int gwidth  = 500;
int gheight = 500;
int scale_ratio = 250;

Vec3f global_light_direction(0, 0, -1);

static void draw_triangle(Triangle triangle, TGAImage& image, TGAColor color, bool fill, bool edge, bool debug)
{
    if (!edge && !fill)
        printf("warning: triangle has neither edge nor solid fill. you may cannot see anything on output.");

    if (edge)
    {
        line(triangle.get_point1(), triangle.get_point2(), image, color);
        line(triangle.get_point2(), triangle.get_point3(), image, color);
        line(triangle.get_point3(), triangle.get_point1(), image, color);
    }

    if (fill)
        fillif(std::bind(&Triangle::inside, &triangle, std::placeholders::_1), image, triangle.bounding_box(), color);

    if (debug)
        printf("\ntriangle (%d, %d), (%d, %d), (%d, %d) is drawed.",
            triangle.get_point1().x, triangle.get_point1().y, 
            triangle.get_point2().x, triangle.get_point2().y, 
            triangle.get_point3().x, triangle.get_point3().y);
}

static void draw_model(Model model, TGAImage& image, TGAColor linecolor, bool fill, bool edge, bool debug)
{
    if (scale_ratio > gwidth / 2 || scale_ratio > gheight / 2)
        printf("warning: too large scale_ratio: %d", scale_ratio);

    const int face_num = model.nfaces();

    for (int facei = 0; facei < face_num; facei++)
    {
        std::vector<int> face = model.face(facei);

        // current vertex batch
        // update at each face loop
        Vec3f world_vertexs[3];
        Vec2i screen_vertexs[3];

        for (int veci = 0; veci < 3; veci++) // vec3f has three element (x, y, z)
        {
            Vec3f world_curr = model.vert(face[veci]);
            Vec3f world_next = model.vert(face[(veci + 1) % 3]);

            // map world coord to screen
            Vec2i screen_curr = Vec2i((world_curr.x + 1.) * scale_ratio, (world_curr.y + 1.) * scale_ratio);
            Vec2i screen_next = Vec2i((world_next.x + 1.) * scale_ratio, (world_next.y + 1.) * scale_ratio);

            world_vertexs[veci] = world_curr;
            screen_vertexs[veci] = screen_curr;
        }

        TGAColor face_color = render_color(Triangle::calc_normal(world_vertexs[2] - world_vertexs[1], world_vertexs[1] - world_vertexs[0]), global_light_direction);

        if (face_color != clear)
            draw_triangle(Triangle(screen_vertexs), image, face_color, fill, edge, debug);

        if (debug)
            printf("\ndrawing model triangle: [%d / %d]", facei, face_num);
    }
}

static void output(TGAImage& image, const std::string filename)
{
    image.flip_vertically();
    image.write_tga_file(("out/" + filename).c_str());
}

int main(int argc, char** argv)
{
    /*Model model("resources/african_head.obj");
    draw_model(model, image, white, true, true, false);*/

    constexpr int yb_width = 800;

    TGAImage line_scene (yb_width, gheight, TGAImage::RGB);
    TGAImage depth_scene(yb_width, 16,      TGAImage::RGB);

    line(Vec2i( 20,  34), Vec2i(744, 400), line_scene, red);
    line(Vec2i(120, 434), Vec2i(444, 400), line_scene, green);
    line(Vec2i(330, 463), Vec2i(594, 200), line_scene, blue);
    
    int ybuffer[yb_width];

    // init ybuffer
    for (int x = 0; x < yb_width; x++)
        ybuffer[x] = std::numeric_limits<int>::min();

    rasterize2d(Vec2i( 20,  34), Vec2i(744, 400), ybuffer, std::size(ybuffer), line_scene, red);
    rasterize2d(Vec2i(120, 434), Vec2i(444, 400), ybuffer, std::size(ybuffer), line_scene, green);
    rasterize2d(Vec2i(330, 463), Vec2i(594, 200), ybuffer, std::size(ybuffer), line_scene, blue);

    // draw ybuffer graph
    for (int x = 0; x < yb_width; x++)
    {
        TGAColor render_color = cyan;

        if (ybuffer[x] > std::numeric_limits<int>::min())
            render_color = white * (ybuffer[x] / (float)gheight);

        for (int y = 1; y < 16; y++)
            depth_scene.set(x, y, render_color);
    }

    output(line_scene, "line.tga");
    output(depth_scene, "depth.tga");

    return 0;
}