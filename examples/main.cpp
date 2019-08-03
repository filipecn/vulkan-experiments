#ifdef _WIN32
#pragma comment(linker, "/subsystem:windows")
#include <windows.h>
#endif
#include <vulkan/vulkan.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <stdio.h>

#include "demos/DemoBase.h"
#include "testlib.h"

int main(const int argc, const char* argv[]) {
#ifdef _WIN32
  // Open a new console window
  AllocConsole();
  AttachConsole(GetCurrentProcessId());
  //-- Associate std input/output with newly opened console window:
  freopen("CONIN$", "r", stdin);
  freopen("CONOUT$", "w", stdout);
  freopen("CONOUT$", "w", stderr);
#endif

  TestLib tl;
  Sleep(1000);
  GLFWwindow* window;

  /* Initialize the library */
  if (!glfwInit()) return -1;
  DemoBase db;
  if (!db.Init()) return 0;

  if (glfwVulkanSupported()) printf("Hello World!\n");
  /* Create a windowed mode window and its OpenGL context */
  window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
  if (!window) {
    glfwTerminate();
    return -1;
  }

  /* Make the window's context current */
  glfwMakeContextCurrent(window);

  /* Loop until the user closes the window */
  while (!glfwWindowShouldClose(window)) {
    /* Render here */

    /* Swap front and back buffers */
    glfwSwapBuffers(window);

    /* Poll for and process events */
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
  return 0;
}
