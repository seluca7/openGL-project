//
//  Camera.cpp
//  Lab5
//
//  Created by CGIS on 28/10/2016.
//  Copyright © 2016 CGIS. All rights reserved.
//

#include "Camera.hpp"

namespace gps {
    
	Camera::Camera(glm::vec3 pos, glm::vec3 front, glm::vec3 up, glm::vec3 right, glm::vec3 worldUp)
	{
		this->Front = front;
		this->Position = pos;
		this->Up = up;
		this->WorldUp = worldUp;
		this->Right = right;
	}
	glm::mat4 Camera::getViewMatrix()
	{
		return glm::lookAt(this->Position, this->Front + this->Position, this->Up);
	}

	void Camera::initialize()
	{
		this->Position = glm::vec3(0.0f, 0.0f, 3.0f);
		this->Up = glm::vec3(0.0f, 1.0f, 0.0f);
		this->Front = glm::vec3(0.0f, 0.0f, -1.0f);
		this->Right = normalize(cross(this->Front, this->Up));
		this->WorldUp = this->Up;
	}

	Camera::Camera()
	{
		initialize();
		this->movementSpeed = SPEED;
		this->mouseSensitivity = SENSITIVITY;
		this->yaw = YAW;
		this->pitch = PITCH;
	}

	void Camera::processKeyboard(camera_Movement direction, GLfloat deltaTime)
	{
		GLfloat velocity = this->movementSpeed * deltaTime;
		switch (direction)
		{
		case FORWARD: this->Position += this->Front * velocity;
			break;
		case BACKWARD: this->Position -= this->Front * velocity;
			break;
		case LEFT:this->Position -= this->Right*velocity;
			break;
		case RIGHT:this->Position += this->Right*velocity;
			break;
		default:
			break;
		}
	}

	void Camera::processMouseMovement(GLfloat xoffset, GLfloat yoffset, GLboolean constrainPitch = true)
	{
		xoffset *= this->mouseSensitivity;
		yoffset *= this->mouseSensitivity;

		this->yaw += xoffset;
		this->pitch += yoffset;

		if (constrainPitch)
		{
			if (this->pitch > 89.0)
				this->pitch = 89.0;
			else if (this->pitch < -89.0)
				this->pitch = -89.0;
		}
		this->updateCameraVectors();
	}

	void Camera::updateCameraVectors()
	{
		glm::vec3 front;

		front.x = cos(glm::radians(this->yaw))* cos(glm::radians(this->pitch));
		front.y = sin(glm::radians(this->pitch));
		front.z = sin(glm::radians(this->yaw))* cos(glm::radians(this->pitch));

		this->Front = glm::normalize(front);
		this->Right = glm::normalize(glm::cross(this->Front, this->WorldUp));
		this->Up = glm::normalize(glm::cross(this->Right, this->Front));
	}
    
}
