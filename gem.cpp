#include "gem.h"



gem::gem()
{
	switch (color) {
	case EColor_Red:
		break;
	case EColor_Blue:
		source_rect.y = 32;
		break;
	case EColor_Pink:
		source_rect.y = 64;
		break;
	case EColor_Green:
		source_rect.y = 96;
		break;
	}

}

void gem::set_location(float x, float y)
{
	location[0] = x;
	location[1] = y;
}

float* gem::get_location()
{
	return location;
}

gem::EColor gem::get_color()
{
	return color;
}