	#include "PlayMode.hpp"

//for the GL_ERRORS() macro:
#include "gl_errors.hpp"

//for glm::value_ptr() :
#include <glm/gtc/type_ptr.hpp>

#include "data_path.hpp"

#include "Sprite.hpp"

#include"Load.hpp"

#include<string>

#include <random>

#include <array>



//Load Objects for crabman's body parts
Sprite const* loadedFace = nullptr;
Sprite const* loadedLegs = nullptr;
Sprite const* loadedLegsWalking = nullptr;
Sprite const* loadedClaw = nullptr;
Sprite const* loadedClimbable = nullptr;
Sprite const* loadedBG = nullptr;

//climbing
Sprite const* loadedArmBase = nullptr;
Sprite const* loadedArmExtension = nullptr;
Sprite const* loadedArmEnd = nullptr;



Load<Sprite> crabmanFacePng(LoadTagEarly, []() {
	loadedFace = Sprites::load("crabmanFace.png");
	return loadedFace;
	});

Load<Sprite> crabmanLegsPng(LoadTagEarly, []() {
	loadedLegs = Sprites::load("crabmanLegs.png");
	return loadedLegs;
	});

Load<Sprite> crabmanLegsWakingPng(LoadTagEarly, []() {
	loadedLegsWalking = Sprites::load("crabmanLegsWalk.png");
	return loadedLegsWalking;
	});

Load<Sprite> crabmanClawPng(LoadTagEarly, []() {
	loadedClaw = Sprites::load("crabmanClaw.png");
	return loadedClaw;
	});

Load<Sprite> climbablePng(LoadTagEarly, []() {
	loadedClimbable = Sprites::load("climbable.png");
	return loadedClimbable;
	});

Load<Sprite> BGPng(LoadTagEarly, []() {
	loadedBG = Sprites::load("background.png");
	return loadedBG;
	});

Load<Sprite> BasePng(LoadTagEarly, []() {
	loadedArmBase = Sprites::load("crabmanArmBase.png");
	return loadedArmBase;
	});

Load<Sprite> ExtensionPng(LoadTagEarly, []() {
	loadedArmExtension = Sprites::load("crabmanArmExtension.png");
	return loadedArmExtension;
	});

Load<Sprite> EndPng(LoadTagEarly, []() {
	loadedArmEnd = Sprites::load("crabmanArmEnd.png");
	return loadedArmEnd;
	});





//utility for making the sprites mirror
uint8_t reverse_uint8(uint8_t bits) {
	uint8_t reversed_bits = 0;
	for (uint32_t i = 0; i < 8; ++i) {
		if ((bits >> i) & 0b1) { 
			reversed_bits |= (1 << (7 - i));
		}
	}
	return reversed_bits;
}

PPU466::Tile &mirrorTileY(PPU466::Tile &tile) {
	for (uint32_t j = 0; j <= 7; ++j) {
		tile.bit0[j] = reverse_uint8(tile.bit0[j]);
		tile.bit1[j] = reverse_uint8(tile.bit1[j]);
	}
	return tile;
}

//this creates a new array
std::array<uint8_t,8> reverseArray(std::array<uint8_t,8> arr) {

	std::array<uint8_t, 8> ret;
	for (uint32_t k = 0; k < 8; ++k) {
		ret[7 - k] = arr[k];
	}
	return ret;
}

PPU466::Tile& mirrorTileX(PPU466::Tile& tile) {
	tile.bit0 = reverseArray(tile.bit0);
	tile.bit1 = reverseArray(tile.bit1);
	return tile;
}


bool PlayMode::playerInAir() {
	//x,y are tile coordinates
	int8_t x = int8_t(std::floor(player_at.x/8.0f));
	int8_t y = int8_t(std::floor((player_at.y-9) / 8.0f));
	if (player_at.y <= 9.0f)return false;
	//std::cout << (int32_t) y<< std::endl;
	return (ppu.background[(y)*ppu.BackgroundWidth+x]==0);
}


void PlayMode::snapGround() {
	player_at.y = std::floor(player_at.y);
	return;
}

