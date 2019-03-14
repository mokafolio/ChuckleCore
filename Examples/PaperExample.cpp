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
    RETURN_ON_ERR(window.open(WindowSettings()));
    window.setVerticalSync(true);
    RETURN_ON_ERR(window.enableDefaultUI());
    
    Document & doc = window.document();

    Path * circle = doc.createCircle(Vec2f(window.width() * 0.5, window.height() * 0.5), 100);
    circle->setFill("red");

    window.setDrawFunction([&](Float64 _deltaTime)
    {
        RenderDevice & rd = window.renderDevice();
        RenderPass * pass = rd.beginPass(ClearSettings(0, 0, 0, 1));
        window.drawDocument(pass);
        rd.endPass(pass);
        return Error();
    });

    RETURN_ON_ERR(window.run());

    return EXIT_SUCCESS;
}