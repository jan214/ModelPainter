#include "shader.h"

Shader::Shader() :
context(),
programIndex(0),
vertexShader(0),
fragmentShader(0),
vao(0){
}

Shader::~Shader(){
}

void Shader::InitializeGLFunctions(QOpenGLContext* context){
    if(context == nullptr || context->surface() == nullptr)
        return;

    context->makeCurrent(context->surface());
    initializeOpenGLFunctions();
    printf("Shader initialized GL function\n");
}

void Shader::CreateProgram(const char* vertexShaderSource, const char* fragmentShaderSource){
    programIndex = glCreateProgram();
    printf("createProgram: %i\n", programIndex);

    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    GLint success = GL_FALSE;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if(!success){
        GLint logLength;
        glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &logLength);
        char* infoLog = new char[logLength];
        glGetShaderInfoLog(vertexShader, logLength, NULL, infoLog);
        printf("vertex: %s\n", infoLog);
    }

    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    success = GL_FALSE;
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if(!success){
        GLint logLength;
        glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &logLength);
        char* infoLog = new char[logLength];
        glGetShaderInfoLog(fragmentShader, logLength, NULL, infoLog);
        printf("fragment: %s\n", infoLog);
    }

    glAttachShader(programIndex, vertexShader);
    glAttachShader(programIndex, fragmentShader);

    glLinkProgram(programIndex);
    success = GL_FALSE;
    glGetProgramiv(programIndex, GL_LINK_STATUS, &success);
    if (!success) {
        printf("program linking failed\n");
    }
}

bool Shader::UseProgram(){
    if(programIndex == 0){
        printf("program index == 0\n");
        return false;
    }
    glUseProgram(programIndex);
    return true;
}

void Shader::AddAttribute(const float* values, const int size, const char* name, const int stride){
    if(programIndex == 0){
        printf("program not initialized\n");
        return;
    }

    if(vao == 0){
        glGenVertexArrays(1, &vao);
        printf("gen vao\n");
    }

    BindVAO();

    GLuint newBuffer;
    glGenBuffers(1, &newBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, newBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*size, &values[0], GL_DYNAMIC_DRAW);

    const GLuint attributeLocation = glGetAttribLocation(programIndex, name);
    glVertexAttribPointer(attributeLocation, stride, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(attributeLocation);

    glBindVertexArray(0);

    printf("AddAttribute: %s %i\n", name, newBuffer);

    attributes.push_back(newBuffer);
}

void Shader::AddUniform(const float* values, const int size, const char* name, const GLboolean transpose){
    const GLuint uniformLocation = glGetUniformLocation(programIndex, name);
    printf("uniformLocation %s: %d\n", name, uniformLocation);

    if(size == 1 && values == nullptr){
        glUniform1i(uniformLocation, uniforms.size());
        uniformSamplers.push_back(uniformLocation);
    }else if(size == 1 && values != nullptr){
        glUniform1fv(uniformLocation, 1, values);
    }else if(size == 2){
        glUniform2fv(uniformLocation, 1, &values[0]);
    }else if(size == 3){
        glUniform3fv(uniformLocation, 1, &values[0]);
    }else if(size == 16){
        glUniformMatrix4fv(uniformLocation, 1, transpose, &values[0]);
    }

    uniforms.push_back(uniformLocation);
    printf("AddUniform\n");
}

GLuint Shader::AddSharedRenderTexture(const int width, const int height, const int sharedRenderTexturesSize) {
    GLuint newRenderTexture;
    glGenTextures(1, &newRenderTexture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, newRenderTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    printf("color attachment %i attached to framebuffer textureID: %d\n", GL_COLOR_ATTACHMENT0 + sharedRenderTexturesSize, newRenderTexture);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + sharedRenderTexturesSize, GL_TEXTURE_2D, newRenderTexture, 0);

    return newRenderTexture;
}

GLuint Shader::AddDepthTexture(const int width, const int height) {
    GLuint newDepthTexture;
    glGenTextures(1, &newDepthTexture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, newDepthTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

    printf("depth buffer attached to framebuffer\n");
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, newDepthTexture, 0);

    //sharedRenderTextures.push_back(newDepthTexture);
    return newDepthTexture;
}

void Shader::ChangeAttribute(const int buffer, const float* values, const int size, const char* name, const int stride) {
    if (size == 0) {
        GLint location = glGetAttribLocation(programIndex, name);

        glDisableVertexAttribArray(location);
        glVertexAttrib2f(location, 0.0f, 0.0f);
        return;
    }

    const GLuint bufferIndex = attributes[buffer];

    BindVAO();

    glBindBuffer(GL_ARRAY_BUFFER, bufferIndex);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * size, &values[0], GL_DYNAMIC_DRAW);

    const GLuint attributeLocation = glGetAttribLocation(programIndex, name);
    glVertexAttribPointer(attributeLocation, stride, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(attributeLocation);

    glBindVertexArray(0);

    printf("ChangeAttribute: %s %i\n", name, bufferIndex);
}

void Shader::ChangeUniform(const int index, const float* values, const int size, const GLboolean transpose){
    if(size == 1 && values == nullptr){
        glUniform1i(uniforms[index], index);
    }else if(size == 1 && values != nullptr){
        glUniform1fv(uniforms[index], 1, values);
    }else if(size == 2){
        glUniform2fv(uniforms[index], 1, &values[0]);
    }else if(size == 3){
        glUniform3fv(uniforms[index], 1, &values[0]);
    }else if(size == 16){
        glUniformMatrix4fv(uniforms[index], 1, transpose, &values[0]);
    }
}

void Shader::BindVAO(){
    glBindVertexArray(vao);
}

void Shader::BindTextures() {
    const int texturesCount = textures.size();
    for (int counter = 0; counter < texturesCount; counter++) {
        glActiveTexture(GL_TEXTURE0 + counter);
        const GLuint& textureId = textures[counter];
        glBindTexture(GL_TEXTURE_2D, textureId);
        const GLint uniformLocation = uniformSamplers[counter];
        glUniform1i(uniformLocation, counter);
        printf("sharedTextureID: %i uniformLocation: %i texture: %i\n", textureId, uniformLocation, GL_TEXTURE0 + counter);
    }
}

void Shader::BindSharedTextures(const std::vector<GLuint>& sharedTextures) {
    const int sharedTexturesCount = sharedTextures.size();
    for (int counter = 0; counter < sharedTexturesCount; counter++) {
        glActiveTexture(GL_TEXTURE0 + counter);
        const GLuint& textureId = sharedTextures[counter];
        glBindTexture(GL_TEXTURE_2D, textureId);
        const GLint uniformLocation = uniformSamplers[counter];
        glUniform1i(uniformLocation, counter);
        printf("sharedTextureID: %i uniformLocation: %i texture: %i\n", textureId, uniformLocation, GL_TEXTURE0 + counter);
    }
}
