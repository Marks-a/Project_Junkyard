#include <SDL.h>
#include <glm/glm.hpp>
#include <iostream>

int main() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }
    std::cout << "SDL2 initialized successfully!" << std::endl;
    SDL_Quit();
    glm::vec3 vec(1.0f, 2.0f, 3.0f);
    std::cout << "GLM vector: (" << vec.x << ", " << vec.y << ", " << vec.z << ")" << std::endl;

    return 0;
}
