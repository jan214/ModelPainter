#include "shader.h"

Shader::Shader() :
IsReady(false),
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

    context = context;

    context->makeCurrent(context->surface());
    initializeOpenGLFunctions();
    printf("Shader initialized GL function\n");
}

void Shader::CreateProgram(const char* vertexShaderSource, const char* fragmentShaderSource){
    programIndex = glCreateProgram();
    printf("createProgram\n");

    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    GLint success;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if(success == GL_FALSE){
        GLint logLength;
        glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &logLength);
        char* infoLog = new char[logLength];
        glGetShaderInfoLog(vertexShader, logLength, NULL, infoLog);
        printf("vertex: %s\n", infoLog);
    }

    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if(success == GL_FALSE){
        GLint logLength;
        glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &logLength);
        char* infoLog = new char[logLength];
        glGetShaderInfoLog(fragmentShader, logLength, NULL, infoLog);
        printf("fragment: %s\n", infoLog);
    }

    glAttachShader(programIndex, vertexShader);
    glAttachShader(programIndex, fragmentShader);

    glLinkProgram(programIndex);
}

void Shader::UseProgram(){
    if(programIndex == 0){
        printf("program index == 0\n");
    }
    glUseProgram(programIndex);
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

    if(size == 1){
        glUniform1i(uniformLocation, uniforms.size());
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
    if(size == 1){
        glUniform1i(uniforms[index], index);
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
