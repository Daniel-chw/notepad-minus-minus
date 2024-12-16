#include <SDL.h>
#include <SDL_ttf.h>
#include <iostream>
#include <cstring>


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

	// Create a font and colour
	TTF_Font* font = TTF_OpenFont("arial.ttf", 24);
	if (!font) std::cout << "Failed to load font! Error:" << TTF_GetError() << std::endl;
	SDL_Color green = { 0, 0, 255, 255 };
	SDL_Texture* textTexture = nullptr;


	// MAIN LOOP
	char text[100] = "Hello";
	bool running = true;
	SDL_Event e;
	while (running) {

		while (SDL_PollEvent(&e)) {
			switch (e.type) {
			case SDL_QUIT:
				running = false;
				break;
			case SDL_KEYDOWN:
				// If any key is pressed, convert that to a string and concat that with initial text
				if (e.key.keysym.sym >= SDLK_SPACE && e.key.keysym.sym <= SDLK_z) {
					char keyPressed[2] = { static_cast<char>(e.key.keysym.sym), '\0' };
					strcat_s(text, sizeof(text), keyPressed);
				}
			default:
				break;
			}
		}

		// Renders text, then converts that into a surface
		SDL_Surface* textSurface = TTF_RenderText_Blended(font, text, green);
		if (!textSurface) { std::cout << "Failed to render text! Error:" << TTF_GetError() << std::endl; return -1; }
		SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

		// draws background
		SDL_SetRenderDrawColor(renderer, 188, 255, 255, 255);
		SDL_RenderClear(renderer);

		// Renders Text into a box
		SDL_Rect textRect = { 10, 10, textSurface->w, textSurface->h };
		SDL_RenderCopy(renderer, textTexture, NULL, &textRect);

		SDL_DestroyTexture(textTexture);
		textTexture = nullptr;

		// updates screen
		SDL_RenderPresent(renderer);
	}

	cleanup(window, renderer, font, textTexture);
	return 0;
}