#include "Player.h"
namespace gps
{
	Player::Player()
	{

	}
	Player::Player(glm::vec3 pos, glm::vec3 rot)
	{
		this->position = pos;
		this->rotation = rot;
	}
}
