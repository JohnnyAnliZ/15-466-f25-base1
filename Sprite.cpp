#include "PPU466.hpp"
#include "load_save_png.hpp"
#include "Sprite.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <unordered_map>
#include <stdint.h>
#include <iostream>
#include <string>



std::unordered_map<std::string, Sprite> Sprites::SpritesDic = {};

Sprite const &Sprites::lookup(std::string const &name) {
	if (SpritesDic.find(name) != SpritesDic.end())return SpritesDic[name];
	std::cerr << "Sprite not found in the spritesDic" << std::endl;
	return SpritesDic[name];
}

	
//This load function loads png that's 8x8 pixels and has only four colors, including one transparent
Sprite const* Sprites::load(std::string const& filename) {

	uint32_t spriteWidth = 8;
	uint32_t spriteHeight = 8;
	uint32_t size = spriteHeight * spriteWidth;
	//this stores the RGBAs of the raw file
	std::vector <glm::u8vec4> data(size);


	PPU466::Tile spriteTile = {};
	PPU466::Palette spritePalette = {};

	//Load the png
	glm::uvec2 sizeVec(8, 8);
	glm::uvec2* sizeVec_ptr = &sizeVec;
	load_png(filename, sizeVec_ptr, &data, LowerLeftOrigin);

	//going through the RGBAs to assign color indices
	std::unordered_map <uint32_t,uint8_t> colors;




	uint8_t currentColorIndex = 1;
	for (uint32_t y = 0; y < spriteHeight; ++y) {
		uint8_t currentRowBit0 = 0;
		uint8_t currentRowBit1 = 0;
		for(uint32_t x = 0; x < spriteWidth; ++x) {
			currentRowBit0 <<= 1;
			currentRowBit1 <<= 1;
			//convert to uint32_t for unordered_map insertion
			uint32_t currRGBA =0;
			currRGBA |= static_cast<uint32_t>(data[y * spriteWidth + x].x) << 24;
			currRGBA |= static_cast<uint32_t>(data[y * spriteWidth + x].y) << 16;
			currRGBA |= static_cast<uint32_t>(data[y * spriteWidth + x].z) << 8;
			currRGBA |= static_cast<uint32_t>(data[y * spriteWidth + x].w);

			//check for transparancy
			if (currRGBA != 0xffffffff) {
				auto it = colors.find(currRGBA);

				//add color to palette if the color is new
				if (it == colors.end()) {
					std::cout<<"current color" << std::hex << currRGBA << std::endl;
					colors.insert({ currRGBA,currentColorIndex });
					spritePalette[currentColorIndex] = data[y * spriteWidth + x];
					//update each of the Tiles' color index
					currentRowBit0 |= currentColorIndex & 0b1;
					currentRowBit1 |= (currentColorIndex>>1) & 0b1;
					//update colorIndex
					currentColorIndex++;
					if (currentColorIndex >= 5) std::cerr << "Too many colors in the PNG" << std::endl;
				}


				//for existing colors, access it's corresponding palette index 
				//update each of the Tiles' color index
				else {
					currentRowBit0 |= it->second & 0b1;
					currentRowBit1 |= (it->second>>1) & 0b1;
				}
			}

		}
		spriteTile.bit0[y] = currentRowBit0;
		spriteTile.bit1[y] = currentRowBit1;
	}

	
	Sprite* ret = new Sprite;
	ret->tile = spriteTile;
	ret->palette = spritePalette;


	Sprites::SpritesDic.emplace(filename,*ret);

	return ret;
}


