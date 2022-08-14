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
	Model M_table;
	Texture T_table;
	DescriptorSet DS_table;

	Model M_handle;
	Texture T_handle;
	DescriptorSet DS_handle1;
	DescriptorSet DS_handle2;

	DescriptorSet DS_global;

	// Here you set the main application parameters
	void setWindowParameters() {
		// window size, titile and initial background
		windowWidth = 800;
		windowHeight = 600;
		windowTitle = "My Project";
		initialBackgroundColor = { 1.0f, 1.0f, 1.0f, 1.0f };

		// Descriptor pool sizes
		uniformBlocksInPool = 4;
		texturesInPool = 3;
		setsInPool = 4;
	}

	// Here you load and setup all your Vulkan objects
	void localInit() {
		// Descriptor Layouts [what will be passed to the shaders]
		DSLobj.init(this, {
			// this array contains the binding:
			// first  element : the binding number
			// second element : the time of element (buffer or texture)
			// third  element : the pipeline stage where it will be used
			{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT},
			{1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT}
			});

		DSLglobal.init(this, {
			// this array contains the binding:
			// first  element : the binding number
			// second element : the time of element (buffer or texture)
			// third  element : the pipeline stage where it will be used
			{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS} }
		);

		// Pipelines [Shader couples]
		// The last array, is a vector of pointer to the layouts of the sets that will
		// be used in this pipeline. The first element will be set 0, and so on..
		P1.init(this, "shaders/vert.spv", "shaders/frag.spv", { &DSLglobal, &DSLobj });


		// Models, textures and Descriptors (values assigned to the uniforms)
		M_table.init(this, "models/tavolo.obj");
		T_table.init(this, "textures/airhockey-background.png");
		DS_table.init(this, &DSLobj, {
			// the second parameter, is a pointer to the Uniform Set Layout of this set
			// the last parameter is an array, with one element per binding of the set.
			// first  elmenet : the binding number
			// second element : UNIFORM or TEXTURE (an enum) depending on the type
			// third  element : only for UNIFORMs, the size of the corresponding C++ object
			// fourth element : only for TEXTUREs, the pointer to the corresponding texture object
						{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
						{1, TEXTURE, 0, &T_table}
			});

		M_handle.init(this, "models/handler.obj");
		T_handle.init(this, "textures/airhockey-background.png");

		DS_handle1.init(this, &DSLobj, {
						{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
						{1, TEXTURE, 0, &T_handle}
			});
		DS_handle2.init(this, &DSLobj, {
						{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
						{1, TEXTURE, 0, &T_handle}
			});

		DS_global.init(this, &DSLglobal, {
				{0, UNIFORM, sizeof(globalUniformBufferObject), nullptr}
			});
	}

	// Here you destroy all the objects you created!		
	void localCleanup() {
		DS_table.cleanup();
		T_table.cleanup();
		M_table.cleanup();


		DS_handle1.cleanup();
		DS_handle2.cleanup();

		T_handle.cleanup();
		M_handle.cleanup();

		P1.cleanup();
		DSLglobal.cleanup();
		DSLobj.cleanup();
	}

	// Here it is the creation of the command buffer:
	// You send to the GPU all the objects you want to draw,
	// with their buffers and textures
	void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) {

		//Table ----------------------------------------------------------
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
			P1.graphicsPipeline);
		vkCmdBindDescriptorSets(commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			P1.pipelineLayout, 0, 1, &DS_global.descriptorSets[currentImage],
			0, nullptr);

		VkBuffer vertexBuffers[] = { M_table.vertexBuffer };
		// property .vertexBuffer of models, contains the VkBuffer handle to its vertex buffer
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
		// property .indexBuffer of models, contains the VkBuffer handle to its index buffer
		vkCmdBindIndexBuffer(commandBuffer, M_table.indexBuffer, 0,
			VK_INDEX_TYPE_UINT32);



		// property .pipelineLayout of a pipeline contains its layout.
		// property .descriptorSets of a descriptor set contains its elements.
		vkCmdBindDescriptorSets(commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			P1.pipelineLayout, 1, 1, &DS_table.descriptorSets[currentImage],
			0, nullptr);

		// property .indices.size() of models, contains the number of triangles * 3 of the mesh.
		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(M_table.indices.size()), 1, 0, 0, 0);

		//Handle 1 and 2 -----------------------------------------------------------------------------------


		VkBuffer vertexBuffers2[] = { M_handle.vertexBuffer };
		VkDeviceSize offsets2[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers2, offsets2);
		vkCmdBindIndexBuffer(commandBuffer, M_handle.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, P1.pipelineLayout, 1, 1, &DS_handle1.descriptorSets[currentImage], 0, NULL);

		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(M_handle.indices.size()), 1, 0, 0, 0);

		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, P1.pipelineLayout, 1, 1, &DS_handle2.descriptorSets[currentImage], 0, NULL);

		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(M_handle.indices.size()), 1, 0, 0, 0);




	}

	// Here is where you update the uniforms.
	// Very likely this will be where you will be writing the logic of your application.
	void updateUniformBuffer(uint32_t currentImage) {
		static auto startTime = std::chrono::high_resolution_clock::now();
		static float lastTime = 0.0f;
		static glm::mat3 CamDir = glm::mat3(1.0f);
		static glm::vec3 player1Pos = glm::vec3(0, 0, 0);
		static glm::vec3 player2Pos = glm::vec3(-1.5f, 0, 0);

		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>
			(currentTime - startTime).count();
		float deltaT = time - lastTime;
		lastTime = time;

		const float MOVE_SPEED = 0.75f;

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
			gubo.view = glm::lookAt(glm::vec3(1.8f, 0.0f, 1.0f),
						glm::vec3(0.5f, 0.0f, 0.0f),
						glm::vec3(0.0f, 0.0f, 1.0f))*
				glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
			break;
		case 2:
			gubo.view = glm::lookAt(glm::vec3(-1.8f, 0.0f, 1.0f),
				glm::vec3(-0.5f, 0.0f, 0.0f),
				glm::vec3(0.0f, 0.0f, 1.0f))*
				glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
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


		// For the table
		ubo.model = glm::translate(glm::mat4(1.0), glm::vec3(0.0f, 0.0f, 0.0f)) *
			glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f)) *
			glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		vkMapMemory(device, DS_table.uniformBuffersMemory[0][currentImage], 0,
			sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(device, DS_table.uniformBuffersMemory[0][currentImage]);

		// For the Handle of player1

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

		ubo.model = glm::translate(glm::mat4(1.0), player1Pos)	*
			glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f)) *
			glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		vkMapMemory(device, DS_handle1.uniformBuffersMemory[0][currentImage], 0,
			sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(device, DS_handle1.uniformBuffersMemory[0][currentImage]);

		// For the Handle of player 2
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
		ubo.model = glm::translate(glm::mat4(1.0), player2Pos);
		vkMapMemory(device, DS_handle2.uniformBuffersMemory[0][currentImage], 0,
			sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(device, DS_handle2.uniformBuffersMemory[0][currentImage]);

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