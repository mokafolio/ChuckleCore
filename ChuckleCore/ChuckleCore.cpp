#include <ChuckleCore/ChuckleCore.hpp>
#if STICK_PLATFORM == STICK_PLATFORM_OSX
#include <ChuckleCore/Libs/noc/noc_file_dialog.h>
#elif STICK_PLATFORM == STICK_PLATFORM_LINUX
#define NOC_FILE_DIALOG_GTK
#define NOC_FILE_DIALOG_IMPLEMENTATION
#include <ChuckleCore/Libs/noc/noc_file_dialog.h>
#endif

#include <whereami.h>

#include <Stick/Thread.hpp>

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

Vec2f randomVec2f(Float32 _minX, Float32 _maxX)
{
    return Vec2f(randomf(_minX, _maxX), randomf(_minX, _maxX));
}

Vec2f randomVec2f(Float32 _minX, Float32 _maxX, Float32 _minY, Float32 _maxY)
{
    return Vec2f(randomf(_minX, _maxX), randomf(_minY, _maxY));
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

String executablePath(Allocator & _alloc)
{
    int length = wai_getExecutablePath(NULL, 0, NULL);
    String ret(length, _alloc);
    // ret.resize(length);
    wai_getExecutablePath((char *)ret.ptr(), length, NULL);
    return ret;
}

String executableDirectoryName(Allocator & _alloc)
{
    int length = wai_getExecutablePath(NULL, 0, NULL);
    int dirPathLen;
    String ret(_alloc);
    ret.resize(length);
    wai_getExecutablePath((char *)ret.ptr(), length, &dirPathLen);
    ret.resize(dirPathLen);
    return ret;
}

struct CursorMap
{
    CursorMap()
    {
        cursorMapping[ImGuiMouseCursor_Arrow] = CursorType::Cursor;
        cursorMapping[ImGuiMouseCursor_TextInput] = CursorType::TextInput;
        cursorMapping[ImGuiMouseCursor_ResizeAll] = CursorType::ResizeAll;
        cursorMapping[ImGuiMouseCursor_ResizeNS] = CursorType::ResizeNS;
        cursorMapping[ImGuiMouseCursor_ResizeEW] = CursorType::ResizeEW;
        cursorMapping[ImGuiMouseCursor_ResizeNESW] = CursorType::ResizeNESW;
        cursorMapping[ImGuiMouseCursor_ResizeNWSE] = CursorType::ResizeNWSE;
    }

    FixedArray<CursorType, 7> cursorMapping;
};

static const FixedArray<CursorType, 7> & cursorMapping()
{
    static CursorMap s_map;
    return s_map.cursorMapping;
}

static const char * ImGui_GetClipboardText(void *)
{
    return luke::clipboardText();
}

static void ImGui_SetClipboardText(void *, const char * _text)
{
    luke::setClipboardText(_text);
}

static void updateModifiers(ImGuiIO & _io)
{
    _io.KeyShift = luke::modifier(luke::KeyModifier::Shift);
    _io.KeyCtrl = luke::modifier(luke::KeyModifier::Control);
    _io.KeyAlt = luke::modifier(luke::KeyModifier::Alt);
    _io.KeySuper = luke::modifier(luke::KeyModifier::Command);
}

ImGuiInterface::ImGuiInterface()
    : m_renderDevice(nullptr), m_time(0), m_bMousePressed{ false, false, false }
{
}

ImGuiInterface::~ImGuiInterface()
{
    if (m_renderDevice)
    {
        //@TODO:Cleanup

        // Program * m_program;
        // Pipeline * m_pipeline;
        // PipelineVariable * m_projPVar;
        // PipelineTexture * m_pipeTex;
        // VertexBuffer * m_vertexBuffer;
        // IndexBuffer * m_indexBuffer;
        // Mesh * m_mesh;
        // Texture * m_texture;
        // Sampler * m_sampler;

        ImGui::DestroyContext();

        m_renderDevice->destroyProgram(m_program);
        m_renderDevice->destroyPipeline(m_pipeline);
        m_renderDevice->destroyVertexBuffer(m_vertexBuffer);
        m_renderDevice->destroyIndexBuffer(m_indexBuffer);
        m_renderDevice->destroyTexture(m_texture);
        m_renderDevice->destroySampler(m_sampler);
    }
}

Error ImGuiInterface::init(RenderDevice & _renderDevice,
                           Window & _window,
                           const char * _fontURI,
                           Float32 _fontSize)
{
    static_assert(sizeof(ImDrawIdx) == 4, "Please set ImDrawIdx to use integers in imconfig.h");

    ImGui::CreateContext();
    m_renderDevice = &_renderDevice;
    m_window = &_window;

    // Setup back-end capabilities flags
    ImGuiIO & io = ImGui::GetIO();
    io.BackendFlags |=
        ImGuiBackendFlags_HasMouseCursors; // We can honor GetMouseCursor() values (optional)

    // Keyboard mapping. ImGui will use those indices to peek into the io.KeysDown[] array.
    io.KeyMap[ImGuiKey_Tab] = (int)KeyCode::Tab;
    io.KeyMap[ImGuiKey_LeftArrow] = (int)KeyCode::Left;
    io.KeyMap[ImGuiKey_RightArrow] = (int)KeyCode::Right;
    io.KeyMap[ImGuiKey_UpArrow] = (int)KeyCode::Up;
    io.KeyMap[ImGuiKey_DownArrow] = (int)KeyCode::Down;
    io.KeyMap[ImGuiKey_PageUp] = (int)KeyCode::PageUp;
    io.KeyMap[ImGuiKey_PageDown] = (int)KeyCode::PageDown;
    io.KeyMap[ImGuiKey_Home] = (int)KeyCode::Home;
    io.KeyMap[ImGuiKey_End] = (int)KeyCode::End;
    io.KeyMap[ImGuiKey_Insert] = (int)KeyCode::Insert;
    io.KeyMap[ImGuiKey_Delete] = (int)KeyCode::Delete;
    io.KeyMap[ImGuiKey_Backspace] = (int)KeyCode::Backspace;
    io.KeyMap[ImGuiKey_Space] = (int)KeyCode::Space;
    io.KeyMap[ImGuiKey_Enter] = (int)KeyCode::Return;
    io.KeyMap[ImGuiKey_Escape] = (int)KeyCode::Escape;
    io.KeyMap[ImGuiKey_A] = (int)KeyCode::A;
    io.KeyMap[ImGuiKey_C] = (int)KeyCode::C;
    io.KeyMap[ImGuiKey_V] = (int)KeyCode::V;
    io.KeyMap[ImGuiKey_X] = (int)KeyCode::X;
    io.KeyMap[ImGuiKey_Y] = (int)KeyCode::Y;
    io.KeyMap[ImGuiKey_Z] = (int)KeyCode::Z;

    io.SetClipboardTextFn = ImGui_SetClipboardText;
    io.GetClipboardTextFn = ImGui_GetClipboardText;
    io.ClipboardUserData = NULL;

    // ImGui::GetStyle().ScaleAllSizes(2.0);

    if (_fontURI)
    {
        io.Fonts->Clear();
        io.Fonts->AddFontFromFileTTF(_fontURI, _fontSize * _window.backingScaleFactor());
    }

    // ImFontConfig cfg;
    // cfg.SizePixels = 32.0;
    // io.Fonts->AddFontDefault(&cfg);

    // subscribe to all events dispatched from the window
    _window.addForwarder(*this);

    // grab the events that imgui is interested in
    _window.addEventCallback([&io](const luke::KeyDownEvent & _evt) {
        if ((UInt32)_evt.key() < IM_ARRAYSIZE(io.KeysDown))
            io.KeysDown[(UInt32)_evt.key()] = true;

        updateModifiers(io);
    });

    _window.addEventCallback([&io](const luke::KeyUpEvent & _evt) {
        if ((UInt32)_evt.key() < IM_ARRAYSIZE(io.KeysDown))
            io.KeysDown[(UInt32)_evt.key()] = false;

        updateModifiers(io);
    });

    _window.addEventCallback([&io](const luke::MouseScrollEvent & _evt) {
        if (_evt.scrollX() > 0)
            io.MouseWheelH += 0.75;
        if (_evt.scrollX() < 0)
            io.MouseWheelH -= 0.75;
        if (_evt.scrollY() > 0)
            io.MouseWheel += 0.75;
        if (_evt.scrollY() < 0)
            io.MouseWheel -= 0.75;
    });

    _window.addEventCallback([&io](const luke::TextInputEvent & _evt) {
        if (io.WantTextInput)
        {
            io.AddInputCharactersUTF8(_evt.text().cString());
            // prevent this event from being passed on to anything else
            _evt.stopPropagation();
        }
    });

    // and prevent all mouse/keyboard events to propagate to forwarders down the chain
    // if imgui is capturing them.
    addEventCategoryFilter<MouseEventCategory>(
        [&io](const stick::Event & _evt) { return io.WantCaptureMouse; });

    addEventCategoryFilter<KeyEventCategory>(
        [&io](const stick::Event & _evt) { return io.WantCaptureKeyboard; });

    const char * vertex_shader =
        "#version 410 core \n"
        "layout(std140) uniform View\n"
        "{\n"
        "mat4 projection;\n"
        "};\n"
        "layout(location = 0) in vec2 vertex;\n"
        "layout(location = 1) in vec2 tc;\n"
        "layout(location = 2) in vec4 color;\n"
        "out vec2 fragTC;\n"
        "out vec4 fragCol;\n"
        "void main()\n"
        "{\n"
        "   fragTC = tc;\n"
        "   fragCol = color / 255.0;\n"
        "   gl_Position = projection * vec4(vertex, 0, 1);\n"
        "}\n";

    const char * fragment_shader =
        "#version 410 core \n"
        "uniform sampler2D tex;\n"
        "in vec2 fragTC;\n"
        "in vec4 fragCol;\n"
        "out vec4 outCol;\n"
        "void main()\n"
        "{\n"
        "   outCol = fragCol * texture(tex, fragTC);\n"
        "}\n";

    auto result = m_renderDevice->createProgram(vertex_shader, fragment_shader);
    if (!result)
        return result.error();

    m_program = result.get();

    PipelineSettings ps;
    ps.multisample = true;
    ps.blendSettings = BlendSettings(); // enable default blending
    ps.program = m_program;
    if (auto res = m_renderDevice->createPipeline(ps))
        m_pipeline = res.get();
    else
        return res.error();

    m_projPVar = m_pipeline->variable("projection");
    m_pipeTex = m_pipeline->texture("tex");

    // Build texture atlas
    unsigned char * pixels;
    int width, height;
    // io.Fonts->Clear();
    // if(_fontURI)
    //     io.Fonts->AddFontFromFileTTF(_fontURI, _fontSize);
    io.Fonts->GetTexDataAsRGBA32(
        &pixels, &width, &height); // Load as RGBA 32-bits for OpenGL3 demo because it is more
                                   // likely to be compatible with user's existing shader.

    if (auto res = m_renderDevice->createTexture())
        m_texture = res.get();
    else
        return res.error();

    if (auto res = m_renderDevice->createSampler())
        m_sampler = res.get();
    else
        return res.error();

    m_texture->loadPixels(width, height, 1, pixels, DataType::UInt8, TextureFormat::RGBA8);
    m_pipeTex->set(m_texture, m_sampler);

    // Store our identifier
    io.Fonts->TexID = (void *)(intptr_t)m_texture;

    //@TODO Proper usage hints once Dab supports that
    if (auto res = m_renderDevice->createVertexBuffer())
        m_vertexBuffer = res.get();
    else
        return res.error();

    if (auto res = m_renderDevice->createIndexBuffer())
        m_indexBuffer = res.get();
    else
        return res.error();

    VertexLayout layout({
        { DataType::Float32, 2 }, // vertex
        { DataType::Float32, 2 }, // tc
        { DataType::UInt8, 4 }    // color
    });

    if (auto res = m_renderDevice->createMesh(&m_vertexBuffer, &layout, 1, m_indexBuffer))
        m_mesh = res.get();
    else
        return res.error();

    return Error();
}

Error ImGuiInterface::newFrame(Float64 _deltaTime)
{
    ImGuiIO & io = ImGui::GetIO();

    // Setup display size (every frame to accommodate for window resizing)
    int w = m_window->width();
    int h = m_window->height();
    int display_w = m_window->widthInPixels();
    int display_h = m_window->heightInPixels();

    io.DisplaySize = ImVec2((float)w, (float)h);
    io.DisplayFramebufferScale =
        ImVec2(w > 0 ? ((float)display_w / w) : 0, h > 0 ? ((float)display_h / h) : 0);
    io.FontGlobalScale = 1.0 / std::max(io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
    io.DeltaTime = _deltaTime;

    MouseState ms = luke::mouseState();
    io.MousePos = ImVec2(ms.x(), ms.y());

    io.MouseDown[0] = ms.isButtonDown(MouseButton::Left);
    io.MouseDown[1] = ms.isButtonDown(MouseButton::Right);
    io.MouseDown[2] = ms.isButtonDown(MouseButton::Middle);

    // Update OS/hardware mouse cursor if imgui isn't drawing a software cursor
    if ((io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange) == 0)
    {
        ImGuiMouseCursor cursor = ImGui::GetMouseCursor();
        if (io.MouseDrawCursor || cursor == ImGuiMouseCursor_None)
        {
            m_window->hideCursor();
        }
        else
        {
            m_window->setCursor(cursorMapping()[cursor]);
            m_window->showCursor();
        }
    }

    // Start the frame. This call will update the io.WantCaptureMouse, io.WantCaptureKeyboard flag
    // that you can use to dispatch inputs (or not) to your application.
    ImGui::NewFrame();

    return Error();
}

Error ImGuiInterface::finalizeFrame(RenderPass * _pass)
{
    ImGui::Render();
    ImDrawData * drawData = ImGui::GetDrawData();

    // Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates !=
    // framebuffer coordinates)
    ImGuiIO & io = ImGui::GetIO();
    int fb_width = (int)(io.DisplaySize.x * io.DisplayFramebufferScale.x);
    int fb_height = (int)(io.DisplaySize.y * io.DisplayFramebufferScale.y);
    if (fb_width == 0 || fb_height == 0)
        return Error();

    drawData->ScaleClipRects(io.DisplayFramebufferScale);

    // Setup viewport, orthographic projection matrix
    // glViewport(0, 0, (GLsizei)fb_width, (GLsizei)fb_height);
    float ortho_projection[4][4] = {
        { 2.0f / io.DisplaySize.x, 0.0f, 0.0f, 0.0f },
        { 0.0f, 2.0f / -io.DisplaySize.y, 0.0f, 0.0f },
        { 0.0f, 0.0f, -1.0f, 0.0f },
        { -1.0f, 1.0f, 0.0f, 1.0f },
    };
    m_projPVar->setMat4f(&ortho_projection[0][0]);

    _pass->setViewport(0, 0, fb_width, fb_height);

    m_vertexDrawData.clear();
    m_indexDrawData.clear();
    for (int n = 0; n < drawData->CmdListsCount; n++)
    {
        const ImDrawList * cmd_list = drawData->CmdLists[n];
        m_vertexDrawData.append(cmd_list->VtxBuffer.Data,
                                cmd_list->VtxBuffer.Data + cmd_list->VtxBuffer.Size);
        m_indexDrawData.append(cmd_list->IdxBuffer.Data,
                               cmd_list->IdxBuffer.Data + cmd_list->IdxBuffer.Size);
    }

    m_vertexBuffer->loadDataRaw((void *)m_vertexDrawData.ptr(),
                                m_vertexDrawData.count() * sizeof(ImDrawVert));
    m_indexBuffer->loadDataRaw((void *)m_indexDrawData.ptr(),
                               m_indexDrawData.count() * sizeof(ImDrawIdx));

    Size vtxOffset = 0;
    Size idxOffset = 0;
    for (int n = 0; n < drawData->CmdListsCount; n++)
    {
        const ImDrawList * cmd_list = drawData->CmdLists[n];

        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
        {
            const ImDrawCmd * pcmd = &cmd_list->CmdBuffer[cmd_i];
            if (pcmd->UserCallback)
            {
                pcmd->UserCallback(cmd_list, pcmd);
            }
            else
            {
                m_pipeTex->set((Texture *)(intptr_t)pcmd->TextureId, m_sampler);
                _pass->setScissor((int)pcmd->ClipRect.x,
                                  (int)(fb_height - pcmd->ClipRect.w),
                                  (int)(pcmd->ClipRect.z - pcmd->ClipRect.x),
                                  (int)(pcmd->ClipRect.w - pcmd->ClipRect.y));
                _pass->drawMesh(m_mesh,
                                m_pipeline,
                                idxOffset,
                                pcmd->ElemCount,
                                vtxOffset,
                                VertexDrawMode::Triangles);
            }
            idxOffset += pcmd->ElemCount;
        }
        vtxOffset += cmd_list->VtxBuffer.Size;
    }

    return Error();
}

static void _addToGeometryBuffer(QuickDraw::GeometryBuffer & _buff,
                                 const QuickDraw::Vertex * _vertices,
                                 Size _count)
{
    _buff.append(_vertices, _vertices + _count);
}

static void _addVertex(QuickDraw::GeometryBuffer & _buff, const QuickDraw::Vertex & _vert)
{
    _addToGeometryBuffer(_buff, &_vert, 1);
}

static void addToGeometryBuffer(QuickDraw::GeometryBuffer & _buff,
                                const Vec2f * _ptr,
                                Size _count,
                                const ColorRGBA & _col)
{
    for (Size i = 0; i < _count; ++i)
        _addVertex(_buff, { Vec3f(_ptr[i].x, _ptr[i].y, 0), _col, Vec2f(0) });
}

static void addToGeometryBuffer(QuickDraw::GeometryBuffer & _buff,
                                const Vec3f * _ptr,
                                Size _count,
                                const ColorRGBA & _col)
{
    for (Size i = 0; i < _count; ++i)
        _addVertex(_buff, { _ptr[i], _col, Vec2f(0) });
}

static void addToGeometryBuffer(QuickDraw::GeometryBuffer & _buff,
                                const QuickDraw::Vertex * _ptr,
                                Size _count,
                                const ColorRGBA & _col)
{
    // _buff.append(_ptr, _ptr + _count);
    _addToGeometryBuffer(_buff, _ptr, _count);
}

QuickDraw::QuickDraw() : m_renderDevice(nullptr)
{
}

Error QuickDraw::init(RenderDevice * _rd, Allocator & _alloc)
{
    m_renderDevice = _rd;
    m_transformStack = MatrixStack(_alloc);
    m_projectionStack = MatrixStack(_alloc);
    m_geometryBuffer = GeometryBuffer(_alloc);
    m_drawCalls = DrawCallBuffer(_alloc);

    m_transform = Mat4f::identity();
    m_projection = Mat4f::identity();

    const char * vertex_shader =
        "#version 410 core \n"
        "layout(std140) uniform View\n"
        "{\n"
        "mat4 transformProjection;\n"
        "};\n"
        "layout(location = 0) in vec3 vertex;\n"
        "layout(location = 1) in vec4 color;\n"
        "layout(location = 2) in vec2 textureCoords;\n"
        "out vec4 fragCol;\n"
        "out vec2 tc;\n"
        "void main()\n"
        "{\n"
        "   fragCol = color;\n"
        "   gl_Position = transformProjection * vec4(vertex, 1);\n"
        "   tc = textureCoords;\n"
        "}\n";

    const char * fragment_shader =
        "#version 410 core \n"
        "uniform sampler2D tex;\n"
        "in vec4 fragCol;\n"
        "in vec2 tc;\n"
        "out vec4 outCol;\n"
        "void main()\n"
        "{\n"
        "   outCol = fragCol * texture(tex, tc);\n"
        "}\n";

    if (auto res = m_renderDevice->createProgram(vertex_shader, fragment_shader))
        m_program = res.get();
    else
        return res.error();

    PipelineSettings ps;
    ps.multisample = true;
    ps.blendSettings = BlendSettings(); // enable default blending
    ps.program = m_program;
    if (auto res = m_renderDevice->createPipeline(ps))
        m_pipeline = res.get();
    else
        return res.error();

    //@TODO Proper usage hints once Dab supports that
    if (auto res = m_renderDevice->createVertexBuffer())
        m_vertexBuffer = res.get();
    else
        return res.error();

    if (auto res = m_renderDevice->createTexture())
        m_whiteTex = res.get();
    else
        return res.error();

    if (auto res = m_renderDevice->createSampler())
        m_sampler = res.get();
    else
        return res.error();

    SamplerSettings ns;
    ns.filtering = TextureFiltering::Bilinear;
    if (auto res = m_renderDevice->createSampler(ns))
        m_samplerNearest = res.get();
    else
        return res.error();

    UInt8 whitePixel[4] = { 255, 255, 255, 255 };
    m_whiteTex->loadPixels(1, 1, 1, &whitePixel, DataType::UInt8, TextureFormat::RGBA8);

    m_tpPVar = m_pipeline->variable("transformProjection");
    m_pipeTex = m_pipeline->texture("tex");

    VertexLayout layout({
        { DataType::Float32, 3 }, // vertex
        { DataType::Float32, 4 }, // color
        { DataType::Float32, 2 }  // texture coordinates
    });

    if (auto res = m_renderDevice->createMesh(&m_vertexBuffer, &layout, 1))
        m_mesh = res.get();
    else
        return res.error();

    return Error();
}

void QuickDraw::setViewport(Float32 _x, Float32 _y, Float32 _w, Float32 _h)
{
    m_viewport = Rectf(_x, _y, _x + _w, _y + _h);
}

void QuickDraw::setTransform(const Mat4f & _transform)
{
    m_transform = _transform;
    m_transformProjection.reset();
}

void QuickDraw::setTransform(const Mat32f & _transform)
{
    setTransform(to3DTransform(_transform));
}

void QuickDraw::setProjection(const Mat4f & _proj)
{
    m_projection = _proj;
    m_transformProjection.reset();
}

void QuickDraw::setProjection(const Mat32f & _proj)
{
    setProjection(to3DTransform(_proj));
}

void QuickDraw::pushTransform()
{
    m_transformStack.append(m_transform);
}

void QuickDraw::popTransform()
{
    STICK_ASSERT(m_transformStack.count());
    setTransform(m_transformStack.last());
    m_transformStack.removeLast();
}

void QuickDraw::pushProjection()
{
    m_projectionStack.append(m_transform);
}

void QuickDraw::popProjection()
{
    STICK_ASSERT(m_projectionStack.count());
    setProjection(m_projectionStack.last());
    m_projectionStack.removeLast();
}

void QuickDraw::applyTransform(const Mat4f & _trans)
{
    setTransform(m_transform * _trans);
}

void QuickDraw::applyTransform(const Mat32f & _trans)
{
    applyTransform(to3DTransform(_trans));
}

const Mat4f & QuickDraw::transform() const
{
    return m_transform;
}

const Mat4f & QuickDraw::projection() const
{
    return m_projection;
}

const Mat4f & QuickDraw::transformProjection() const
{
    if (!m_transformProjection)
        m_transformProjection = m_projection * m_transform;
    return *m_transformProjection;
}

void QuickDraw::setColor(const ColorRGBA & _col)
{
    m_color = _col;
}

void QuickDraw::addToPass(RenderPass * _pass)
{
    if (m_drawCalls.count())
    {
        _pass->setViewport(
            m_viewport.min().x, m_viewport.min().y, m_viewport.width(), m_viewport.height());

        const Texture * lastTex = nullptr;
        for (auto & dc : m_drawCalls)
        {
            m_tpPVar->setMat4f(dc.tp.ptr());
            if (lastTex != dc.texture)
                m_pipeTex->set(dc.texture, dc.sampler);
            _pass->drawMesh(m_mesh, m_pipeline, dc.vertexOffset, dc.vertexCount, dc.mode);
            lastTex = dc.texture;
        }

        m_drawCalls.clear();
    }
}

void QuickDraw::drawVertices(const Vertex * _vertices,
                             Size _count,
                             VertexDrawMode _mode,
                             const Texture * _tex,
                             const Sampler * _sampler)
{
    addDrawCall(_vertices, _count, ColorRGBA(), _mode, _tex, _sampler);
}

void QuickDraw::flush()
{
    if (m_geometryBuffer.count())
    {
        m_vertexBuffer->loadDataRaw((void *)m_geometryBuffer.ptr(),
                                    m_geometryBuffer.count() * sizeof(Vertex));
        m_geometryBuffer.clear();
    }
}

const Sampler * QuickDraw::defaultSampler() const
{
    return bilinearSampler();
}

const Sampler * QuickDraw::bilinearSampler() const
{
    return m_sampler;
}

const Sampler * QuickDraw::nearestSampler() const
{
    return m_samplerNearest;
}

// void QuickDraw::beginPass(RenderPass * _pass)
// {
//     STICK_ASSERT(m_currentPass == nullptr);
//     m_currentPass = _pass;
// }

// void QuickDraw::endPass()
// {
//     STICK_ASSERT(m_currentPass != nullptr);
//     m_vertexBuffer->loadDataRaw((void *)m_geometryBuffer.ptr(),
//                                 m_geometryBuffer.count() * sizeof(Vertex));
//     m_geometryBuffer.clear();
//     m_currentPass = nullptr;
// }

// void QuickDraw::draw(RenderPass * _pass)
// {
//     if (m_drawCalls.count())
//     {
//         bool bInternalPass = !_pass;

//         if (bInternalPass)
//             _pass = m_renderDevice->beginPass();

//         _pass->setViewport(
//             m_viewport.min().x, m_viewport.min().y, m_viewport.width(), m_viewport.height());

//         // m_vertexBuffer->loadDataRaw((void *)m_geometryBuffer.ptr(),
//         //                             m_geometryBuffer.count() * sizeof(Vertex));

//         for (auto & dc : m_drawCalls)
//         {
//             m_tpPVar->setMat4f(dc.tp.ptr());
//             _pass->drawMesh(m_mesh, m_pipeline, dc.vertexOffset, dc.vertexCount, dc.mode);
//         }

//         if (bInternalPass)
//             m_renderDevice->endPass(_pass);

//         m_drawCalls.clear();
//     }
// }

// void QuickDraw::flush()
// {
//     m_vertexBuffer->loadDataRaw((void *)m_geometryBuffer.ptr(),
//                                 m_geometryBuffer.count() * sizeof(Vertex));
//     m_geometryBuffer.clear();
// }

void QuickDraw::rect(Float32 _minX, Float32 _minY, Float32 _maxX, Float32 _maxY)
{
    // m_geometryBuffer.append({ Vec3f(_minX, _minY, 0), m_color });
    // m_geometryBuffer.append({ Vec3f(_minX, _maxY, 0), m_color });
    // m_geometryBuffer.append({ Vec3f(_maxX, _minY, 0), m_color });
    // m_geometryBuffer.append({ Vec3f(_maxX, _maxY, 0), m_color });

    _addVertex(m_geometryBuffer, { Vec3f(_minX, _minY, 0), m_color });
    _addVertex(m_geometryBuffer, { Vec3f(_minX, _maxY, 0), m_color });
    _addVertex(m_geometryBuffer, { Vec3f(_maxX, _minY, 0), m_color });
    _addVertex(m_geometryBuffer, { Vec3f(_maxX, _maxY, 0), m_color });

    m_drawCalls.append({ m_geometryBuffer.count() - 4,
                         4,
                         transformProjection(),
                         VertexDrawMode::TriangleStrip,
                         m_whiteTex,
                         defaultSampler() });

    // setTransformProjectionForDrawCall();
    // m_currentPass->drawMesh(
    //     m_mesh, m_pipeline, m_geometryBuffer.count() - 4, 4, VertexDrawMode::TriangleStrip);
}

void QuickDraw::tex(const Texture * _tex,
                    Float32 _minX,
                    Float32 _minY,
                    Float32 _maxX,
                    Float32 _maxY,
                    const Sampler * _s)
{
    _addVertex(m_geometryBuffer, { Vec3f(_minX, _minY, 0), m_color, Vec2f(0, 0) });
    _addVertex(m_geometryBuffer, { Vec3f(_minX, _maxY, 0), m_color, Vec2f(0, 1) });
    _addVertex(m_geometryBuffer, { Vec3f(_maxX, _minY, 0), m_color, Vec2f(1, 0) });
    _addVertex(m_geometryBuffer, { Vec3f(_maxX, _maxY, 0), m_color, Vec2f(1, 1) });

    m_drawCalls.append({ m_geometryBuffer.count() - 4,
                         4,
                         transformProjection(),
                         VertexDrawMode::TriangleStrip,
                         _tex,
                        _s });
}

void QuickDraw::lineRect(Float32 _minX, Float32 _minY, Float32 _maxX, Float32 _maxY)
{
    // m_geometryBuffer.append({ Vec3f(_minX, _minY, 0), m_color });
    // m_geometryBuffer.append({ Vec3f(_maxX, _minY, 0), m_color });
    // m_geometryBuffer.append({ Vec3f(_maxX, _maxY, 0), m_color });
    // m_geometryBuffer.append({ Vec3f(_minX, _maxY, 0), m_color });

    _addVertex(m_geometryBuffer, { Vec3f(_minX, _minY, 0), m_color });
    _addVertex(m_geometryBuffer, { Vec3f(_maxX, _minY, 0), m_color });
    _addVertex(m_geometryBuffer, { Vec3f(_maxX, _maxY, 0), m_color });
    _addVertex(m_geometryBuffer, { Vec3f(_minX, _maxY, 0), m_color });

    m_drawCalls.append({ m_geometryBuffer.count() - 4,
                         4,
                         transformProjection(),
                         VertexDrawMode::LineLoop,
                         m_whiteTex,
                         defaultSampler() });

    // setTransformProjectionForDrawCall();
    // m_currentPass->drawMesh(
    //     m_mesh, m_pipeline, m_geometryBuffer.count() - 4, 4, VertexDrawMode::LineLoop);
}

static Size _addCircleGeometry(Float32 _x,
                               Float32 _y,
                               Float32 _radius,
                               Size _subdivisionCount,
                               QuickDraw::GeometryBuffer & _buff,
                               const ColorRGBA & _color,
                               bool _bIsSolid)
{
    Size ret = 0;
    if (_bIsSolid)
    {
        _addVertex(_buff, { Vec3f(_x, _y, 0), _color, Vec2f(0) });
        ++ret;
    }

    Float32 radStep = crunch::Constants<Float32>::twoPi() / static_cast<Float32>(_subdivisionCount);
    for (Size i = 0; i <= _subdivisionCount; ++i)
    {
        Float32 currentStep = radStep * i;
        _addVertex(_buff,
                   { Vec3f(_x + std::cos(currentStep) * _radius,
                           _y + std::sin(currentStep) * _radius,
                           0.0),
                     _color,
                     Vec2f(0) });
        ++ret;
    }
    return ret;
}

void QuickDraw::circle(Float32 _x, Float32 _y, Float32 _radius, Size _subdivisionCount)
{
    Size off = m_geometryBuffer.count();
    Size count =
        _addCircleGeometry(_x, _y, _radius, _subdivisionCount, m_geometryBuffer, m_color, true);
    m_drawCalls.append(
        { off, count, transformProjection(), VertexDrawMode::TriangleFan, m_whiteTex, m_sampler });
}

void QuickDraw::lineCircle(Float32 _x, Float32 _y, Float32 _radius, Size _subdivisionCount)
{
    Size off = m_geometryBuffer.count();
    Size count =
        _addCircleGeometry(_x, _y, _radius, _subdivisionCount, m_geometryBuffer, m_color, false);
    m_drawCalls.append(
        { off, count, transformProjection(), VertexDrawMode::LineLoop, m_whiteTex, m_sampler });
}

template <class T>
void QuickDraw::addDrawCall(const T * _ptr,
                            Size _count,
                            const ColorRGBA & _col,
                            VertexDrawMode _drawMode,
                            const Texture * _tex,
                            const Sampler * _sampler)
{
    Size voff = m_geometryBuffer.count();
    addToGeometryBuffer(m_geometryBuffer, _ptr, _count, _col);
    m_drawCalls.append({ voff,
                         _count,
                         transformProjection(),
                         _drawMode,
                         _tex ? _tex : m_whiteTex,
                         _sampler ? _sampler : defaultSampler() });

    // setTransformProjectionForDrawCall();
    // m_currentPass->drawMesh(m_mesh, m_pipeline, voff, _count, _drawMode);
}

void QuickDraw::lineStrip(const Vec2f * _ptr, Size _count, bool _bClosed)
{
    addDrawCall(
        _ptr, _count, m_color, _bClosed ? VertexDrawMode::LineLoop : VertexDrawMode::LineStrip);
}

void QuickDraw::lineStrip(const Vec3f * _ptr, Size _count, bool _bClosed)
{
    addDrawCall(
        _ptr, _count, m_color, _bClosed ? VertexDrawMode::LineLoop : VertexDrawMode::LineStrip);
}

void QuickDraw::lineStrip(const QuickDraw::Vertex * _ptr, Size _count, bool _bClosed)
{
    addDrawCall(
        _ptr, _count, m_color, _bClosed ? VertexDrawMode::LineLoop : VertexDrawMode::LineStrip);
}

void QuickDraw::lines(const Vec2f * _ptr, Size _count)
{
    addDrawCall(_ptr, _count, m_color, VertexDrawMode::Lines);
}

void QuickDraw::lines(const Vec3f * _ptr, Size _count)
{
    addDrawCall(_ptr, _count, m_color, VertexDrawMode::Lines);
}

void QuickDraw::lines(const Vertex * _ptr, Size _count)
{
    addDrawCall(_ptr, _count, m_color, VertexDrawMode::Lines);
}

void QuickDraw::points(const Vec2f * _ptr, Size _count)
{
    addDrawCall(_ptr, _count, m_color, VertexDrawMode::Points);
}

void QuickDraw::points(const Vec3f * _ptr, Size _count)
{
    addDrawCall(_ptr, _count, m_color, VertexDrawMode::Points);
}

void QuickDraw::points(const Vertex * _ptr, Size _count)
{
    addDrawCall(_ptr, _count, m_color, VertexDrawMode::Points);
}

void QuickDraw::convexPolygon(const Vec2f * _ptr, Size _count)
{
    addDrawCall(_ptr, _count, m_color, VertexDrawMode::TriangleFan);
}

void QuickDraw::rects(const Vec2f * _points, Size _count, Float32 _radius)
{
    Size off = m_geometryBuffer.count();
    Vec3f pos;
    Vec3f tla(-_radius, -_radius);
    Vec3f bla(-_radius, _radius);
    Vec3f tra(_radius, -_radius);
    Vec3f bra(_radius, _radius);
    for (Size i = 0; i < _count; ++i)
    {
        pos = Vec3f(_points[i].x, _points[i].y, 0);
        // m_geometryBuffer.append({ pos + tla, m_color });
        // m_geometryBuffer.append({ pos + tra, m_color });
        // m_geometryBuffer.append({ pos + bla, m_color });
        // m_geometryBuffer.append({ pos + tra, m_color });
        // m_geometryBuffer.append({ pos + bra, m_color });
        // m_geometryBuffer.append({ pos + bla, m_color });
        _addVertex(m_geometryBuffer, { pos + tla, m_color, Vec2f(0) });
        _addVertex(m_geometryBuffer, { pos + tra, m_color, Vec2f(0) });
        _addVertex(m_geometryBuffer, { pos + bla, m_color, Vec2f(0) });
        _addVertex(m_geometryBuffer, { pos + tra, m_color, Vec2f(0) });
        _addVertex(m_geometryBuffer, { pos + bra, m_color, Vec2f(0) });
        _addVertex(m_geometryBuffer, { pos + bla, m_color, Vec2f(0) });
    }
    m_drawCalls.append({ off,
                         _count * 6,
                         transformProjection(),
                         VertexDrawMode::Triangles,
                         m_whiteTex,
                         m_sampler });
}

void QuickDraw::lineRects(const Vec2f * _points, Size _count, Float32 _radius)
{
    Size off = m_geometryBuffer.count();
    Vec3f pos;
    Vec3f tla(-_radius, -_radius);
    Vec3f bla(-_radius, _radius);
    Vec3f tra(_radius, -_radius);
    Vec3f bra(_radius, _radius);
    for (Size i = 0; i < _count; ++i)
    {
        pos = Vec3f(_points[i].x, _points[i].y, 0);
        // m_geometryBuffer.append({ pos + tla, m_color });
        // m_geometryBuffer.append({ pos + tra, m_color });
        // m_geometryBuffer.append({ pos + tra, m_color });
        // m_geometryBuffer.append({ pos + bra, m_color });
        // m_geometryBuffer.append({ pos + bra, m_color });
        // m_geometryBuffer.append({ pos + bla, m_color });
        // m_geometryBuffer.append({ pos + bla, m_color });
        // m_geometryBuffer.append({ pos + tla, m_color });

        _addVertex(m_geometryBuffer, { pos + tla, m_color });
        _addVertex(m_geometryBuffer, { pos + tra, m_color });
        _addVertex(m_geometryBuffer, { pos + tra, m_color });
        _addVertex(m_geometryBuffer, { pos + bra, m_color });
        _addVertex(m_geometryBuffer, { pos + bra, m_color });
        _addVertex(m_geometryBuffer, { pos + bla, m_color });
        _addVertex(m_geometryBuffer, { pos + bla, m_color });
        _addVertex(m_geometryBuffer, { pos + tla, m_color });
    }
    m_drawCalls.append(
        { off, _count * 8, transformProjection(), VertexDrawMode::Lines, m_whiteTex, m_sampler });
}

// QuickDraw::GeometryBuffer & QuickDraw::geometryBuffer()
// {
//     return m_geometryBuffer;
// }

QuickDraw::DrawCallBuffer & QuickDraw::drawCalls()
{
    return m_drawCalls;
}

RenderWindow::RenderWindow()
    : m_renderDevice(nullptr)
    , m_bShowWindowMetrics(false)
    , m_fpsIndex(0)
    , m_fpsSMASum(0)
    , m_fpsAvg(0)
    , m_frameCount(0)
{
    for (Size i = 0; i < m_fpsBuffer.count(); ++i)
        m_fpsBuffer[i] = 0.0;
}

RenderWindow::~RenderWindow()
{
    if (m_gui)
        m_gui.reset();
    destroyRenderDevice(m_renderDevice);
}

Error RenderWindow::open(const WindowSettings & _settings)
{
    this->addEventCallback([this](const WindowResizeEvent & _evt) { this->updateQuickDrawSize(); });

    Error ret = Window::open(_settings);
    if (ret)
        return ret;
    auto res = createRenderDevice();
    if (!res)
        return res.error();
    m_renderDevice = res.get();
    m_tmpImage = makeUnique<ImageRGBA8>(widthInPixels(), heightInPixels());
    ret = m_quickDraw.init(m_renderDevice, defaultAllocator());
    if (ret)
        return ret;
    updateQuickDrawSize();
    return Error();
}

void RenderWindow::updateQuickDrawSize()
{
    m_quickDraw.setViewport(0, 0, widthInPixels(), heightInPixels());
    m_quickDraw.setProjection(Mat4f::ortho(0, width(), height(), 0, -1, 1));
}

Error RenderWindow::enableDefaultUI(const char * _uiFontURI, Float32 _uiFontSize)
{
    STICK_ASSERT(!m_gui);
    m_gui = makeUnique<ImGuiInterface>();
    return m_gui->init(*m_renderDevice, *this, _uiFontURI, _uiFontSize);
}

void RenderWindow::setShowWindowMetrics(bool _b)
{
    STICK_ASSERT(m_gui);
    m_bShowWindowMetrics = _b;
}

void RenderWindow::toggleShowWindowMetrics()
{
    setShowWindowMetrics(!m_bShowWindowMetrics);
}

bool RenderWindow::isShowingWindowMetrics() const
{
    return m_bShowWindowMetrics;
}

ImGuiInterface * RenderWindow::imGuiInterface()
{
    return m_gui.get();
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
    m_tmpImage->flipRows();
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

void RenderWindow::setDrawFunction(DrawFunction _func)
{
    m_drawFunc = _func;
}

void RenderWindow::setFrameFinishedCallback(FrameFinishedCallback _cb)
{
    m_frameFinishedCallback = _cb;
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
        this->enableRenderContext();

        Error err;
        if (m_gui)
        {
            err = m_gui->newFrame(dur);
            if (err)
                return err;
        }

        err = m_drawFunc(dur);
        if (err)
            return err;

        RenderPass * defaultPass = m_renderDevice->beginPass();
        m_quickDraw.addToPass(defaultPass);

        if (m_gui)
        {
            if (m_bShowWindowMetrics)
            {
                // based on imgui simple overlay sample. What else should be in here?
                const Float32 dist = 10.0f * backingScaleFactor();
                static int corner = 0;
                ImGuiIO & io = ImGui::GetIO();
                if (corner != -1)
                {
                    ImVec2 window_pos = ImVec2((corner & 1) ? io.DisplaySize.x - dist : dist,
                                               (corner & 2) ? io.DisplaySize.y - dist : dist);
                    ImVec2 window_pos_pivot =
                        ImVec2((corner & 1) ? 1.0f : 0.0f, (corner & 2) ? 1.0f : 0.0f);
                    ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
                }
                ImGui::SetNextWindowBgAlpha(0.3f); // Transparent background
                if (ImGui::Begin("Window Metrics",
                                 &m_bShowWindowMetrics,
                                 (corner != -1 ? ImGuiWindowFlags_NoMove : 0) |
                                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                                     ImGuiWindowFlags_AlwaysAutoResize |
                                     ImGuiWindowFlags_NoSavedSettings |
                                     ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav))
                {
                    ImGui::Text("FPS: %.2f\n", fps());
                    ImGui::Separator();
                    if (ImGui::IsMousePosValid())
                        ImGui::Text("Mouse Position: (%.1f,%.1f)", io.MousePos.x, io.MousePos.y);
                    else
                        ImGui::Text("Mouse Position: <invalid>");

                    if (ImGui::BeginPopupContextWindow())
                    {
                        if (ImGui::MenuItem("Custom", NULL, corner == -1))
                            corner = -1;
                        if (ImGui::MenuItem("Top-left", NULL, corner == 0))
                            corner = 0;
                        if (ImGui::MenuItem("Top-right", NULL, corner == 1))
                            corner = 1;
                        if (ImGui::MenuItem("Bottom-left", NULL, corner == 2))
                            corner = 2;
                        if (ImGui::MenuItem("Bottom-right", NULL, corner == 3))
                            corner = 3;
                        if (m_bShowWindowMetrics && ImGui::MenuItem("Close"))
                            m_bShowWindowMetrics = false;
                        ImGui::EndPopup();
                    }
                }
                ImGui::End();
            }

            err = m_gui->finalizeFrame(defaultPass);
            if (err)
                return err;
        }

        m_quickDraw.flush();
        err = m_renderDevice->endPass(defaultPass);
        if (err)
            return err;

        if (m_frameFinishedCallback)
        {
            err = m_frameFinishedCallback();
            if (err)
                return err;
        }

        // update the fps calculation
        Float64 fps = 1.0 / dur;
        m_fpsSMASum -= m_fpsBuffer[m_fpsIndex];
        m_fpsSMASum += fps;
        m_fpsBuffer[m_fpsIndex] = fps;
        if (++m_fpsIndex == m_fpsBuffer.count())
            m_fpsIndex = 0;
        m_fpsAvg = m_fpsSMASum / (Float64)m_fpsBuffer.count();

        ++m_frameCount;
        m_lastFrameTime = now;
        Window::swapBuffers();

        // figure out the time to sleep if any (to hit target fps)
        if (m_targetFps)
        {
            auto workTime = m_clock.now() - now;
            auto targetFrameTimeMS = 1.0 / *m_targetFps * 1000;
            if (workTime.milliseconds() < targetFrameTimeMS)
            {
                auto sleepDur = Duration::fromMilliseconds(targetFrameTimeMS) - workTime;
                Thread::sleepFor(sleepDur);
            }
        }
    }

    return Error();
}

void RenderWindow::setTargetFps(Float64 _fps)
{
    m_targetFps = _fps;
}

void RenderWindow::removeTargetFps()
{
    m_targetFps.reset();
}

Float64 RenderWindow::fps() const
{
    return m_fpsAvg;
}

Float64 RenderWindow::targetFps() const
{
    return m_targetFps ? *m_targetFps : std::numeric_limits<Float64>::infinity();
}

Size RenderWindow::frameCount() const
{
    return m_frameCount;
}

QuickDraw & RenderWindow::quickDraw()
{
    return m_quickDraw;
}

void RenderWindow::drawPathOutlineHelper(Path * _path,
                                         RenderInterface & _paperRenderer,
                                         bool _bDrawChildren)
{
    Vec2f * verts;
    Size count;
    _paperRenderer.flattenedPathVertices(_path, &verts, &count, _path->absoluteTransform());
    quickDraw().lineStrip(verts, count, _path->isClosed());
    if (_bDrawChildren)
    {
        for (Item * child : _path->children())
            drawPathOutlineHelper(static_cast<Path *>(child), _paperRenderer, _bDrawChildren);
    }
}

void RenderWindow::drawPathOutline(Path * _path,
                                   RenderInterface & _paperRenderer,
                                   const ColorRGBA & _col,
                                   bool _bDrawChildren)
{
    quickDraw().setTransform(_path->absoluteTransform());
    quickDraw().setColor(_col);
    drawPathOutlineHelper(_path, _paperRenderer, _bDrawChildren);
}

void RenderWindow::drawMultiplePathOutlines(Path ** _paths,
                                            Size _count,
                                            RenderInterface & _paperRenderer,
                                            const ColorRGBA & _col,
                                            bool _bDrawChildren)
{
    for (Size i = 0; i < _count; ++i)
        drawPathOutline(_paths[i], _paperRenderer, _col, _bDrawChildren);
}

static void _addHandleDrawData(Path * _path,
                               DynamicArray<Vec2f> & _outPositions,
                               DynamicArray<Vec2f> & _outLines,
                               bool _bDrawChildren)
{
    const Mat32f & absTrans = _path->absoluteTransform();
    for (auto seg : _path->segments())
    {
        const Vec2f & a = absTrans * seg.handleInAbsolute();
        const Vec2f & b = absTrans * seg.handleOutAbsolute();
        const Vec2f & c = absTrans * seg.position();

        _outPositions.append(a);
        _outPositions.append(b);
        _outLines.append(c);
        _outLines.append(a);
        _outLines.append(c);
        _outLines.append(b);
    }

    if (_bDrawChildren)
    {
        for (Item * child : _path->children())
            _addHandleDrawData(
                static_cast<Path *>(child), _outPositions, _outLines, _bDrawChildren);
    }
}

void RenderWindow::drawPathHandles(Path * _path,
                                   const ColorRGBA & _col,
                                   Float32 _radius,
                                   bool _bDrawChildren)
{
    drawMultiplePathHandles(&_path, 1, _col, _radius, _bDrawChildren);
}

void RenderWindow::drawMultiplePathHandles(
    Path ** _paths, Size _count, const ColorRGBA & _col, Float32 _radius, bool _bDrawChildren)
{
    //@TODO: put the buffer into either paperwindow or quickdraw
    DynamicArray<Vec2f> rects;
    rects.reserve(_count * 16);
    DynamicArray<Vec2f> lines;
    lines.reserve(_count * 32);
    for (Size i = 0; i < _count; ++i)
        _addHandleDrawData(_paths[i], rects, lines, _bDrawChildren);
    quickDraw().setTransform(Mat4f::identity());
    quickDraw().setColor(_col);
    quickDraw().lines(&lines[0], lines.count());
    quickDraw().rects(&rects[0], rects.count(), _radius);
}

static void _drawBoundingBoxHelper(Item * _item,
                                   const ColorRGBA & _col,
                                   bool _bDrawChildren,
                                   DynamicArray<QuickDraw::Vertex> & _outVerts)
{
    const Rectf & bounds = _item->bounds();
    _outVerts.append({ Vec3f(bounds.min().x, bounds.min().y, 0), _col, Vec2f(0) });
    _outVerts.append({ Vec3f(bounds.max().x, bounds.min().y, 0), _col, Vec2f(0) });
    _outVerts.append({ Vec3f(bounds.max().x, bounds.min().y, 0), _col, Vec2f(0) });
    _outVerts.append({ Vec3f(bounds.max().x, bounds.max().y, 0), _col, Vec2f(0) });
    _outVerts.append({ Vec3f(bounds.max().x, bounds.max().y, 0), _col, Vec2f(0) });
    _outVerts.append({ Vec3f(bounds.min().x, bounds.max().y, 0), _col, Vec2f(0) });
    _outVerts.append({ Vec3f(bounds.min().x, bounds.max().y, 0), _col, Vec2f(0) });
    _outVerts.append({ Vec3f(bounds.min().x, bounds.min().y, 0), _col, Vec2f(0) });
    if (_bDrawChildren)
    {
        for (Item * child : _item->children())
            _drawBoundingBoxHelper(child, _col, _bDrawChildren, _outVerts);
    }
}

void RenderWindow::drawItemBoundingBox(Item * _item, const ColorRGBA & _col, bool _bDrawChildren)
{
    DynamicArray<QuickDraw::Vertex> vertices;
    vertices.reserve(8);
    _drawBoundingBoxHelper(_item, _col, _bDrawChildren, vertices);
    quickDraw().setTransform(Mat4f::identity());
    quickDraw().lines(&vertices[0], vertices.count());
}

void RenderWindow::drawMultipleItemBoundingBoxes(Item ** _items,
                                                 Size _count,
                                                 const ColorRGBA & _col,
                                                 bool _bDrawChildren)
{
    DynamicArray<QuickDraw::Vertex> vertices;
    vertices.reserve(_count * 8);
    for (Size i = 0; i < _count; ++i)
        _drawBoundingBoxHelper(_items[i], _col, _bDrawChildren, vertices);
    quickDraw().setTransform(Mat4f::identity());
    quickDraw().lines(&vertices[0], vertices.count());
}

PaperWindow::PaperWindow() : m_bAutoResize(true)
{
}

Error PaperWindow::open(const WindowSettings & _settings)
{
    this->addEventCallback([this](const WindowResizeEvent & _evt) { this->updateDocumentSize(); });

    Error err = RenderWindow::open(_settings);
    if (err)
        return err;

    err = m_paperRenderer.init(m_doc);
    if (err)
        return err;

    updateDocumentSize();
    return Error();
}

void PaperWindow::updateDocumentSize()
{
    if (m_bAutoResize)
    {
        m_doc.setSize(width(), height());
        m_paperRenderer.setViewport(0, 0, widthInPixels(), heightInPixels());
        m_paperRenderer.setDefaultProjection();
    }
}

Document & PaperWindow::document()
{
    return m_doc;
}

tarp::TarpRenderer & PaperWindow::paperRenderer()
{
    return m_paperRenderer;
}

void PaperWindow::setAutoResize(bool _b)
{
    m_bAutoResize = _b;
}

bool PaperWindow::autoResize() const
{
    return m_bAutoResize;
}

void PaperWindow::drawDocument(RenderPass * _pass)
{
    _pass->drawCustom([this] { return m_paperRenderer.draw(); });
}

void PaperWindow::drawPathOutline(Path * _path, const ColorRGBA & _col, bool _bDrawChildren)
{
    RenderWindow::drawPathOutline(_path, m_paperRenderer, _col, _bDrawChildren);
}

void PaperWindow::drawMultiplePathOutlines(Path ** _paths,
                                           Size _count,
                                           const ColorRGBA & _col,
                                           bool _bDrawChildren)
{
    RenderWindow::drawMultiplePathOutlines(_paths, _count, m_paperRenderer, _col, _bDrawChildren);
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
        p->smooth(Smoothing::Continuous, false);
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

stick::Maybe<const char *> fileDialog(FileDialogFlags _flags,
                                      const char * _filters,
                                      const char * _defaultPath,
                                      const char * _defaultName)
{
    int flags = 0;
    if ((_flags & FileDialogFlags::Open) == FileDialogFlags::Open)
        flags |= NOC_FILE_DIALOG_OPEN;
    if ((_flags & FileDialogFlags::Save) == FileDialogFlags::Save)
        flags |= NOC_FILE_DIALOG_SAVE;
    if ((_flags & FileDialogFlags::Directory) == FileDialogFlags::Directory)
        flags |= NOC_FILE_DIALOG_DIR;
    if ((_flags & FileDialogFlags::OverwriteConfirmation) == FileDialogFlags::OverwriteConfirmation)
        flags |= NOC_FILE_DIALOG_OVERWRITE_CONFIRMATION;

    const char * res = noc_file_dialog_open(flags, _filters, _defaultPath, _defaultName);
    if (res)
        return res;
    return stick::Maybe<const char *>();
}

} // namespace chuckle
