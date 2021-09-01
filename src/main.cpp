#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"
#include "GLTFLoader.h"

#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(150.0f, 225.0f, 225.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    //创建窗口并设置其大小，名称，与检测是否创建成功
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Shader", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    //创建完毕之后，需要让当前窗口的环境在当前线程上成为当前环境，就是接下来的画图都会画在我们刚刚创建的窗口上
    glfwMakeContextCurrent(window);
    //告诉GLFW我们希望每当窗口调整大小的时候调用这个函数
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    //监听鼠标移动事件 回调鼠标响应函数
    glfwSetCursorPosCallback(window, mouse_callback);
    //监听鼠标滚轮的移动
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse隐藏鼠标的光标 光标会一直停留在窗口中
//    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    stbi_set_flip_vertically_on_load(true);

    // configure global opengl state
    // 启用深度测试，默认是关闭的
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile shaders
    // -------------------------
    Shader ourShader("../../src/test.vs", "../../src/test.fs");

    // load models
    // -----------
    Model ourModel("../../data/car/scene.gltf");



    // draw in wireframe
    // 绘制线框
//    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

//    std::cout<<ourModel.meshes[0].primitives[0].VVAO<<"\n";

    // 分配纹理单元
    ourShader.use(); // 不要忘记在设置uniform变量之前激活着色器程序！
    ourShader.setInt("baseColorTexture", 0); // 或者使用着色器类设置
    ourShader.setInt("metallicRoughnessTexture", 1);
    ourShader.setInt("normalTexture", 2);


    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // 计算上下两帧的渲染时间差
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        //响应键盘输入
        // -----
        processInput(window);

        // render
        //设置清除颜色
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        //清空的缓冲位可能有GL_COLOR_BUFFER_BIT，GL_DEPTH_BUFFER_BIT和GL_STENCIL_BUFFER_BIT
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // don't forget to enable shader before setting uniforms
        //激活链接程序，激活着色器，开始渲染
        ourShader.use();

        // view/projection transformations
        // 视口，投影矩阵
        //视野(Field of View) 宽高比 第三和第四个参数设置了平截头体的近和远平面
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
        glm::mat4 view = camera.GetViewMatrix();
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);


        ourModel.Draw(ourShader);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // 检查并调用事件，交换缓冲
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();

//        break;
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    //释放前面所申请的内存
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    //ESC 退出窗口
    //glfwGetKey()用来判断一个键是否按下。第一个参数是GLFW窗口句柄，第二个参数是一个GLFW常量，代表一个键。
    //GLFW_KEY_ESCAPE表示Esc键。如果Esc键按下了，glfwGetKey将返回GLFW_PRESS（值为1），否则返回GLFW_RELEASE（值为0）。
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    //WASD移动摄像机
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime*100);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime*100);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime*100);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime*100);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
// 当用户改变窗口的大小的时候，视口也应该被调整。
//对窗口注册一个回调函数(Callback Function)，它会在每次窗口大小被调整的时候被调用
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
//响应鼠标事件 创建鼠标的回调函数
//xpos,ypos鼠标的位置
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    //计算当前帧和上一帧的位置差
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
//响应鼠标滚轮
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}