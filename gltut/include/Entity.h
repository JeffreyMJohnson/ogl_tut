#include <vector>
#include "glm\glm.hpp"

class Entity
{
public:
	virtual void Initialize() = 0;
	virtual void Destroy() = 0;
	virtual void Update() = 0;
	virtual void Draw() = 0;

protected:
	float* vertices;
	std::vector<glm::vec2> vertexPositions;
	std::vector<unsigned int> vertexIndeces;
	glm::vec4 color;

	void UpdateVertices()
	{
		for (int i = 0; i < vertexPositions.size(); i++)
		{

		}
	}

private:


};