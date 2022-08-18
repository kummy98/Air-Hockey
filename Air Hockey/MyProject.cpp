// This has been adapted from the Vulkan tutorial

#include "MyProject.hpp"


// The uniform buffer object used in this example
struct globalUniformBufferObject {
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
};

struct UniformBufferObject {
	alignas(16) glm::mat4 model;
};


// MAIN ! 
class MyProject : public BaseProject {
protected:
	// Here you list all the Vulkan objects you need:
    
	// Descriptor Layouts [what will be passed to the shaders]
	DescriptorSetLayout DSLglobal;
	DescriptorSetLayout DSLobj;

	// Pipelines [Shader couples]
	Pipeline P1;

	// Models, textures and Descriptors (values assigned to the uniforms)
	//Table
	Model M_table;
	Texture T_table;
	DescriptorSet DS_table;
    
    //Scores
    Model M_score1;
    Texture T_score1;
    DescriptorSet DS_score1;
    
    Model M_score2;
    Texture T_score2;
    DescriptorSet DS_score2;

	//Paddle 1 and 2
	Model M_paddle;
	Texture T_paddle1;
	Texture T_paddle2;
	DescriptorSet DS_paddle1;
	DescriptorSet DS_paddle2;
	float radiusPaddle = 0.07f;
    glm::vec3 initialPlayer1Pos = glm::vec3(0.57f, 0, 0);
    glm::vec3 initialPlayer2Pos = glm::vec3(-0.57f, 0, 0);

	//Disk
	Model M_disk;
	Texture T_disk;
	DescriptorSet DS_disk;
    float radiusDisk = 0.04f;

	DescriptorSet DS_global;

	//Collision maps

	//For the 2 paddles
	int collisionMapWidth, collisionMapHeight;
	stbi_uc* collisionMap;

	//For the disk
	int diskCollisionMapWidth, diskCollisionMapHeight;
	stbi_uc* diskCollisionMap;
    
	const float checkRadius = radiusPaddle;
	const float checkRadiusDisk = radiusDisk;
	const int checkSteps = 200;
    
    enum Direction {
        UP,
        RIGHT,
        DOWN,
        LEFT
    };
    
    glm::vec3 GetTableNormal(float diskX, float diskZ) {
        
        /*
        //Corners
        if(diskZ > 0.25f && diskX > 0.68f) return glm::normalize(glm::vec3(-1.0f, 0.0f, -1.0f));
        else if(diskZ > 0.25f && diskX < -0.68f) return glm::normalize(glm::vec3(1.0f, 0.0f, -1.0f));
        else if(diskZ < -0.25f && diskX > 0.68f) return glm::normalize(glm::vec3(-1.0f, 0.0f, 1.0f));
        else if(diskZ < -0.25f && diskX < -0.68f) return glm::normalize(glm::vec3(1.0f, 0.0f, 1.0f));
         */
        
        //Edges
        if(diskZ > 0.32f) return glm::vec3(0.0f, 0.0f, -1.0f);
        else if(diskZ < -0.32f) return glm::vec3(0.0f, 0.0f, 1.0f);
        else if(diskX > 0.76f) return glm::vec3(-1.0f, 0.0f, 0.0f);
        else return glm::vec3(1.0f, 0.0f, 0.0f);
    }

	bool canStepPoint(float x, float y) {
		int pixX = round((x + 1.893 / 2) * (collisionMapWidth / 1.893));
		int pixY = round((y + 1.014 / 2) * (collisionMapHeight / 1.014));
		int pix = (int)collisionMap[collisionMapWidth * pixY + pixX];
		//std::cout << pixX << " " << pixY << " " << x << " " << y << " \t P = " << pix << "\n";		
		return pix > 128;
	}

	bool canDiskStepPoint(float x, float y) {
		int pixX = round((x + 1.893 / 2) * (diskCollisionMapWidth / 1.893));
		int pixY = round((y + 1.014 / 2) * (diskCollisionMapHeight / 1.014));
		int pix = (int)diskCollisionMap[diskCollisionMapWidth * pixY + pixX];
		//std::cout << pixX << " " << pixY << " " << x << " " << y << " \t P = " << pix << "\n";		
		return pix > 128;
	}

