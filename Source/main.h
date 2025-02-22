#pragma once
#include <vector>

	
float location[2] = { 0.0f, 0.0f };
float target_location[2] = { 0.0f, 0.0f };


enum EColor {
	EColor_Red,
	EColor_Green,
	EColor_Blue,
	EColor_Pink
};

struct Gem {
	bool active = true;
	float location[2] = { 0.0f, 0.0f };
	float target_location[2] = { 0.0f, 0.0f };
	EColor color { EColor_Blue };
	SDL_FRect source_rect{ 0,0,16,16 };
};



void init_spawn_gems();

void render_gem(Gem* gem);
void update_gem_location(Gem* gem);

void handle_mouse_button(SDL_Event* event);
void update_selected_index(int* indexes);

void set_color(Gem& gem, EColor target_color);

int* find_indexes_from_location(float x, float y);

bool check_init_match_at(int x, int y, EColor color);

bool check_around_the_gem(int x, int y);
std::vector<int> check_x_axis(EColor clicked_gem_color, int x, int y);
std::vector<int> check_y_axis(EColor clicked_gem_color, int x, int y);

void move_gems(std::vector<int> effected_columns);
void add_gems(std::vector<int> columns_to_add);
void swap_gems(int x1, int y1, int x2, int y2);
Uint32 timer_callback_check_new_gems_match(void* userdata, SDL_TimerID timerID, Uint32 interval);
