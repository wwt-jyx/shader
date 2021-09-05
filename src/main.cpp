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
unsigned int loadCubemap(vector<std::string> faces);
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

glm::vec3 lightPos(200.0f, 0.0f, 0.0f);

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
    Shader ourShader("../../src/phong.vs", "../../src/phong.fs");
        // load models
    // -----------
    Model ourModel("../../data/car/scene.gltf");

    //frame screenShader
    Shader screenShader("../../src/screen_cube.vs", "../../src/screen_cube.fs");
    //lightCubeShader
    Shader lightCubeShader("../../src/light.vs", "../../src/light.fs");
    //skyboxShader
    Shader skyboxShader("../../src/skybox.vs", "../../src/skybox.fs");

    //Uniform块设置绑定点
    unsigned int uniformBlockIndexLight   = glGetUniformBlockIndex(lightCubeShader.ID, "Matrices");
    unsigned int uniformBlockIndexSkybox  = glGetUniformBlockIndex(skyboxShader.ID, "Matrices");
    unsigned int uniformBlockIndexShader   = glGetUniformBlockIndex(ourShader.ID, "Matrices");
    glUniformBlockBinding(lightCubeShader.ID,    uniformBlockIndexLight, 0);
    glUniformBlockBinding(skyboxShader.ID,  uniformBlockIndexSkybox, 0);
    glUniformBlockBinding(ourShader.ID,   uniformBlockIndexShader, 0);

    //创建Uniform缓冲对象本身，并将其绑定到绑定点
    unsigned int uboMatrices;
    glGenBuffers(1, &uboMatrices);
    glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
    glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferRange(GL_UNIFORM_BUFFER, 0, uboMatrices, 0, 2 * sizeof(glm::mat4));

    //灯的立方体顶点初始化
    float vertices[] = {
            -0.5f, -0.5f, -0.5f,
            0.5f, -0.5f, -0.5f,
            0.5f,  0.5f, -0.5f,
            0.5f,  0.5f, -0.5f,
            -0.5f,  0.5f, -0.5f,
            -0.5f, -0.5f, -0.5f,

            -0.5f, -0.5f,  0.5f,
            0.5f, -0.5f,  0.5f,
            0.5f,  0.5f,  0.5f,
            0.5f,  0.5f,  0.5f,
            -0.5f,  0.5f,  0.5f,
            -0.5f, -0.5f,  0.5f,

            -0.5f,  0.5f,  0.5f,
            -0.5f,  0.5f, -0.5f,
            -0.5f, -0.5f, -0.5f,
            -0.5f, -0.5f, -0.5f,
            -0.5f, -0.5f,  0.5f,
            -0.5f,  0.5f,  0.5f,

            0.5f,  0.5f,  0.5f,
            0.5f,  0.5f, -0.5f,
            0.5f, -0.5f, -0.5f,
            0.5f, -0.5f, -0.5f,
            0.5f, -0.5f,  0.5f,
            0.5f,  0.5f,  0.5f,

            -0.5f, -0.5f, -0.5f,
            0.5f, -0.5f, -0.5f,
            0.5f, -0.5f,  0.5f,
            0.5f, -0.5f,  0.5f,
            -0.5f, -0.5f,  0.5f,
            -0.5f, -0.5f, -0.5f,

            -0.5f,  0.5f, -0.5f,
            0.5f,  0.5f, -0.5f,
            0.5f,  0.5f,  0.5f,
            0.5f,  0.5f,  0.5f,
            -0.5f,  0.5f,  0.5f,
            -0.5f,  0.5f, -0.5f,
    };
    float planeVertices[] = {
            // positions          // texture Coords
            5.0f, -0.5f,  5.0f,  2.0f, 0.0f,
            -5.0f, -0.5f,  5.0f,  0.0f, 0.0f,
            -5.0f, -0.5f, -5.0f,  0.0f, 2.0f,

            5.0f, -0.5f,  5.0f,  2.0f, 0.0f,
            -5.0f, -0.5f, -5.0f,  0.0f, 2.0f,
            5.0f, -0.5f, -5.0f,  2.0f, 2.0f
    };
    float quadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
            // positions   // texCoords
            -1.0f,  1.0f,  0.0f, 1.0f,
            -1.0f, -1.0f,  0.0f, 0.0f,
            1.0f, -1.0f,  1.0f, 0.0f,

            -1.0f,  1.0f,  0.0f, 1.0f,
            1.0f, -1.0f,  1.0f, 0.0f,
            1.0f,  1.0f,  1.0f, 1.0f
    };
    float skyboxVertices[] = {
            // positions
            -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            -1.0f,  1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f,  1.0f
    };
    //lightcube VAO
    unsigned int lightVAO,lightVBO;
    glGenVertexArrays(1, &lightVAO);
    glGenBuffers(1, &lightVBO);
    glBindVertexArray(lightVAO);
    glBindBuffer(GL_ARRAY_BUFFER, lightVBO);
    glBufferData(GL_ARRAY_BUFFER,sizeof(vertices),vertices,GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
    // plane VAO
    unsigned int planeVAO, planeVBO;
    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);
    glBindVertexArray(planeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), &planeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    // screen quad VAO
    unsigned int quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    //skyboxVertices VAO
    unsigned int skyboxVAO,skyboxVBO;
    glGenVertexArrays(1,&skyboxVAO);
    glGenBuffers(1,&skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER,skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER,sizeof(skyboxVertices),&skyboxVertices,GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);




    vector<std::string> faces
    {
        "right.jpg",
        "left.jpg",
        "bottom.jpg",
        "top.jpg",
        "front.jpg",
        "back.jpg"
    };
    unsigned int skyboxTexture = loadCubemap(faces);


    // draw in wireframe
    // 绘制线框
