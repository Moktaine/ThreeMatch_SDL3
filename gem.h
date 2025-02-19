#pragma once
#include <SDL3/SDL.h>
#include <stdlib.h>
class gem
{
private:
	float location[2] = { 50.0f, 50.0f };
public:

	enum EColor {
		EColor_Red,
		EColor_Green,
		EColor_Blue,
		EColor_Pink
	};


	EColor color = static_cast<EColor>(rand()%4);

	SDL_FRect source_rect { 0,0,16,16 };

	gem();
	void set_location(float x, float y);
	float* get_location();
	EColor get_color();
};

