#include <SDL.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "SDL2 + GLM Cube Test",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        800, 600,
        SDL_WINDOW_SHOWN
    );

    if (!window) {
        std::cerr << "Window creation failed: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Renderer creation failed: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    bool running = true;
    SDL_Event event;
    float angle = 0.0f;


    glm::vec2 rect[4] = {
        {-50, -50},
        { 50, -50},
        { 50,  50},
        {-50,  50}
    };

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                running = false;
        }

        SDL_SetRenderDrawColor(renderer, 20, 20, 30, 255);
        SDL_RenderClear(renderer);

     
        glm::mat2 rot = glm::mat2(
            glm::vec2(cos(glm::radians(angle)), -sin(glm::radians(angle))),
            glm::vec2(sin(glm::radians(angle)),  cos(glm::radians(angle)))
        );

        glm::vec2 center(400, 300); 
        glm::vec2 rotated[4];
        SDL_Point points[5]; 

        for (int i = 0; i < 4; ++i) {
            glm::vec2 r = rot * rect[i] + center;
            points[i].x = static_cast<int>(r.x);
            points[i].y = static_cast<int>(r.y);
        }
        points[4] = points[0]; 

        SDL_SetRenderDrawColor(renderer, 255, 100, 50, 255);
        SDL_RenderDrawLines(renderer, points, 5);

        SDL_RenderPresent(renderer);

        angle += 1.0f;
        if (angle > 360.0f) angle -= 360.0f;

        SDL_Delay(16); 
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
