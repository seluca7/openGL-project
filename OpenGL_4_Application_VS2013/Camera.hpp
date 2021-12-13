//
//  Camera.hpp
//
//  Created by CGIS on 28/10/2016.
//  Copyright © 2016 CGIS. All rights reserved.
//

#ifndef Camera_hpp
#define Camera_hpp

#include <stdio.h>
#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"
#include "GLEW\glew.h"
#include "Player.h"

namespace gps {
    
	enum camera_Movement{
		FORWARD,
		BACKWARD,
		LEFT,
		RIGHT
	};
	const GLfloat SPEED = 1.5f;
	const GLfloat YAW = -90.0f;
	const GLfloat PITCH = 0.0f;
	const GLfloat ZOOM = 45.0f;
	const GLfloat SENSITIVITY = 0.3f;

	class Camera
	{
	public:
		GLfloat movementSpeed;

		glm::vec3 Position;
		glm::vec3 Front;
		glm::vec3 Up;
		glm::vec3 Right;
		glm::vec3 WorldUp;

		//angle for the mouse
		GLfloat yaw;
		GLfloat pitch;

		GLfloat mouseSensitivity;
		GLfloat zoom;
		Player player;


		Camera::Camera(glm::vec3 pos, glm::vec3 front, glm::vec3 up, glm::vec3 right, glm::vec3 worldUp);
		glm::mat4 getViewMatrix();

		void initialize();
		Camera::Camera();
		Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), GLfloat yaw = YAW, GLfloat pitch = PITCH) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), movementSpeed(SPEED), mouseSensitivity(SENSITIVITY), zoom(ZOOM)
		{
			this->Position = position;
			this->WorldUp = up;
			this->yaw = yaw;
			this->pitch = pitch;
			this->updateCameraVectors();
		}
		// Constructor with scalar values
		Camera(GLfloat posX, GLfloat posY, GLfloat posZ, GLfloat upX, GLfloat upY, GLfloat upZ, GLfloat yaw, GLfloat pitch) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), movementSpeed(SPEED), mouseSensitivity(SENSITIVITY), zoom(ZOOM)
		{
			this->Position = glm::vec3(posX, posY, posZ);
			this->WorldUp = glm::vec3(upX, upY, upZ);
			this->yaw = yaw;
			this->pitch = pitch;
			this->updateCameraVectors();
		}

		void processKeyboard(camera_Movement direction, GLfloat deltaTime);
		void processMouseMovement(GLfloat xoffset, GLfloat yoffset, GLboolean constrainPitch);
	private:
		void updateCameraVectors();
	};
    
}

#endif /* Camera_hpp */
