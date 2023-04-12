#include <cppgl.h>

#include <iostream>
#include <fstream>


using namespace cppgl;

// ------------------------------------------
// main

int main(int argc, char** argv) {
    // init GL
    ContextParameters params;
        

    params.title = "Looking Glass Output";
    params.swap_interval = 1;
    Context::init(params);
    glClearColor(1,1,1,1);

    std::cout << "test" << std::endl;

    return 0;

}