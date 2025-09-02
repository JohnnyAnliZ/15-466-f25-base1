#include "PPU466.hpp"
#include "Mode.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>

struct PlayMode : Mode {
	PlayMode();
	virtual ~PlayMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	//----- game state -----

	//input tracking:
	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} left, right, down, up;

	//some weird background animation:
	float background_fade = 0.0f;



	//-------ANIMATION-----------


	//crabman facing
	bool facingLeft = false;

	//used for frame-wise animation
	uint32_t frame = 0;
	bool standing = true;
	bool justIdle = true;

	//--------MECHANICS-----------
	//Falling
	bool falling = false;

	//player position:
	glm::vec2 player_at = glm::vec2(8.0f,70.0f);
	float clawY = 0.0f;

	//checkFalling
	bool playerInAir();
	void snapGround();



	//Hooking
	bool hooking = false;
	bool hooked = false;
	//climbed to the hook
	bool top = false;
	//check clawReaching;
	bool clawTouched();
	void snapClaw();

	bool headTouched();
	void snapCeiling();

	float hookingTime;
	//----- drawing handled by PPU466 -----

	PPU466 ppu;
};
