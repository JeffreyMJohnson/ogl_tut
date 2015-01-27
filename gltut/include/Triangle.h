//#include "Entity.h"
#include <vector>
#include "glm\glm.hpp"


class Triangle
	//: public Entity
{
public:
	void Initialize()
	{
		vertices = new float[9];
		vertexPositions.push_back(glm::vec2(-.5, -.5));
		vertexPositions.push_back(glm::vec2(0, .5));
		vertexPositions.push_back(glm::vec2(.5, -.5));

	}

	void Destroy()
	{
		delete vertices[];
	}

	void Update()
	{

	}

	void Draw()
	{

	}

private:
	float* vertices;
	std::vector<glm::vec2> vertexPositions;
	std::vector<unsigned int> vertexIndeces;
	glm::vec4 color;

	void UpdateVertices()
	{
		for (int i = 0; i < vertexPositions.size(); i++)
		{

		}

};