// for modern openGl header files and linking to function implementations
#include <GL/glew.h>
// providing simple cross platform api 
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <csignal>
#include <fstream>
#define GLwrapper(x) clearErrorLog(); \
    x;\
    if(checkLogError()) std::raise(SIGINT)

static void clearErrorLog(){
    while(glGetError()!=GL_NO_ERROR);
}
static bool checkLogError(){
    GLenum errorCode = glGetError();
    if(errorCode != GL_NO_ERROR){
        std::cout << "error occured " << errorCode << std::endl;
        return true;
    }
    return false;
}

struct Shader
{
    std::string vertex;
    std::string fragment;
};
enum ShaderType
{
    VERTEX,
    FRAGMENT
};
// getting shaders from file
static Shader getShaders()
{
    std::string temp;
    Shader shader;
    ShaderType stype;
    std::ifstream file("../res/shaders/basic.shader");
    while (getline(file, temp))
    {
        if (temp.find("#shader") != std::string::npos)
        {
            if (temp.find("vertex") != std::string::npos)
            {
                stype = VERTEX;
            }
            else
            {
                stype = FRAGMENT;
            }
        }
        else
        {
            switch (stype)
            {
            case VERTEX:
                shader.vertex += "\n" + temp;
                break;
            case FRAGMENT:
                shader.fragment += "\n" + temp;
                break;

            default:
                break;
            }
        }
    }
    file.close();
    return shader;
}
// compiling shader 
static unsigned int CompileShader(unsigned int type, const std::string &source)
{
    unsigned int id = glCreateShader(type);
    const char *src = source.c_str();
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);
    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);

    if (result == GL_FALSE)
    {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        char *message = (char *)alloca(length * sizeof(char));
        glGetShaderInfoLog(id, length, &length, message);
        std::cout << "compilation failed" << std::endl;
        std::cout << message << std::endl;
        glDeleteShader(id);
        return 0;
    }
    return id;
}
// creating all shaders and linking with program
static unsigned int CreateShader(const std::string &vertexShader, const std::string &fragmentShader)
{
    unsigned int program = glCreateProgram();
    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    //checking for link error 
    GLint program_linked;
    
    glGetProgramiv(program, GL_LINK_STATUS, &program_linked);
    std::cout << "Program link status: " << program_linked << std::endl;
    if (program_linked != GL_TRUE)
    {
        GLsizei log_length = 0;
        GLchar message[1024];
        glGetProgramInfoLog(program, 1024, &log_length, message);
        std::cout << "Failed to link program" << std::endl;
        std::cout << message << std::endl;
    }
    glValidateProgram(program);
    glDeleteShader(vs);
    glDeleteShader(fs);
    return program;
}
int main(int argc, char * argv[])
{
    GLFWwindow *window;
    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);
    // catching glew error 
    if (glewInit() != GLEW_OK)
    {
        std::cout << "ERROR" << std::endl;
    }
    else
    {
        std::cout << "Working" << std::endl;
    }
    // vertices for triangle
    float vertices[] = {
        -0.5f, -0.5f,
        0.5f, -0.5f,
        0.5f, 0.5f,
        -0.5f, 0.5f,

        
    };
    // creating variable to store buffer 
    unsigned int buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    // storing out vertices data into it
    glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(float), vertices, GL_STATIC_DRAW);
    // enabling vertex attrib
    glEnableVertexAttribArray(0);
    // telling gpu how to interpret the data
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0);
    // index buffer for stop repeating ourselves and hence saving gpu memory
    unsigned int  indices[] = {
        0,1,2,
        2,3,0
    };
    unsigned int ibo;
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), indices, GL_STATIC_DRAW);

    // defining our custom shaders
    Shader shaders = getShaders();
    unsigned int shader = CreateShader(shaders.vertex, shaders.fragment);
    glUseProgram(shader);
    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        GLwrapper(glDrawElements(GL_TRIANGLES,6,GL_INT,nullptr));
        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }
    glDeleteProgram(shader);
    glfwTerminate();
    return 0;
}