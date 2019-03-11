#ifndef CHUCKLECORE_CHUCKLECORE_HPP
#define CHUCKLECORE_CHUCKLECORE_HPP

#include <Crunch/PerlinNoise.hpp>
#include <Crunch/Randomizer.hpp>
#include <Dab/Dab.hpp>
#include <Luke/Luke.hpp>
#include <Paper2/Document.hpp>
#include <Paper2/Group.hpp>
#include <Paper2/Path.hpp>
#include <Paper2/Symbol.hpp>
#include <Paper2/Tarp/TarpRenderer.hpp>
#include <Pic/Image.hpp>
#include <Stick/SystemClock.hpp>
#include <Stick/FixedArray.hpp>

namespace chuckle
{
using namespace stick;
using namespace crunch;
using namespace paper;
using namespace luke;
using namespace dab;
using namespace pic;

STICK_API PerlinNoise & noiseInstance();
STICK_API Randomizer & randomizerInstance();

STICK_API void setRandomSeed(typename Randomizer::IntegerType _seed);
STICK_API void randomizeSeed();
STICK_API Float32 randomf(Float32 _min = 0.0f, Float32 _max = 1.0f);
STICK_API Float64 randomd(Float64 _min = 0.0, Float64 _max = 1.0);
STICK_API Int32 randomi(Int32 _min = 0, Int32 _max = std::numeric_limits<Int32>::max());
STICK_API UInt32 randomui(UInt32 _min = 0, UInt32 _max = std::numeric_limits<UInt32>::max());

STICK_API void setNoiseSeed(Int32 _seed);
STICK_API void randomizeNoiseSeed();
STICK_API Float32 noise(Float32 _x);
STICK_API Float32 noise(Float32 _x, Float32 _y);
STICK_API Float32 noise(Float32 _x, Float32 _y, Float32 _z);
STICK_API Float32 noise(Float32 _x, Float32 _y, Float32 _z, Float32 _w);

class STICK_API RenderWindow : public Window
{
  public:
    using DrawFunction = std::function<Error(Float64)>;

    RenderWindow();
    virtual ~RenderWindow();

    Error open(const WindowSettings & _settings);
    ImageUniquePtr frameImage(UInt32 _x, UInt32 _y, UInt32 _w, UInt32 _h);
    ImageUniquePtr frameImage();
    Error saveFrame(const char * _path, UInt32 _x, UInt32 _y, UInt32 _w, UInt32 _h);
    Error saveFrame(const char * _path);
    RenderDevice & renderDevice() const;
    void setDrawFunction(DrawFunction _func);
    Error run();
    Float64 fps() const;
    Size frameCount() const;

  protected:
    RenderDevice * m_renderDevice;
    ImageUniquePtr m_tmpImage;
    DrawFunction m_drawFunc;
    SystemClock m_clock;
    Maybe<SystemClock::TimePoint> m_lastFrameTime;
    //helpers to compute FPS (simple moving average)
    FixedArray<Float64, 100> m_fpsBuffer;
    Size m_fpsIndex;
    Float64 m_fpsSMASum;
    Float64 m_fpsAvg;
    Size m_frameCount;
};

class STICK_API PaperWindow : public RenderWindow
{
  public:
    PaperWindow();
    Error open(const WindowSettings & _settings);
    Document & document();
    tarp::TarpRenderer & paperRenderer();
    void drawDocument(RenderPass * _pass);
    void setAutoResize(bool _b);
    bool autoResize() const;

  protected:
    Document m_doc;
    tarp::TarpRenderer m_paperRenderer;
    bool m_bAutoResize;
};

// This mainly houses paper related path utilities that don't really make sense to sit inside paper
// directly.
namespace path
{
STICK_API void longestCurves(Path * _path, Size _count, DynamicArray<Curve> & _output);

STICK_API void matchSegmentCount(Path * _a, Path * _b);

STICK_API void applyNoise(
    Item * _item, Float32 _noiseSeed, Float32 _noiseDiv, Float32 _noiseScale, Float32 _sampleDist);

STICK_API void morph(Path * _a, Path * _b, Float32 _t, Path * _output);
} // namespace path

} // namespace chuckle

#endif // CHUCKLECORE_CHUCKLECORE_HPP