bool PlayMode::clawTouched() {
	//x,y tile coordinates for the claw
	int8_t x = int8_t(std::floor((player_at.x + 8.0f)/8.0f));
	int8_t y = int8_t(std::floor(clawY/8.0f));
	if (y > 30)return true;
	return (ppu.background[(y+1)*ppu.BackgroundWidth + x] == 0b0000000100000001);
}

void PlayMode::snapClaw() {
	clawY = std::floor(clawY/8.0f) * 8.0f;
	return;
}

bool PlayMode::headTouched() {
	//x,y are tile coordinates
	int8_t x = int8_t(std::floor((player_at.x+4.0f) / 8.0f));
	int8_t y = int8_t(std::floor((player_at.y + 8) / 8.0f));
	if (y >= 60)return true;
	//std::cout << (int32_t)y << std::endl;
	return (ppu.background[(y)*ppu.BackgroundWidth + x] == 0b0000000100000001);
}


void PlayMode::snapCeiling() {
	player_at.y = std::floor(player_at.y);
	return;
}






PlayMode::PlayMode() {
	//TODO:
	// you *must* use an asset pipeline of some sort to generate tiles.
	// don't hardcode them like this!
	// or, at least, if you do hardcode them like this,
	//  make yourself a script that spits out the code that you paste in here
	//   and check that script into your repository.

	//Also, *don't* use these tiles in your game:

	{ //use tiles 0-16 as some weird dot pattern thing:
		std::array< uint8_t, 8*8 > distance;
		for (uint32_t y = 0; y < 8; ++y) {
			for (uint32_t x = 0; x < 8; ++x) {
				float d = glm::length(glm::vec2((x + 0.5f) - 4.0f, (y + 0.5f) - 4.0f));
				d /= glm::length(glm::vec2(4.0f, 4.0f));
				distance[x+8*y] = uint8_t(std::max(0,std::min(255,int32_t( 255.0f * d ))));
			}
		}
		for (uint32_t index = 0; index < 16; ++index) {
			PPU466::Tile tile;
			uint8_t t = uint8_t((255 * index) / 16);
			for (uint32_t y = 0; y < 8; ++y) {
				uint8_t bit0 = 0;
				uint8_t bit1 = 0;
				for (uint32_t x = 0; x < 8; ++x) {
					uint8_t d = distance[x+8*y];
					if (d > t) {
						bit0 |= (1 << x);
					} else {
						bit1 |= (1 << x);
					}
				}
				tile.bit0[y] = bit0;
				tile.bit1[y] = bit1;
			}
			ppu.tile_table[index] = tile;
		}
	}

	



	//use tiles 32,31,(30,29) as crabman's face,legs,claws:
	ppu.tile_table[32].bit0 = loadedFace->tile.bit0;
	ppu.tile_table[32].bit1 = loadedFace->tile.bit1;

	ppu.tile_table[31].bit0 = loadedLegs->tile.bit0;
	ppu.tile_table[31].bit1 = loadedLegs->tile.bit1;

	//leftClaw (from perspective of crab)
	ppu.tile_table[30].bit0 = loadedClaw->tile.bit0;
	ppu.tile_table[30].bit1 = loadedClaw->tile.bit1;
	ppu.tile_table[30] = mirrorTileY(ppu.tile_table[30]);

	//rightClaw
	ppu.tile_table[29].bit0 = reverseArray(loadedClaw->tile.bit0);
	ppu.tile_table[29].bit1 = reverseArray(loadedClaw->tile.bit1);

	//ArmBase
	ppu.tile_table[28].bit0 = loadedArmBase->tile.bit0;
	ppu.tile_table[28].bit1 = loadedArmBase->tile.bit1;
	ppu.tile_table[28] = mirrorTileY(ppu.tile_table[28]);
	//ArmExtension
	ppu.tile_table[27].bit0 = loadedArmExtension->tile.bit0;
	ppu.tile_table[27].bit1 = loadedArmExtension->tile.bit1;
	ppu.tile_table[27] = mirrorTileY(ppu.tile_table[27]);
	//ArmEnd
	ppu.tile_table[26].bit0 = loadedArmEnd->tile.bit0;
	ppu.tile_table[26].bit1 = loadedArmEnd->tile.bit1;
	ppu.tile_table[26] = mirrorTileY(ppu.tile_table[26]);


	//use tile 0 for background,tile 1 for climbable
	ppu.tile_table[0].bit0 = loadedBG->tile.bit0;
	ppu.tile_table[0].bit1 = loadedBG->tile.bit1;

	ppu.tile_table[1].bit0 = loadedClimbable->tile.bit0;
	ppu.tile_table[1].bit1 = loadedClimbable->tile.bit1;
	


	//used for the player:
	ppu.palette_table[7] = loadedFace->palette;
	ppu.palette_table[6] = loadedLegs->palette;
	ppu.palette_table[5] = loadedClimbable->palette;
	ppu.palette_table[0] = loadedBG->palette;
	ppu.palette_table[1] = loadedClimbable->palette;

}

