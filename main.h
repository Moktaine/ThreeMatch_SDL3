#pragma once
#include <vector>
#include "gem.h"


void handle_mouse_button(SDL_Event* event);
int* find_indexes_from_location(float x, float y);
bool check_init_match_at(int x, int y, gem::EColor color);
void check_around_the_gem(int x, int y);
void move_gems(std::vector<int> effected_columns);
void add_gems(std::vector<int> columns_to_add);