	bool canStep(float x, float y, int playerNumber) {
		for (int i = 0; i < checkSteps; i++) {
			if (!canStepPoint(x + cos(6.2832 * i / (float)checkSteps) * checkRadius,
							  y + sin(6.2832 * i / (float)checkSteps) * checkRadius) || invadeEnemyTeam(x, playerNumber)) {
				return false;
			}
		}
		return true;
	}

	bool diskCanStep(float x, float y) {
		for (int i = 0; i < checkSteps; i++) {
			if (!canDiskStepPoint(x + cos(6.2832 * i / (float)checkSteps) * checkRadiusDisk,
				y + sin(6.2832 * i / (float)checkSteps) * checkRadiusDisk)) {
				return false;
			}
		}
		return true;
	}
    
    bool detectDiskCollision(float paddleX, float paddleZ, float diskX, float diskZ) {
        return (sqrt(pow((paddleX - diskX),2) + pow((paddleZ - diskZ),2)) < (radiusPaddle + radiusDisk));
    }

	bool invadeEnemyTeam(float playerPos, int playerNumber) {
		if (playerPos - radiusPaddle < 0 && playerNumber==1)
			return true;
		else if (playerPos + radiusPaddle > 0 && playerNumber == 2)
			return true;
		return false;
	}

	// Here you set the main application parameters
	void setWindowParameters() {
		// window size, titile and initial background
		windowWidth = 800;
		windowHeight = 600;
		windowTitle = "Air Hockey";
		initialBackgroundColor = { 1.0f, 1.0f, 1.0f, 1.0f };

		// Descriptor pool sizes
		uniformBlocksInPool = 7;
		texturesInPool = 6;
		setsInPool = 7;
	}

