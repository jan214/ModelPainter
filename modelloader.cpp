#include "modelloader.h"

#include <QStringList>

void ModelLoader::LoadModel(QTextStream& modelFileText) {
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
            int counter = 0;
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
                counter++;
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

    printf("customModelVerticesSize: %i\n", customModelVertices.size());
}

ModelLoader::ModelLoader() :
ModelSize(6),
ModelChanged(false),
customModelVertices(),
customModelTextureCoordinates(),
customModelNormals(),
indices(),
vertices(),
texturesIndices(),
textureCoordinates(),
normalsIndices(),
normals() {

}