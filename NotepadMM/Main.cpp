﻿#include <SDL.h>
#include <SDL_render.h>
#include <SDL_ttf.h>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <utility>
#include <functional>
#include <fstream>
#include <filesystem>
#include <cstdlib>

//cleanups up
void cleanup(SDL_Window* window, SDL_Renderer* renderer, TTF_Font* font) {
	if (window) SDL_DestroyWindow(window);
	if (renderer) SDL_DestroyRenderer(renderer);
	if (font) TTF_CloseFont(font);

	TTF_Quit();
	SDL_Quit();
}


// updates the window title by adding a star when ever called
void updateWindowTitle(SDL_Window* window, const std::string& currentFileName, bool isSaved) {
	std::string title = "Notepad--";
	if (!currentFileName.empty()) {
		title = title + "    " + currentFileName;
		if (!isSaved) {
			title = title + " *";
		}
	}
	SDL_SetWindowTitle(window, title.c_str());
}


// saves the current text into a file
// if no file name then user promted for it
// places file in a NotepadMM directory
bool isSaved = true; // Start with a saved state
std::string currentFileName = "";
void saveFile(std::vector<std::string>& lines, std::string& currentFileName, SDL_Window* window, std::string& feedbackGutterText, bool& feedbackMode, std::string& feedbackGutterOutput, bool& isEnteringFileName) {

	const std::string folderName = "NotepadMM";

	// Check if the folder exists, and create it if necessary
	if (!std::filesystem::exists(folderName)) {
		if (!std::filesystem::create_directory(folderName)) {
			feedbackGutterText = "Error: Failed to create folder: " + folderName;
			return;
		}
	}

	if (currentFileName.size() == 0) {
		feedbackMode = true;
		isEnteringFileName = true;
		feedbackGutterText = "Enter file name: ";
		feedbackGutterOutput = ""; // Clear any previous input
		return; // File save will resume after file name is entered
	}

	std::string fullPath = folderName + "/" + currentFileName;

	std::ofstream currentFile(fullPath);

	for (const auto& line : lines) {
		currentFile << line << "\n";
	}

	currentFile.close();
	feedbackGutterText = "File saved as: " + fullPath;
	isSaved = true;
	updateWindowTitle(window, currentFileName, isSaved);
}


// asks user to enter name for file
// the rest of save as is handled in update code where ctrl+shift+s is pressed
void saveAsFile(std::vector<std::string>& lines, std::string& currentFileName, SDL_Window* window, std::string& feedbackGutterText, bool& feedbackMode, std::string& feedbackGutterOutput, bool& isEnteringFileName) {
	feedbackMode = true;
	isEnteringFileName = true;
	feedbackGutterText = "Enter file name to save as: ";
	feedbackGutterOutput = ""; // Clear any previous input
}


// asks user to enter name of file to open
bool isOpeningFile = false;
void openFile(std::string& feedbackGutterText, bool& feedbackMode, std::string& feedbackGutterOutput) {
	feedbackMode = true;
	isOpeningFile = true;
	feedbackGutterText = "Enter file name to open: ";
	feedbackGutterOutput = "";
}

// runs command in terminal to run python code
void runCode(std::string& feedbackGutterText, const std::string& currentFileName, bool isSaved) {

	if (!isSaved) {
		feedbackGutterText = "Save file first before running it!";
		return;
	}

	if (currentFileName.empty()) {
		feedbackGutterText = "No file to run!";
		return;
	}

	// Construct the full path to the file
	std::filesystem::path filePath = std::filesystem::current_path() / "NotepadMM" / currentFileName;
	std::string fullPath = filePath.string();

	// Escape or quote the path to handle special characters
	std::string command = "python \"" + fullPath + "\"";

	// Run the Python script
	int result = std::system(command.c_str());
	if (result != 0) {
		feedbackGutterText = "Error running file. Check for errors.";
	}
	else {
		feedbackGutterText = "File ran successfully!";
	}

}


// for each line in paste, adds this to each line in lines
void pasteText(std::vector<std::string>& lines, int& curserLine, int& curserPosition) {

	char* clipboardText = SDL_GetClipboardText();
	std::string textToPaste(clipboardText);
	SDL_free(clipboardText);

	lines[curserLine].insert(curserPosition, textToPaste);

	curserPosition += textToPaste.size();

}


