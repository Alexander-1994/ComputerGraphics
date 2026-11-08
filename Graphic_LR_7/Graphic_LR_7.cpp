#define GLEW_DLL
#define GLFW_DLL
#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "Shader.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "Model.h"
#include <cstdio>
#include <cmath>
#include <vector>

// Константы приложения
const unsigned int SCR_WIDTH = 768;
const unsigned int SCR_HEIGHT = 768;
const float FOV = 45.0f;
const float NEAR_PLANE = 0.1f;
const float FAR_PLANE = 100.0f;
const float CAMERA_SPEED = 0.05f;
const float SENSITIVITY = 0.05f;
const float ROTATION_SPEED = 30.0f;
const float MOVE_SPEED = 30.0f;

// Пределы степеней свободы: X01 и X03 — единицы, X02 — градусы
const float DOF1_MAX = 1.0f;
const float DOF1_MIN = -1.0f;
const float DOF2_MAX = 90.0f;
const float DOF2_MIN = -90.0f;
const float DOF3_MAX = 0.2f;
const float DOF3_MIN = -0.2f;

// PIVOT для звена X02
const glm::vec3 PIVOT_X02 = glm::vec3(-0.005882f, -1.26029f, -3.58985f);

// Глобальные переменные для камеры
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
float yaw = -90.0f;
float pitch = 0.0f;
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
bool shouldClose = false;

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    xoffset *= SENSITIVITY;
    yoffset *= SENSITIVITY;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        shouldClose = true;
    }
}

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

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OpenGL Window", NULL, NULL);

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

    // Настройка обработки событий
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetKeyCallback(window, key_callback);

    const GLubyte* version_str = glGetString(GL_VERSION);
    const GLubyte* device_str = glGetString(GL_RENDERER);
    printf("OpenGL версия: %s\n", version_str);
    printf("Устройство: %s\n", device_str);

    // Загрузка шейдеров из файлов
    Shader shader;
    if (!shader.load("model.vert", "model.frag")) {
        glfwTerminate();
        return 1;
    }

    // Загрузка модели через ASSIMP
    Model model("../Graphic_LR_3/industrial_robot.obj");

    unsigned int meshCount = model.getMeshCount();

    // Привязка звеньев по именам (порядок мешей в OBJ может отличаться)
    int idxBase = model.getMeshIndex("Base");
    int idxX01 = model.getMeshIndex("X01");
    int idxX02 = model.getMeshIndex("X02");
    int idxX03 = model.getMeshIndex("X03");

    if (idxBase < 0 || idxX01 < 0 || idxX02 < 0 || idxX03 < 0) {
        fprintf(stderr, "ERROR: не найдены меши Base/X01/X02/X03 — проверьте имена в консоли.\n");
        glfwTerminate();
        return 1;
    }

    glm::mat4 projection = glm::perspective(
        glm::radians(FOV),
        (float)SCR_WIDTH / (float)SCR_HEIGHT,
        NEAR_PLANE,
        FAR_PLANE
    );

    // Настройки OpenGL
    glEnable(GL_DEPTH_TEST);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    // Значения степеней свободы
    float dof1 = 0.0f; // X01
    float dof2 = 0.0f; // X02
    float dof3 = 0.0f; // X03

    // deltaTime
    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    // Основной цикл
    while (!glfwWindowShouldClose(window) && !shouldClose) {
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Обработка ввода (WASD) — камера
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            cameraPos += CAMERA_SPEED * cameraFront;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            cameraPos -= CAMERA_SPEED * cameraFront;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * CAMERA_SPEED;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * CAMERA_SPEED;

        // Управление звеньями: на каждую степень свободы две клавиши
        // X01: Q — сдвиг влево, E — сдвиг вправо
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
            float newV = dof1 + MOVE_SPEED * deltaTime;
            if (newV <= DOF1_MAX) dof1 = newV;
        }
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
            float newV = dof1 - MOVE_SPEED * deltaTime;
            if (newV >= DOF1_MIN) dof1 = newV;
        }

        // X02: R — поворот по часовой стрелке, F — поворот против часовой стрелки
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
            float newV = dof2 + glm::radians(ROTATION_SPEED) * deltaTime;
            if (newV <= glm::radians(DOF2_MAX)) dof2 = newV;
        }
        if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
            float newV = dof2 - glm::radians(ROTATION_SPEED) * deltaTime;
            if (newV >= glm::radians(DOF2_MIN)) dof2 = newV;
        }

        // X03: T — сдвиг вверх, G — сдвиг вниз
        if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) {
            float newV = dof3 + MOVE_SPEED * deltaTime;
            if (newV <= DOF3_MAX) dof3 = newV;
        }
        if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS) {
            float newV = dof3 - MOVE_SPEED * deltaTime;
            if (newV >= DOF3_MIN) dof3 = newV;
        }

        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Базовая матрица
        glm::mat4 baseMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f, 0.5f, 0.5f));

        // Матрицы звеньев с учётом зависимости
        // X01
        glm::mat4 mX01 = baseMatrix * glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, dof1));

        // X02
        glm::mat4 mX02 = mX01 * glm::rotate(glm::mat4(1.0f), dof2, glm::vec3(0.0f, 1.0f, 0.0f));
  
        // X03
        glm::mat4 mX03 = mX02 * glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, dof3, 0.0f));

        // Раздача матриц по индексам
        std::vector<glm::mat4> partMatrices(meshCount, baseMatrix);
        partMatrices[idxBase] = baseMatrix;
        partMatrices[idxX01] = mX01;
        partMatrices[idxX02] = mX02;
        partMatrices[idxX03] = mX03;

        shader.use();

        // Цвет света
        shader.setUniform("lightColor", 1.0f, 1.0f, 1.0f);

        // Параметры источника света
        shader.setUniform("light.position", 1.2f, 1.0f, 2.0f);
        shader.setUniform("light.ambient", 0.1f, 0.1f, 0.1f);
        shader.setUniform("light.diffuse", 1.0f, 1.0f, 1.0f);
        shader.setUniform("light.specular", 1.0f, 1.0f, 1.0f);

        // Свойства материала
        shader.setUniform("material.ambient", 1.0f, 0.5f, 0.31f);
        shader.setUniform("material.diffuse", 1.0f, 0.5f, 0.31f);
        shader.setUniform("material.specular", 0.5f, 0.5f, 0.5f);
        shader.setUniform("material.shininess", 32.0f);

        // Позиция камеры
        shader.setUniform("viewPos", cameraPos.x, cameraPos.y, cameraPos.z);

        // Матрицы вида и проекции
        shader.setUniformMat4("projection", glm::value_ptr(projection));
        shader.setUniformMat4("view", glm::value_ptr(view));

        model.Draw(shader, partMatrices);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
