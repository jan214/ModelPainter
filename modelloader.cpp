#include "modelloader.h"

#include <QWidget>
#include <QStringList>

#include <algorithm>
#include <stack>

BvhTree::BvhTree() :
rootNode(),
nodes{} {

}

BvhTree::~BvhTree() {

}

void BvhTree::Initialize(std::vector<float>& modelData) {
    Node& rootNode = ModelLoader::GetInstance().bvhTree->rootNode;

    for (int counter = 0; counter < modelData.size(); counter += 9) {
        const float triangle[9] = { modelData[counter], modelData[counter + 1],modelData[counter + 2],
                                    modelData[counter+3], modelData[counter + 4],modelData[counter + 5],
                                    modelData[counter+6], modelData[counter + 7],modelData[counter + 8] };
        rootNode.Insert(counter / 9, triangle);
    }

    //printf("rootNodeSize: %f %f %f %f %f %f\n", rootNode.mins[0], rootNode.maxs[0], rootNode.mins[1], rootNode.maxs[1], rootNode.mins[2], rootNode.maxs[2]);

    rootNode.Sort(modelData);
}

void BvhTree::Initialize1(std::vector<float>& modelData) {
    std::vector<int> nodeIndexStack;
    nodeIndexStack.push_back(0);
    const int triangleCount = modelData.size() / 9;
    nodes.reserve(triangleCount);
    nodes.push_back({});
    nodes[0].selfIndex = 0;

    while (!nodeIndexStack.empty()) {
        const int index = nodeIndexStack.back();
        nodeIndexStack.pop_back();

        nodes[index].Insert1(modelData, nodes, nodeIndexStack);
    }
}

BvhTree::Node::Node() :
mins{ 0.0f },
maxs{ 0.0f },
left(nullptr),
right(nullptr),
selfIndex(-1),
leftIndex(-1),
rightIndex(-1),
indices{} {

}

BvhTree::Node::~Node() {
    delete left;
    delete right;
}

