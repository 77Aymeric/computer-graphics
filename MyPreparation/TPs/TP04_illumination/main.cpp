#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "../../../common/GLShader.h"
#include "../../PreparationAuProjet/src/Math.hpp"
#include "../../PreparationAuProjet/src/DragonMesh.hpp"

#include "../../PreparationAuProjet/src/DragonMesh.cpp" // simple pour TP: évite de refaire un .cpp

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

int main()
{
    if (!glfwInit()) return -1;

    GLFWwindow* window = glfwCreateWindow(960, 540, "TP Illumination", nullptr, nullptr);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK)
        std::printf("Erreur GLEW\n");

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    GLShader shader;
    shader.LoadVertexShader("shaders/illum.vs.glsl");
    shader.LoadFragmentShader("shaders/illum.fs.glsl");
    if (!shader.Create()) return -1;
    uint32_t program = shader.GetProgram();
    glUseProgram(program);

    DragonMesh dragon;
    if (!dragon.Initialize(program)) return -1;

    int locModel = glGetUniformLocation(program, "u_Model");
    int locView = glGetUniformLocation(program, "u_View");
    int locProj = glGetUniformLocation(program, "u_Projection");
    int locNormalMat = glGetUniformLocation(program, "u_NormalMatrix");
    int locLightDir = glGetUniformLocation(program, "u_LightDir");
    int locBaseColor = glGetUniformLocation(program, "u_BaseColor");

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
        Mat4 V = Translation({ 0.f, -1.0f, -8.f });
        float t = (float)glfwGetTime();
        Mat4 M = RotationY(t * 0.3f);
        Mat3 N = NormalMatrixFromModel(M);

        glUseProgram(program);
        glUniformMatrix4fv(locModel, 1, GL_FALSE, M.m);
        glUniformMatrix4fv(locView, 1, GL_FALSE, V.m);
        glUniformMatrix4fv(locProj, 1, GL_FALSE, P.m);
        glUniformMatrix3fv(locNormalMat, 1, GL_FALSE, N.m);
        glUniform3f(locLightDir, 0.2f, 0.9f, 0.4f);
        glUniform3f(locBaseColor, 0.85f, 0.65f, 0.45f);

        dragon.Draw();

        if (autoCapture && !captured)
        {
            SaveScreenshotPPM("screens/tp04", w, h);
            captured = true;
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    dragon.Destroy();
    shader.Destroy();
    glfwTerminate();
    return 0;
}

