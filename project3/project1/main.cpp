// ============================================================
//  مشروع الرسوميات الحاسوبية - الدرس الثالث
//  المتطلبات المُنفَّذة:
//    1) أربعة أزرار للتحكم بخصائص الأشكال (Q, W, E, R)
//    2) تطبيق محور Z على جسمين مرسومين
//    3) خاصية الشفافية (Transparency)
// ============================================================

#include <iostream>
#include <cmath>
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

const unsigned int SCR_WIDTH = 900;
const unsigned int SCR_HEIGHT = 700;

// ======================== متغيرات التحكم ========================
bool animationOn = false;      // Q: تشغيل/إيقاف الحركة التلقائية
int colorScheme = 0;           // W: تبديل نمط الألوان (0-3)
int transparencyLevel = 0;     // E: تبديل مستوى الشفافية (0-3)
bool wireframeOn = false;      // R: تبديل وضع الرسم (تعبئة / شبكة سلكية)

// ======================== الشيدرات ========================

const char* vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"layout (location = 1) in vec3 aColor;\n"
"out vec3 ourColor;\n"
"uniform vec3 uOffset;\n"
"void main()\n"
"{\n"
"   gl_Position = vec4(aPos + uOffset, 1.0);\n"
"   ourColor = aColor;\n"
"}\0";

const char* fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"in vec3 ourColor;\n"
"uniform float uAlpha;\n"
"uniform vec3 uColorMod;\n"
"void main()\n"
"{\n"
"   vec3 finalColor = clamp(ourColor + uColorMod, 0.0, 1.0);\n"
"   FragColor = vec4(finalColor, uAlpha);\n"
"}\n\0";

// ======================== الدوال المساعدة ========================

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// ======================== معالجة المدخلات بأسلوب Polling ========================
// نفس أسلوب كود المحاضرة - يُستدعى كل فريم مع debounce بمتغيرات static
void processInput(GLFWwindow* window)
{
    // ESC: إغلاق النافذة
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // --- Q: تشغيل / إيقاف الحركة ---
    {
        static bool prevQ = false;
        bool currQ = (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS);
        if (currQ && !prevQ) {
            animationOn = !animationOn;
            std::cout << "[Q] Animation: " << (animationOn ? "ON" : "OFF") << std::endl;
        }
        prevQ = currQ;
    }

    // --- W: تغيير نمط الألوان ---
    {
        static bool prevW = false;
        bool currW = (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS);
        if (currW && !prevW) {
            colorScheme = (colorScheme + 1) % 4;
            const char* names[] = { "Original", "Green Tint", "Purple Tint", "Yellow Tint" };
            std::cout << "[W] Color: " << names[colorScheme] << std::endl;
        }
        prevW = currW;
    }

    // --- E: تغيير مستوى الشفافية ---
    {
        static bool prevE = false;
        bool currE = (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS);
        if (currE && !prevE) {
            transparencyLevel = (transparencyLevel + 1) % 4;
            const char* names[] = { "100%", "75%", "50%", "25%" };
            std::cout << "[E] Transparency: " << names[transparencyLevel] << std::endl;
        }
        prevE = currE;
    }

    // --- R: تبديل وضع الرسم (تعبئة / شبكة) ---
    {
        static bool prevR = false;
        bool currR = (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS);
        if (currR && !prevR) {
            wireframeOn = !wireframeOn;
            std::cout << "[R] Wireframe: " << (wireframeOn ? "ON" : "OFF") << std::endl;
        }
        prevR = currR;
    }
}

// ======================== البرنامج الرئيسي ========================

