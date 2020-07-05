#ifndef DB_H
#define DB_H

#include "Model.h"
#include "Camera.h"

class DB
{
public:
	DB(int windowWidth, int windowHeight);

	void run();

	~DB();
private:
	int windowWidth, windowHeight, bufferWidth, bufferHeight;
	GLFWwindow* window;
	std::vector<Model*> models;
	int keys[1024];
	Camera* camera;
	bool mouseFirstMoved;
	GLfloat lastX, lastY, xChange, yChange;

	void initOpenGL();

	static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void reshapeCallback(GLFWwindow* window, int width, int height);
	static void mouseCallback(GLFWwindow* window, double xPos, double yPos);

	void addModel(const std::string &fileLoc, const std::string &vertexLoc, const std::string &fragmentLoc, const glm::vec3 &position, const glm::vec3 &scale);
};

#endif // !DB_H