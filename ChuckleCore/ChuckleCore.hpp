#ifndef CHUCKLECORE_CHUCKLECORE_HPP
#define CHUCKLECORE_CHUCKLECORE_HPP

#include <Crunch/MatrixFunc.hpp>
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
#include <Stick/FixedArray.hpp>
#include <Stick/SystemClock.hpp>

//@TODO: do we really need to include imgui here? maybe forward declare some stuff instead?
#include "imgui.h"

namespace chuckle
{
using namespace stick;
using namespace crunch;
using namespace paper;
using namespace luke;
using namespace dab;
using namespace pic;

// to resolve ambiguity between crunch::LineSegment and paper::LineSegment
using LineSegment = paper::LineSegment;

STICK_API_ENUM_CLASS(FileDialogFlags){ Open = 1 << 0,      // Create an open file dialog.
                                       Save = 1 << 1,      // Create a save file dialog.
                                       Directory = 1 << 2, // Open a directory.
                                       OverwriteConfirmation = 1 << 3 };

static constexpr bool enableBitmaskOperators(FileDialogFlags)
{
    return true;
}

STICK_API PerlinNoise & noiseInstance();
STICK_API Randomizer & randomizerInstance();

STICK_API void setRandomSeed(typename Randomizer::IntegerType _seed);
STICK_API void randomizeSeed();
STICK_API Float32 randomf(Float32 _min = 0.0f, Float32 _max = 1.0f);
STICK_API Float64 randomd(Float64 _min = 0.0, Float64 _max = 1.0);
STICK_API Int32 randomi(Int32 _min = 0, Int32 _max = std::numeric_limits<Int32>::max());
STICK_API UInt32 randomui(UInt32 _min = 0, UInt32 _max = std::numeric_limits<UInt32>::max());
STICK_API Vec2f randomVec2f(Float32 _minX = -1.0f, Float32 _maxX = 1.0f);
STICK_API Vec2f randomVec2f(Float32 _minX, Float32 _maxX, Float32 _minY, Float32 _maxY);


STICK_API void setNoiseSeed(Int32 _seed);
STICK_API void randomizeNoiseSeed();
STICK_API Float32 noise(Float32 _x);
STICK_API Float32 noise(Float32 _x, Float32 _y);
STICK_API Float32 noise(Float32 _x, Float32 _y, Float32 _z);
STICK_API Float32 noise(Float32 _x, Float32 _y, Float32 _z, Float32 _w);

STICK_API String executablePath(Allocator & _alloc = defaultAllocator());
STICK_API String executableDirectoryName(Allocator & _alloc = defaultAllocator());

STICK_API stick::Maybe<const char *> fileDialog(FileDialogFlags _flags,
                                                const char * _filters = NULL,
                                                const char * _defaultPath = NULL,
                                                const char * _defaultName = NULL);

class STICK_API ImGuiInterface : public stick::EventForwarder
{
  public:
    ImGuiInterface();
    ~ImGuiInterface();
    Error init(RenderDevice & _renderDevice,
               Window & _window,
               const char * _fontURI,
               Float32 _fontSize);
    Error newFrame(Float64 _deltaTime);
    Error finalizeFrame(RenderPass * _pass);

  private:
    // the window that the ui is drawn to and dispatches the events to the UI
    Window * m_window;
    // all rendering primitives to render the imgui ui
    RenderDevice * m_renderDevice;
    Program * m_program;
    Pipeline * m_pipeline;
    PipelineVariable * m_projPVar;
    PipelineTexture * m_pipeTex;
    VertexBuffer * m_vertexBuffer;
    IndexBuffer * m_indexBuffer;
    Mesh * m_mesh;
    Texture * m_texture;
    Sampler * m_sampler;
    // some other helpers
    UInt64 m_time;
    bool m_bMousePressed[3];
    DynamicArray<ImDrawVert> m_vertexDrawData;
    DynamicArray<ImDrawIdx> m_indexDrawData;
};

class STICK_API QuickDraw
{
    //@TODO: Add matrix stack operations (i.e. translate, rotate etc.)
    //@TODO: Add more drawing operations
    //@TODO: Add a way to draw textures (mainly for debugging purposes)
  public:

    static constexpr Size s_defaultCircleSubdivisionCount = 20;

    struct Vertex
    {
        Vec3f vertex;
        ColorRGBA color;
        Vec2f tc;
    };

    using MatrixStack = stick::DynamicArray<Mat4f>;
    using GeometryBuffer = stick::DynamicArray<Vertex>;

