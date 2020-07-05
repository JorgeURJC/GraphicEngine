#include "Model.h"

Model::Model(std::string objectLoc, std::string vertexLoc, std::string fragmentLoc, glm::vec3 position, glm::vec3 scale, Camera* camera)
	: objectLoc(objectLoc), vertexLoc(vertexLoc), fragmentLoc(fragmentLoc),
	position(position), scale(scale),
	camera(camera)
{
	init();
}

void Model::render()
{
	shader->use();
	uploadUniforms();
	int i = 0;
	for (Mesh* mesh : meshes)
	{
		unsigned int textureIndex = meshToTex[i];

		if (textureIndex < textures.size() && textures[textureIndex])
		{
			textures[textureIndex]->use(0);
		}

		mesh->render();

		if (textureIndex < textures.size() && textures[textureIndex])
		{
			textures[textureIndex]->unuse();
		}

		i++;
	}
	shader->unuse();
}

Model::~Model()
{
	delete camera;
	delete shader;
	meshes.clear();
	textures.clear();
}

void Model::init()
{
	shader = new Shader(vertexLoc, fragmentLoc,
		DirectionalLight(glm::vec3(1.0f, 1.0f, 1.0f), 0.2f, glm::vec3(0.0f, 0.0f, -1.0f), 1.2f),
		SpecularLight(1.5f, 25.0f));

	model = glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), scale) /** glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f))*/;

	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(objectLoc,
		aiProcess_Triangulate |
		aiProcess_FlipUVs |
		aiProcess_GenSmoothNormals |
		aiProcess_JoinIdenticalVertices);

	if (!scene)
	{
		printf("Cannot load model %s: %s\n", objectLoc.c_str(), importer.GetErrorString());
		return;
	}

	aiNode* rootNode = scene->mRootNode;
	loadNode(rootNode, scene);
	loadTextures(scene);
}

void Model::loadNode(aiNode* node, const aiScene* scene)
{
	for (int i = 0; i < node->mNumMeshes; i++)
	{
		loadModel(scene->mMeshes[node->mMeshes[i]], scene);
	}
	for (int i = 0; i < node->mNumChildren; i++)
	{
		loadNode(node->mChildren[i], scene);
	}
}

void Model::loadModel(aiMesh* mesh, const aiScene* scene)
{
	std::vector<GLfloat> vertex;
	std::vector<unsigned int> indices;

	for (int i = 0; i < mesh->mNumVertices; i++)
	{
		vertex.push_back(mesh->mVertices[i].x);
		vertex.push_back(mesh->mVertices[i].y);
		vertex.push_back(mesh->mVertices[i].z);

		if (mesh->mTextureCoords[0])
		{
			vertex.push_back(mesh->mTextureCoords[0][i].x);
			vertex.push_back(mesh->mTextureCoords[0][i].y);
		}
		else
		{
			vertex.push_back(0.0f);
			vertex.push_back(0.0f);
		}

		if (mesh->mColors[0])
		{
			vertex.push_back(mesh->mColors[0][i].r);
			vertex.push_back(mesh->mColors[0][i].g);
			vertex.push_back(mesh->mColors[0][i].b);
		}
		else
		{
			vertex.push_back(0.0f);
			vertex.push_back(0.0f);
			vertex.push_back(0.0f);
		}

		vertex.push_back(mesh->mNormals[i].x);
		vertex.push_back(mesh->mNormals[i].y);
		vertex.push_back(mesh->mNormals[i].z);
	}

	for (int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for (int j = 0; j < face.mNumIndices; j++)
		{
			indices.push_back(face.mIndices[j]);
		}
	}

	Mesh* newMesh = new Mesh(&vertex[0],
		&indices[0],
		vertex.size(),
		indices.size());
	meshes.push_back(newMesh);
	// Map the texture to the mesh
	meshToTex.push_back(mesh->mMaterialIndex);
}

void Model::loadTextures(const aiScene* scene)
{
	textures.resize(scene->mNumMaterials);

	for (int i = 0; i < scene->mNumMaterials; i++)
	{
		textures[i] = nullptr;

		// Get the current material
		aiMaterial* material = scene->mMaterials[i];
		// Material name
		aiString materialName;

		// Get if the loading has been success
		aiReturn ret = material->Get(AI_MATKEY_NAME, materialName);
		if (ret != AI_SUCCESS) materialName = "";

		// Diffuse maps
		int numTextures = material->GetTextureCount(aiTextureType_DIFFUSE);
		aiString textureName;

		if (numTextures)
		{
			// Get the file name of the texture
			ret = material->Get(AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, 0), textureName);

			std::string name = textureName.data;

			if (name.substr(name.length() - 3, name.length() - 1) == "tif")
			{
				name = name.substr(0, name.length() - 3) + "png";
			}
			std::string realName = "";
			int p = name.length() - 1;
			while (p >= 0 && name[p] != '/' && name[p] != '\\')
			{
				realName.push_back(name[p]);
				p--;
			}
			std::reverse(realName.begin(), realName.end());
			std::string location = "../Textures/";
			std::string textureFileName = location + realName;

			textures[i] = new Texture(textureFileName);
		}
		else
		{
			textures[i] = new Texture("../Textures/pjan.png");
		}
	}
}

void Model::uploadUniforms()
{
	glUniformMatrix4fv(shader->getUniforms().modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(shader->getUniforms().viewLoc, 1, GL_FALSE, glm::value_ptr(camera->getViewMatrix()));
	glUniformMatrix4fv(shader->getUniforms().projLoc, 1, GL_FALSE, glm::value_ptr(camera->getProjectionMatrix()));
	glUniform3fv(shader->getUniforms().eyePositionLoc, 1, glm::value_ptr(camera->getPosition()));

	glUniform3fv(shader->getLightUniforms().colourLoc, 1, glm::value_ptr(shader->getPhongLight()->getDirectionalLight().colour));
	glUniform1f(shader->getLightUniforms().ambientIntensityLoc, shader->getPhongLight()->getDirectionalLight().ambientIntensity);
	glUniform3fv(shader->getLightUniforms().directionLoc, 1, glm::value_ptr(shader->getPhongLight()->getDirectionalLight().direction));
	glUniform1f(shader->getLightUniforms().diffuseIntensityLoc, shader->getPhongLight()->getDirectionalLight().diffuseIntensity);

	glUniform1f(shader->getLightUniforms().specularIntensity, shader->getPhongLight()->getSpecularLight().specularIntensity);
	glUniform1f(shader->getLightUniforms().specularPower, shader->getPhongLight()->getSpecularLight().specularPower);
}