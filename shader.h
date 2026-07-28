#ifndef SHADER_H
#define SHADER_H

#if defined(__EMSCRIPTEN__)
#include <QOpenGLFunctions>
#else
#include <QOpenGLFunctions_3_0>
#endif

#include <QOpenGLContext>

#if defined(__EMSCRIPTEN__)
class Shader : protected QOpenGLFunctions{
#else
class Shader : protected QOpenGLFunctions_3_0{
#endif
public:
    Shader();
    ~Shader();

    // initializes the open gl functions for this shader to use
    void InitializeGLFunctions(QOpenGLContext* context);

    // creates a opengl program. Logs error if failing
    void CreateProgram(const char* vertexShaderSource, const char* fragmentShaderSource);
    // uses the program of the shader
    bool UseProgram();
    // adds an attribute to be useable in the shader
    void AddAttribute(const float* values, const int size, const char* name, const int stride);
    // adds a uniform to be useable in the shader
    void AddUniform(const float* values, const int size, const char* name, const GLboolean transpose);
    // creates a texture, attaches it to the color attachment of the current bound framebuffer and returns the GL id
    GLuint AddSharedRenderTexture(const int width, const int height, const int sharedRenderTextureSize);
    // creates a depth texture, attaches it to the depth attachment of the current bound framebuffer and returns the GL id
    GLuint AddDepthTexture(const int width, const int height);

    // changes the values of an attribute
    void ChangeAttribute(const int buffer, const float* values, const int size, const char* name, const int stride);
    // changes the values of an uniform
    void ChangeUniform(const int index, const float* values, const int size, const GLboolean transpose);

    // binds the vertex array object of this shader
    void BindVAO();
    // binds and activates the texture of this shader
    void BindTextures();
    // binds and activates shared textures, this needs changes when there is a shader that has textures and textures that are shared
    void BindSharedTextures(const std::vector<GLuint>& sharedTextures);

protected:
    QOpenGLContext* context;    // the context for the shader
    GLuint programIndex;        // the program id of the shader
    GLuint vertexShader;        // the vertex shader id
    GLuint fragmentShader;      // the fragment shader id
    GLuint vao;                 // the id of the vertex array object

    std::vector<GLuint> attributes;         // the ids of the attributes this shader has
    std::vector<GLuint> uniforms;           // the ids of the uniforms this shader has

    std::vector<GLuint> textures;           // the ids of the textures this shader has
    std::vector<GLuint> renderTextures;     // the ids of the render textures this shader has
    std::vector<GLuint> uniformSamplers;    // the ids of the texture samplers this shader has
};

#endif // SHADER_H