    struct DrawCall
    {
        Size vertexOffset;
        Size vertexCount;
        Mat4f tp;
        VertexDrawMode mode;
        const Texture * texture;
        const Sampler * sampler;
    };

    using DrawCallBuffer = stick::DynamicArray<DrawCall>;

    QuickDraw();

    Error init(RenderDevice * _rd, stick::Allocator & _alloc);

    void setViewport(Float32 _x, Float32 _y, Float32 _w, Float32 _h);
    void setTransform(const Mat4f & _transform);
    void setTransform(const Mat32f & _transform);
    void setProjection(const Mat4f & _proj);
    void setProjection(const Mat32f & _proj);
    void pushTransform();
    void popTransform();
    void pushProjection();
    void popProjection();
    void applyTransform(const Mat4f & _trans);
    void applyTransform(const Mat32f & _trans);
    void setColor(const ColorRGBA & _col);

    const Mat4f & transform() const;
    const Mat4f & projection() const;
    const Mat4f & transformProjection() const;

    void addToPass(
        RenderPass * _pass); // will queue the currently buffered draw commands on the provided pass
    void flush(); // should be called once per frame just before any renderpass that has been
                  // submitted too is finalized

    void rect(Float32 _minX, Float32 _minY, Float32 _maxX, Float32 _maxY);
    void lineRect(Float32 _minX, Float32 _minY, Float32 _maxX, Float32 _maxY);

    void circle(Float32 _x, Float32 _y, Float32 _radius, Size _subdivisionCount = s_defaultCircleSubdivisionCount);
    void lineCircle(Float32 _x, Float32 _y, Float32 _radius, Size _subdivisionCount = s_defaultCircleSubdivisionCount);

    void tex(const Texture * _tex,
             Float32 _minX,
             Float32 _minY,
             Float32 _maxX,
             Float32 _maxY,
             const Sampler * _sampler = nullptr);

    void lines(const Vec2f * _ptr, Size _count);
    void lines(const Vec3f * _ptr, Size _count);
    void lines(const Vertex * _ptr, Size _count);

    void lineStrip(const Vec2f * _ptr, Size _count, bool _bClosed = false);
    void lineStrip(const Vec3f * _ptr, Size _count, bool _bClosed = false);
    void lineStrip(const Vertex * _ptr, Size _count, bool _bClosed = false);

    void points(const Vec2f * _ptr, Size _count);
    void points(const Vec3f * _ptr, Size _count);
    void points(const Vertex * _ptr, Size _count);

    void rects(const Vec2f * _points, Size _count, Float32 _radius);
    void lineRects(const Vec2f * _points, Size _count, Float32 _radius);
    void convexPolygon(const Vec2f * _points, Size _count);

    void drawVertices(const Vertex * _vertices,
                      Size _count,
                      VertexDrawMode _mode,
                      const Texture * _tex = nullptr,
                      const Sampler * _sampler = nullptr);

    // GeometryBuffer & geometryBuffer();
    DrawCallBuffer & drawCalls();

    const Sampler * defaultSampler() const;
    const Sampler * bilinearSampler() const;
    const Sampler * nearestSampler() const;

  private:
    template <class T>
    void addDrawCall(const T *,
                     Size,
                     const ColorRGBA &,
                     VertexDrawMode,
                     const Texture * _t = nullptr,
                     const Sampler * _s = nullptr);

    RenderDevice * m_renderDevice;
    MatrixStack m_transformStack;
    MatrixStack m_projectionStack;
    Mat4f m_transform;
    Mat4f m_projection;
    mutable stick::Maybe<Mat4f> m_transformProjection;
    Rectf m_viewport;
    ColorRGBA m_color;

    Program * m_program;
    Pipeline * m_pipeline;
    Texture * m_whiteTex; // default texture used if no other one is bound
    Sampler * m_sampler;
    Sampler * m_samplerNearest;
    PipelineVariable * m_tpPVar;
    PipelineTexture * m_pipeTex;
    // PipelineTexture * m_pipeTex;
    VertexBuffer * m_vertexBuffer;
    Mesh * m_mesh;
    GeometryBuffer m_geometryBuffer;
    DrawCallBuffer m_drawCalls;
};

// namespace detail
// {
//     template<class...Behvs>
//     class WindowBehaviors
//     {
//     public:

//         std::tuple<Behvs...> m_behaviors;
//     };
// }

class STICK_API RenderWindow : public Window
{
  public:
    using DrawFunction = std::function<Error(Float64)>;
    using FrameFinishedCallback = std::function<Error()>;

