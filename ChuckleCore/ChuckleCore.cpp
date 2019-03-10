#include <ChuckleCore/ChuckleCore.hpp>

namespace chuckle
{

PerlinNoise & noiseInstance()
{
    static PerlinNoise s_noise;
    return s_noise;
}

Randomizer & randomizerInstance()
{
    static Randomizer s_rnd;
    return s_rnd;
}

void setRandomSeed(typename Randomizer::IntegerType _seed)
{
    randomizerInstance().setSeed(_seed);
}

void randomizeSeed()
{
    randomizerInstance().randomizeSeed();
}

Float32 randomf(Float32 _min, Float32 _max)
{
    return randomizerInstance().randomf(_min, _max);
}

Float64 randomd(Float64 _min, Float64 _max)
{
    return randomizerInstance().randomd(_min, _max);
}

Int32 randomi(Int32 _min, Int32 _max)
{
    return randomizerInstance().randomi(_min, _max);
}

UInt32 randomui(UInt32 _min, UInt32 _max)
{
    return randomizerInstance().randomui(_min, _max);
}

void setNoiseSeed(Int32 _seed)
{
    noiseInstance().setSeed(_seed);
}

void randomizeNoiseSeed()
{
    noiseInstance().randomize();
}

Float32 noise(Float32 _x)
{
    return noiseInstance().noise(_x);
}

Float32 noise(Float32 _x, Float32 _y)
{
    return noiseInstance().noise(_x, _y);
}

Float32 noise(Float32 _x, Float32 _y, Float32 _z)
{
    return noiseInstance().noise(_x, _y, _z);
}

Float32 noise(Float32 _x, Float32 _y, Float32 _z, Float32 _w)
{
    return noiseInstance().noise(_x, _y, _z, _w);
}

RenderWindow::RenderWindow() : m_renderDevice(nullptr)
{
}

Error RenderWindow::open(const WindowSettings & _settings)
{
    Error ret = Window::open(_settings);
    if (ret)
        return ret;
    auto res = createRenderDevice();
    if (!res)
        return res.error();
    m_renderDevice = res.get();
    m_tmpImage = makeUnique<ImageRGBA8>(widthInPixels(), heightInPixels());
    return Error();
}

ImageUniquePtr RenderWindow::frameImage(UInt32 _x, UInt32 _y, UInt32 _w, UInt32 _h)
{
    ImageUniquePtr img = makeUnique<ImageRGBA8>(_w, _h);
    m_renderDevice->readPixels(_x, _y, _w, _h, TextureFormat::RGBA8, (void *)img->bytePtr());
    return img;
}

ImageUniquePtr RenderWindow::frameImage()
{
    return frameImage(0, 0, widthInPixels(), heightInPixels());
}

Error RenderWindow::saveFrame(const char * _path, UInt32 _x, UInt32 _y, UInt32 _w, UInt32 _h)
{
    m_tmpImage->resize(_w, _h);
    m_renderDevice->readPixels(_x, _y, _w, _h, TextureFormat::RGBA8, (void *)m_tmpImage->bytePtr());
    return m_tmpImage->save(_path);
}

Error RenderWindow::saveFrame(const char * _path)
{
    return saveFrame(_path, 0, 0, widthInPixels(), heightInPixels());
}

RenderDevice & RenderWindow::renderDevice() const
{
    STICK_ASSERT(m_renderDevice);
    return *m_renderDevice;
}

void RenderWindow::setDrawFunc(DrawFunction _func)
{
    m_drawFunc = _func;
}

Error RenderWindow::run()
{
    if (!m_drawFunc)
        return Error(ec::InvalidOperation,
                     "Attempting to run with no draw function",
                     STICK_FILE,
                     STICK_LINE);

    while (!shouldClose())
    {
        auto now = m_clock.now();
        Float64 dur = m_lastFrameTime ? (now - *m_lastFrameTime).seconds() : 1.0 / 60.0;
        luke::pollEvents();
        m_renderDevice->beginFrame();
        Error err = m_drawFunc(dur);
        if (err)
            return err;
        err = m_renderDevice->endFrame();
        if (err)
            return err;
        m_lastFrameTime = now;
        Window::swapBuffers();
    }

    return Error();
}

PaperWindow::PaperWindow() :
m_bAutoResize(true)
{
}

Error PaperWindow::open(const WindowSettings & _settings)
{
    this->addEventCallback([this](const WindowResizeEvent & _evt) {
        if (this->m_bAutoResize)
        {
            this->m_doc.setSize(_evt.width(), _evt.height());
            this->m_paperRenderer.setViewport(0, 0, this->widthInPixels(), this->heightInPixels());
        }
    });

    Error err = RenderWindow::open(_settings);
    if (err)
        return err;

    return m_paperRenderer.init(m_doc);
}

Document & PaperWindow::document()
{
    return m_doc;
}

const Document & PaperWindow::document() const
{
    return m_doc;
}

tarp::TarpRenderer & PaperWindow::paperRenderer()
{
    return m_paperRenderer;
}

const tarp::TarpRenderer & PaperWindow::paperRenderer() const
{
    return m_paperRenderer;
}

void PaperWindow::drawDocument(RenderPass * _pass)
{
    _pass->drawCustom([this] { return m_paperRenderer.draw(); });
}

void PaperWindow::setAutoResize(bool _b)
{
    m_bAutoResize = _b;
}

bool PaperWindow::autoResize() const
{
    return m_bAutoResize;
}

namespace path
{
void longestCurves(Path * _path, Size _count, DynamicArray<Curve> & _output)
{
    _output.reserve(_path->curves().count());
    for (auto curve : _path->curves())
    {
        STICK_ASSERT(curve.path());
        _output.append(curve);
    }

    std::sort(_output.begin(), _output.end(), [](const Curve & _a, const Curve & _b) {
        return _a.length() > _b.length();
    });
    if (_output.count() > _count)
        _output.resize(_count);
}

void matchSegmentCount(Path * _a, Path * _b)
{
    if (_a->segmentCount() == _b->segmentCount())
        return;

    if (_a->segmentCount() > _b->segmentCount())
        std::swap(_a, _b);

    DynamicArray<Curve> tmp;
    while (_a->segmentCount() < _b->segmentCount())
    {
        Size diff = _b->segmentCount() - _a->segmentCount();
        longestCurves(_a, diff, tmp);

        for (Curve curve : tmp)
        {
            STICK_ASSERT(curve.path());
            curve.divideAtParameter(0.5);
        }
    }
}

void applyNoise(
    Item * _item, Float32 _noiseSeed, Float32 _noiseDiv, Float32 _noiseScale, Float32 _sampleDist)
{
    if (_item->itemType() == ItemType::Path)
    {
        Path * p = static_cast<Path *>(_item);
        if (p->length() < _sampleDist)
            return;

        p->flattenRegular(_sampleDist, false);
        for (Segment seg : p->segments())
        {
            Vec2f pos = seg.position();
            Float32 n1 = noise(pos.x / _noiseDiv, pos.y / _noiseDiv, _noiseSeed);
            Float32 ang = n1 * crunch::Constants<Float32>::twoPi();
            pos += Vec2f(std::cos(ang) * _noiseScale, std::sin(ang) * _noiseScale);
            seg.setPosition(pos);
        }
    }

    for (Item * child : _item->children())
        applyNoise(child, _noiseSeed, _noiseDiv, _noiseScale, _sampleDist);
}

template <class T, class B>
T mix(const T & _a, const T & _b, B _fact)
{
    return _a + (_b - _a) * _fact;
}

void morph(Path * _a, Path * _b, Float32 _t, Path * _output)
{
    _output->removeSegments();
    for (Size i = 0; i < _a->segmentCount(); ++i)
    {
        Segment a = _a->segment(i);
        Segment b = _b->segment(i);
        _output->addSegment(mix(a.position(), b.position(), _t),
                            mix(a.handleIn(), b.handleIn(), _t),
                            mix(a.handleOut(), b.handleOut(), _t));
    }

    if (_a->isClosed())
        _output->closePath();
}
} // namespace path

} // namespace chuckle
