#ifndef SHADER_H
#define SHADER_H

#if defined(__EMSCRIPTEN__)
#include <QOpenGLFunctions>
#else
#include <QOpenGLFunctions_3_0>
#endif

#if defined(__EMSCRIPTEN__)
class Shader : protected QOpenGLFunctions{
#else
class Shader : protected QOpenGLFunctions_3_0{
#endif
public:
    Shader();
    ~Shader();

    void InitializeGLFunctions(QOpenGLContext* context);

    void CreateProgram(const char* vertexShaderSource, const char* fragmentShaderSource);
    void UseProgram();
    void AddAttribute(const float* values, const int size, const char* name, const int stride);
    void AddUniform(const float* values, const int size, const char* name, const GLboolean transpose);

    void ChangeUniform(const int index, const float* values, const int size, const GLboolean transpose);

    void BindVAO();

    bool IsReady;

protected:
    GLuint programIndex;
    GLuint vertexShader;
    GLuint fragmentShader;
    GLuint vao;

    std::vector<GLuint> uniforms;
};

#endif // SHADER_H
