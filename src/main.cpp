
#include <flatshaper/main.hpp>

#include <IL/il.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/vec3.hpp>

int main() {
    glm::vec3 test_vector;
    GLFWwindow *test_window = glfwCreateWindow(640, 480, u8"Test", nullptr, nullptr);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    ilGenImage();
}
