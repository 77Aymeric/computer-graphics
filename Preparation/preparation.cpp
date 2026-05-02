#include <array>
#include <cmath>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "common/GLShader.h"

#include "../Context/DragonData.h"

static const int WINDOW_WIDTH = 960;
static const int WINDOW_HEIGHT = 540;

struct Vec3
{
    float x{};
    float y{};
    float z{};
};

struct Mat4
{
    std::array<float, 16> m{};
};

Mat4 Multiply(const Mat4& A, const Mat4& B)
{
    Mat4 C{};
    for (int col = 0; col < 4; ++col)
    {
        for (int row = 0; row < 4; ++row)
        {
            float s = 0.0f;
            for (int k = 0; k < 4; ++k)
            {
                const float a = A.m[k * 4 + row];
                const float b = B.m[col * 4 + k];
                s += a * b;
            }
            C.m[col * 4 + row] = s;
        }
    }
    return C;
}

static Vec3 Sub(const Vec3& a, const Vec3& b)
{
    return { a.x - b.x, a.y - b.y, a.z - b.z };
}

static float Dot(const Vec3& a, const Vec3& b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

static Vec3 Cross(const Vec3& a, const Vec3& b)
{
    return { a.y * b.z - a.z * b.y,
             a.z * b.x - a.x * b.z,
             a.x * b.y - a.y * b.x };
}

static Vec3 Normalize(const Vec3& v)
{
    float len = std::sqrt(Dot(v, v));
    if (len <= 1e-8f) return { 0.0f, 0.0f, 0.0f };
    float inv = 1.0f / len;
    return { v.x * inv, v.y * inv, v.z * inv };
}

static Mat4 Identity()
{
    Mat4 I{};
    I.m = { 1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1 };
    return I;
}

Mat4 LookAt(const Vec3& position, const Vec3& target, const Vec3& up)
{
    Vec3 forward = Normalize(Sub(position, target));
    Vec3 right = Normalize(Cross(up, forward));
    Vec3 up2 = Cross(forward, right);

    float tx = -Dot(position, right);
    float ty = -Dot(position, up2);
    float tz = -Dot(position, forward);

    Mat4 M = Identity();

    M.m[0] = right.x;
    M.m[1] = right.y;
    M.m[2] = right.z;

    M.m[4] = up2.x;
    M.m[5] = up2.y;
    M.m[6] = up2.z;

    M.m[8] = forward.x;
    M.m[9] = forward.y;
    M.m[10] = forward.z;

    M.m[12] = tx;
    M.m[13] = ty;
    M.m[14] = tz;

    return M;
}

#ifdef __APPLE__
#define GL_GEN_VERTEX_ARRAYS glGenVertexArraysAPPLE
#define GL_BIND_VERTEX_ARRAY glBindVertexArrayAPPLE
#define GL_DELETE_VERTEX_ARRAYS glDeleteVertexArraysAPPLE
#else
#define GL_GEN_VERTEX_ARRAYS glGenVertexArrays
#define GL_BIND_VERTEX_ARRAY glBindVertexArray
#define GL_DELETE_VERTEX_ARRAYS glDeleteVertexArrays
#endif

struct Mat3
{
    std::array<float, 9> m{};
};

static float Clamp(float v, float a, float b)
{
    if (v < a) return a;
    if (v > b) return b;
    return v;
}

static float WrapPi(float a)
{
    const float pi = 3.1415926535f;
    const float twoPi = 2.0f * pi;
    while (a > pi) a -= twoPi;
    while (a < -pi) a += twoPi;
    return a;
}

static Mat4 Perspective(float fovyRadians, float aspect, float zNear, float zFar)
{
    float f = 1.0f / std::tan(fovyRadians * 0.5f);
    Mat4 p{};
    for (int i = 0; i < 16; ++i) p.m[i] = 0.0f;
    p.m[0] = f / aspect;
    p.m[5] = f;
    p.m[10] = (zFar + zNear) / (zNear - zFar);
    p.m[11] = -1.0f;
    p.m[14] = (2.0f * zFar * zNear) / (zNear - zFar);
    return p;
}

static Mat3 Mat3FromMat4(const Mat4& M)
{
    Mat3 r{};
    r.m = {
        M.m[0], M.m[1], M.m[2],
        M.m[4], M.m[5], M.m[6],
        M.m[8], M.m[9], M.m[10]
    };
    return r;
}

static Mat3 Transpose(const Mat3& A)
{
    Mat3 T{};
    T.m = {
        A.m[0], A.m[3], A.m[6],
        A.m[1], A.m[4], A.m[7],
        A.m[2], A.m[5], A.m[8]
    };
    return T;
}

static float Determinant(const Mat3& A)
{
    return
        A.m[0] * (A.m[4] * A.m[8] - A.m[7] * A.m[5]) -
        A.m[3] * (A.m[1] * A.m[8] - A.m[7] * A.m[2]) +
        A.m[6] * (A.m[1] * A.m[5] - A.m[4] * A.m[2]);
}

static Mat3 Inverse(const Mat3& A)
{
    Mat3 inv{};
    float det = Determinant(A);
    if (std::fabs(det) < 1e-8f)
    {
        inv.m = { 1, 0, 0,
                  0, 1, 0,
                  0, 0, 1 };
        return inv;
    }

    float id = 1.0f / det;
    inv.m = {
        (A.m[4] * A.m[8] - A.m[5] * A.m[7]) * id,
        -(A.m[1] * A.m[8] - A.m[2] * A.m[7]) * id,
        (A.m[1] * A.m[5] - A.m[2] * A.m[4]) * id,
        -(A.m[3] * A.m[8] - A.m[5] * A.m[6]) * id,
        (A.m[0] * A.m[8] - A.m[2] * A.m[6]) * id,
        -(A.m[0] * A.m[5] - A.m[2] * A.m[3]) * id,
        (A.m[3] * A.m[7] - A.m[4] * A.m[6]) * id,
        -(A.m[0] * A.m[7] - A.m[1] * A.m[6]) * id,
        (A.m[0] * A.m[4] - A.m[1] * A.m[3]) * id
    };
    return inv;
}

static Mat3 NormalMatrixFromModel(const Mat4& model)
{
    Mat3 m3 = Mat3FromMat4(model);
    return Transpose(Inverse(m3));
}

struct OrbitCamera
{
    Vec3 target{ 0.0f, 0.0f, 0.0f };
    float distance = 8.0f;
    float phi = 0.0f;
    float theta = 0.3f;

    float rotateSpeed = 0.01f;
    float zoomSpeed = 0.6f;
    float minDistance = 1.5f;
    float maxDistance = 50.0f;

    void OnMouseDrag(float dxPixels, float dyPixels)
    {
        phi = WrapPi(phi + dxPixels * rotateSpeed);
        theta = Clamp(theta + dyPixels * rotateSpeed, -1.55f, 1.55f);
    }

    void OnScroll(float yOffset)
    {
        distance = Clamp(distance - yOffset * zoomSpeed, minDistance, maxDistance);
    }

    Mat4 GetViewMatrix() const
    {
        float y = distance * std::sin(theta);
        float x = distance * std::cos(theta) * std::cos(phi);
        float z = distance * std::cos(theta) * std::sin(phi);

        Vec3 eye{ target.x + x, target.y + y, target.z + z };
        Vec3 up{ 0.0f, 1.0f, 0.0f };
        return LookAt(eye, target, up);
    }
};

static OrbitCamera g_Camera;
static bool g_LeftMouseDown = false;
static double g_LastMouseX = 0.0;
static double g_LastMouseY = 0.0;

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
        g_LeftMouseDown = (action == GLFW_PRESS);
}

