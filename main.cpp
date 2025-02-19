#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL.h>
#include <SDL3_Image/SDL_image.h>
#include <SDL3/SDL_main.h>
#include "main.h"
#include "gem.h"


constexpr int  WINDOW_WIDTH = 700;
constexpr int  WINDOW_HEIGHT = 700;
constexpr int  CELL_COUNT = 13;
	
static SDL_Window* window = NULL;
static SDL_Renderer* renderer = NULL;
Uint64 lastTick = 0;
Uint64 currentTick = 0;
float deltaTime;


SDL_Texture* texture_gems;
gem* gems[CELL_COUNT][CELL_COUNT];

SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[]) {
	if (!SDL_Init(SDL_INIT_VIDEO)) {
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "SDL_Init failed: %s\n", SDL_GetError());
		return SDL_APP_FAILURE;
	}
	if (!SDL_CreateWindowAndRenderer("Three Match", WINDOW_WIDTH, WINDOW_HEIGHT, 0, &window, &renderer)) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Could not create window and renderer: %s\n", SDL_GetError());
		return SDL_APP_FAILURE;
	}

	SDL_SetPointerProperty(SDL_GetRendererProperties(renderer), SDL_PROP_RENDERER_CREATE_WINDOW_POINTER, window);
	SDL_SetNumberProperty(SDL_GetRendererProperties(renderer), SDL_PROP_RENDERER_CREATE_OUTPUT_COLORSPACE_NUMBER, SDL_COLORSPACE_SRGB_LINEAR);

	texture_gems = IMG_LoadTexture(renderer, "Sprites/gems.png");
	
	
	for (int i = 0; i < CELL_COUNT; i++) {
		for (int j = 0; j < CELL_COUNT; j++) {
			gems[i][j] = new gem();
			gems[i][j]->set_location(38.0f + i * 48, 38.0f + j * 48);
		}
	}

	return SDL_APP_CONTINUE;
}


SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event)
{
	if (event->type == SDL_EVENT_QUIT) {
		return SDL_APP_SUCCESS; 
	}
	else if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
		handle_mouse_button(event);
	}
	
	
	return SDL_APP_CONTINUE;  
}


SDL_AppResult SDL_AppIterate(void* appstate)
{
	lastTick = currentTick;
	currentTick = SDL_GetTicks();
	deltaTime = ((float)currentTick - lastTick)/1000.0f;


	SDL_SetRenderDrawColor(renderer, 52, 145, 77, 0);
	SDL_RenderClear(renderer);


	SDL_FRect rect = { 38, 38, WINDOW_WIDTH-76, WINDOW_HEIGHT-76 };
	
	SDL_SetRenderDrawColor(renderer, 149, 189, 160, 0);
	SDL_RenderFillRect(renderer, &rect);
	

	if (!texture_gems) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Could not load texture: %s\n", SDL_GetError());
		return SDL_APP_FAILURE;
	}


	for (int i = 0; i < CELL_COUNT; i++) {
		for (int j = 0; j < CELL_COUNT; j++) {
			if (gems[i][j] == NULL) {
				continue;
			}
			gem* newGem = gems[i][j];
			float* location = newGem->get_location();

			SDL_FRect srcrect = newGem->source_rect;
			SDL_FRect dstrect{ location[0], location[1],48,48 };
			SDL_RenderTexture(renderer, texture_gems, &srcrect, &dstrect);


			if (location[1] < WINDOW_HEIGHT - 38 - 48 - (j*48)) {
				//newGem->set_location(location[0], location[1] + (100.0f * deltaTime));
			}
		}
	}
	

	SDL_RenderPresent(renderer);
	return SDL_APP_CONTINUE;  
}



void SDL_AppQuit(void* appstate, SDL_AppResult result) {

}

void handle_mouse_button(SDL_Event* event)
{
	SDL_assert(event->type == SDL_EVENT_MOUSE_BUTTON_DOWN);

	float x = 0.0f;
	float y = 0.0f;

	SDL_GetMouseState(&x, &y);

	int* indexes = find_indexes_from_location(x, y);
	if (indexes == NULL) {
		return;
	}
	SDL_Log("%f, %f", x, y);
	SDL_Log("%d, %d", indexes[0], indexes[1]);


	check_around_the_gem(indexes[0], indexes[1]);

}

int* find_indexes_from_location(float x, float y) {

	if (x < 38 || y < 38 || x > 660 || y > 660) {
		return NULL;
	}

	int indexes[2] = { (x - 38) / 48 , (y - 38) / 48 };
	return indexes;
}

void check_around_the_gem(int x, int y)
{
	if (gems[x][y] == NULL) {
		return;
	}

	gem* clicked_gem = gems[x][y];
	gem::EColor clicked_gem_color = clicked_gem->get_color();

	std::vector<int> y_indexes(1, y);
	std::vector<int> x_indexes(1, x);
	int index = y+1;
	bool decrement = false;
	while (1) {

		if (gems[x][index] == NULL || index < 0 || index >= CELL_COUNT) {
			if (decrement) { break; }
			if (gems[x][y+1] == NULL) { break; }
			decrement = true;
			index = y - 1;
		}

		if (decrement) {
			if (gems[x][index]->get_color() == clicked_gem_color) {
				y_indexes.push_back(index);
				index--;
			}
			else { 
				break;
			}
		}
		else {
			if (gems[x][index]->get_color() == clicked_gem_color) {
				y_indexes.push_back(index);
				index++;
			}
			else {
				decrement = true;
				index = y - 1;
			}
		}
		
	}


	index = x - 1;
	decrement = true;
	while (1) {
		if (gems[index][y] == NULL || index < 0 || index >= CELL_COUNT) {
			if (!decrement) { break; }
			if (gems[x + 1][y] == NULL) { break; }
			decrement = false;
			index = x + 1;
		}

		if (decrement) {
			if (gems[index][y]->get_color() == clicked_gem_color) {
				x_indexes.push_back(index);
				index--;
			}
			else {
				decrement = false;
				index = x + 1;
			}
		}
		else {
			if (gems[index][y]->get_color() == clicked_gem_color) {
				x_indexes.push_back(index);
				index++;
			}
			else {
				break;
			}
		}

	}
	std::vector<gem*> first_gems_to_fall_on_column;
	if (x_indexes.size() >= 3) {
		for (int index_x : x_indexes) {
			gems[index_x][y] = NULL;
			
			
			if (!(y_indexes.size() >= 3 && index_x == x)) {
				first_gems_to_fall_on_column.push_back(gems[index_x][y - 1]);
			}
		}
	}
	
	if (y_indexes.size() >= 3) {
		for (int index_y : y_indexes) {
			gems[x][index_y] = NULL;
		}
		first_gems_to_fall_on_column.push_back(gems[x][y_indexes[y_indexes.size() - 1] - 1]);
	}
	
	for (int i = 0; i < first_gems_to_fall_on_column.size(); i++) {
		float* location = first_gems_to_fall_on_column[i]->get_location();
		int* indexes = find_indexes_from_location(location[0], location[1]);
		SDL_Log("%d %d", indexes[0], indexes[1]);
	}
}