	// Here you load and setup all your Vulkan objects
	void localInit() {

		DSLobj.init(this, {
			{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT},
			{1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT}
			});

		DSLglobal.init(this, {
			{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS} }
		);
		P1.init(this, "shaders/vert.spv", "shaders/frag.spv", { &DSLglobal, &DSLobj });


		// Table
		M_table.init(this, "models/Tavolo.obj");
		T_table.init(this, "textures/TableTexture.jpg");
		DS_table.init(this, &DSLobj, {
						{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
						{1, TEXTURE, 0, &T_table}
			});
        
        //Scores
        M_score1.init(this, "models/Score1.obj");
        T_score1.init(this, "textures/Score0.png");
        DS_score1.init(this, &DSLobj, {
                        {0, UNIFORM, sizeof(UniformBufferObject), nullptr},
                        {1, TEXTURE, 0, &T_score1}
            });
        
        M_score2.init(this, "models/Score2.obj");
        T_score2.init(this, "textures/Score0.png");
        DS_score2.init(this, &DSLobj, {
                        {0, UNIFORM, sizeof(UniformBufferObject), nullptr},
                        {1, TEXTURE, 0, &T_score2}
            });

		//Paddle 1 and 2
		M_paddle.init(this, "models/paddle.obj");
		T_paddle1.init(this, "textures/paddle1.png");
		T_paddle2.init(this, "textures/paddle2.png");

		DS_paddle1.init(this, &DSLobj, {
						{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
						{1, TEXTURE, 0, &T_paddle1}
			});
		DS_paddle2.init(this, &DSLobj, {
						{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
						{1, TEXTURE, 0, &T_paddle2}
			});

		DS_global.init(this, &DSLglobal, {
				{0, UNIFORM, sizeof(globalUniformBufferObject), nullptr}
			});


		//Disk
		M_disk.init(this, "models/disk.obj");
		T_disk.init(this, "textures/disk.png");
		DS_disk.init(this, &DSLobj, {
						{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
						{1, TEXTURE, 0, &T_disk}
			});

		//Collision map
		collisionMap = stbi_load("textures/collision_map.png", &collisionMapWidth, &collisionMapHeight, NULL, 1);
		if (collisionMap) {

			std::cout << "Table collision map width: " << collisionMapWidth << ", table height: " << collisionMapHeight<< "\n";
		}
		else {
			std::cout << "failed to load map image!";
		}

		diskCollisionMap = stbi_load("textures/disk_collision_map.png", &diskCollisionMapWidth, &diskCollisionMapHeight, NULL, 1);
		if (diskCollisionMap) {

			std::cout << "Table collision map width: " << diskCollisionMapWidth << ", table height: " << diskCollisionMapHeight << "\n";
		}
		else {
			std::cout << "failed to load map image!";
		}

	}

	// Here you destroy all the objects you created!		
	void localCleanup() {
		DS_table.cleanup();
		T_table.cleanup();
		M_table.cleanup();
        
        DS_score1.cleanup();
        T_score1.cleanup();
        M_score1.cleanup();

		DS_paddle1.cleanup();
		DS_paddle2.cleanup();
		T_paddle1.cleanup();
		T_paddle2.cleanup();
		M_paddle.cleanup();

		DS_disk.cleanup();
		T_disk.cleanup();
		M_disk.cleanup();

		P1.cleanup();
		DSLglobal.cleanup();
		DSLobj.cleanup();
	}

	// Here it is the creation of the command buffer:
	// You send to the GPU all the objects you want to draw,
	// with their buffers and textures
	void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) {

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
			P1.graphicsPipeline);
		vkCmdBindDescriptorSets(commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			P1.pipelineLayout, 0, 1, &DS_global.descriptorSets[currentImage],
			0, nullptr);

		//Table ----------------------------------------------------------
		VkBuffer vertexBuffers[] = { M_table.vertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
		vkCmdBindIndexBuffer(commandBuffer, M_table.indexBuffer, 0,
			VK_INDEX_TYPE_UINT32);

		vkCmdBindDescriptorSets(commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			P1.pipelineLayout, 1, 1, &DS_table.descriptorSets[currentImage],
			0, nullptr);

		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(M_table.indices.size()), 1, 0, 0, 0);
        
        //Scores -----------------------------------------------------------------------------------
        VkBuffer vertexBuffers1[] = { M_score1.vertexBuffer };
        VkDeviceSize offsets1[] = { 0 };
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers1, offsets1);
        vkCmdBindIndexBuffer(commandBuffer, M_score1.indexBuffer, 0,
            VK_INDEX_TYPE_UINT32);

        vkCmdBindDescriptorSets(commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            P1.pipelineLayout, 1, 1, &DS_score1.descriptorSets[currentImage],
            0, nullptr);

        vkCmdDrawIndexed(commandBuffer,
            static_cast<uint32_t>(M_score1.indices.size()), 1, 0, 0, 0);
        
        VkBuffer vertexBuffers4[] = { M_score2.vertexBuffer };
        VkDeviceSize offsets4[] = { 0 };
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers4, offsets4);
        vkCmdBindIndexBuffer(commandBuffer, M_score2.indexBuffer, 0,
            VK_INDEX_TYPE_UINT32);

        vkCmdBindDescriptorSets(commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            P1.pipelineLayout, 1, 1, &DS_score2.descriptorSets[currentImage],
            0, nullptr);

        vkCmdDrawIndexed(commandBuffer,
            static_cast<uint32_t>(M_score2.indices.size()), 1, 0, 0, 0);

		//Paddle of player 1 and 2 -----------------------------------------------------------------------------------

		VkBuffer vertexBuffers2[] = { M_paddle.vertexBuffer };
		VkDeviceSize offsets2[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers2, offsets2);
		vkCmdBindIndexBuffer(commandBuffer, M_paddle.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
		//player1
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, P1.pipelineLayout, 1, 1, &DS_paddle1.descriptorSets[currentImage], 0, NULL);
		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(M_paddle.indices.size()), 1, 0, 0, 0);
		//player2
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, P1.pipelineLayout, 1, 1, &DS_paddle2.descriptorSets[currentImage], 0, NULL);
		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(M_paddle.indices.size()), 1, 0, 0, 0);

		// Disk

		VkBuffer vertexBuffers3[] = { M_disk.vertexBuffer };
		VkDeviceSize offsets3[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers3, offsets3);
		vkCmdBindIndexBuffer(commandBuffer, M_disk.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, P1.pipelineLayout, 1, 1, &DS_disk.descriptorSets[currentImage], 0, NULL);
		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(M_disk.indices.size()), 1, 0, 0, 0);

	}

	// Here is where you update the uniforms.
	// Very likely this will be where you will be writing the logic of your application.
	void updateUniformBuffer(uint32_t currentImage) {
		static auto startTime = std::chrono::high_resolution_clock::now();
		static float lastTime = 0.0f;
		static glm::mat3 CamDir = glm::mat3(1.0f);
        static glm::vec3 player1Pos = initialPlayer1Pos;
        static glm::vec3 player2Pos = initialPlayer2Pos;
        static glm::vec3 diskPos = glm::vec3(0.0f);
        static glm::vec3 diskDirection;
        static float diskVelocity;
        static int player1Score;
        static int player2Score;

		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>
			(currentTime - startTime).count();
		float deltaT = time - lastTime;
		lastTime = time;

		const float MOVE_SPEED = 0.75f;
        const float DISK_SPEED_INCREASE = 1.8f;
        const float DISK_DECELERATION = 0.3f;

		globalUniformBufferObject gubo{};
		UniformBufferObject ubo{};

		void* data;
		static int viewMode=0;
		static float debounce = time;

		if (glfwGetKey(window, GLFW_KEY_SPACE)) {
			if (time - debounce > 0.33) {
				viewMode = (viewMode + 1)%3;
				debounce = time;
				std::cout << "viewMode: " << viewMode << "\n";
			}
		}

		switch (viewMode) {
		case 0:
			gubo.view = glm::lookAt(glm::vec3(0.0f, 1.8f, 0.0f),
									glm::vec3(0.0f, 0.0f, 0.0f),
									glm::vec3(0.0f, 0.0f, 1.0f));
			break;
		case 1:
			gubo.view = glm::lookAt(glm::vec3(1.7f, 0.5f, 0.0f),
									glm::vec3(0.0f, 0.0f, 0.0f),
									glm::vec3(0.0f, 1.0f, 0.0f));
			break;		
		case 2:
			gubo.view = glm::lookAt(glm::vec3(-1.7f, 0.5f, 0.0f),
									glm::vec3(0.0f, 0.0f, 0.0f),
									glm::vec3(0.0f, 1.0f, 0.0f));
			break;
		}


		gubo.proj = glm::perspective(glm::radians(45.0f),
			swapChainExtent.width / (float)swapChainExtent.height,
			0.1f, 10.0f);
		gubo.proj[1][1] *= -1;

		vkMapMemory(device, DS_global.uniformBuffersMemory[0][currentImage], 0,
			sizeof(gubo), 0, &data);
		memcpy(data, &gubo, sizeof(gubo));
		vkUnmapMemory(device, DS_global.uniformBuffersMemory[0][currentImage]);


		// Table
		ubo.model = glm::mat4(1.0);
		vkMapMemory(device, DS_table.uniformBuffersMemory[0][currentImage], 0,
			sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(device, DS_table.uniformBuffersMemory[0][currentImage]);
        
        //Scores
        ubo.model = glm::mat4(1.0);
        vkMapMemory(device, DS_score1.uniformBuffersMemory[0][currentImage], 0,
            sizeof(ubo), 0, &data);
        memcpy(data, &ubo, sizeof(ubo));
        vkUnmapMemory(device, DS_score1.uniformBuffersMemory[0][currentImage]);
        
        ubo.model = glm::mat4(1.0);
        vkMapMemory(device, DS_score2.uniformBuffersMemory[0][currentImage], 0,
            sizeof(ubo), 0, &data);
        memcpy(data, &ubo, sizeof(ubo));
        vkUnmapMemory(device, DS_score2.uniformBuffersMemory[0][currentImage]);

		// Paddle of player1
		glm::vec3 oldPlayer1Pos = player1Pos;

		switch (viewMode) {
		case 0:
			if (glfwGetKey(window, GLFW_KEY_A)) {
				diskPos += MOVE_SPEED * glm::vec3(CamDir[0]) * deltaT;
			}
			if (glfwGetKey(window, GLFW_KEY_D)) {
                diskPos -= MOVE_SPEED * glm::vec3(CamDir[0]) * deltaT;
			}

			if (glfwGetKey(window, GLFW_KEY_S)) {
                diskPos -= MOVE_SPEED * glm::vec3(CamDir[2]) * deltaT;
			}
			if (glfwGetKey(window, GLFW_KEY_W)) {
                diskPos += MOVE_SPEED * glm::vec3(CamDir[2]) * deltaT;
			}
			break;
		case 1:
			if (glfwGetKey(window, GLFW_KEY_A)) {
				player1Pos += MOVE_SPEED * glm::vec3(CamDir[2]) * deltaT;
			}
			if (glfwGetKey(window, GLFW_KEY_D)) {
				player1Pos -= MOVE_SPEED * glm::vec3(CamDir[2]) * deltaT;
			}

			if (glfwGetKey(window, GLFW_KEY_S)) {
				player1Pos += MOVE_SPEED * glm::vec3(CamDir[0]) * deltaT;
			}
			if (glfwGetKey(window, GLFW_KEY_W)) {
				player1Pos -= MOVE_SPEED * glm::vec3(CamDir[0]) * deltaT;
			}
			break;
		case 2:
			if (glfwGetKey(window, GLFW_KEY_A)) {
				player1Pos -= MOVE_SPEED * glm::vec3(CamDir[2]) * deltaT;

			}
			if (glfwGetKey(window, GLFW_KEY_D)) {
				player1Pos += MOVE_SPEED * glm::vec3(CamDir[2]) * deltaT;

			}

			if (glfwGetKey(window, GLFW_KEY_S)) {
				player1Pos -= MOVE_SPEED * glm::vec3(CamDir[0]) * deltaT;

			}
			if (glfwGetKey(window, GLFW_KEY_W)) {
				player1Pos += MOVE_SPEED * glm::vec3(CamDir[0]) * deltaT;

			}
			break;
		}


		if (!canStep(player1Pos.x, player1Pos.z, 1)) {
			player1Pos = oldPlayer1Pos;
		}

		// Paddle of player 2
		glm::vec3 oldPlayer2Pos = player2Pos;

		switch (viewMode) {
		case 0:
			if (glfwGetKey(window, GLFW_KEY_RIGHT)) {
				player2Pos -= MOVE_SPEED * glm::vec3(CamDir[0]) * deltaT;

			}
			if (glfwGetKey(window, GLFW_KEY_LEFT)) {
				player2Pos += MOVE_SPEED * glm::vec3(CamDir[0]) * deltaT;

			}

			if (glfwGetKey(window, GLFW_KEY_UP)) {
				player2Pos += MOVE_SPEED * glm::vec3(CamDir[2]) * deltaT;

			}
			if (glfwGetKey(window, GLFW_KEY_DOWN)) {
				player2Pos -= MOVE_SPEED * glm::vec3(CamDir[2]) * deltaT;

			}
			break;
		case 1:
			if (glfwGetKey(window, GLFW_KEY_RIGHT)) {
				player2Pos -= MOVE_SPEED * glm::vec3(CamDir[2]) * deltaT;
			}
			if (glfwGetKey(window, GLFW_KEY_LEFT)) {
				player2Pos += MOVE_SPEED * glm::vec3(CamDir[2]) * deltaT;
			}

			if (glfwGetKey(window, GLFW_KEY_UP)) {
				player2Pos -= MOVE_SPEED * glm::vec3(CamDir[0]) * deltaT;
			}
			if (glfwGetKey(window, GLFW_KEY_DOWN)) {
				player2Pos += MOVE_SPEED * glm::vec3(CamDir[0]) * deltaT;
			}
			break;
		case 2:
			if (glfwGetKey(window, GLFW_KEY_RIGHT)) {
				player2Pos += MOVE_SPEED * glm::vec3(CamDir[2]) * deltaT;
			}
			if (glfwGetKey(window, GLFW_KEY_LEFT)) {
				player2Pos -= MOVE_SPEED * glm::vec3(CamDir[2]) * deltaT;
			}

			if (glfwGetKey(window, GLFW_KEY_UP)) {
				player2Pos += MOVE_SPEED * glm::vec3(CamDir[0]) * deltaT;
			}
			if (glfwGetKey(window, GLFW_KEY_DOWN)) {
				player2Pos -= MOVE_SPEED * glm::vec3(CamDir[0]) * deltaT;
			}
			break;

		}

		if (!canStep(player2Pos.x, player2Pos.z,2)) {
			player2Pos = oldPlayer2Pos;
		}
        
        //Collisions
                
        if(detectDiskCollision(player1Pos.x, player1Pos.z, diskPos.x, diskPos.z)) {
            //std::cout << "Disk collision player1" << "\n";
                        
            float distance = (sqrt(pow((diskPos.x - player1Pos.x),2) + pow((diskPos.z - player1Pos.z),2)));
            if(distance == 0) return;
            
            diskDirection = glm::normalize(diskPos - player1Pos);
            diskVelocity = fmaxf(diskVelocity, DISK_SPEED_INCREASE * (sqrt(pow((oldPlayer1Pos.x - player1Pos.x),2) + pow((oldPlayer1Pos.z - player1Pos.z),2)))/ deltaT);
            
            player1Pos = oldPlayer1Pos;
        };
        
        if(detectDiskCollision(player2Pos.x, player2Pos.z, diskPos.x, diskPos.z)) {
            //std::cout << "Disk collision player2" << "\n";
                        
            float distance = (sqrt(pow((diskPos.x - player2Pos.x),2) + pow((diskPos.z - player2Pos.z),2)));
            if(distance == 0) return;
            
            diskDirection = glm::normalize(diskPos - player2Pos);
            diskVelocity = fmaxf(diskVelocity, DISK_SPEED_INCREASE * (sqrt(pow((oldPlayer2Pos.x - player2Pos.x),2) + pow((oldPlayer2Pos.z - player2Pos.z),2)))/ deltaT);
            
            player2Pos = oldPlayer2Pos;
        }
        
        glm::vec3 oldDiskPos = diskPos;
        diskPos += diskVelocity * diskDirection * deltaT;
        diskVelocity = fmaxf(0.0, diskVelocity - DISK_DECELERATION * deltaT);
        
        if(diskPos.x > 0.8 && diskPos.z < 0.12 && diskPos.z > -0.12)
        {
            diskVelocity = 0.0f;
            player1Pos = initialPlayer1Pos;
            player2Pos = initialPlayer2Pos;
            diskPos.x = 0.3f;
            diskPos.z = 0.0f;
            player1Score++;
                        
            std::stringstream ss;
            ss << "textures/Score" << player1Score << ".png";
            //TODO: change texture
        }
        
        else if(diskPos.x < -0.8 && diskPos.z < 0.12 && diskPos.z > -0.12)
        {
            diskVelocity = 0.0f;
            player1Pos = initialPlayer1Pos;
            player2Pos = initialPlayer2Pos;
            diskPos.x = -0.3f;
            diskPos.z = 0.0f;
            player2Score++;
            
            std::stringstream ss;
            ss << "textures/Score" << player2Score << ".png";
            //TODO: change texture
        }
    
        else if(!diskCanStep(diskPos.x, diskPos.z)) {
            diskPos = oldDiskPos;
            diskDirection = glm::reflect(diskDirection, glm::normalize(GetTableNormal(diskPos.x, diskPos.z)));
        }
            
        // Models
        
        ubo.model = glm::translate(glm::mat4(1.0), player1Pos)    *
            glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
            glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f)) *
            glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        vkMapMemory(device, DS_paddle1.uniformBuffersMemory[0][currentImage], 0,
            sizeof(ubo), 0, &data);
        memcpy(data, &ubo, sizeof(ubo));
        vkUnmapMemory(device, DS_paddle1.uniformBuffersMemory[0][currentImage]);

        
        ubo.model = glm::translate(glm::mat4(1.0), player2Pos);
        vkMapMemory(device, DS_paddle2.uniformBuffersMemory[0][currentImage], 0,
            sizeof(ubo), 0, &data);
        memcpy(data, &ubo, sizeof(ubo));
        vkUnmapMemory(device, DS_paddle2.uniformBuffersMemory[0][currentImage]);

        
        ubo.model = glm::translate(glm::mat4(1.0), diskPos);
        vkMapMemory(device, DS_disk.uniformBuffersMemory[0][currentImage], 0,
            sizeof(ubo), 0, &data);
        memcpy(data, &ubo, sizeof(ubo));
        vkUnmapMemory(device, DS_disk.uniformBuffersMemory[0][currentImage]);
        
        /*
        std::cout << "posx"<< diskPos.x << "\n";
        std::cout << "posy"<< diskPos.z << "\n";
        std::cout << "dirx" << diskDirection.x << "\n";
        std::cout << "dirz" << diskDirection.x << "\n";
        std::cout << "vel" << diskVelocity << "\n";
         */
    }
};

// This is the main: probably you do not need to touch this!
int main() {
	MyProject app;

	try {

		app.run();

	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
