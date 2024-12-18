#include <SDL.h>
#include <SDL_render.h>
#include <SDL_ttf.h>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <utility>

void cleanup(SDL_Window* window, SDL_Renderer* renderer, TTF_Font* font) {
	if (window) SDL_DestroyWindow(window);
	if (renderer) SDL_DestroyRenderer(renderer);
	if (font) TTF_CloseFont(font);

	TTF_Quit();
	SDL_Quit();
}

int main(int argc, char* argv[]) {

	SDL_Init(SDL_INIT_EVERYTHING);
	

	// Creates a window
	SDL_Window* window = SDL_CreateWindow("Notepad--", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 600, 400, SDL_WINDOW_SHOWN);
	if (!window) { 
		std::cout << "Failed to create window! Error: " << SDL_GetError() << std::endl;
		SDL_Quit();
		return -1;
	}

	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
	if (!renderer) {
		std::cout << "Failed to create renderer! Error: " << SDL_GetError() << std::endl;
		cleanup(window, nullptr, nullptr);
		return -1;
	}

	TTF_Init();

	// Create a font and colour
	TTF_Font* font = TTF_OpenFont("Consolas.ttf", 24);
	if (!font) {
		std::cout << "Failed to load font! Error:" << TTF_GetError() << std::endl;
		cleanup(window, renderer, nullptr);
		return -1;
	}
	SDL_Color green = { 0, 0, 255, 255 };
	SDL_Color gutterGrey = { 150, 150, 150, 255 };
	SDL_Texture* textTexture = nullptr;


	// MAIN LOOP

	std::unordered_map<SDL_KeyCode, char> shiftSymbols = {
		{SDLK_1, '!'},
		{SDLK_2, '"'},
		{SDLK_3, '£'}, 
		{SDLK_4, '$'},
		{SDLK_5, '%'},
		{SDLK_6, '^'},
		{SDLK_7, '&'},
		{SDLK_8, '*'},
		{SDLK_9, '('},
		{SDLK_0, ')'},
		{SDLK_MINUS, '_'},
		{SDLK_EQUALS, '+'},
		{SDLK_LEFTBRACKET, '{'},
		{SDLK_RIGHTBRACKET, '}'},
		{SDLK_SEMICOLON, ':'},
		{SDLK_QUOTE, '@'}, 
		{SDLK_BACKSLASH, '|'}, 
		{SDLK_COMMA, '<'},
		{SDLK_PERIOD, '>'}, 
		{SDLK_SLASH, '?'},
		{SDLK_BACKQUOTE, '¬'},
		{SDLK_HASH,'~'},
		{SDLK_RETURN, '+'}
	};
	bool shiftActive = false;

	int scrollOffsetY = 0;
	int scrollOffsetX = 0;
	int yOffset;

	int gutterWidth = 30;
	int gutterIncreaseCorrection = 0;

	std::vector<std::string> lines = {""};
	int lineSpacing = 0;
	int maxWidth = 570-gutterWidth;

	bool curserVisible = true;
	int curserPosition = 0;
	int curserLine = 0;

	bool running = true;
	SDL_Event e;
	while (running) {

		int lineHeight = TTF_FontHeight(font);
		int maxContentHeight = lines.size() * lineHeight;

		while (SDL_PollEvent(&e)) {
			switch (e.type) {
			case SDL_QUIT:
				running = false;
				break;
			case SDL_KEYDOWN:

				if (e.key.keysym.sym == SDLK_RETURN) {
					// Move to a new line
					curserPosition = 0;
					curserLine += 1;
					lines.push_back("");
				}

				// If any key is pressed, convert that to a string and concat that with initial text

				else if (e.key.keysym.sym >= SDLK_SPACE && e.key.keysym.sym <= SDLK_z) {
					
					// appends letter to last line
					char keyPressed = static_cast<char>(e.key.keysym.sym);

					// Check if Shift or Caps Lock is active
					SDL_Keymod modState = SDL_GetModState();
					shiftActive = (modState & KMOD_SHIFT);
					bool capsActive = (modState & KMOD_CAPS);

					// Shift Chars
					if (shiftActive && shiftSymbols.find(static_cast<SDL_KeyCode>(e.key.keysym.sym)) != shiftSymbols.end()) {
						keyPressed = shiftSymbols[static_cast<SDL_KeyCode>(e.key.keysym.sym)];
					}
					// Upper Case
					if ((shiftActive ^ capsActive) && keyPressed >= 'a' && keyPressed <= 'z') {
						keyPressed -= 32;
					}


					//lines.back() += keyPressed;
					lines.back().insert(curserPosition, 1, keyPressed);
					curserPosition += 1;

				}

				// Handles deleting letters
				else if (e.key.keysym.sym == SDLK_BACKSPACE)
				{
					if (lines.back().size() >= 1) {
						lines.back().erase(curserPosition - 1, 1);
						curserPosition -= 1;
					}
					else if (lines.size() >= 2) {
						lines.pop_back();
						curserLine -= 1;
						curserPosition = lines.back().size();
						lines.back().erase(curserPosition - 1, 1);
					}
				}

				// Handles moving cursor
				else if (e.key.keysym.sym == SDLK_LEFT) {
					if (curserPosition >= 1) {
						curserPosition -= 1;
					}
				}
				else if (e.key.keysym.sym == SDLK_RIGHT) {
					if (curserPosition < lines.back().size()) {
						curserPosition += 1;
					}
				}
				break;

			case SDL_MOUSEWHEEL:
				// Check if Shift is pressed
				shiftActive = (SDL_GetModState() & KMOD_SHIFT);

				int textWidth, textHeight;
				TTF_SizeText(font, lines[curserLine].c_str(), &textWidth, &textHeight);

				if (e.wheel.y > 0) {
					if (shiftActive) {
						scrollOffsetX -= 10;
					}
					else {
						scrollOffsetY += 10; // Regular Scroll Up
					}
				}
				else if (e.wheel.y < 0) {
					if (shiftActive) {
						scrollOffsetX += 10;
					}
					else {
						scrollOffsetY -= 10; // Regular Scroll Down
					}
				}

				// Clamp the scroll offset to prevent scrolling beyond bounds
				scrollOffsetY = std::min(scrollOffsetY, 0);
				scrollOffsetX = std::min(scrollOffsetX, 0);

				//std::cout << textWidth << std::endl;
				//std::cout << "Scroll Offset X: " << scrollOffsetX << std::endl;
				break;


			default:
				break;
			}
		}

		//std::cout << curserPosition << ", " << lines.back().size() << std::endl;

		// draws background
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		SDL_RenderClear(renderer);


		// draws char on each line of vector
		int yOffset = 10;
		for (const std::string& line : lines) {

			// Renders text, then converts that into a surface
			SDL_Surface* textSurface = TTF_RenderText_Blended(font, line.c_str(), green);
			if (textSurface) {
				SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

				// Renders Text into a box
				SDL_Rect textRect = { 10 + gutterWidth + scrollOffsetX, yOffset + scrollOffsetY, textSurface->w, textSurface->h };
				SDL_RenderCopy(renderer, textTexture, NULL, &textRect);

				yOffset += textSurface->h + lineSpacing;

				SDL_DestroyTexture(textTexture);
			}
			SDL_FreeSurface(textSurface);
		}

		// draws gutter
		SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255); // Light gray
		SDL_Rect gutterRect = { 0, 0, gutterWidth, 400 };
		SDL_RenderFillRect(renderer, &gutterRect);

		// gutter text
		for (int i = 0; i < lines.size(); i++) {
			int textWidth, textHeight;
			TTF_SizeText(font, std::to_string(i).c_str(), &textWidth, &textHeight);

			if ((textWidth+5) > gutterWidth) {
				gutterWidth += 5;
				maxWidth -= 5;
			}

			SDL_Surface* textSurface = TTF_RenderText_Blended(font, std::to_string(i).c_str(), gutterGrey);
			SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
			SDL_Rect textRect = { gutterWidth - textWidth-5, textHeight*(i) + 10 + scrollOffsetY,textSurface->w, textSurface->h};
			SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
		}


		// curser blinking
		bool curserVisible = true;
		//bool curserVisible = (SDL_GetTicks() / 500) % 2 == 0;

		// draws curser
		int textWidth, textHeight;
		TTF_SizeText(font, lines.back().substr(0, curserPosition).c_str(), &textWidth, &textHeight);

		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		if (textWidth > 0 && curserVisible) {
			SDL_RenderDrawLine(renderer, textWidth + 10 + gutterWidth + scrollOffsetX, yOffset - textHeight + scrollOffsetY, textWidth + 10 + gutterWidth + scrollOffsetX, yOffset + scrollOffsetY);
		}
		

		// updates screen
		SDL_RenderPresent(renderer);
	}

	cleanup(window, renderer, font);
	return 0;
}