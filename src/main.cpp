#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "hgt.h"
#include <iostream>
#include <algorithm>
#include <filesystem>
#include <regex>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <sstream>

int currentLOD = 0;

bool use3DView = false;  // false -> 2D, true -> 3D

// 3D
float cameraSpeed = 30.0f;
float fov3D = 35.0f;
float minFov3D = 15.0f;
float maxFov3D = 75.0f;

// 2D
glm::vec2 cam2DOffset(0.0f, 0.0f);
float cam2DSpeed = 200.0f;
float zoom2D = 1.0f;
float minZoom2D = 0.2f;
float maxZoom2D = 5.0f;

glm::vec3 camPos = glm::vec3(0.0f, 150.0f, 150.0f);  // pozycje
glm::vec3 camFront = glm::vec3(0.0f, -0.5f, -1.0f);  // kierunek
glm::vec3 camUp = glm::vec3(0.0f, 1.0f, 0.0f);  // "góra"

// obs³uga myszy: yaw -> lewo/prawo, pitch -> góra/dó³
float yaw = -90.0f;
float pitch = -30.0f;
float lastX = 640, lastY = 400;  // pozycja myszy
bool firstMouse = true;

// czas i fps
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// strktura wierzcho³ka
struct Vertex {
    float h;
};

// wczytywanie shaderów (open txt)
std::string readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "NIe mo¿na otworzyæ pliku: " << path << std::endl;
        return "";
    }
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

// rozmiar okna
void framebuffer_size_callback(GLFWwindow* window, int w, int h) {
    glViewport(0, 0, w, h);
}

// kompilacja shadera
GLuint compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "B³¹d kompialcji shadera:\n" << infoLog << std::endl;
    }
    return shader;
}

// program shadera: ³¹czy vertex i fragment i linkuje w jeden program 
GLuint createShaderProgram(const char* vertexSrc, const char* fragmentSrc) {
    GLuint vertex = compileShader(GL_VERTEX_SHADER, vertexSrc);
    GLuint fragment = compileShader(GL_FRAGMENT_SHADER, fragmentSrc);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    glLinkProgram(program);

    int success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cerr << "B³¹d:\n" << infoLog << std::endl;
    }

    glDeleteShader(vertex);
    glDeleteShader(fragment);
    return program;
}

// klawiatura
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        switch (key) {
        case GLFW_KEY_1: currentLOD = 0; std::cout << "[LOD] Zmiana -> LOD0" << std::endl; break;
        case GLFW_KEY_2: currentLOD = 1; std::cout << "[LOD] Zmiana -> LOD1" << std::endl; break;
        case GLFW_KEY_3: currentLOD = 2; std::cout << "[LOD] Zmiana -> LOD2" << std::endl; break;
        case GLFW_KEY_SPACE:
            use3DView = !use3DView;
            if (use3DView)
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            else
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            break;
        case GLFW_KEY_ESCAPE: glfwSetWindowShouldClose(window, true); break;
        }
    }
}

// ruch kamery 3D (WSAD)
void processCameraInput(GLFWwindow* window)
{
    float velocity = cameraSpeed * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camPos += velocity * camFront;

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camPos -= velocity * camFront;

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camPos -= glm::normalize(glm::cross(camFront, camUp)) * velocity;

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camPos += glm::normalize(glm::cross(camFront, camUp)) * velocity;
}

// mysz - obrót
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (!use3DView) return;

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.05f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    camFront = glm::normalize(front);
}

// scroll - zoom
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    if (use3DView)
    {
        fov3D -= (float)yoffset * 2.0f;
        fov3D = std::clamp(fov3D, minFov3D, maxFov3D);
    }
    else
    {
        zoom2D -= (float)yoffset * 0.1f;
        zoom2D = std::clamp(zoom2D, minZoom2D, maxZoom2D);
    }
}

