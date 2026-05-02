#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "../../../common/GLShader.h"
#include "../../PreparationAuProjet/src/Math.hpp"

#ifdef __APPLE__
#define GL_GEN_VERTEX_ARRAYS glGenVertexArraysAPPLE
#define GL_BIND_VERTEX_ARRAY glBindVertexArrayAPPLE
#define GL_DELETE_VERTEX_ARRAYS glDeleteVertexArraysAPPLE
#else
#define GL_GEN_VERTEX_ARRAYS glGenVertexArrays
#define GL_BIND_VERTEX_ARRAY glBindVertexArray
#define GL_DELETE_VERTEX_ARRAYS glDeleteVertexArrays
#endif

static Mat4 Scale(float s)
{
    Mat4 m = Identity();
    m.m[0] = s;
    m.m[5] = s;
    m.m[10] = s;
    return m;
}

static bool WritePPM(const char* filename, int w, int h, const unsigned char* rgb)
{
    FILE* f = std::fopen(filename, "wb");
    if (!f) return false;
    std::fprintf(f, "P6\n%d %d\n255\n", w, h);
    std::fwrite(rgb, 1, (size_t)(w * h * 3), f);
    std::fclose(f);
    return true;
}

static bool SaveScreenshotPPM(const char* filenameNoExt, int w, int h)
{
    unsigned char* pixels = (unsigned char*)std::malloc((size_t)w * (size_t)h * 3);
    unsigned char* flipped = (unsigned char*)std::malloc((size_t)w * (size_t)h * 3);
    if (!pixels || !flipped) { std::free(pixels); std::free(flipped); return false; }

    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, pixels);
    for (int y = 0; y < h; ++y)
        std::memcpy(flipped + (size_t)y * (size_t)w * 3,
                    pixels + (size_t)(h - 1 - y) * (size_t)w * 3,
                    (size_t)w * 3);

    char path[256];
    std::snprintf(path, sizeof(path), "%s.ppm", filenameNoExt);
    bool ok = WritePPM(path, w, h, flipped);
    std::free(pixels);
    std::free(flipped);
    return ok;
}

struct V3 { float x, y, z; };
struct Vertex { V3 pos; V3 col; };

int main()
{
    if (!glfwInit()) return -1;

    GLFWwindow* window = glfwCreateWindow(800, 600, "TP Transformations", nullptr, nullptr);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    if (glewInit() != GLEW_OK) std::printf("Erreur GLEW\n");

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    GLShader shader;
    shader.LoadVertexShader("shaders/transform.vs.glsl");
    shader.LoadFragmentShader("shaders/transform.fs.glsl");
    if (!shader.Create()) return -1;
    uint32_t program = shader.GetProgram();
    glUseProgram(program);

    const Vertex cubeVerts[] = {
        {{-1.f,-1.f, 1.f}, {1,0,0}}, {{ 1.f,-1.f, 1.f}, {0,1,0}}, {{ 1.f, 1.f, 1.f}, {0,0,1}}, {{-1.f, 1.f, 1.f}, {1,1,0}},
        {{-1.f,-1.f,-1.f}, {1,0,1}}, {{ 1.f,-1.f,-1.f}, {0,1,1}}, {{ 1.f, 1.f,-1.f}, {1,1,1}}, {{-1.f, 1.f,-1.f}, {0.5f,0.5f,0.5f}},
    };
    const uint16_t cubeIdx[] = {
        0,1,2, 2,3,0, 1,5,6, 6,2,1, 7,6,5, 5,4,7,
        4,0,3, 3,7,4, 4,5,1, 1,0,4, 3,2,6, 6,7,3
    };

    GLuint vbo = 0, ebo = 0, vao = 0;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVerts), cubeVerts, GL_STATIC_DRAW);
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIdx), cubeIdx, GL_STATIC_DRAW);

    GL_GEN_VERTEX_ARRAYS(1, &vao);
    GL_BIND_VERTEX_ARRAY(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

    int locPos = glGetAttribLocation(program, "a_Position");
    int locCol = glGetAttribLocation(program, "a_Color");
    glVertexAttribPointer(locPos, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(locPos);
    glVertexAttribPointer(locCol, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(locCol);

    int locModel = glGetUniformLocation(program, "u_Model");
    int locView = glGetUniformLocation(program, "u_View");
    int locProj = glGetUniformLocation(program, "u_Projection");

    const bool autoCapture = (std::getenv("AUTO_CAPTURE") != nullptr);
    bool captured = false;

    while (!glfwWindowShouldClose(window))
    {
        int w = 0, h = 0;
        glfwGetFramebufferSize(window, &w, &h);
        glViewport(0, 0, w, h);
        glClearColor(0.12f, 0.12f, 0.14f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float aspect = (h > 0) ? float(w) / float(h) : 1.f;
        Mat4 P = Perspective(60.f * 3.1415926f / 180.f, aspect, 0.1f, 100.f);
        Mat4 V = Translation({ 0.f, 0.f, -7.f });

        float t = (float)glfwGetTime();
        Mat4 S = Scale(0.8f);
        Mat4 R = RotationY(t * 0.8f);
        Mat4 T = Translation({ 0.f, 0.f, 0.f });
        // ordre “classique” : T * R * S
        Mat4 M = Mul(T, Mul(R, S));

        glUseProgram(program);
        glUniformMatrix4fv(locModel, 1, GL_FALSE, M.m);
        glUniformMatrix4fv(locView, 1, GL_FALSE, V.m);
        glUniformMatrix4fv(locProj, 1, GL_FALSE, P.m);

        GL_BIND_VERTEX_ARRAY(vao);
        glDrawElements(GL_TRIANGLES, (GLsizei)(sizeof(cubeIdx) / sizeof(cubeIdx[0])), GL_UNSIGNED_SHORT, nullptr);

        if (autoCapture && !captured)
        {
            SaveScreenshotPPM("screens/tp03", w, h);
            captured = true;
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    GL_DELETE_VERTEX_ARRAYS(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
    shader.Destroy();
    glfwTerminate();
    return 0;
}

