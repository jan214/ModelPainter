#ifndef MODELLOADER_H
#define MODELLOADER_H

#include <vector>

#include <QString>
#include <QTextStream>

struct Mesh {
	QString name;
	std::vector<int> indices;
	std::vector<float> vertices;
	std::vector<int> textureIndices;
	std::vector<float> textureCoordinates;
	std::vector<int> normalIndices;
	std::vector<float> normals;
};

class ModelLoader {
public:
	static ModelLoader& GetInstance() {
		static ModelLoader modelLoader;
		return modelLoader;
	}

	void LoadModel(QTextStream& modelFileText);

	inline float* GetVertices() { return customModelVertices.data(); }
	inline int GetVerticesSize() { return customModelVertices.size(); }
	inline float* GetNormals() { return customModelNormals.data(); }
	inline int GetNormalsSize() { return customModelNormals.size(); }
	inline float* GetTextureCoordinates() { return customModelTextureCoordinates.data(); }
	inline int GetTextureCoordinatesSize() { return customModelTextureCoordinates.size(); }

	void Subscribe(std::function<void()> onUpdate);

	int ModelSize;
	bool ModelChanged;
protected:
	ModelLoader();
	~ModelLoader(){}

	std::vector<float> customModelVertices;
	std::vector<float> customModelTextureCoordinates;
	std::vector<float> customModelNormals;

	std::vector<int> indices;
	std::vector<float> vertices;

	std::vector<int> texturesIndices;
	std::vector<float> textureCoordinates;

	std::vector<int> normalsIndices;
	std::vector<float> normals;

	std::vector<std::function<void()>> updateFunctions;
};

#endif // MODELLOADER_H