void BvhTree::Node::Insert1(std::vector<float>& modelData, std::vector<Node>& nodes, std::vector<int>& nodeIndexStack) {
    if (selfIndex == 0) {
        const int modelDataSize = modelData.size();
        for (int index = 0; index < modelDataSize; index += 9) {
            const float triangle[9] = { modelData[index], modelData[index + 1],modelData[index + 2],
                                        modelData[index + 3],modelData[index + 4],modelData[index + 5],
                                        modelData[index + 6],modelData[index + 7],modelData[index + 8] };

            for (int counter = 0; counter < 3; counter++) {
                if (triangle[counter * 3] < mins[0]) {
                    mins[0] = triangle[counter * 3];
                }
                
                if (triangle[counter * 3] > maxs[0]) {
                    maxs[0] = triangle[counter * 3];
                }

                if (triangle[counter * 3 + 1] < mins[1]) {
                    mins[1] = triangle[counter * 3 + 1];
                }
                
                if (triangle[counter * 3 + 1] > maxs[1]) {
                    maxs[1] = triangle[counter * 3 + 1];
                }

                if (triangle[counter * 3 + 2] < mins[2]) {
                    mins[2] = triangle[counter * 3 + 2];
                }
                
                if (triangle[counter * 3 + 2] > maxs[2]) {
                    maxs[2] = triangle[counter * 3 + 2];
                }
            }

            indices.emplace_back(index / 9);
        }
    } else {
        for (const int& index : indices) {
            const float triangle[9] = { modelData[index * 9], modelData[index * 9 + 1],modelData[index * 9 + 2],
                                        modelData[index * 9 + 3],modelData[index * 9 + 4],modelData[index * 9 + 5],
                                        modelData[index * 9 + 6],modelData[index * 9 + 7],modelData[index * 9 + 8] };

            for (int counter = 0; counter < 3; counter++) {
                if (triangle[counter * 3] < mins[0]) {
                    mins[0] = triangle[counter * 3];
                }

                if (triangle[counter * 3] > maxs[0]) {
                    maxs[0] = triangle[counter * 3];
                }

                if (triangle[counter * 3 + 1] < mins[1]) {
                    mins[1] = triangle[counter * 3 + 1];
                }

                if (triangle[counter * 3 + 1] > maxs[1]) {
                    maxs[1] = triangle[counter * 3 + 1];
                }

                if (triangle[counter * 3 + 2] < mins[2]) {
                    mins[2] = triangle[counter * 3 + 2];
                }

                if (triangle[counter * 3 + 2] > maxs[2]) {
                    maxs[2] = triangle[counter * 3 + 2];
                }
            }
        }
    }

    //printf("mins: %f %f %f maxs: %f %f %f\n", mins[0], mins[1], mins[2], maxs[0], maxs[1], maxs[2]);
    
    if (indices.size() < 2)
        return;

    const int cutAxis = (maxs[0] - mins[0]) > (maxs[1] - mins[1]) ? 0 : (maxs[1] - mins[1] > maxs[2] - mins[2] ? 1 : 2);
    //printf("cutAxis: %i\n", cutAxis);

    std::sort(indices.begin(), indices.end(), [&modelData, &cutAxis](const int lhs, const int rhs) {
        const float triangles[9] = { modelData[lhs * 9], modelData[lhs * 9 + 1], modelData[lhs * 9 + 2],
                        modelData[lhs * 9 + 3], modelData[lhs * 9 + 4], modelData[lhs * 9 + 5],
                        modelData[lhs * 9 + 6], modelData[lhs * 9 + 7], modelData[lhs * 9 + 8] };

        const float center[3] = {(triangles[0] + triangles[3] + triangles[6]) / 3.0f,
                            (triangles[1] + triangles[4] + triangles[7]) / 3.0f,
                            (triangles[2] + triangles[5] + triangles[8]) / 3.0f };

        const float triangles2[9] = { modelData[rhs * 9], modelData[rhs * 9 + 1], modelData[rhs * 9 + 2],
                modelData[rhs * 9 + 3], modelData[rhs * 9 + 4], modelData[rhs * 9 + 5],
                modelData[rhs * 9 + 6], modelData[rhs * 9 + 7], modelData[rhs * 9 + 8] };

        const float center2[3] = { (triangles2[0] + triangles2[3] + triangles2[6]) / 3.0f,
                    (triangles2[1] + triangles2[4] + triangles2[7]) / 3.0f,
                    (triangles2[2] + triangles2[5] + triangles2[8]) / 3.0f };

        return center[cutAxis] < center2[cutAxis];
    });

    const int indicesSize = indices.size();
    const int halfSize = indicesSize / 2;

    if (leftIndex == -1) {
        leftIndex = nodes.size();
        nodeIndexStack.push_back(leftIndex);
        nodes.emplace_back();
        nodes[leftIndex].indices.reserve(halfSize);
    }

    if (rightIndex == -1) {
        rightIndex = nodes.size();
        nodeIndexStack.push_back(rightIndex);
        nodes.emplace_back();
        nodes[rightIndex].indices.reserve(halfSize);
    }

    for (int index = 0; index < indicesSize; index++) {
        const int indice = indices[index];

        if (index < halfSize) {
            nodes[leftIndex].indices.emplace_back(indice);
            continue;
        }

        nodes[rightIndex].indices.emplace_back(indice);
    }
}

void BvhTree::Node::Insert(const int index, const float triangle[9]) {
    for (int counter = 0; counter < 3; counter++) {
        if (triangle[counter*3] < mins[0]) {
            mins[0] = triangle[counter * 3];
        } else if (triangle[counter*3] > maxs[0]) {
            maxs[0] = triangle[counter * 3];
        }

        if (triangle[counter*3+1] < mins[1]) {
            mins[1] = triangle[counter * 3 + 1];
        } else if (triangle[counter*3+1] > maxs[1]) {
            maxs[1] = triangle[counter * 3 + 1];
        }

        if (triangle[counter*3+2] < mins[2]) {
            mins[2] = triangle[counter * 3 + 2];
        } else if (triangle[counter*3+2] > maxs[2]) {
            maxs[2] = triangle[counter * 3 + 2];
        }
    }

    indices.push_back(index);
}

