#include <ChuckleCore/ChuckleCore.hpp>

using namespace chuckle;

#define RETURN_ON_ERR(_err)                                                                        \
    if (_err)                                                                                      \
    {                                                                                              \
        printf("Error: %s\n", _err.message().cString());                                           \
        return EXIT_FAILURE;                                                                       \
    }

int main(int _argc, const char * _args[])
{
    PaperWindow window;
    WindowSettings settings;
    settings.setTitle("Yoyoyoyo");
    RETURN_ON_ERR(window.open(settings));
    window.setVerticalSync(true);
    RETURN_ON_ERR(window.enableDefaultUI());

    Document & doc = window.document();
    auto & pr = window.paperRenderer();

    Path * circle = doc.createCircle(Vec2f(window.width() * 0.5, window.height() * 0.5), 100);
    circle->setFill("red");

    Path * circle2 = doc.createCircle(Vec2f(window.width() * 0.5, window.height() * 0.5), 50);
    circle2->setFill("yellow");
    circle->addChild(circle2);

    circle->translateTransform(100, 0);
    circle2->translateTransform(100, 0);

    Vec2f * verts;
    Size count;
    pr.flattenedPathVertices(circle2, &verts, &count, circle->transform());
    DynamicArray<Vec2f> vertsa;
    vertsa.append(verts, verts + count);

    DynamicArray<Vec2f> handles;
    for(auto seg : circle->segments())
    {
        handles.append(seg.handleInAbsolute());
        handles.append(seg.handleOutAbsolute());
    }

    printf("handles.count() %lu\n", handles.count());

    window.setDrawFunction([&](Float64 _deltaTime)
    {
        RenderDevice & rd = window.renderDevice();
        RenderPass * pass = rd.beginPass(ClearSettings(0, 0, 0, 1));
        window.drawDocument(pass);
        rd.endPass(pass);

        window.quickDraw().setColor(ColorRGBA(0, 1, 0, 1));
        window.drawPathOutline(circle, ColorRGBA(1, 1, 0, 1));
        window.drawPathHandles(circle, ColorRGBA(0, 0, 1, 1));
        auto bounds = circle2->bounds();

        return Error();
    });

    RETURN_ON_ERR(window.run());

    return EXIT_SUCCESS;
}