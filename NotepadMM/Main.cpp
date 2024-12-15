#include <SDL.h>
#include <SDL_ttf.h>
#include <iostream>



void cleanup(SDL_Window* window, SDL_Renderer* renderer, TTF_Font* font, SDL_Texture* texture) {
	if (window) SDL_DestroyWindow;
	if (renderer) SDL_DestroyRenderer;
	if (font) TTF_CloseFont;
	if (texture) SDL_DestroyTexture;

	TTF_Quit();
	SDL_Quit();
}

int main(int argc, char* argv[]) {

	SDL_Init(SDL_INIT_EVERYTHING);
	TTF_Init();

	// Creates a window
	SDL_Window* window = SDL_CreateWindow("Notepad--", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 600, 400, SDL_WINDOW_SHOWN);
	if (!window) { std::cout << "Failed to create window! Error:" << SDL_GetError() << std::endl; }
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);


	// MAIN LOOP
	bool running = true;
	SDL_Event e;
	while (running) {

		while (SDL_PollEvent(&e)) {
			switch (e.type) {
			case SDL_QUIT:
				running = false;
				break;
			case SDL_KEYDOWN:
				if (e.key.keysym.scancode == SDL_SCANCODE_A) {
					running = false;
				}
			default:
				break;
			}
		}

		// draws background
		SDL_SetRenderDrawColor(renderer, 188, 255, 255, 255);
		SDL_RenderClear(renderer);


		// updates screen
		SDL_RenderPresent(renderer);
	}

	SDL_Quit();
	return 0;
}