void BvhTree::Node::Sort(std::vector<float>& modelData) {
    //printf("index:");
    //for (const int& index : indices) {
    //    printf("%i,", index);
    //}
    //printf("\n");

    //printf("mins: %f %f %f maxs: %f %f %f\n", mins[0], mins[1], mins[2], maxs[0], maxs[1], maxs[2]);

    if (indices.size() <= 2) {
        //printf("index:\n");
        //for (const int& index : indices) {
        //    printf("%i,", index);
        //    printf(" triangle1: %f %f %f/%f %f %f/%f %f %f\n", modelData[index * 9], modelData[index * 9 + 1], modelData[index * 9 + 2], modelData[index * 9 + 3], modelData[index * 9 + 4], modelData[index * 9 + 5], modelData[index * 9 + 6], modelData[index * 9 + 7], modelData[index * 9 + 8]);
        //}
        //printf("\n");
        return;
    }

    const int cutAxis = (maxs[0] - mins[0]) > (maxs[1] - mins[1]) ? 0 : (maxs[1] - mins[1] > maxs[2] - mins[2] ? 1 : 2);
    //printf("cutAxis: %i\n", cutAxis);

    std::sort(indices.begin(), indices.end(), [&modelData, &cutAxis](const int lhs, const int rhs) {
        const float triangles[9] = { modelData[lhs * 9], modelData[lhs * 9 + 1], modelData[lhs * 9 + 2],
                       modelData[lhs * 9 + 3], modelData[lhs * 9 + 4], modelData[lhs * 9 + 5],
                        modelData[lhs * 9 + 6], modelData[lhs * 9 + 7], modelData[lhs * 9 + 8] };

        const float center[3] = { (triangles[0] + triangles[3] + triangles[6]) / 3.0f,
                            (triangles[1] + triangles[4] + triangles[7]) / 3.0f,
                            (triangles[2] + triangles[5] + triangles[8]) / 3.0f };

        const float triangles2[9] = { modelData[rhs * 9], modelData[rhs * 9 + 1], modelData[rhs * 9 + 2],
               modelData[rhs * 9 + 3], modelData[rhs * 9 + 4], modelData[rhs * 9 + 5],
                modelData[rhs * 9 + 6], modelData[rhs * 9 + 7], modelData[rhs * 9 + 8] };

        const float center2[3] = { (triangles2[0] + triangles2[3] + triangles2[6]) / 3.0f,
                    (triangles2[1] + triangles2[4] + triangles2[7]) / 3.0f,
                    (triangles2[2] + triangles2[5] + triangles2[8]) / 3.0f };

        return center[cutAxis] < center2[cutAxis];
    });

    //printf("cutAxis: %i\n", cutAxis);

    const int indicesSize = indices.size();
    const int halfSize = indicesSize / 2;
    //printf("halfSize: %i\n", halfSize);
    for (int index = 0; index < indicesSize; index++) {
        const float triangles[9] = { modelData[indices[index] * 9], modelData[indices[index] * 9 + 1], modelData[indices[index] * 9 + 2],
                       modelData[indices[index] * 9 + 3], modelData[indices[index] * 9 + 4], modelData[indices[index] * 9 + 5],
                        modelData[indices[index] * 9 + 6], modelData[indices[index] * 9 + 7], modelData[indices[index] * 9 + 8] };

        //printf("x:%f y:%f z:%f/x:%f y:%f z:%f/x:%f y:%f z:%f\n", triangles[0], triangles[1], triangles[2], triangles[3], triangles[4], triangles[5], triangles[6], triangles[7], triangles[8]);

        if (index < halfSize) {
            if (left == nullptr) {
                left = new Node();
            }
            left->Insert(indices[index], triangles);
            continue;
        }

        if (right == nullptr) {
            right = new Node();
        }
        right->Insert(indices[index], triangles);
    }

    if (left != nullptr) {
        //printf("left size: x: %f %f y: %f %f z: %f %f\n", mins[0], maxs[0], mins[1], maxs[1], mins[2], maxs[2]);
        //printf("left\n");
        left->Sort(modelData);
    }
    if (right != nullptr) {
        //printf("right size: x: %f %f y: %f %f z: %f %f\n", mins[0], maxs[0], mins[1], maxs[1], mins[2], maxs[2]);
        //printf("right\n");
        right->Sort(modelData);
    }
}