PlayMode::~PlayMode() {
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

	if (evt.type == SDL_EVENT_KEY_DOWN) {
		if (evt.key.key == SDLK_LEFT) {

			//change crabman facing
			if (!facingLeft) { 
				facingLeft = true; 
				ppu.tile_table[32] = mirrorTileY(ppu.tile_table[32]);
				ppu.tile_table[31] = mirrorTileY(ppu.tile_table[31]);
			}

			left.downs += 1;
			left.pressed = true;
			return true;
		} else if (evt.key.key == SDLK_RIGHT) {

			//change crabman facing
			if (facingLeft) { 
				facingLeft = false; 
				ppu.tile_table[32] = mirrorTileY(ppu.tile_table[32]);
				ppu.tile_table[31] = mirrorTileY(ppu.tile_table[31]);
			}

			right.downs += 1;
			right.pressed = true;
			return true;
		} else if (evt.key.key == SDLK_UP) {
			up.downs += 1;
			up.pressed = true;
			return true;
		} else if (evt.key.key == SDLK_DOWN) {
			down.downs += 1;
			down.pressed = true;
			return true;
		}
	} else if (evt.type == SDL_EVENT_KEY_UP) {
		if (evt.key.key == SDLK_LEFT) {
			left.pressed = false;
			return true;
		} else if (evt.key.key == SDLK_RIGHT) {
			right.pressed = false;
			return true;
		} else if (evt.key.key == SDLK_UP) {
			up.pressed = false;
			return true;
		} else if (evt.key.key == SDLK_DOWN) {
			down.pressed = false;
			return true;
		}
	}

	return false;
}

