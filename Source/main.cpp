#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL.h>
#include <SDL3_Image/SDL_image.h>
#include <SDL3/SDL_main.h>
#include "main.h"
#include <time.h>


constexpr int  WINDOW_WIDTH = 700;
constexpr int  WINDOW_HEIGHT = 700;
constexpr int  CELL_COUNT = 13;
constexpr float  CELL_SPEED = 300.0f;
	
static SDL_Window* window = NULL;
static SDL_Renderer* renderer = NULL;

Uint64 lastTick = 0;
Uint64 currentTick = 0;

float deltaTime;


SDL_Texture* texture_gems;
Gem gems[CELL_COUNT][CELL_COUNT];

int indexes_of_clicked_cell[2] = {-1, -1};
std::vector<int> queued_columns;

bool input_active = true;



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
	
	init_spawn_gems();

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
			
			Gem* curr_gem = &gems[i][j];

			if (!curr_gem->active) { continue; }

			render_gem(curr_gem);

			update_gem_location(curr_gem);
		}
	}

	
	SDL_RenderPresent(renderer);
	return SDL_APP_CONTINUE;  
}




void init_spawn_gems() {
	srand(time(NULL));
	for (int i = 0; i < CELL_COUNT; i++) {
		for (int j = 0; j < CELL_COUNT; j++) {
			bool gem_match = true;
			while (gem_match) {
				gems[i][j] = Gem();
				set_color(gems[i][j], static_cast<EColor>(rand() % 4));
				gem_match = check_init_match_at(i, j, gems[i][j].color);
			}
			gems[i][j].location[0] = 38.0f + i * 48;
			gems[i][j].location[1] = 38.0f + j * 48;
			gems[i][j].target_location[0] = 38.0f + i * 48;
			gems[i][j].target_location[1] = 38.0f + j * 48;
		}
	}
}




void render_gem(Gem* gem) {
	SDL_FRect srcrect = gem->source_rect;
	SDL_FRect dstrect{ gem->location[0], gem->location[1],48,48 };
	SDL_RenderTexture(renderer, texture_gems, &srcrect, &dstrect);
}




void update_gem_location(Gem* gem) {

	float* location = gem->location;
	float* target_location = gem->target_location;

	float x_distance_to_target_location = SDL_fabsf(target_location[0] - location[0]);
	float y_distance_to_target_location = SDL_fabsf(target_location[1] - location[1]);

	if (x_distance_to_target_location < 0.1f && y_distance_to_target_location < 0.1f) {
		return;
	}
	
	
	if (y_distance_to_target_location > 2) {

		gem->location[1] += (CELL_SPEED * deltaTime);
	}
	else {
		gem->location[0] = target_location[0];
		gem->location[1] = target_location[1];
	}
}




void SDL_AppQuit(void* appstate, SDL_AppResult result) {

}




void set_color(Gem& gem, EColor target_color)
{
	gem.color = target_color;
	switch (gem.color) {
	case EColor_Red:
		gem.source_rect.y = 0;
		break;
	case EColor_Blue:
		gem.source_rect.y = 32;
		break;
	case EColor_Pink:
		gem.source_rect.y = 64;
		break;
	case EColor_Green:
		gem.source_rect.y = 96;
		break;
	}
}




void handle_mouse_button(SDL_Event* event)
{
	SDL_assert(event->type == SDL_EVENT_MOUSE_BUTTON_DOWN);

	if (!input_active) { return; }

	float x = 0.0f;
	float y = 0.0f;

	SDL_GetMouseState(&x, &y);

	int* indexes = find_indexes_from_location(x, y);

	SDL_Log("%d %d", indexes[0], indexes[1]);
	
	if (indexes == NULL) {
		return;
	}
	update_selected_index(indexes);
}




void update_selected_index(int* indexes) {
	int distance_to_selected_index;
	if (indexes_of_clicked_cell[0] == -1 && indexes_of_clicked_cell[1] == -1) {
		indexes_of_clicked_cell[0] = indexes[0];
		indexes_of_clicked_cell[1] = indexes[1];
	}
	else {
		distance_to_selected_index = abs(indexes_of_clicked_cell[0] - indexes[0]) + abs(indexes_of_clicked_cell[1] - indexes[1]);

		if (distance_to_selected_index == 1) {
			swap_gems(indexes_of_clicked_cell[0], indexes_of_clicked_cell[1], indexes[0], indexes[1]);
			indexes_of_clicked_cell[0] = -1;
			indexes_of_clicked_cell[1] = -1;
		}
		else if (distance_to_selected_index == 0) {
			indexes_of_clicked_cell[0] = -1;
			indexes_of_clicked_cell[1] = -1;
		}
		else {
			indexes_of_clicked_cell[0] = indexes[0];
			indexes_of_clicked_cell[1] = indexes[1];
		}
	}
}




void swap_gems(int x1, int y1, int x2, int y2)
{
	EColor temp_color = gems[x1][y1].color;

	set_color(gems[x1][y1], gems[x2][y2].color);
	set_color(gems[x2][y2], temp_color);
	bool check1 = !check_around_the_gem(x1, y1);
	bool check2 = !check_around_the_gem(x2, y2);
	if (check1 && check2) {
		EColor temp_color = gems[x1][y1].color;

		set_color(gems[x1][y1], gems[x2][y2].color);
		set_color(gems[x2][y2], temp_color);
	}
}




int* find_indexes_from_location(float x, float y) {

	if (x < 38 || y < 38 || x > 660 || y > 660) {
		return NULL;
	}

	int indexes[2] = { ((int)x - 38) / 48, ((int)y - 38) / 48 };
	return indexes;
}