void ModelLoader::LoadModel(QTextStream& modelFileText) {
    if (bvhTree != nullptr) {
        bvhTree.reset();
    }

    customModelVertices.clear();
    customModelTextureCoordinates.clear();
    customModelNormals.clear();

    indices.clear();
    vertices.clear();

    texturesIndices.clear();
    textureCoordinates.clear();

    normalsIndices.clear();
    normals.clear();

    while (!modelFileText.atEnd()) {
        const QString line = modelFileText.readLine();
        if (line.startsWith("v ")) {
            const QStringList lineStringList = line.split(" ");
            for (const QString& string : lineStringList) {
                bool ok = false;
                const float value = string.toFloat(&ok);
                if (ok) {
                    //printf("string: %f\n", value);
                    vertices.push_back(value);
                }
            }
        } else if (line.startsWith("vt ")) {
            const QStringList lineStringList = line.split(" ");
            for (const QString& string : lineStringList) {
                bool ok = false;
                const float value = string.toFloat(&ok);
                if (ok) {
                    //printf("string: %f\n", value);
                    textureCoordinates.push_back(value);
                }
            }
        } else if (line.startsWith("vn ")) {
            const QStringList lineStringList = line.split(" ");
            for (const QString& string : lineStringList) {
                bool ok = false;
                const float value = string.toFloat(&ok);
                if (ok) {
                    //printf("string: %f\n", value);
                    normals.push_back(value);
                }
            }
        } else if (line.startsWith("f ")) {
            const QStringList lineStringList = line.split(" ");
            const bool isTriangle = lineStringList.size() == 4;
            //if (isTriangle) {
            //    printf("0: %s 1: %s 2: %s 3: %s\n", lineStringList[0].toStdString().c_str(), lineStringList[1].toStdString().c_str(), lineStringList[2].toStdString().c_str(), lineStringList[3].toStdString().c_str());
            //} else {
            //    printf("0: %s 1: %s 2: %s 3: %s 4: %s\n", lineStringList[0].toStdString().c_str(), lineStringList[1].toStdString().c_str(), lineStringList[2].toStdString().c_str(), lineStringList[3].toStdString().c_str(), lineStringList[3].toStdString().c_str());
            //}

            //printf("lineStringList: %i / isTriangle: %s\n", lineStringList.size(), isTriangle ? "true" : "false");
            int vertexIndices[4] = { -1, -1, -1, -1 };
            int textureIndices[4] = { -1, -1, -1, -1 };
            int normalIndices[4] = { -1, -1, -1, -1 };
            for (const QString& string : lineStringList) {
                //printf("lineStringList: %s\n", string.toStdString().c_str());
                if (string.startsWith("f"))
                    continue;

                const QStringList componentsString = string.split("/");
                bool ok = false;
                const int vertexIndex = componentsString[0].toInt(&ok);
                if (ok) {
                    //printf("vertexIndex: %i currentValue: %i\n", vertexIndex, vertexIndices[counter]);
                    //if (vertexIndices[counter] != -1)
                    //    continue;

                    for (int& value : vertexIndices) {
                        if (value == -1) {
                            value = vertexIndex - 1;
                            break;
                        }
                    }
                    ok = false;
                    //printf("value: %i\n", vertexIndex - 1);
                }
                const int textureIndex = componentsString[1].toInt(&ok);
                //printf("texture ok: %s\n", ok ? "true" : "false");
                if (ok) {
                    //printf("textureIndices: %i currentValue: %i\n", textureIndex, textureIndices[counter]);
                    for (int& value : textureIndices) {
                        if (value == -1) {
                            value = textureIndex - 1;
                            break;
                        }
                    }
                    ok = false;
                }
                const int normalIndex = componentsString[2].toInt(&ok);
                if (ok) {
                    for (int& value : normalIndices) {
                        if (value == -1) {
                            value = normalIndex - 1;
                            break;
                        }
                    }
                }
                //printf("vertexIndices: %i %i %i %i\n", vertexIndices[0], vertexIndices[1], vertexIndices[2], vertexIndices[3]);
                //printf("textureIndices: %i %i %i %i\n", textureIndices[0], textureIndices[1], textureIndices[2], textureIndices[3]);
                //printf("==========================\n");
            }
            //printf("-------------------\n");

            //printf("triangleIndices: %i %i %i %i\n", vertexIndices[0], vertexIndices[1], vertexIndices[2], vertexIndices[3]);

            // triangle 1
            indices.push_back(vertexIndices[0]);
            indices.push_back(vertexIndices[1]);
            indices.push_back(vertexIndices[2]);

            texturesIndices.push_back(textureIndices[0]);
            texturesIndices.push_back(textureIndices[1]);
            texturesIndices.push_back(textureIndices[2]);

            normalsIndices.push_back(normalIndices[0]);
            normalsIndices.push_back(normalIndices[1]);
            normalsIndices.push_back(normalIndices[2]);

            // triangle 2
            if (!isTriangle) {
                indices.push_back(vertexIndices[0]);
                indices.push_back(vertexIndices[2]);
                indices.push_back(vertexIndices[3]);

                texturesIndices.push_back(textureIndices[0]);
                texturesIndices.push_back(textureIndices[2]);
                texturesIndices.push_back(textureIndices[3]);

                normalsIndices.push_back(normalIndices[0]);
                normalsIndices.push_back(normalIndices[2]);
                normalsIndices.push_back(normalIndices[3]);
            }
        } else if (line.startsWith("o ")) {
            printf("line: %s\n", line.toStdString().c_str());
        }
        //printf("%s\n", line.toUtf8().data());
    }

    //for (int count = 0; count < indices.size(); count+=3) {
    //    printf("%i %i %i: %f %f %f / %f %f %f / %f %f %f\n", indices[count], indices[count+1], indices[count+2], 
    //        vertices[indices[count]*3], vertices[indices[count]*3+1], vertices[indices[count]*3+2],
    //        vertices[indices[count+1]*3], vertices[indices[count+1]*3+1], vertices[indices[count+1]*3+2],
    //        vertices[indices[count+2]*3], vertices[indices[count+2]*3+1], vertices[indices[count+2]*3+2]);
    //}
    //printf("================================================\n");
    //for (int count = 0; count < indices.size(); count += 2) {
    //    printf("%i %i %i: %f %f / %f %f / %f %f\n", texturesIndices[count], texturesIndices[count + 1], texturesIndices[count+2],
    //        textureCoordinates[texturesIndices[count] * 2], textureCoordinates[texturesIndices[count] * 2 + 1],
    //        textureCoordinates[texturesIndices[count + 1] * 2], textureCoordinates[texturesIndices[count + 1] * 2 + 1],
    //        textureCoordinates[texturesIndices[count + 2] * 2], textureCoordinates[texturesIndices[count + 2] * 2 + 1]);
    //}
    //printf("verticesSize: %i normalsSize: %i indicesSize: %i\n", vertices.size(), normals.size(), indices.size());

    for (int counter = 0; counter < indices.size(); counter += 3) {
        //printf("indices: %i %i %i / %i %i %i / %i %i %i\n", indices[counter] * 3, indices[counter]*3+1, indices[counter]*3+2,
        //    indices[counter+1] * 3, indices[counter+1] * 3 + 1, indices[counter+1] * 3 + 2,
        //    indices[counter+2] * 3, indices[counter+2] * 3 + 1, indices[counter+2] * 3 + 2);
        customModelVertices.push_back(vertices[indices[counter] * 3]);
        customModelVertices.push_back(vertices[indices[counter] * 3 + 1]);
        customModelVertices.push_back(vertices[indices[counter] * 3 + 2]);

        customModelVertices.push_back(vertices[indices[counter + 1] * 3]);
        customModelVertices.push_back(vertices[indices[counter + 1] * 3 + 1]);
        customModelVertices.push_back(vertices[indices[counter + 1] * 3 + 2]);

        customModelVertices.push_back(vertices[indices[counter + 2] * 3]);
        customModelVertices.push_back(vertices[indices[counter + 2] * 3 + 1]);
        customModelVertices.push_back(vertices[indices[counter + 2] * 3 + 2]);

        customModelNormals.push_back(normals[normalsIndices[counter] * 3]);
        customModelNormals.push_back(normals[normalsIndices[counter] * 3 + 1]);
        customModelNormals.push_back(normals[normalsIndices[counter] * 3 + 2]);

        customModelNormals.push_back(normals[normalsIndices[counter + 1] * 3]);
        customModelNormals.push_back(normals[normalsIndices[counter + 1] * 3 + 1]);
        customModelNormals.push_back(normals[normalsIndices[counter + 1] * 3 + 2]);

        customModelNormals.push_back(normals[normalsIndices[counter + 2] * 3]);
        customModelNormals.push_back(normals[normalsIndices[counter + 2] * 3 + 1]);
        customModelNormals.push_back(normals[normalsIndices[counter + 2] * 3 + 2]);
    }

    //printf("=================================================================\n");

    //for (int counter = 0; counter < textureCoordinates.size(); counter+=2) {
    //    printf("textureCoordinates %i %i: %f %f\n", counter, counter+1, textureCoordinates[counter], textureCoordinates[counter+1]);
    //}

    for (int counter = 0; counter < texturesIndices.size(); counter++) {
        //printf("textures: %i %i / %i %i / %i %i\n", texturesIndices[counter] * 2, texturesIndices[counter] * 2 + 1,
        //    texturesIndices[counter + 1] * 2, texturesIndices[counter + 1] * 2 + 1,
        //    texturesIndices[counter + 2] * 2, texturesIndices[counter + 2] * 2 + 1);
        //printf("counter: %i %i %i %i %i %i\n", ((counter) * 2), ((counter) * 2+1), ((counter + 1) * 2), ((counter+1) * 2 + 1), ((counter + 2) * 2), ((counter+2) * 2 + 1));
        if (texturesIndices[counter] * 2 == -1) {
            customModelTextureCoordinates.push_back(0.0f);
            customModelTextureCoordinates.push_back(0.0f);
        } else {
            customModelTextureCoordinates.push_back(textureCoordinates[texturesIndices[counter] * 2]);
            customModelTextureCoordinates.push_back(textureCoordinates[texturesIndices[counter] * 2 + 1]);
        }

        //if (texturesIndices[counter + 1] * 2 == -1) {
        //    customModelTextureCoordinates.push_back(0.0f);
        //    customModelTextureCoordinates.push_back(0.0f);
        //} else {
        //    customModelTextureCoordinates.push_back(textureCoordinates[texturesIndices[counter + 1] * 2]);
        //    customModelTextureCoordinates.push_back(textureCoordinates[texturesIndices[counter + 1] * 2 + 1]);
        //}

        //if (texturesIndices[counter + 2] * 2 == -1) {
        //    customModelTextureCoordinates.push_back(0.0f);
        //    customModelTextureCoordinates.push_back(0.0f);
        //} else {
        //    customModelTextureCoordinates.push_back(textureCoordinates[texturesIndices[counter + 2] * 2]);
        //    customModelTextureCoordinates.push_back(textureCoordinates[texturesIndices[counter + 2] * 2 + 1]);
        //}
    }

    //for (int counter = 0; counter < customModelVertices.size(); counter += 3) {
    //    printf("vertex %i: %f %f %f\n", counter, customModelVertices[counter], customModelVertices[counter + 1], customModelVertices[counter + 2]);
    //}

    //for (int counter = 0; counter < texturesIndices.size(); counter++) {
    //    printf("%i texture %i %i: %f %f\n", texturesIndices[counter], texturesIndices[counter] * 2, texturesIndices[counter] * 2 + 1, customModelTextureCoordinates[texturesIndices[counter] * 2], customModelTextureCoordinates[texturesIndices[counter] * 2 + 1]);
    //}

    //for (int counter = 0; counter < customModelTextureCoordinates.size(); counter += 2) {
    //    printf("texture %i: %f %f\n", counter, customModelTextureCoordinates[counter], customModelTextureCoordinates[counter + 1]);
    //}

    //for (int counter = 0; counter < customModelNormals.size(); counter += 3) {
    //    printf("normal %i: %f %f %f\n", counter, customModelNormals[counter], customModelNormals[counter + 1], customModelNormals[counter + 2]);
    //}

    ModelChanged = true;
    ModelSize = GetVerticesSize() / 3;

    if (bvhTree = std::make_shared<BvhTree>()) {
        bvhTree->Initialize(customModelVertices);
        //bvhTree.Initialize1(customModelVertices);
    }

    printf("customModelVerticesSize: %zi\n", customModelVertices.size());

    for (std::function<void()> updateFunction : updateFunctions) {
        updateFunction();
    }
}

void ModelLoader::Subscribe(std::function<void()> onUpdate) {
    updateFunctions.push_back(onUpdate);
}

ModelLoader::ModelLoader() :
ModelSize(36),
ModelChanged(false),
bvhTree(nullptr),
customModelVertices(),
customModelTextureCoordinates(),
customModelNormals(),
indices(),
vertices(),
texturesIndices(),
textureCoordinates(),
normalsIndices(),
normals(),
updateFunctions(){

}