// main code
int main(int argc, char* argv[]) {

	SDL_Init(SDL_INIT_EVERYTHING);
	

	// Creates a window
	SDL_Window* window = SDL_CreateWindow("Notepad--", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 600, 400, SDL_WINDOW_SHOWN);
	if (!window) { 
		std::cout << "Failed to create window! Error: " << SDL_GetError() << std::endl;
		SDL_Quit();
		return -1;
	}

	// Creates icon
	SDL_Surface* iconSurface = SDL_LoadBMP("NotepadMMIcon.bmp");
	if (!iconSurface) {
		std::cerr << "Failed to load icon! Error: " << SDL_GetError() << std::endl;
	}
	else {
		SDL_SetWindowIcon(window, iconSurface);
		SDL_FreeSurface(iconSurface); // Free the surface after setting it as an icon
	}

	// creates renderer
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
	SDL_Color feedbackGutterRed = {220,20,60,255};
	SDL_Texture* textTexture = nullptr;


	//----------------------- MAIN LOOP -----------------------------


	// vars for shift-[n] function
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
		{SDLK_RETURN, '+'},
	};
	bool shiftActive = false;

	// vars for scrolling
	int scrollOffsetY = 0;
	int scrollOffsetX = 0;
	int yOffset;
	int scrollSpeed = 50;

	// vars for gutter
	int gutterWidth = 30;
	int gutterIncreaseCorrection = 0;

	// vars for feedback gutter
	std::string feedbackGutterText = " ";
	bool feedbackMode = false;
	bool isEnteringFileName = false;
	std::string feedbackGutterOutput;

	// vars for vector to hold all text in document
	std::vector<std::string> lines = {""};
	int lineSpacing = 0;
	int maxWidth = 570-gutterWidth;

	// vars for curser
	bool curserVisible = true;
	int curserPosition = 0;
	int curserLine = 0;

	// vars for ctrl-[n] functions
	std::unordered_map<SDL_KeyCode, std::function<void()>> ctrlKeyActions = {
		{SDLK_s, [&]() { saveFile(lines, currentFileName, window, feedbackGutterText, feedbackMode, feedbackGutterOutput, isEnteringFileName); }},
		{SDLK_o, [&]() { openFile(feedbackGutterText, feedbackMode, feedbackGutterOutput); }},     
		{SDLK_r, [&]() { runCode(feedbackGutterText, currentFileName, isSaved); }},    
		{SDLK_v, [&]() { pasteText(lines, curserLine, curserPosition); }}
	};


	// running loop
	bool running = true;
	SDL_Event e;
	while (running) {

		int lineHeight = TTF_FontHeight(font);
		int maxContentHeight = lines.size() * lineHeight;

		// checks each periphal given to see if user input is given
		while (SDL_PollEvent(&e)) {
			switch (e.type) {
			
			// when window closed
			case SDL_QUIT:
				running = false;
				break;

			// when any key is pressed down
			case SDL_KEYDOWN: {

				SDL_Keymod modState = SDL_GetModState();

				// handles user typing into feedback gutter
				if (feedbackMode) {
					// Append user input to feedbackGutterOutput
					if (e.key.keysym.sym >= SDLK_SPACE && e.key.keysym.sym <= SDLK_z) {
						char keyPressed = static_cast<char>(e.key.keysym.sym);

						// Handle Shift or Caps Lock for upper case
						bool shiftActive = (modState & KMOD_SHIFT);
						bool capsActive = (modState & KMOD_CAPS);

						if ((shiftActive ^ capsActive) && keyPressed >= 'a' && keyPressed <= 'z') {
							keyPressed -= 32; // Convert to uppercase
						}

						feedbackGutterOutput += keyPressed; // Append character
						feedbackGutterText += keyPressed; // Update feedback gutter
					}

					// Handle backspace for feedbackMode
					else if (e.key.keysym.sym == SDLK_BACKSPACE) {
						if (!feedbackGutterOutput.empty()) {
							feedbackGutterOutput.pop_back(); // Remove last character
							feedbackGutterText.pop_back(); // Update feedback gutter
						}
					}
					else if (e.key.keysym.sym == SDLK_RETURN)
					{
						if (isEnteringFileName) {
							currentFileName = feedbackGutterOutput;
							isEnteringFileName = false;
							feedbackMode = false;
							feedbackGutterText = "";

							saveFile(lines, currentFileName, window, feedbackGutterText, feedbackMode, feedbackGutterOutput, isEnteringFileName);
						}

						if (isOpeningFile) {

							std::ifstream inputFile(feedbackGutterOutput);
							if (inputFile.is_open()) {
								lines.clear(); // Clear current lines
								std::string line;
								while (std::getline(inputFile, line)) {
									lines.push_back(line);
								}
								inputFile.close();

								// Extract file name from the full path
								std::filesystem::path filePath(feedbackGutterOutput);
								currentFileName = filePath.filename().string(); // Store only the file name

								feedbackGutterText = "File loaded successfully!";
								updateWindowTitle(window, currentFileName, true); // Update window title
							}
							else {
								feedbackGutterText = "Error: Could not open file.";
							}
							feedbackMode = false;
							isOpeningFile = false;

						}
					}
				}
				
				// handles user typing into main text area
				else {
					
					updateWindowTitle(window, currentFileName, isSaved);

					feedbackGutterText.clear();

					// RETURN AND TAB
					if (e.key.keysym.sym == SDLK_RETURN) {
						curserLine += 1;
						curserPosition = 0;
						lines.insert(lines.begin() + curserLine, ""); // Insert a new line at the cursor line
					}
					else if (e.key.keysym.sym == SDLK_TAB) {
						lines[curserLine].insert(curserPosition, "    ");
						curserPosition += 4;
					}


					// CTRL KEYS
					bool ctrlActive = (modState & KMOD_CTRL);
					if (ctrlActive && (modState & KMOD_SHIFT) && e.key.keysym.sym == SDLK_s) {
						saveAsFile(lines, currentFileName, window, feedbackGutterText, feedbackMode, feedbackGutterOutput, isEnteringFileName);
					}
					else if (ctrlActive) {
						SDL_KeyCode key = static_cast<SDL_KeyCode>(e.key.keysym.sym);

						// Check if the key is in the map
						if (ctrlKeyActions.find(key) != ctrlKeyActions.end()) {
							ctrlKeyActions[key](); // Call the corresponding function
						}
					}
					
					// CHARS
					// If any key is pressed, convert that to a string and concat that with initial text

					else if (e.key.keysym.sym >= SDLK_SPACE && e.key.keysym.sym <= SDLK_z) {

						feedbackGutterOutput = "";
						isSaved = false;

						// appends letter to last line
						char keyPressed = static_cast<char>(e.key.keysym.sym);

						// Check if Shift or Caps Lock is active
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

						lines[curserLine].insert(curserPosition, 1, keyPressed);
						curserPosition += 1;

					}

					// Handles deleting letters
					else if (e.key.keysym.sym == SDLK_BACKSPACE) {
						if (curserPosition > 0) {
							// Remove a character from the current line
							lines[curserLine].erase(curserPosition - 1, 1);
							curserPosition -= 1;
						}
						else if (curserLine > 0) {
							// Merge the current line with the previous line
							curserPosition = lines[curserLine - 1].size();
							lines[curserLine - 1] += lines[curserLine];
							lines.erase(lines.begin() + curserLine);
							curserLine -= 1;
						}
					}


					// Handles moving cursor
					else if (e.key.keysym.sym == SDLK_LEFT) {
						if (curserPosition >= 1) {
							curserPosition -= 1;
						}
						else if (curserLine >= 1)
						{
							curserLine -= 1;
							curserPosition = lines[curserLine].size();
						}
					}
					else if (e.key.keysym.sym == SDLK_RIGHT) {
						if (curserPosition < lines[curserLine].size()) {
							curserPosition += 1;
						}
						else if (curserLine < lines.size() - 1)
						{
							curserLine += 1;
							curserPosition = 0;
						}
					}
					else if (e.key.keysym.sym == SDLK_UP && curserLine >= 1)
					{
						// if curser bigger than text above then go to end, else go to same pos but above
						if (curserPosition > lines[curserLine - 1].size()) {
							curserLine -= 1;
							curserPosition = lines[curserLine].size();
						}
						else
						{
							curserLine -= 1;
						}
						//std::cout << "test" << std::endl;
					}
					else if (e.key.keysym.sym == SDLK_DOWN && curserLine < lines.size() - 1) {
						if (curserPosition > lines[curserLine + 1].size()) {
							curserLine += 1;
							curserPosition = lines[curserLine].size();
						}
						else
						{
							curserLine += 1;
						}
					}
				}
				break;
			}

			// when scroll wheel moved
			case SDL_MOUSEWHEEL:
				// Check if Shift is pressed
				shiftActive = (SDL_GetModState() & KMOD_SHIFT);

				int textWidth, textHeight;
				TTF_SizeText(font, lines[curserLine].c_str(), &textWidth, &textHeight);

				if (e.wheel.y > 0) {
					if (shiftActive) {
						scrollOffsetX -= scrollSpeed/2; // horizontal scroll
					}
					else {
						scrollOffsetY += scrollSpeed; // Regular Scroll Up
					}
				}
				else if (e.wheel.y < 0) {
					if (shiftActive) {
						scrollOffsetX += scrollSpeed/2; // horizontal scroll
					}
					else {
						scrollOffsetY -= scrollSpeed; // Regular Scroll Down
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


		// draws background
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		SDL_RenderClear(renderer);


		// draws char on each line of vector
		int yOffset = 10 + scrollOffsetY; 
		for (int i = 0; i < lines.size(); ++i) {
			const std::string& line = lines[i];

			// Handle empty lines explicitly
			if (line.empty()) {
				// Advance yOffset even if the line is empty
				yOffset += lineHeight + lineSpacing;
				continue;
			}

			SDL_Surface* textSurface = TTF_RenderText_Blended(font, line.c_str(), green);
			if (textSurface) {
				SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

				SDL_Rect textRect = { 10 + gutterWidth + scrollOffsetX, yOffset, textSurface->w, textSurface->h };
				SDL_RenderCopy(renderer, textTexture, NULL, &textRect);

				SDL_DestroyTexture(textTexture);
			}
			SDL_FreeSurface(textSurface);

			yOffset += lineHeight + lineSpacing; // Move to the next line
		}


		// curser blinking
		bool curserVisible = true;
		//bool curserVisible = (SDL_GetTicks() / 500) % 2 == 0;

		// draws curser
		int cursorX = 0, cursorY = curserLine * lineHeight + scrollOffsetY + 10;

		// If the cursor is in the middle of a line, calculate its position
		if (curserPosition > 0 && !lines[curserLine].empty()) {
			TTF_SizeText(font, lines[curserLine].substr(0, curserPosition).c_str(), &cursorX, nullptr);
		}
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderDrawLine(renderer, 10 + gutterWidth + scrollOffsetX + cursorX, cursorY, 10 + gutterWidth + scrollOffsetX + cursorX, cursorY + lineHeight);

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


		// feedback gutter
		SDL_SetRenderDrawColor(renderer, 220, 220, 220, 255); // Lighter gray
		SDL_Rect feedbackGutterRect = { 0,400 - gutterWidth, 600, 400};
		SDL_RenderFillRect(renderer, &feedbackGutterRect);

		// feedback gutter text
		if (!feedbackGutterText.empty()) {
			int textWidth, textHeight;
			TTF_SizeText(font, feedbackGutterText.c_str(), &textWidth, &textHeight);

			SDL_Surface* textSurface = TTF_RenderText_Blended(font, feedbackGutterText.c_str(), feedbackGutterRed);
			SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
			SDL_Rect textRect = { 10, 400 - gutterWidth + (gutterWidth - textHeight) / 2,textSurface->w, textSurface->h };
			SDL_RenderCopy(renderer, textTexture, NULL, &textRect);

			SDL_FreeSurface(textSurface);
			SDL_DestroyTexture(textTexture);
		}

		SDL_RenderPresent(renderer);
	}

	cleanup(window, renderer, font);
	return 0;
}