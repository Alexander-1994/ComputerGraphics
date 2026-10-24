#define GLEW_DLL
#define GLFW_DLL
#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include <cstdio>

int main() {
    // Инициализация GLFW
    if (!glfwInit()) {
        fprintf(stderr, "ERROR: could not start GLFW3.\n");
        return 1;
    }

    // Установка версии OpenGL
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 1);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    // Создание окна
    GLFWwindow* window = glfwCreateWindow(512, 512, "OpenGL Window", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    // Настройка контекста
    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE;

    // Инициализация GLEW
    GLenum ret = glewInit();
    if (GLEW_OK != ret) {
        fprintf(stderr, "Error: %s\n", glewGetErrorString(ret));
        return 1;
    }

    // Информация о версии
    const GLubyte* version_str = glGetString(GL_VERSION);
    const GLubyte* device_str = glGetString(GL_RENDERER);
    printf("OpenGL версия: %s\n", version_str);
    printf("Устройство: %s\n", device_str);

    // Основной цикл
    while (!glfwWindowShouldClose(window)) {
        // Очистка буфера с белым фоном
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Рисование 5-конечной звезды
        glBegin(GL_LINES);
        glColor3f(0.7f, 1.0f, 0.3f); // Цвет звезды

        // Координаты для 5-конечной звезды
        glVertex2f(0.0f, 0.8f);
        glVertex2f(-0.5f, -0.5f);

        glVertex2f(0.0f, 0.8f);
        glVertex2f(0.5f, -0.5f);

        glVertex2f(0.5f, 0.5f);
        glVertex2f(-0.5f, -0.5f);

        glVertex2f(-0.5f, 0.5f);
        glVertex2f(0.5f, -0.5f);

        glVertex2f(0.5f, 0.5f);
        glVertex2f(-0.5f, 0.5f);

        glEnd();

        // Переключение буферов
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Завершение работы
    glfwTerminate();
    return 0;
}
