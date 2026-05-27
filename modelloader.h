#ifndef MODELLOADER_H
#define MODELLOADER_H

#include <vector>
#include <stack>

#include <QString>
#include <QTextStream>

class BvhTree {
public:
	struct Node {
	public:
		Node();
		~Node();

		void Insert(const int index, const float triangle[9]);
		void Sort(std::vector<float>& modelData);
		void Insert1(std::vector<float>& modelData, std::vector<Node>& nodes, std::vector<int>& nodeIndexStack);

		float mins[3];
		float maxs[3];

		struct Node* left;
		struct Node* right;

		int selfIndex;
		int leftIndex;
		int rightIndex;

		std::vector<int> indices;
	};

	BvhTree();
	~BvhTree();

	void Initialize(std::vector<float>& modelData);
	void Initialize1(std::vector<float>& modelData);

	Node rootNode;
	std::vector<Node> nodes;
};

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

	std::shared_ptr<BvhTree> bvhTree;
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