//    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    // positions of the point lights
    //面剔除
//    glEnable(GL_CULL_FACE);

    glm::vec3 pointLightPositions[] = {
            glm::vec3( 0.7f,  0.2f,  302.0f),
            glm::vec3( 102.3f, -3.3f, -4.0f),
            glm::vec3(-4.0f,  252.0f, -12.0f)
    };

    // 分配纹理单元
    ourShader.use(); // 不要忘记在设置uniform变量之前激活着色器程序！
    ourShader.setInt("material.baseColorTexture", 0); // 或者使用着色器类设置
    ourShader.setInt("metallicRoughnessTexture", 1);
    ourShader.setInt("normalTexture", 2);

    screenShader.use();
    screenShader.setInt("screenTexture",0);





////////////////////////////////////////////////////////////帧缓冲
    //创建一个帧缓冲对象，并绑定它
    unsigned int framebuffer;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    // 生成纹理
    unsigned int texColorBuffer;
    glGenTextures(1, &texColorBuffer);
    glBindTexture(GL_TEXTURE_2D, texColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 800, 600, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);//区别:给纹理的data参数传递了NULL
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    // 将它附加到当前绑定的帧缓冲对象
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texColorBuffer, 0);


    //创建一个渲染缓冲对象
    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 800, 600);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    //将渲染缓冲对象附加到帧缓冲的深度和模板附件上
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    //检查帧缓冲是否完整
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

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

        //////////////////////////////////////////////////帧缓冲渲染
        // 第一处理阶段(Pass)
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
//        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
//        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // 我们现在不使用模板缓冲
        glEnable(GL_DEPTH_TEST);


        // render
        //设置清除颜色
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        //清空的缓冲位可能有GL_COLOR_BUFFER_BIT，GL_DEPTH_BUFFER_BIT和GL_STENCIL_BUFFER_BIT
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);




        // don't forget to enable shader before setting uniforms
        // be sure to activate shader when setting uniforms/drawing objects
        //激活链接程序，激活着色器
        ourShader.use();
        ourShader.setVec3("viewPos", camera.Position);


        //材质
        ourShader.setVec3("material.ambient",  1.0f, 1.0f, 1.0f);
        ourShader.setVec3("material.diffuse",  1.0f, 1.0f, 1.0f);
        ourShader.setVec3("material.specular", 1.0f, 1.0f, 1.0f);
        ourShader.setFloat("material.shininess", 32.0f);

        /*
           Here we set all the uniforms for the 5/6 types of lights we have. We have to set them manually and index
           the proper PointLight struct in the array to set each uniform variable. This can be done more code-friendly
           by defining light types as classes and set their values in there, or by using a more efficient uniform approach
           by using 'Uniform buffer objects', but that is something we'll discuss in the 'Advanced GLSL' tutorial.
        */
        // directional light
        ourShader.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
        ourShader.setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
        ourShader.setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
        ourShader.setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);
        // point light 1
        ourShader.setVec3("pointLights[0].position", pointLightPositions[0]);
        ourShader.setVec3("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
        ourShader.setVec3("pointLights[0].diffuse", 0.8f, 0.8f, 0.8f);
        ourShader.setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
        ourShader.setFloat("pointLights[0].constant", 1.0f);
        ourShader.setFloat("pointLights[0].linear", 0.09);
        ourShader.setFloat("pointLights[0].quadratic", 0.032);
        // point light 2
        ourShader.setVec3("pointLights[1].position", pointLightPositions[1]);
        ourShader.setVec3("pointLights[1].ambient", 0.05f, 0.05f, 0.05f);
        ourShader.setVec3("pointLights[1].diffuse", 0.8f, 0.8f, 0.8f);
        ourShader.setVec3("pointLights[1].specular", 1.0f, 1.0f, 1.0f);
        ourShader.setFloat("pointLights[1].constant", 1.0f);
        ourShader.setFloat("pointLights[1].linear", 0.09);
        ourShader.setFloat("pointLights[1].quadratic", 0.032);
        // point light 3
        ourShader.setVec3("pointLights[2].position", pointLightPositions[2]);
        ourShader.setVec3("pointLights[2].ambient", 0.05f, 0.05f, 0.05f);
        ourShader.setVec3("pointLights[2].diffuse", 0.8f, 0.8f, 0.8f);
        ourShader.setVec3("pointLights[2].specular", 1.0f, 1.0f, 1.0f);
        ourShader.setFloat("pointLights[2].constant", 1.0f);
        ourShader.setFloat("pointLights[2].linear", 0.09);
        ourShader.setFloat("pointLights[2].quadratic", 0.032);

        // spotLight
        ourShader.setVec3("spotLight.position", camera.Position);
        ourShader.setVec3("spotLight.direction", camera.Front);
        ourShader.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
        ourShader.setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
        ourShader.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
        ourShader.setFloat("spotLight.constant", 1.0f);
        ourShader.setFloat("spotLight.linear", 0.09);
        ourShader.setFloat("spotLight.quadratic", 0.032);
        ourShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
        ourShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));

        //hemisphereLight
        ourShader.setVec3("hemisphereLight.skyColor",  1.0f, 1.0f, 1.0f);
        ourShader.setVec3("hemisphereLight.groundColor", 0.0f, 0.0f, 0.0f);
        ourShader.setFloat("hemisphereLight.skyIntensity", 0.8f);
        ourShader.setFloat("hemisphereLight.groundIntensity", 0.2f);
        ourShader.setVec3("hemisphereLight.direction", 0.0f, -1.0f, 0.0f);


        // view/projection transformations
        // 视口，投影矩阵
        //视野(Field of View) 宽高比 第三和第四个参数设置了平截头体的近和远平面
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
        glm::mat4 view = camera.GetViewMatrix();

        //填充Uniform缓冲
        glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
        glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));
        glBindBuffer(GL_UNIFORM_BUFFER, 0);


        //渲染主体
        glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
        ourModel.Draw(ourShader);

        // also draw the lamp object(s)
        //渲染光源
        lightCubeShader.use();
        glBindVertexArray(lightVAO);
        for (unsigned int i = 0; i < 4; i++)
        {
            //光源位置
            glm::mat4 lightModel = glm::mat4(1.0f);
            lightModel = glm::translate(lightModel,pointLightPositions[i]);
            lightModel = glm::scale(lightModel, glm::vec3(10.2f));
            lightCubeShader.setMat4("model", lightModel);
//            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        glBindVertexArray(0);

        //渲染skybox
        glDepthFunc(GL_LEQUAL);
        skyboxShader.use();

        view =  glm::mat4(glm::mat3(camera.GetViewMatrix()));
        glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
        glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        glBindVertexArray(skyboxVAO);
        glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glDepthFunc(GL_LESS);

        // 第二处理阶段
        glBindFramebuffer(GL_FRAMEBUFFER, 0); // 返回默认
        glDisable(GL_DEPTH_TEST);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        screenShader.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texColorBuffer);
        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);


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

unsigned int loadCubemap(vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        std::string img_path = "../../data/skybox/" + faces[i];
        unsigned char *data = stbi_load(img_path.c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                         0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
            );
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}