#define GLEW_DLL
#define GLFW_DLL
#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "Shader.h"
#include <cstdio>
#include <cmath>

int main() {
    // Инициализация GLFW
    if (!glfwInit()) {
        fprintf(stderr, "ERROR: could not start GLFW3.\n");
        return 1;
    }

    // Версия OpenGL 4.6 + core profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(512, 512, "OpenGL Window", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE;

    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "ERROR: glewInit failed.\n");
        return 1;
    }

    const GLubyte* version_str = glGetString(GL_VERSION);
    const GLubyte* device_str = glGetString(GL_RENDERER);
    printf("OpenGL версия: %s\n", version_str);
    printf("Устройство: %s\n", device_str);

    // Загрузка шейдеров из файлов
    Shader shader;
    if (!shader.load("star.vert", "star.frag")) {
        glfwTerminate();
        return 1;
    }

    // Геометрия: 5 вершин звезды.
    // 0: верхняя, 1: левая нижняя, 2: правая нижняя,
    // 3: правая верхняя, 4: левая верхняя
    float points[] = {
         0.0f,  0.8f, 0.0f,
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
         0.5f,  0.5f, 0.0f,
        -0.5f,  0.5f, 0.0f
    };

    // Индексы линий (те же 5 отрезков, что были в glBegin(GL_LINES))
    GLuint indices[] = {
        0, 1,  // верх — левая нижняя
        0, 2,  // верх — правая нижняя
        3, 1,  // правая верхняя — левая нижняя
        4, 2,  // левая верхняя — правая нижняя
        3, 4   // правая верхняя — левая верхняя
    };

    // Создание VAO, VBO, EBO
    GLuint VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Основной цикл
    while (!glfwWindowShouldClose(window)) {
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        float timeValue = glfwGetTime();

        // Задание 1: цвет меняется по периодическому закону (sin/cos)
        float r = sin(timeValue * 2.0f) * 0.5f + 0.5f;
        float g = 0.7f;
        float b = cos(timeValue * 3.0f) * 0.5f + 0.5f;

        // Движение: звезда плавно качается по кругу
        float offsetX = 0.3f * sin(timeValue);
        float offsetY = 0.3f * cos(timeValue);

        shader.use();
        shader.setUniform("ourColor", r, g, b, 1.0f);
        shader.setUniform("u_offset", offsetX, offsetY, 0.0f);

        glBindVertexArray(VAO);
        glDrawElements(GL_LINES, 10, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Очистка
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    glfwTerminate();
    return 0;
}
