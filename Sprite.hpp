#pragma once



#include "PPU466.hpp"
#include <glm/glm.hpp>
#include <unordered_map>
#include <string>





struct Sprite {
	PPU466::Tile tile;
	PPU466::Palette palette;
	void draw(int8_t x, int8_t y) const;
};



struct Sprites {

	static std::unordered_map<std::string, Sprite> SpritesDic;

	Sprite const &lookup(std::string const &name);

	static Sprite const *load(std::string const& filename);

};