void PlayMode::update(float elapsed) {


	if (hooking)hookingTime += elapsed;
	else hookingTime = 0;



	frame += 1;
	//--------Falling------------
	//if player is in air and falling is false, set it to true
	if (!falling && playerInAir() && !hooked) {
		// std::cout << "falling is true" << std::endl;
		falling = true;
	}

	//check if crabman hits the ground
	if (!hooked && falling&& !playerInAir() ){
		falling = false;
		snapGround();
	}

	constexpr float fallingSpeed = 10.0f;
	if (falling && frame%10) {
		player_at.y -= fallingSpeed * elapsed;
	}



	//-------------hOOOKING-----------
	//hooking mechanic

	//only release arm when it's neither hooking or hooked
	if (!hooking && !hooked && up.pressed) {
		clawY = player_at.y + 7;
		hooking = true;
	}

	//use hookingTime to move claw up a grid every so often
	if (hooking && (hookingTime > 0.5f)) {
		hookingTime = 0;
		clawY += 8.0f;
	}

	//snap and swich to hooked after touching
	if (hooking && clawTouched()) {
		snapClaw();
		hooking = false;
		hooked = true;
	}
	

	constexpr float climbingSpeed = 100.0f;
	if (hooked && !top && up.pressed) {
		//std::cout << "falling:" <<falling<< std::endl;
		player_at.y += climbingSpeed * elapsed;
	}

	if (hooked && down.pressed) {
		hooked = false;
		top = false;
	}
	
	if (hooking && down.pressed) {
		hooking = false;
	}

	if (hooked && headTouched()) {
		snapCeiling();
		top = true;
	}

	//when on the ceiling and the crab walks off
	if (hooked && top && !headTouched()) {
		top = false;
		hooked = false;
	}

	//------------CrabWALKING-----------
	constexpr float PlayerSpeed = 30.0f;
	//can't crabWalk when hooking or (hooked and not at the top yet)
	if (!hooking && !(hooked && !top) && left.pressed) player_at.x -= PlayerSpeed * elapsed;
	if (!hooking && !(hooked && !top) && right.pressed) player_at.x += PlayerSpeed * elapsed;


	//animate leges
	if (left.pressed ^ right.pressed) {
		justIdle = true;
		if (frame%5) {
			standing = !standing;
		}
		if (!standing) {
			ppu.tile_table[31].bit0 = loadedLegsWalking->tile.bit0;
			ppu.tile_table[31].bit1 = loadedLegsWalking->tile.bit1;
		}
		else {
			ppu.tile_table[31].bit0 = loadedLegs->tile.bit0;
			ppu.tile_table[31].bit1 = loadedLegs->tile.bit1;
		}

		if(left.pressed)ppu.tile_table[31] = mirrorTileY(ppu.tile_table[31]);
	}

	//reset when idle
	if (!(left.pressed | right.pressed)) {
		standing = true;
		if (justIdle) {
			ppu.tile_table[31].bit0 = loadedLegs->tile.bit0;
			ppu.tile_table[31].bit1 = loadedLegs->tile.bit1;
		}
		if (facingLeft && justIdle) {
			
			//std::cout << "flip it" << std::endl;
			justIdle = false;
			ppu.tile_table[31] = mirrorTileY(ppu.tile_table[31]);
		}
	}

	//reset button press counters:
	left.downs = 0;
	right.downs = 0;
	up.downs = 0;
	down.downs = 0;
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	//--- set ppu state based on game state ---


	ppu.background_color = glm::u8vec4(0x1f,	0x79,0xec,0xff);

	//make a tile map for my tiles
	//every tile is represented by a 8-bit int, bit0 means it's a terrain tile
	uint8_t tileMap[ppu.BackgroundHeight][ppu.BackgroundWidth];

	//clear the background
	for (uint32_t i = 0; i < ppu.BackgroundWidth * ppu.BackgroundHeight;++i) {
		ppu.background[i] = 0;
	}

	//make the level
	for (uint32_t y = 5; y < 60; y+=10) {
		for (uint32_t x = 0; x < 20; ++x) {
			tileMap[y][x] = 1;
			ppu.background[y * ppu.BackgroundWidth + x] = 0b0000000100000001;
		}
	}
	for (uint32_t yy = 10; yy < 60; yy += 10) {
		for (uint32_t xx = 32; xx >13; --xx) {
			tileMap[yy][xx] = 1;
			ppu.background[yy * ppu.BackgroundWidth + xx] = 0b0000000100000001;
		}
	}




	//background scroll:
	/*ppu.background_position.x = int32_t(-0.5f * player_at.x);
	ppu.background_position.y = int32_t(-0.5f * player_at.y);*/

	//player sprite face and legs:
	ppu.sprites[0].x = int8_t(player_at.x);
	ppu.sprites[0].y = int8_t(player_at.y);
	ppu.sprites[0].index = 32;
	ppu.sprites[0].attributes = 7;
	
	//except for face, crabman's other features use palette (they should all be able to use 7)
	ppu.sprites[1].x = int8_t(player_at.x);
	ppu.sprites[1].y = int8_t(player_at.y)-8;
	ppu.sprites[1].index = 31;
	ppu.sprites[1].attributes = 6;

	//hooking claw
	ppu.sprites[2].x = int8_t(player_at.x)+8;
	ppu.sprites[2].y = (hooking || hooked) ? int8_t(clawY) : (int8_t(player_at.y)-1);
	ppu.sprites[2].index = hooking ? 26 : 30;
	ppu.sprites[2].attributes = 6;

	//other claw
	ppu.sprites[3].x = int8_t(player_at.x) -8;
	ppu.sprites[3].y = int8_t(player_at.y) - 8;
	ppu.sprites[3].index = 29;
	ppu.sprites[3].attributes = 6;

	//hooking claw base
	if (hooking) {
		ppu.sprites[4].x = int8_t(player_at.x) + 8;
		ppu.sprites[4].y = int8_t(player_at.y) - 1;
		ppu.sprites[4].index = 28;
		ppu.sprites[4].attributes = 6;
	}
	else ppu.sprites[4].y = 240;




	//--- actually draw ---
	ppu.draw(drawable_size);
}
