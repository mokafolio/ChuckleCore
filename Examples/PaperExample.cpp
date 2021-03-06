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
    {
        PaperWindow window;
        WindowSettings settings;
        settings.setTitle("Yoyoyoyo");
        settings.setSampleCount(4);
        RETURN_ON_ERR(window.open(settings));
        window.setVerticalSync(true);
        RETURN_ON_ERR(window.enableDefaultUI());
        window.setShowWindowMetrics(true);

        Document & doc = window.document();
        auto & pr = window.paperRenderer();

        Path * circle = doc.createCircle(Vec2f(window.width() * 0.5, window.height() * 0.5), 100);
        circle->setFill("red");

        Path * circle2 = doc.createCircle(Vec2f(window.width() * 0.5, window.height() * 0.5), 50);
        circle2->setFill("yellow");
        circle->addChild(circle2);

        circle->translateTransform(100.323, 0);
        circle2->translateTransform(100.26246, 0);

        Vec2f * verts;
        Size count;
        pr.flattenedPathVertices(circle2, &verts, &count, circle->transform());
        DynamicArray<Vec2f> vertsa;
        vertsa.append(verts, verts + count);

        DynamicArray<Vec2f> handles;
        for (auto seg : circle->segments())
        {
            handles.append(seg.handleInAbsolute());
            handles.append(seg.handleOutAbsolute());
        }

        printf("handles.count() %lu\n", handles.count());

        window.setDrawFunction([&](Float64 _deltaTime) {
            RenderDevice & rd = window.renderDevice();
            RenderPass * pass = rd.beginPass(ClearSettings(0, 0, 0, 1));
            window.drawDocument(pass);

            window.quickDraw().setColor(ColorRGBA(1.0f, 1.0f, 1.0f, 1.0f));
            window.quickDraw().rects(&handles[0], handles.count(), 10);
            window.quickDraw().setColor(ColorRGBA(1.0f, 0.0f, 0.0f, 1.0f));
            window.quickDraw().lines(&handles[0], handles.count());

            window.drawPathOutline(circle, ColorRGBA(1, 1, 0, 1));
            window.drawItemBoundingBox(circle, ColorRGBA(1, 0, 1, 1), true);

            window.quickDraw().setColor(ColorRGBA(1.0f, 1.0f, 0.0f, 1.0f));
            window.quickDraw().lineCircle(100, 100, 100);

            return rd.endPass(pass);
        });

        RETURN_ON_ERR(window.run());
    }
    printf("the end \n");

    return EXIT_SUCCESS;
}