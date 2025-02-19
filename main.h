#pragma once
#include <vector>


void handle_mouse_button(SDL_Event* event);
int* find_indexes_from_location(float x, float y);
void check_around_the_gem(int x, int y);
