#include <cppgl.h>
#include <iostream>
#include <fstream>
#include "lightfield.h"


using namespace cppgl;

bool ourAlgorithm = true;
bool moveToLightfieldDisplay = false;
int xpos; int ypos;
Lightfield* lightfield;


//IO function for keyboard input
void keyboard_callback(int key, int scancode, int action, int mods) {
    if (ImGui::GetIO().WantCaptureKeyboard) return;
    if (key == GLFW_KEY_ENTER && action == GLFW_PRESS)
        Context::screenshot("screenshot.png");
    if (key == GLFW_KEY_T && action == GLFW_PRESS) ourAlgorithm = !ourAlgorithm;
    if (key == GLFW_KEY_M && action == GLFW_PRESS) { 
        moveToLightfieldDisplay = !moveToLightfieldDisplay;
        if (moveToLightfieldDisplay) {
            glfwSetWindowPos(Context::instance().glfw_window, xpos, ypos);
            glfwSetWindowSize(Context::instance().glfw_window, lightfield->imageWidth, lightfield->imageHeight);
        }
        else {
            glfwSetWindowPos(Context::instance().glfw_window, 100, 100);
            glfwSetWindowSize(Context::instance().glfw_window, lightfield->imageWidth/2, lightfield->imageHeight/2);
        }
    }
}

void display_text() {
    ImGui::SetNextWindowPos(ImVec2(Context::resolution().x - Context::resolution().x / 4, Context::resolution().y / 24));
    ImGui::SetNextWindowSize(ImVec2(250, 100));
    if (ImGui::Begin("Algorithm", 0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoBackground)){
        ImGui::SetWindowFontScale(1.2f);
        if (ourAlgorithm) {
            ImGui::TextColored(ImVec4(0.0f, 0.0f, 0.0f, 1.0f), "Our Algorithm");
        }
        else {
            ImGui::TextColored(ImVec4(0.0f, 0.0f, 0.0f, 1.0f), "Standard Algorithm");
        }
    }
    ImGui::End();
}



// --------------------------------------------------------------------
// main
int main(int argc, char** argv) {   

    //Initialize parameters for our adapted projective mapping
    lightfield = new Lightfield();
    lightfield->setLightfieldParameters();

    //Init GL and window
    ContextParameters params;
    params.title = "Looking Glass Output";
    params.width = lightfield->imageWidth/2;
    params.height = lightfield->imageHeight/2;
    params.gl_major = 3;
    params.gl_minor = 3;
    params.resizable = GLFW_FALSE;
    params.decorated = GLFW_FALSE;
    params.swap_interval = 1;
    Context::init(params);
    int count; 
    GLFWmonitor** monitors = glfwGetMonitors(&count);
    glfwMakeContextCurrent(Context::instance().glfw_window);
    glfwGetMonitorPos(monitors[count-1], &xpos, &ypos);
    std::cout << "Light field display at: " << xpos << ", " << ypos << std::endl;
    glClearColor(1,1,1,1);

    //Set IO functions
    Context::set_keyboard_callback(keyboard_callback);

    //setup draw shader
    Shader("draw", "draw.vs", "draw.fs");

    //Camera init
    auto defaultcam = Camera("std");
    make_camera_current(defaultcam);
    current_camera()->dir = glm::vec3(current_camera()->dir.z, current_camera()->dir.y, current_camera()->dir.x);
    current_camera()->pos -= current_camera()->dir * 2.f;
    current_camera()->update();

    //Load meshes
    for (auto& mesh : load_meshes_gpu("../teapot/teapot.obj", true))
        Drawelement(mesh->name, Shader::find("draw"), mesh);

    //Calculate additional parameters for our adapted projective mapping
    lightfield->calculateRotatedBoundingBoxDimensions();
    lightfield->getFrustumParameters();
    lightfield->setupQuilts();

    //Run
    while (Context::running()) {
        //Render the individual views
        lightfield->viewRendering(ourAlgorithm);

        //Interlace the views and display the interlaced image
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        lightfield->interlacing(ourAlgorithm);
        Context::swap_buffers();
        glFinish();

        //Display which method is currently rendered
        if (!moveToLightfieldDisplay) display_text();
    }

    return 0;

}


