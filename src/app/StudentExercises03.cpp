#include <iostream>
#include <string>
#include "student/VKSetup.hpp"

using namespace std;
using namespace student;

int main(int argc, char **argv) {
    cout << "BEGIN EXERCISES!!!!" << endl;

    string windowTitle = "Exercises";
    int windowWidth = 640;
    int windowHeight = 480;

    GLFWwindow *window = createGLFWWindow(windowTitle, windowWidth, windowHeight);

    cleanupGLFWwindow(window);

    return 0;
}