bool check_init_match_at(int x, int y, EColor color)
{
	if (x > 1) {
		if (gems[x - 1][y].color == color && gems[x - 2][y].color == color) {
			return true;
		}
			
	}

	if (y > 1) {
		if (gems[x][y - 1].color == color && gems[x][y - 2].color == color) {
			return true;
		}	
	}

	return false;
}




bool check_around_the_gem(int x, int y)
{
	if (x >= CELL_COUNT || x < 0 || y >= CELL_COUNT || y < 0) {
		return false;
	}

	if (!(gems[x][y].active)) {
		return false;
	}

	EColor clicked_gem_color = gems[x][y].color;

	std::vector<int> x_indexes = check_x_axis(clicked_gem_color, x, y);
	std::vector<int> y_indexes = check_y_axis(clicked_gem_color, x,y);
	
	std::vector<int> effected_columns;

	if (x_indexes.size() >= 3) {
		for (int index_x : x_indexes) {
			gems[index_x][y].active = false;
			effected_columns.push_back(index_x);
		}
	}
	
	if (y_indexes.size() >= 3) {
		
		for (int index_y : y_indexes) {
			gems[x][index_y].active = false;
		}
		
		if(effected_columns.size() == 0){
			effected_columns.push_back(x);
		}
	}


	if (effected_columns.size() != 0) {
		move_gems(effected_columns);
		return true;
	}
	return false;
}




std::vector<int> check_x_axis(EColor clicked_gem_color, int x, int y)
{
	std::vector<int> x_indexes(1, x);

	int index = x - 1;
	int decrement = true;
	while (1) {
		if (!(gems[index][y].active) || index < 0 || index >= CELL_COUNT) {
			if (!decrement) { break; }
			if (!(gems[x + 1][y].active)) { break; }
			decrement = false;
			index = x + 1;
		}

		if (decrement) {
			if (gems[index][y].color == clicked_gem_color) {
				x_indexes.push_back(index);
				index--;
			}
			else {
				decrement = false;
				index = x + 1;
			}
		}
		else {
			if (gems[index][y].color == clicked_gem_color) {
				x_indexes.push_back(index);
				index++;
			}
			else {
				break;
			}
		}
	}
	return x_indexes;
}




std::vector<int> check_y_axis(EColor clicked_gem_color, int x, int y)
{
	std::vector<int> y_indexes(1, y);

	int index = y + 1;
	bool decrement = false;
	while (1) {

		if (!gems[x][index].active || index < 0 || index >= CELL_COUNT) {
			if (decrement) { break; }
			if (!gems[x][y - 1].active) { break; }
			decrement = true;
			index = y - 1;
		}

		if (decrement) {
			if (gems[x][index].color == clicked_gem_color) {
				y_indexes.push_back(index);
				index--;
			}
			else {
				break;
			}
		}
		else {
			if (gems[x][index].color == clicked_gem_color) {
				y_indexes.push_back(index);
				index++;
			}
			else {
				decrement = true;
				index = y - 1;
			}
		}
	}

	return y_indexes;
}




void move_gems(std::vector<int> effected_columns) {
	for (int i : effected_columns) {
		int last_empty_block_y = -1;
		std::vector<std::vector<int>> non_effected_gems_coordinates;
		for (int j = CELL_COUNT-1; j >= 0; j--) {
			if (!gems[i][j].active && last_empty_block_y == -1) {
				last_empty_block_y = j;
			}
			else if(gems[i][j].active && last_empty_block_y != -1) {
				non_effected_gems_coordinates.push_back({i,j});
			}
		}
		std::reverse(non_effected_gems_coordinates.begin(), non_effected_gems_coordinates.end());

		while (non_effected_gems_coordinates.size() > 0) {
			
			int moving_gem_x = non_effected_gems_coordinates.back()[0];
			int moving_gem_y = non_effected_gems_coordinates.back()[1];

			gems[i][last_empty_block_y] = gems[moving_gem_x][moving_gem_y];
			gems[moving_gem_x][moving_gem_y].active = false;

			gems[i][last_empty_block_y].target_location[0] = 38.0f + i * 48;
			gems[i][last_empty_block_y].target_location[1] = 38.0f + last_empty_block_y * 48;
			
			non_effected_gems_coordinates.pop_back();
			last_empty_block_y--;
		}
	}

	add_gems(effected_columns);
}




void add_gems(std::vector<int> columns_to_add)
{
	for (int i : columns_to_add) {

		int bottom_gem_index_y = -1;
		for (int j = CELL_COUNT - 1; j >= 0; j--) {
			if (bottom_gem_index_y == -1 && gems[i][j].active == false) {
				bottom_gem_index_y = j;
				break;
			}
		}

		for (int j = 0; j < CELL_COUNT; j++) {
			if (gems[i][j].active == true) {
				break;
			}
			gems[i][j] = Gem();
			set_color(gems[i][j], static_cast<EColor>(rand() % 4));

			gems[i][j].location[0] = 38.0f + i * 48;
			gems[i][j].location[1] = (float)(bottom_gem_index_y + 1 - j) * 48 * -1;

			gems[i][j].target_location[0] = 38.0f + i * 48;
			gems[i][j].target_location[1] = 38.0f + j * 48;

		}
	}

	input_active = false;
	queued_columns = columns_to_add; 

	SDL_AddTimer(1000, timer_callback_check_new_gems_match, NULL);
}




Uint32 timer_callback_check_new_gems_match(void* userdata, SDL_TimerID timerID, Uint32 interval) {

	for (int i : queued_columns) {
		for (int j = 0; j < CELL_COUNT; j++) {
			check_around_the_gem(i, j);
		}
	}
	input_active = true;
	return 0;
}
	