    RenderWindow();
    virtual ~RenderWindow();

    Error open(const WindowSettings & _settings);
    Error enableDefaultUI(const char * _uiFontURI = NULL, Float32 _uiFontSize = 14.0f);
    void setShowWindowMetrics(bool _b);
    void toggleShowWindowMetrics();
    ImageUniquePtr frameImage(UInt32 _x, UInt32 _y, UInt32 _w, UInt32 _h);
    ImageUniquePtr frameImage();
    Error saveFrame(const char * _path, UInt32 _x, UInt32 _y, UInt32 _w, UInt32 _h);
    Error saveFrame(const char * _path);
    RenderDevice & renderDevice() const;
    virtual void setDrawFunction(DrawFunction _func);
    virtual void setFrameFinishedCallback(FrameFinishedCallback _cb);
    Error run();

    void setTargetFps(Float64 _fps);
    void removeTargetFps();
    Float64 fps() const;
    Float64 targetFps() const;
    Size frameCount() const;
    bool isShowingWindowMetrics() const;
    ImGuiInterface * imGuiInterface();
    QuickDraw & quickDraw();

    void drawPathOutline(Path * _path,
                         RenderInterface & _paperRenderer,
                         const ColorRGBA & _col,
                         bool _bDrawChildren = true);
    void drawMultiplePathOutlines(Path ** _paths,
                                  Size _count,
                                  RenderInterface & _paperRenderer,
                                  const ColorRGBA & _col,
                                  bool _bDrawChildren = true);
    void drawPathHandles(Path * _path,
                         const ColorRGBA & _col,
                         Float32 _radius = 2,
                         bool _bDrawChildren = true);
    void drawMultiplePathHandles(Path ** _paths,
                                 Size _count,
                                 const ColorRGBA & _col,
                                 Float32 _radius = 2,
                                 bool _bDrawChildren = true);
    void drawItemBoundingBox(Item * _item, const ColorRGBA & _col, bool _bDrawChildren = false);
    void drawMultipleItemBoundingBoxes(Item ** _items,
                                       Size _count,
                                       const ColorRGBA & _col,
                                       bool _bDrawChildren = false);

  protected:
    void drawPathOutlineHelper(Path * _path, RenderInterface & _paperRenderer, bool _bDrawChildren);
    void updateQuickDrawSize();

    RenderDevice * m_renderDevice;
    ImageUniquePtr m_tmpImage;
    DrawFunction m_drawFunc;
    FrameFinishedCallback m_frameFinishedCallback;
    SystemClock m_clock;
    Maybe<SystemClock::TimePoint> m_lastFrameTime;
    QuickDraw m_quickDraw;

    // imgui stuffs
    stick::UniquePtr<ImGuiInterface> m_gui;
    bool m_bShowWindowMetrics;

    // helpers to compute FPS (simple moving average)
    FixedArray<Float64, 100> m_fpsBuffer;
    Size m_fpsIndex;
    Float64 m_fpsSMASum;
    Float64 m_fpsAvg;

    stick::Maybe<Float64> m_targetFps;
    Size m_frameCount;
};

class STICK_API PaperWindow : public RenderWindow
{
  public:
    PaperWindow();
    Error open(const WindowSettings & _settings);
    Document & document();
    tarp::TarpRenderer & paperRenderer();

    void setAutoResize(bool _b);
    bool autoResize() const;
    void drawDocument(RenderPass * _pass);
    void drawPathOutline(Path * _path, const ColorRGBA & _col, bool _bDrawChildren = true);
    void drawMultiplePathOutlines(Path ** _paths,
                                  Size _count,
                                  const ColorRGBA & _col,
                                  bool _bDrawChildren = true);

  protected:
    void updateDocumentSize();

    Document m_doc;
    tarp::TarpRenderer m_paperRenderer;
    bool m_bAutoResize;
};

// template <class BaseWindowT>
// class RenderBufferWindowT : public BaseWindowT
// {
//   public:
//     using DrawFunction = typename BaseWindowT::DrawFunction;

//     RenderBufferWindowT()
//     {
//     }

//     ~RenderBufferWindowT()
//     {
//     }

//     Error open(const WindowSettings & _settings)
//     {
//     }

//     void setDrawFunction(DrawFunction _func) override
//     {
//     }

//   private:
//     RenderBuffer * m_renderBuffer;
//     Texture * m_colorTarget;
//     Texture * m_depthTarget;
//     Program * m_textureProgram;
// };

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
