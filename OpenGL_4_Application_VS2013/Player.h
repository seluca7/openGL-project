#ifndef Player_hpp
#define Player_hpp

#include <stdio.h>
#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"
#include "GLEW\glew.h"

namespace gps
{
	class Player
	{
		glm::vec3 position;
		glm::vec3 rotation;
	public:
		Player(glm::vec3 position, glm::vec3 rotation);
		Player();

	private:

	};


}
#endif