static void ScrollCallback(GLFWwindow* window, double xOffset, double yOffset)
{
    (void)window;
    (void)xOffset;
    g_Camera.OnScroll((float)yOffset);
}

struct DragonMesh
{
    uint32_t vao = 0;
    uint32_t vbo = 0;
    uint32_t ebo = 0;
    int indexCount = 0;

    bool Initialize(uint32_t program)
    {
        const int strideBytes = 8 * (int)sizeof(float);
        const int locPos = glGetAttribLocation(program, "a_Position");
        const int locNormal = glGetAttribLocation(program, "a_Normal");
        if (locPos < 0 || locNormal < 0) return false;

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
        GL_BIND_VERTEX_ARRAY(0);
    }
};

int main()
{
    if (!glfwInit()) return 1;
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Preparation", nullptr, nullptr);
    if (!window)
    {
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);
    glfwSetCursorPosCallback(window, CursorPosCallback);
    glfwSetMouseButtonCallback(window, MouseButtonCallback);
    glfwSetScrollCallback(window, ScrollCallback);

    GLenum err = glewInit();
    if (err != GLEW_OK)
    {
        glfwTerminate();
        return 1;
    }

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    GLShader shader;
    if (!shader.LoadVertexShader("shaders/project.vs.glsl")) return 1;
    if (!shader.LoadFragmentShader("shaders/project.fs.glsl")) return 1;
    if (!shader.Create()) return 1;

    uint32_t program = shader.GetProgram();
    glUseProgram(program);

    DragonMesh dragon;
    if (!dragon.Initialize(program)) return 1;

    GLint locModel = glGetUniformLocation(program, "u_Model");
    GLint locView = glGetUniformLocation(program, "u_View");
    GLint locProj = glGetUniformLocation(program, "u_Projection");
    GLint locNormalMat = glGetUniformLocation(program, "u_NormalMatrix");

    g_Camera.target = { 0.0f, 1.0f, 0.0f };
    g_Camera.distance = 8.0f;
    g_Camera.phi = 0.2f;
    g_Camera.theta = 0.3f;

    Mat4 model = Identity();
    Mat3 normalMat = NormalMatrixFromModel(model);

    while (!glfwWindowShouldClose(window))
    {
        int w = 0, h = 0;
        glfwGetFramebufferSize(window, &w, &h);
        glViewport(0, 0, w, h);
        glClearColor(0.12f, 0.12f, 0.14f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float aspect = (h > 0) ? (float(w) / float(h)) : 1.0f;
        Mat4 projection = Perspective(60.0f * 3.1415926535f / 180.0f, aspect, 0.1f, 100.0f);
        Mat4 view = g_Camera.GetViewMatrix();

        glUseProgram(program);
        if (locModel >= 0) glUniformMatrix4fv(locModel, 1, GL_FALSE, model.m.data());
        if (locView >= 0) glUniformMatrix4fv(locView, 1, GL_FALSE, view.m.data());
        if (locProj >= 0) glUniformMatrix4fv(locProj, 1, GL_FALSE, projection.m.data());
        if (locNormalMat >= 0) glUniformMatrix3fv(locNormalMat, 1, GL_FALSE, normalMat.m.data());

        dragon.Draw();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    dragon.Destroy();
    shader.Destroy();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
