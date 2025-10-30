#include <glm/glm.hpp>
#include <SDL.h>
#include <iostream>
#include <cassert>

int test_glm();
int test_sdl();
int failed_Test();

int main() {    
    test_glm();
    test_sdl();
    failed_Test();
    std::cout << "Tests passed" << std::endl;
    return 0;
}

int test_glm() {
    glm::vec3 v(1.0f, 2.0f, 3.0f);
    assert(v.x == 1.0f && v.y == 2.0f && v.z == 3.0f);
    return 0;
}

int test_sdl() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return -1;
    }
    SDL_Quit();
    return 0;
}

int failed_Test() {
    assert(false && "This test is designed to fail");
    return 0;
}