int main()
{
    // --- تهيئة GLFW ---
    if (!glfwInit()) {
        std::cout << "ERROR: Failed to init GLFW" << std::endl;
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT,
        "CG Project 3 | Q=Move  W=Color  E=Alpha  R=Wire", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "ERROR: Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // --- تهيئة GLEW ---
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        std::cout << "ERROR: Failed to initialize GLEW" << std::endl;
        return -1;
    }

    // --- تفعيل قدرات OpenGL ---
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // --- بناء وتجميع الشيدرات ---
    int success;
    char infoLog[512];

    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::VERTEX_SHADER\n" << infoLog << std::endl;
    }

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::FRAGMENT_SHADER\n" << infoLog << std::endl;
    }

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::LINKING\n" << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // ============================================================
    //  بيانات الأشكال: مثلثان بقيم Z مختلفة
    //  التنسيق: X, Y, Z, R, G, B
    //  المثلث 1: Z = 0.0 (قريب) - أحمر
    //  المثلث 2: Z = 0.5 (بعيد) - أزرق
    // ============================================================
    float vertices[] = {
        // المثلث الأول (أحمر - Z = 0.0)
        -0.6f, -0.4f, 0.0f,   1.0f, 0.2f, 0.2f,
         0.2f, -0.4f, 0.0f,   1.0f, 0.2f, 0.2f,
        -0.2f,  0.4f, 0.0f,   1.0f, 0.4f, 0.3f,

        // المثلث الثاني (أزرق - Z = 0.5)
        -0.1f, -0.3f, 0.5f,   0.2f, 0.3f, 1.0f,
         0.7f, -0.3f, 0.5f,   0.2f, 0.3f, 1.0f,
         0.3f,  0.5f, 0.5f,   0.3f, 0.4f, 1.0f
    };

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // السمة 0: الموقع (X, Y, Z)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // السمة 1: اللون (R, G, B)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // --- طباعة تعليمات التحكم ---
    std::cout << "========================================" << std::endl;
    std::cout << "  CG Project 3 - Interactive Controls   " << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "  [Q] Toggle Animation  (Move shapes)"    << std::endl;
    std::cout << "  [W] Change Colors     (4 schemes)"      << std::endl;
    std::cout << "  [E] Change Transparency (4 levels)"     << std::endl;
    std::cout << "  [R] Toggle Wireframe  (Fill/Line)"      << std::endl;
    std::cout << "  [ESC] Exit"                              << std::endl;
    std::cout << "========================================" << std::endl;

    // --- مواقع المتغيرات في الشيدر ---
    glUseProgram(shaderProgram);
    int offsetLoc   = glGetUniformLocation(shaderProgram, "uOffset");
    int alphaLoc    = glGetUniformLocation(shaderProgram, "uAlpha");
    int colorModLoc = glGetUniformLocation(shaderProgram, "uColorMod");

    // مستويات الشفافية
    float alphaLevels[] = { 1.0f, 0.75f, 0.5f, 0.25f };

    // ======================== حلقة الرسم ========================
    while (!glfwWindowShouldClose(window))
    {
        // 1) معالجة الأحداث أولاً
        glfwPollEvents();

        // 2) قراءة المدخلات (polling)
        processInput(window);

        // 3) وضع الرسم: تعبئة أو شبكة (R)
        if (wireframeOn)
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        else
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        // 4) تنظيف الشاشة
        glClearColor(0.08f, 0.08f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);

        // 5) حساب الحركة (Q)
        float t = (float)glfwGetTime();
        float moveX1 = animationOn ? sinf(t * 1.5f) * 0.25f : 0.0f;
        float moveY1 = animationOn ? cosf(t * 1.2f) * 0.15f : 0.0f;
        float moveX2 = animationOn ? sinf(t * 1.0f + 3.14f) * 0.20f : 0.0f;
        float moveY2 = animationOn ? cosf(t * 0.8f + 1.57f) * 0.18f : 0.0f;

        // 6) معدِّل اللون (W)
        float cr = 0.0f, cg = 0.0f, cb = 0.0f;
        switch (colorScheme) {
        case 0: cr = 0.0f;  cg = 0.0f;  cb = 0.0f;  break;
        case 1: cr = -0.3f; cg = 0.4f;  cb = -0.2f; break;
        case 2: cr = 0.3f;  cg = -0.2f; cb = 0.4f;  break;
        case 3: cr = 0.3f;  cg = 0.3f;  cb = -0.3f; break;
        }

        // 7) الشفافية (E)
        float currentAlpha = alphaLevels[transparencyLevel];

        // 8) رسم المثلث الأزرق البعيد (Z=0.5) أولاً
        glUniform3f(offsetLoc, moveX2, moveY2, 0.0f);
        glUniform3f(colorModLoc, cr, cg, cb);
        glUniform1f(alphaLoc, currentAlpha);
        glDrawArrays(GL_TRIANGLES, 3, 3);

        // 9) رسم المثلث الأحمر القريب (Z=0.0) ثانياً - أكثر شفافية
        glUniform3f(offsetLoc, moveX1, moveY1, 0.0f);
        glUniform3f(colorModLoc, cr, cg, cb);
        glUniform1f(alphaLoc, currentAlpha * 0.7f);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        // 10) عرض الإطار
        glfwSwapBuffers(window);
    }

    // --- تنظيف ---
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);
    glfwTerminate();
    return 0;
}