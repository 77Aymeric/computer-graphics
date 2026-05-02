#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "../../../common/GLShader.h"

#ifdef __APPLE__
#define GL_GEN_VERTEX_ARRAYS glGenVertexArraysAPPLE
#define GL_BIND_VERTEX_ARRAY glBindVertexArrayAPPLE
#define GL_DELETE_VERTEX_ARRAYS glDeleteVertexArraysAPPLE
#else
#define GL_GEN_VERTEX_ARRAYS glGenVertexArrays
#define GL_BIND_VERTEX_ARRAY glBindVertexArray
#define GL_DELETE_VERTEX_ARRAYS glDeleteVertexArrays
#endif

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

struct Vertex
{
    float x, y;
    float r, g, b;
};

int main()
{
    if (!glfwInit()) return -1;

    GLFWwindow* window = glfwCreateWindow(640, 480, "TP01 - Pipeline", nullptr, nullptr);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK)
        std::printf("Erreur GLEW\n");

    GLShader shader;
    shader.LoadVertexShader("shaders/basic.vs.glsl");
    shader.LoadFragmentShader("shaders/basic.fs.glsl");
    if (!shader.Create()) return -1;

    uint32_t program = shader.GetProgram();
    glUseProgram(program);

    const Vertex vertices[] = {
        {  0.0f,  0.6f, 1.0f, 0.0f, 0.0f },
        { -0.6f, -0.6f, 0.0f, 1.0f, 0.0f },
        {  0.6f, -0.6f, 0.0f, 0.0f, 1.0f },
    };
    const uint16_t indices[] = { 0, 1, 2 };

    GLuint vbo = 0, ebo = 0, vao = 0;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    GL_GEN_VERTEX_ARRAYS(1, &vao);
    GL_BIND_VERTEX_ARRAY(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

    int locPos = glGetAttribLocation(program, "a_Position");
    int locCol = glGetAttribLocation(program, "a_Color");
    glVertexAttribPointer(locPos, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(locPos);
    glVertexAttribPointer(locCol, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(locCol);

    GL_BIND_VERTEX_ARRAY(0);

    const bool autoCapture = (std::getenv("AUTO_CAPTURE") != nullptr);
    bool captured = false;

    while (!glfwWindowShouldClose(window))
    {
        int w = 0, h = 0;
        glfwGetFramebufferSize(window, &w, &h);
        glViewport(0, 0, w, h);
        glClearColor(0.2f, 0.2f, 0.2f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(program);
        GL_BIND_VERTEX_ARRAY(vao);
        glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, nullptr);

        if (autoCapture && !captured)
        {
            SaveScreenshotPPM("screens/tp01", w, h);
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

