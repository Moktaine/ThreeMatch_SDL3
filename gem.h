#pragma once
#include <SDL3/SDL.h>
#include <stdlib.h>
class gem
{
private:
	float location[2] = { 0.0f, 0.0f };
	float target_location[2] = { 0.0f, 0.0f };
public:

	enum EColor {
		EColor_Red,
		EColor_Green,
		EColor_Blue,
		EColor_Pink
	};


	EColor color = static_cast<gem::EColor>(rand() % 4);

	SDL_FRect source_rect{ 0,0,16,16 };

	gem();
	void set_target_location(float x, float y);
	void set_location(float x, float y);
	void set_color(EColor target_color);
	float* get_target_location();
	float* get_location();
	EColor get_color();
};

