#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <cstdint>
#include <string>

#include "../../common/GLShader.h"

#include "../../Context/DragonData.h"

static const int WINDOW_WIDTH = 960;
static const int WINDOW_HEIGHT = 540;

static bool g_LeftMouseDown = false;
static double g_LastMouseX = 0.0;
static double g_LastMouseY = 0.0;

// ------------------------------------------------------------
// Minimal math (formerly `src/Math.hpp`)
// ------------------------------------------------------------
struct Vec3
{
    float x = 0.f;
    float y = 0.f;
    float z = 0.f;
};

static inline Vec3 operator+(const Vec3& a, const Vec3& b) { return { a.x + b.x, a.y + b.y, a.z + b.z }; }
static inline Vec3 operator-(const Vec3& a, const Vec3& b) { return { a.x - b.x, a.y - b.y, a.z - b.z }; }
static inline Vec3 operator*(const Vec3& v, float s) { return { v.x * s, v.y * s, v.z * s }; }
static inline float Dot(const Vec3& a, const Vec3& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

static inline Vec3 Cross(const Vec3& a, const Vec3& b)
{
    return {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

static inline float Length(const Vec3& v) { return std::sqrt(Dot(v, v)); }

static inline Vec3 Normalize(const Vec3& v)
{
    float len = Length(v);
    if (len <= 1e-8f) return { 0.f, 0.f, 0.f };
    return v * (1.f / len);
}

// Column-major 4x4 matrix (OpenGL style)
struct Mat4
{
    float m[16] = {
        1,0,0,0,
        0,1,0,0,
        0,0,1,0,
        0,0,0,1
    };
};

static inline Mat4 Identity() { return Mat4{}; }

static inline Mat4 RotationY(float radians)
{
    Mat4 r = Identity();
    float c = std::cos(radians);
    float s = std::sin(radians);

    // column-major
    r.m[0] = c;  r.m[4] = 0; r.m[8] = -s; r.m[12] = 0;
    r.m[1] = 0;  r.m[5] = 1; r.m[9] = 0;  r.m[13] = 0;
    r.m[2] = s;  r.m[6] = 0; r.m[10] = c; r.m[14] = 0;
    r.m[3] = 0;  r.m[7] = 0; r.m[11] = 0; r.m[15] = 1;
    return r;
}

static inline Mat4 Perspective(float fovyRadians, float aspect, float zNear, float zFar)
{
    float f = 1.f / std::tan(fovyRadians * 0.5f);
    Mat4 p{};
    for (int i = 0; i < 16; ++i) p.m[i] = 0.f;

    p.m[0] = f / aspect;
    p.m[5] = f;
    p.m[10] = (zFar + zNear) / (zNear - zFar);
    p.m[11] = -1.f;
    p.m[14] = (2.f * zFar * zNear) / (zNear - zFar);
    return p;
}

// Right-handed LookAt (like gluLookAt): returns VIEW matrix (world -> camera)
static inline Mat4 LookAt(const Vec3& eye, const Vec3& target, const Vec3& up)
{
    Vec3 f = Normalize(target - eye);
    Vec3 s = Normalize(Cross(f, up));
    Vec3 u = Cross(s, f);

    Mat4 m = Identity();

    // Column-major: basis vectors in columns
    m.m[0] = s.x;  m.m[4] = s.y;  m.m[8] = s.z;
    m.m[1] = u.x;  m.m[5] = u.y;  m.m[9] = u.z;
    m.m[2] = -f.x; m.m[6] = -f.y; m.m[10] = -f.z;

    // Translation
    m.m[12] = -Dot(s, eye);
    m.m[13] = -Dot(u, eye);
    m.m[14] = Dot(f, eye);

    return m;
}

struct Mat3
{
    float m[9] = { 1,0,0, 0,1,0, 0,0,1 };
};

static inline Mat3 Mat3FromMat4(const Mat4& M)
{
    Mat3 r{};
    // upper-left 3x3, column-major
    r.m[0] = M.m[0]; r.m[1] = M.m[1]; r.m[2] = M.m[2];
    r.m[3] = M.m[4]; r.m[4] = M.m[5]; r.m[5] = M.m[6];
    r.m[6] = M.m[8]; r.m[7] = M.m[9]; r.m[8] = M.m[10];
    return r;
}

static inline Mat3 Transpose(const Mat3& A)
{
    Mat3 T{};
    T.m[0] = A.m[0]; T.m[3] = A.m[1]; T.m[6] = A.m[2];
    T.m[1] = A.m[3]; T.m[4] = A.m[4]; T.m[7] = A.m[5];
    T.m[2] = A.m[6]; T.m[5] = A.m[7]; T.m[8] = A.m[8];
    return T;
}

static inline float Det(const Mat3& A)
{
    return
        A.m[0] * (A.m[4] * A.m[8] - A.m[7] * A.m[5]) -
        A.m[3] * (A.m[1] * A.m[8] - A.m[7] * A.m[2]) +
        A.m[6] * (A.m[1] * A.m[5] - A.m[4] * A.m[2]);
}

static inline Mat3 Inverse(const Mat3& A)
{
    Mat3 inv{};
    float det = Det(A);
    if (std::fabs(det) < 1e-8f)
        return inv; // identity fallback

    float id = 1.f / det;
    inv.m[0] = (A.m[4] * A.m[8] - A.m[5] * A.m[7]) * id;
    inv.m[1] = -(A.m[1] * A.m[8] - A.m[2] * A.m[7]) * id;
    inv.m[2] = (A.m[1] * A.m[5] - A.m[2] * A.m[4]) * id;

    inv.m[3] = -(A.m[3] * A.m[8] - A.m[5] * A.m[6]) * id;
    inv.m[4] = (A.m[0] * A.m[8] - A.m[2] * A.m[6]) * id;
    inv.m[5] = -(A.m[0] * A.m[5] - A.m[2] * A.m[3]) * id;

    inv.m[6] = (A.m[3] * A.m[7] - A.m[4] * A.m[6]) * id;
    inv.m[7] = -(A.m[0] * A.m[7] - A.m[1] * A.m[6]) * id;
    inv.m[8] = (A.m[0] * A.m[4] - A.m[1] * A.m[3]) * id;

    return inv;
}

static inline Mat3 NormalMatrixFromModel(const Mat4& model)
{
    Mat3 m3 = Mat3FromMat4(model);
    return Transpose(Inverse(m3));
}

// ------------------------------------------------------------
// Orbit camera (formerly `src/OrbitCamera.*`)
// ------------------------------------------------------------
static inline float Clamp(float v, float a, float b)
{
    if (v < a) return a;
    if (v > b) return b;
    return v;
}

struct OrbitCamera
{
    Vec3 target{ 0.f, 0.f, 0.f };
    float distance = 6.0f;
    float yaw = 0.0f;   // radians
    float pitch = 0.4f; // radians

    float minDistance = 1.5f;
    float maxDistance = 25.0f;

    float rotateSpeed = 0.01f;
    float zoomSpeed = 0.5f;

    Mat4 GetViewMatrix() const
    {
        // spherical coordinates around target
        float cp = std::cos(pitch);
        float sp = std::sin(pitch);
        float cy = std::cos(yaw);
        float sy = std::sin(yaw);

        Vec3 eyeOffset{
            distance * cp * sy,
            distance * sp,
            distance * cp * cy
        };

        Vec3 eye = target + eyeOffset;
        Vec3 up{ 0.f, 1.f, 0.f };
        return LookAt(eye, target, up);
    }

    void OnMouseDrag(float dxPixels, float dyPixels)
    {
        yaw += dxPixels * rotateSpeed;
        pitch += dyPixels * rotateSpeed;
        pitch = Clamp(pitch, -1.5f, 1.5f);
    }

    void OnScroll(float yOffset)
    {
        distance -= yOffset * zoomSpeed;
        distance = Clamp(distance, minDistance, maxDistance);
    }
};

static OrbitCamera g_Camera;

// ------------------------------------------------------------
// Mesh setup (formerly `src/DragonMesh.*`)
// ------------------------------------------------------------
#ifdef __APPLE__
#define GL_GEN_VERTEX_ARRAYS glGenVertexArraysAPPLE
#define GL_BIND_VERTEX_ARRAY glBindVertexArrayAPPLE
#define GL_DELETE_VERTEX_ARRAYS glDeleteVertexArraysAPPLE
#else
#define GL_GEN_VERTEX_ARRAYS glGenVertexArrays
#define GL_BIND_VERTEX_ARRAY glBindVertexArray
#define GL_DELETE_VERTEX_ARRAYS glDeleteVertexArrays
#endif

struct DragonMesh
{
    uint32_t vao = 0;
    uint32_t vbo = 0;
    uint32_t ebo = 0;
    int indexCount = 0;

    bool Initialize(uint32_t program)
    {
        const int strideBytes = 8 * (int)sizeof(float); // X Y Z  NX NY NZ  U V

        const int locPos = glGetAttribLocation(program, "a_Position");
        const int locNormal = glGetAttribLocation(program, "a_Normal");
        if (locPos < 0 || locNormal < 0)
            return false;

        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(DragonVertices), DragonVertices, GL_STATIC_DRAW);

        glGenBuffers(1, &ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(DragonIndices), DragonIndices, GL_STATIC_DRAW);

        GL_GEN_VERTEX_ARRAYS(1, &vao);
        GL_BIND_VERTEX_ARRAY(vao);

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

        glVertexAttribPointer(locPos, 3, GL_FLOAT, GL_FALSE, strideBytes, (void*)0);
        glEnableVertexAttribArray(locPos);

        glVertexAttribPointer(locNormal, 3, GL_FLOAT, GL_FALSE, strideBytes, (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(locNormal);

        GL_BIND_VERTEX_ARRAY(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        indexCount = (int)(sizeof(DragonIndices) / sizeof(DragonIndices[0]));
        return true;
    }

    void Destroy()
    {
        if (vao) GL_DELETE_VERTEX_ARRAYS(1, &vao);
        if (vbo) glDeleteBuffers(1, &vbo);
        if (ebo) glDeleteBuffers(1, &ebo);
        vao = vbo = ebo = 0;
        indexCount = 0;
    }

    void Draw() const
    {
        GL_BIND_VERTEX_ARRAY(vao);
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_SHORT, nullptr);
    }
};

static void CursorPosCallback(GLFWwindow* window, double x, double y)
{
    (void)window;
    if (!g_LeftMouseDown)
    {
        g_LastMouseX = x;
        g_LastMouseY = y;
        return;
    }

    float dx = float(x - g_LastMouseX);
    float dy = float(y - g_LastMouseY);
    g_LastMouseX = x;
    g_LastMouseY = y;

    g_Camera.OnMouseDrag(dx, dy);
}

static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    (void)window;
    (void)mods;
    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        g_LeftMouseDown = (action == GLFW_PRESS);
    }
}

static void ScrollCallback(GLFWwindow* window, double xOffset, double yOffset)
{
    (void)window;
    (void)xOffset;
    g_Camera.OnScroll((float)yOffset);
}

int main()
{
    // Trace TP/TD:
    // - TD OpenGL moderne 4A partie 01: pipeline + VBO/IBO/VAO + attributs
    // - TP OpenGL moderne 02: rendu 3D (projection perspective + depth test + culling)
    // - TP Transformations: matrices M/V/P envoyées en uniforms
    // - TP illumination (partie 01): éclairage minimal diffuse + ambient pour valider les normales
    // - Devoir préparation projet: caméra orbitale + LookAt + normal matrix

    if (!glfwInit())
        return -1;

    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Preparation au projet - Dragon", nullptr, nullptr);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetCursorPosCallback(window, CursorPosCallback);
    glfwSetMouseButtonCallback(window, MouseButtonCallback);
    glfwSetScrollCallback(window, ScrollCallback);

    GLenum err = glewInit();
    if (err != GLEW_OK)
    {
        std::printf("Erreur GLEW\n");
    }

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    GLShader shader;
    shader.LoadVertexShader("shaders/project.vs.glsl");
    shader.LoadFragmentShader("shaders/project.fs.glsl");
    if (!shader.Create())
    {
        std::printf("Erreur: shader program\n");
        return -1;
    }

    uint32_t program = shader.GetProgram();
    glUseProgram(program);

    DragonMesh dragon;
    if (!dragon.Initialize(program))
    {
        std::printf("Erreur: init dragon mesh (attribs)\n");
        return -1;
    }

    // Uniform locations
    int locModel = glGetUniformLocation(program, "u_Model");
    int locView = glGetUniformLocation(program, "u_View");
    int locProj = glGetUniformLocation(program, "u_Projection");
    int locNormalMat = glGetUniformLocation(program, "u_NormalMatrix");
    int locLightDir = glGetUniformLocation(program, "u_LightDir");
    int locBaseColor = glGetUniformLocation(program, "u_BaseColor");

    // Camera defaults (simple)
    g_Camera.target = { 0.f, 1.0f, 0.f };
    g_Camera.distance = 8.0f;
    g_Camera.yaw = 0.2f;
    g_Camera.pitch = 0.3f;

    while (!glfwWindowShouldClose(window))
    {
        int w = 0, h = 0;
        glfwGetFramebufferSize(window, &w, &h);
        glViewport(0, 0, w, h);

        glClearColor(0.12f, 0.12f, 0.14f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // TP Transformations: M / V / P
        float aspect = (h > 0) ? (float(w) / float(h)) : 1.f;
        Mat4 projection = Perspective(60.f * 3.1415926f / 180.f, aspect, 0.1f, 100.f);
        Mat4 view = g_Camera.GetViewMatrix();

        // Simple slow rotation to show 3D
        float t = (float)glfwGetTime();
        Mat4 model = RotationY(t * 0.3f);

        // Devoir: normal matrix dédiée (dépend de M seulement, pas de V)
        Mat3 normalMat = NormalMatrixFromModel(model);

        glUseProgram(program);
        glUniformMatrix4fv(locModel, 1, GL_FALSE, model.m);
        glUniformMatrix4fv(locView, 1, GL_FALSE, view.m);
        glUniformMatrix4fv(locProj, 1, GL_FALSE, projection.m);
        glUniformMatrix3fv(locNormalMat, 1, GL_FALSE, normalMat.m);

        if (locLightDir >= 0) glUniform3f(locLightDir, 0.2f, 0.9f, 0.4f);
        if (locBaseColor >= 0) glUniform3f(locBaseColor, 0.85f, 0.65f, 0.45f);

        dragon.Draw();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    dragon.Destroy();
    shader.Destroy();

    glfwTerminate();
    return 0;
}