int main(int argc, char** argv) {
    double lastTime = glfwGetTime();  // rozpoczecie liczenia fps
    int frameCount = 0;  // liczba wygenerowanych klatek

    if (argc < 2) {
        std::cerr << "program <data_folder> [-lon min max] [-lat min max]\n";
        return -1;
    }

    std::string dataFolder = argv[1];

    bool useRange = false;
    int minLon = 0, maxLon = 0, minLat = 0, maxLat = 0;

    // parsowanie argumentów
    for (int i = 2; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-lon" && i + 2 < argc) {
            minLon = std::stoi(argv[++i]);
            maxLon = std::stoi(argv[++i]);
            useRange = true;
        }
        else if (arg == "-lat" && i + 2 < argc) {
            minLat = std::stoi(argv[++i]);
            maxLat = std::stoi(argv[++i]);
            useRange = true;
        }
    }

    if (!glfwInit()) {
        std::cerr << "GLFW init failed\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // twrzenie okna
    GLFWwindow* window = glfwCreateWindow(1280, 800, "terrain-render", nullptr, nullptr);
    if (!window) {
        std::cerr << "Nie mo¿na odtworzyæ okna\n";
        glfwTerminate();
        return -1;
    }

    // callbacki
    glfwSetScrollCallback(window, scroll_callback);  // scroll
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);  // resize okna
    glfwSetKeyCallback(window, key_callback);  // klawiatura
    glfwSetCursorPosCallback(window, mouse_callback);  // mysz

    // kursor myszy 3d -> ukryty, 2d - > normal
    if (use3DView)
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    else
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "GLAD init failed\n";
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    std::string vertexSrc = readFile("shader/vertex_shader.glsl");
    std::string fragmentSrc = readFile("shader/fragment_shader.glsl");
    GLuint shaderProgram = createShaderProgram(vertexSrc.c_str(), fragmentSrc.c_str());  // kompilacja i linkowanie

    // ³adowanie terenu srtm
    TerrainData terrain = loadAllHGT(dataFolder, useRange, minLon, maxLon, minLat, maxLat);
    if (terrain.fullData.empty()) {
        std::cerr << "Brak\n";
        return -1;
    }

    // tworzenie wierzcho³ków
    std::vector<Vertex> vertices;
    vertices.reserve(terrain.width * terrain.height);
    for (int i = 0; i < terrain.height; i++) {
        for (int j = 0; j < terrain.width; j++) {
            vertices.push_back({ static_cast<float>(terrain.fullData[i * terrain.width + j]) });
        }
    }

    // lod - generowanie indeksów
    std::vector<unsigned int> indicesLOD0, indicesLOD1, indicesLOD2;
    auto generateIndices = [&](int step, std::vector<unsigned int>& out) {
        out.clear();
        for (int i = 0; i < terrain.height - step; i += step) {
            for (int j = 0; j < terrain.width - step; j += step) {
                unsigned int tl = i * terrain.width + j;
                unsigned int tr = tl + step;
                unsigned int bl = (i + step) * terrain.width + j;
                unsigned int br = bl + step;

                out.push_back(tl); out.push_back(bl); out.push_back(tr);
                out.push_back(tr); out.push_back(bl); out.push_back(br);
            }
        }
        };
    generateIndices(1, indicesLOD0); // pe³na siatka
    generateIndices(2, indicesLOD1); // co drugi punkt
    generateIndices(8, indicesLOD2); // rzadka == ka¿dy kwadrat = 2 trójk¹gty

    const int trianglesPerFrame = indicesLOD0.size() / 3;  // 3 indeksy = 1 trójk¹t
    std::cout << "Liczba trójk¹tów: " << trianglesPerFrame << std::endl;

    // wierzcho³ki
    GLuint VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    // indeksy
    GLuint EBO_LOD[3];
    glGenBuffers(3, EBO_LOD);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_LOD[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesLOD0.size() * sizeof(unsigned int), indicesLOD0.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_LOD[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesLOD1.size() * sizeof(unsigned int), indicesLOD1.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_LOD[2]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesLOD2.size() * sizeof(unsigned int), indicesLOD2.data(), GL_STATIC_DRAW);

    // konfiuracja
    GLuint VAO_LOD[3];
    for (int lod = 0; lod < 3; lod++) {
        glGenVertexArrays(1, &VAO_LOD[lod]);
        glBindVertexArray(VAO_LOD[lod]);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_LOD[lod]);
    }
    glBindVertexArray(0);

    // skala terenu
    float scaleXZ = 0.02;
    float scaleY = 0.002f;
    float halfW = terrain.width * scaleXZ * 0.5f;
    float halfH = terrain.height * scaleXZ * 0.5f;

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        if (deltaTime > 0.05f)
            deltaTime = 0.05f;

        // ruch kamery
        if (use3DView)
            processCameraInput(window);

        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glUniform1f(glGetUniformLocation(shaderProgram, "scaleXZ"), scaleXZ);
        glUniform1f(glGetUniformLocation(shaderProgram, "scaleY"), scaleY);
        glUniform1i(glGetUniformLocation(shaderProgram, "terrainWidth"), terrain.width);
        glUniform1i(glGetUniformLocation(shaderProgram, "terrainHeight"), terrain.height);

        glm::mat4 projection;
        glm::mat4 view;

        if (use3DView) {
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);
            float aspect = (float)width / (float)height;

            projection = glm::perspective(
                glm::radians(fov3D),
                aspect,
                0.1f,
                1000.0f
            );
            view = glm::lookAt(camPos, camPos + camFront, camUp);
        }
        else {
            float hw = halfW * zoom2D;
            float hh = halfH * zoom2D;

            projection = glm::ortho(
                -hw + cam2DOffset.x,
                hw + cam2DOffset.x,
                -hh + cam2DOffset.y,
                hh + cam2DOffset.y,
                -100.0f,
                100.0f
            );
            view = glm::lookAt(glm::vec3(0.0f, 50.0f, 0.0f), glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
        }

        glm::mat4 mvp = projection * view * glm::mat4(1.0f);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "mvp"), 1, GL_FALSE, glm::value_ptr(mvp));

        glBindVertexArray(VAO_LOD[currentLOD]);
        size_t indexCount = currentLOD == 0 ? indicesLOD0.size() :
            currentLOD == 1 ? indicesLOD1.size() :
            indicesLOD2.size();
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);

        frameCount++;
        double currentTime = glfwGetTime();

        if (currentTime - lastTime >= 1.0) {
            double fps = frameCount / (currentTime - lastTime);
            double trisPerSec = fps * trianglesPerFrame;
            std::cout << "FPS: " << fps << " | Triangles/sec: " << trisPerSec << std::endl;

            if (fps < 10.0 && currentLOD < 2) {
                currentLOD++; // zmniejsz szczegó³owoœæ
                std::cout << "AutoLOD: na LOD" << currentLOD << std::endl;
            }
            else if (fps > 25.0 && currentLOD > 0) {
                currentLOD--; // zwiêksz szczegó³owoœæ
                std::cout << "AutoLOD: na LOD" << currentLOD << std::endl;
            }

            frameCount = 0;
            lastTime = currentTime;
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(3, EBO_LOD);
    glDeleteVertexArrays(3, VAO_LOD);
    glDeleteProgram(shaderProgram);
    glfwTerminate();
    return 0;
}
