#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#define _USE_MATH_DEFINES
#include <math.h>
#include <GL/glew.h>
//#include <OpenGL/gl3.h>   // The GL Header File
#include <GLFW/glfw3.h> // The GLFW header
#include <glm/glm.hpp> // GL Math library header
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> 
//#include <glm/gtc/quaternion.hpp>
//#include <glm/gtx/quaternion.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define BUFFER_OFFSET(i) ((char*)NULL + (i))

const GLfloat skyboxVertices[] = {
    // positions          
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
};

const GLuint skyboxIndices[] = {
	0,  1,  2,
	3,  4,  5,

	6,  7,  8,
	9,  10, 11,
	
	12, 13, 14,
	15, 16, 17,

	18, 19, 20,
	21, 22, 23,
	
	24, 25, 26,
	27, 28, 29,

	30, 31, 32,
	33, 34, 35
};

using namespace std;

//0: body, 1: skybox
GLuint gProgram[10];
int gWidth, gHeight;

GLint modelingMatrixLoc[10];
GLint viewingMatrixLoc[10];
GLint projectionMatrixLoc[10];
GLint eyePosLoc[10];

glm::mat4 projectionMatrix[10];
glm::mat4 viewingMatrix[10];
glm::mat4 modelingMatrix[10];
glm::vec3 eyePos(0, 0, 0);

int activeProgramIndex = 0;

struct Vertex
{
	Vertex(GLfloat inX, GLfloat inY, GLfloat inZ) : x(inX), y(inY), z(inZ) { }
	GLfloat x, y, z;
};

struct Texture
{
	Texture(GLfloat inU, GLfloat inV) : u(inU), v(inV) { }
	GLfloat u, v;
};

struct Normal
{
	Normal(GLfloat inX, GLfloat inY, GLfloat inZ) : x(inX), y(inY), z(inZ) { }
	GLfloat x, y, z;
};

struct Face
{
	Face(int v[], int t[], int n[]) {
		vIndex[0] = v[0];
		vIndex[1] = v[1];
		vIndex[2] = v[2];
		tIndex[0] = t[0];
		tIndex[1] = t[1];
		tIndex[2] = t[2];
		nIndex[0] = n[0];
		nIndex[1] = n[1];
		nIndex[2] = n[2];
	}
	GLuint vIndex[3], tIndex[3], nIndex[3];
};

vector<Vertex> gVertices;
vector<Texture> gTextures;
vector<Normal> gNormals;
vector<Face> gFaces;

GLuint vao[10];

GLuint gBodyVertexBuffer;
GLuint gBodyIndexBuffer;
GLint gBodyFaceCount;

GLuint gSkyVertexBuffer;
GLuint gSkyIndexBuffer;
GLint skyTexLoc;
GLuint gTexCube;

GLuint gGroundVertexBuffer;
GLuint gGroundIndexBuffer;
GLint gGroundFaceCount;
GLint groundTexLoc;
GLuint gTexGround;

GLuint gStatueVertexBuffer;
GLuint gStatueIndexBuffer;
GLint gStatueFaceCount;

GLint gInVertexLoc, gInNormalLoc;
int skyboxBufferSize = 3 * 6 * 6;

GLuint CubeSampler;

GLfloat angle = 0;

bool ParseObj(const string& fileName)
{
	fstream myfile;

	gVertices.clear();
	gTextures.clear();
	gNormals.clear();
	gFaces.clear();
	
	// Open the input 
	myfile.open(fileName.c_str(), std::ios::in);

	if (myfile.is_open())
	{
		string curLine;

		while (getline(myfile, curLine))
		{
			stringstream str(curLine);
			GLfloat c1, c2, c3;
			GLuint index[9];
			string tmp;

			if (curLine.length() >= 2)
			{
				if (curLine[0] == 'v')
				{
					if (curLine[1] == 't') // texture
					{
						str >> tmp; // consume "vt"
						str >> c1 >> c2;
						gTextures.push_back(Texture(c1, c2));
					}
					else if (curLine[1] == 'n') // normal
					{
						str >> tmp; // consume "vn"
						str >> c1 >> c2 >> c3;
						gNormals.push_back(Normal(c1, c2, c3));
					}
					else // vertex
					{
						str >> tmp; // consume "v"
						str >> c1 >> c2 >> c3;
						gVertices.push_back(Vertex(c1, c2, c3));
					}
				}
				else if (curLine[0] == 'f') // face
				{
					str >> tmp; // consume "f"
					char c;
					int vIndex[3], nIndex[3], tIndex[3];
					str >> vIndex[0]; str >> c >> c; // consume "//"
					str >> nIndex[0];
					str >> vIndex[1]; str >> c >> c; // consume "//"
					str >> nIndex[1];
					str >> vIndex[2]; str >> c >> c; // consume "//"
					str >> nIndex[2];

					assert(vIndex[0] == nIndex[0] &&
						vIndex[1] == nIndex[1] &&
						vIndex[2] == nIndex[2]); // a limitation for now

					// make indices start from 0
					for (int c = 0; c < 3; ++c)
					{
						vIndex[c] -= 1;
						nIndex[c] -= 1;
						tIndex[c] -= 1;
					}

					gFaces.push_back(Face(vIndex, tIndex, nIndex));
				}
				else
				{
					cout << "Ignoring unidentified line in obj file: " << curLine << endl;
				}
			}

			//data += curLine;
			if (!myfile.eof())
			{
				//data += "\n";
			}
		}

		myfile.close();
	}
	else
	{
		return false;
	}

	assert(gVertices.size() == gNormals.size());

	return true;
}

bool ReadDataFromFile(
	const string& fileName, ///< [in]  Name of the shader file
	string& data)     ///< [out] The contents of the file
{
	fstream myfile;

	// Open the input 
	myfile.open(fileName.c_str(), std::ios::in);

	if (myfile.is_open())
	{
		string curLine;

		while (getline(myfile, curLine))
		{
			data += curLine;
			if (!myfile.eof())
			{
				data += "\n";
			}
		}

		myfile.close();
	}
	else
	{
		return false;
	}

	return true;
}

GLuint createVS(const char* shaderName)
{
	string shaderSource;

	string filename(shaderName);
	if (!ReadDataFromFile(filename, shaderSource))
	{
		cout << "Cannot find file name: " + filename << endl;
		exit(-1);
	}

	GLint length = shaderSource.length();
	const GLchar* shader = (const GLchar*)shaderSource.c_str();

	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &shader, &length);
	glCompileShader(vs);

	char output[1024] = { 0 };
	glGetShaderInfoLog(vs, 1024, &length, output);
	printf("VS compile log: %s\n", output);

	return vs;
}

GLuint createFS(const char* shaderName)
{
	string shaderSource;

	string filename(shaderName);
	if (!ReadDataFromFile(filename, shaderSource))
	{
		cout << "Cannot find file name: " + filename << endl;
		exit(-1);
	}

	GLint length = shaderSource.length();
	const GLchar* shader = (const GLchar*)shaderSource.c_str();

	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &shader, &length);
	glCompileShader(fs);

	char output[1024] = { 0 };
	glGetShaderInfoLog(fs, 1024, &length, output);
	printf("FS compile log: %s\n", output);

	return fs;
}

void initShaders()
{
	// Create the programs

	gProgram[0] = glCreateProgram();
	gProgram[1] = glCreateProgram();
	gProgram[2] = glCreateProgram();
	gProgram[3] = glCreateProgram();

	// Create the shaders for both programs

	GLuint vs1 = createVS("shaders/sky_vert.glsl");
	GLuint fs1 = createFS("shaders/sky_frag.glsl");

	GLuint vs2 = createVS("shaders/body_vert.glsl");
	GLuint fs2 = createFS("shaders/body_frag.glsl");

	GLuint vs3 = createVS("shaders/ground_vert.glsl");
	GLuint fs3 = createFS("shaders/ground_frag.glsl");

	GLuint vs4 = createVS("shaders/statue_vert.glsl");
	GLuint fs4 = createFS("shaders/statue_frag.glsl");

	// Attach the shaders to the programs

	glAttachShader(gProgram[0], vs1);
	glAttachShader(gProgram[0], fs1);

	glAttachShader(gProgram[1], vs2);
	glAttachShader(gProgram[1], fs2);

	glAttachShader(gProgram[2], vs3);
	glAttachShader(gProgram[2], fs3);

	glAttachShader(gProgram[3], vs4);
	glAttachShader(gProgram[3], fs4);

	// Link the programs

	GLint status;

	glLinkProgram(gProgram[0]);
	glGetProgramiv(gProgram[0], GL_LINK_STATUS, &status);

	if (status != GL_TRUE)
	{
		cout << "Program link failed" << endl;
		exit(-1);
	}

	glLinkProgram(gProgram[1]);
	glGetProgramiv(gProgram[1], GL_LINK_STATUS, &status);

	if (status != GL_TRUE)
	{
		cout << "Program link failed" << endl;
		exit(-1);
	}

	glLinkProgram(gProgram[2]);
	glGetProgramiv(gProgram[2], GL_LINK_STATUS, &status);

	if (status != GL_TRUE)
	{
		cout << "Program link failed" << endl;
		exit(-1);
	}

	glLinkProgram(gProgram[3]);
	glGetProgramiv(gProgram[3], GL_LINK_STATUS, &status);

	if (status != GL_TRUE)
	{
		cout << "Program link failed" << endl;
		exit(-1);
	}

	// Get the locations of the uniform variables from both programs

	for (int i = 0; i < 4; ++i)
	{
		modelingMatrixLoc[i] = glGetUniformLocation(gProgram[i], "modelingMatrix");
		viewingMatrixLoc[i] = glGetUniformLocation(gProgram[i], "viewingMatrix");
		projectionMatrixLoc[i] = glGetUniformLocation(gProgram[i], "projectionMatrix");
		eyePosLoc[i] = glGetUniformLocation(gProgram[i], "eyePos");
	}

	groundTexLoc = glGetUniformLocation(gProgram[2], "groundTex");
	skyTexLoc = glGetUniformLocation(gProgram[0], "skyTex");
}

void initObj(GLuint &vertexBuffer, GLuint &indexBuffer, const std::string &fileName, int vaoIndex, GLint &FaceCount)
{
	ParseObj(fileName);

	glBindVertexArray(vao[vaoIndex]);
	cout << "vao = " << vao[vaoIndex] << endl;

	assert(glGetError() == GL_NONE);

	glGenBuffers(1, &vertexBuffer);
	glGenBuffers(1, &indexBuffer);

	assert(vertexBuffer > 0 && indexBuffer > 0);

	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);

	int gVertexDataSizeInBytes, gNormalDataSizeInBytes;
	gVertexDataSizeInBytes = gVertices.size() * 3 * sizeof(GLfloat);
	gNormalDataSizeInBytes = gNormals.size() * 3 * sizeof(GLfloat);
	int indexDataSizeInBytes = gFaces.size() * 3 * sizeof(GLuint);
	GLfloat* vertexData = new GLfloat[gVertices.size() * 3];
	GLfloat* normalData = new GLfloat[gNormals.size() * 3];
	GLuint* indexData = new GLuint[gFaces.size() * 3];

	float minX = 1e6, maxX = -1e6;
	float minY = 1e6, maxY = -1e6;
	float minZ = 1e6, maxZ = -1e6;

	for (int i = 0; i < gVertices.size(); ++i)
	{
		vertexData[3 * i] = gVertices[i].x;
		vertexData[3 * i + 1] = gVertices[i].y;
		vertexData[3 * i + 2] = gVertices[i].z;

		minX = std::min(minX, gVertices[i].x);
		maxX = std::max(maxX, gVertices[i].x);
		minY = std::min(minY, gVertices[i].y);
		maxY = std::max(maxY, gVertices[i].y);
		minZ = std::min(minZ, gVertices[i].z);
		maxZ = std::max(maxZ, gVertices[i].z);
	}

	std::cout << "minX = " << minX << std::endl;
	std::cout << "maxX = " << maxX << std::endl;
	std::cout << "minY = " << minY << std::endl;
	std::cout << "maxY = " << maxY << std::endl;
	std::cout << "minZ = " << minZ << std::endl;
	std::cout << "maxZ = " << maxZ << std::endl;

	for (int i = 0; i < gNormals.size(); ++i)
	{
		normalData[3 * i] = gNormals[i].x;
		normalData[3 * i + 1] = gNormals[i].y;
		normalData[3 * i + 2] = gNormals[i].z;
	}

	for (int i = 0; i < gFaces.size(); ++i)
	{
		indexData[3 * i] = gFaces[i].vIndex[0];
		indexData[3 * i + 1] = gFaces[i].vIndex[1];
		indexData[3 * i + 2] = gFaces[i].vIndex[2];
	}


	glBufferData(GL_ARRAY_BUFFER, gVertexDataSizeInBytes + gNormalDataSizeInBytes, 0, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, gVertexDataSizeInBytes, vertexData);
	glBufferSubData(GL_ARRAY_BUFFER, gVertexDataSizeInBytes, gNormalDataSizeInBytes, normalData);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexDataSizeInBytes, indexData, GL_STATIC_DRAW);

	// done copying; can free now
	delete[] vertexData;
	delete[] normalData;
	delete[] indexData;

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(gVertexDataSizeInBytes));

	FaceCount = gFaces.size();
}

void initVBO()
{
	glGenVertexArrays(10, &vao[0]);
	assert(vao[0] > 0);

	////////////////////////////////////////////////////////////////

	glBindVertexArray(vao[0]);
	cout << "vao[0] = " << vao[0] << endl;

	assert(glGetError() == GL_NONE);

	glGenBuffers(1, &gSkyVertexBuffer);
	glGenBuffers(1, &gSkyIndexBuffer);

	assert(&gSkyVertexBuffer > 0 && &gSkyIndexBuffer > 0);

	glBindBuffer(GL_ARRAY_BUFFER, gSkyVertexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gSkyIndexBuffer);

	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(skyboxIndices), &skyboxIndices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid *)0);

	/////////////////////////////////////////////////////////////////

	initObj(gBodyVertexBuffer, gBodyIndexBuffer, "hw2_support_files/obj/cybertruck/cybertruck_body.obj", 1, gBodyFaceCount);

	///////////////////////////////////////

	initObj(gGroundVertexBuffer, gGroundIndexBuffer, "hw2_support_files/obj/ground.obj", 2, gGroundFaceCount);

	////////////////////////////////////////

	initObj(gStatueVertexBuffer, gStatueIndexBuffer, "hw2_support_files/obj/armadillo.obj", 3, gStatueFaceCount);

}

void initTextures()
{
	int width, height, nrChannels;
	unsigned char *data;

	glGenTextures(1, &gTexCube);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, gTexCube);

	const char* skybox_images[] = {"hw2_support_files/skybox_texture_abandoned_village/right.png", 
							"hw2_support_files/skybox_texture_abandoned_village/left.png", 
							"hw2_support_files/skybox_texture_abandoned_village/top.png", 
							"hw2_support_files/skybox_texture_abandoned_village/bottom.png", 
							"hw2_support_files/skybox_texture_abandoned_village/front.png", 
							"hw2_support_files/skybox_texture_abandoned_village/back.png"};

	//const char* skybox_images[] = {"hw2_support_files/skybox_texture_test/right.jpg", 
	//						"hw2_support_files/skybox_texture_test/left.jpg", 
	//						"hw2_support_files/skybox_texture_test/top.jpg", 
	//						"hw2_support_files/skybox_texture_test/bottom.jpg", 
	//						"hw2_support_files/skybox_texture_test/front.jpg", 
	//						"hw2_support_files/skybox_texture_test/back.jpg"};

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	//stbi_set_flip_vertically_on_load(true);
	for (int i = 0; i < 6; i ++)
	{
		data = stbi_load(skybox_images[i], &width, &height, &nrChannels, 0);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		stbi_image_free(data);
	}

	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	glGenSamplers(1, &CubeSampler);
	glSamplerParameteri(CubeSampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glSamplerParameteri(CubeSampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glSamplerParameteri(CubeSampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glSamplerParameteri(CubeSampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glSamplerParameteri(CubeSampler, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glBindSampler(0, CubeSampler);

	/////////////////////////////////////

	glGenTextures(1, &gTexGround);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gTexGround);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	const char* ground_image = "hw2_support_files/ground_texture_sand.jpg";

	data = stbi_load(ground_image, &width, &height, &nrChannels, 0);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
}

void init()
{
	glEnable(GL_DEPTH_TEST);
	initShaders();
	initVBO();
	initTextures();
}

void setShader(int index)
{
	glUseProgram(gProgram[index]);
	glUniformMatrix4fv(projectionMatrixLoc[index], 1, GL_FALSE, glm::value_ptr(projectionMatrix[index]));
	glUniformMatrix4fv(viewingMatrixLoc[index], 1, GL_FALSE, glm::value_ptr(viewingMatrix[index]));
	glUniformMatrix4fv(modelingMatrixLoc[index], 1, GL_FALSE, glm::value_ptr(modelingMatrix[index]));
	glUniform3fv(eyePosLoc[index], 1, glm::value_ptr(eyePos));

	switch (index)
	{
	case 0:
		glUniform1i(skyTexLoc, 0);
		break;
	
	case 2:
		glUniform1i(groundTexLoc, 1);
		break;
	}
}

void display()
{

	float angleRad = (float)(angle / 180.0) * M_PI;

	glClearColor(0, 0, 0, 1);
	glClearDepth(1.0f);
	glClearStencil(0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	for (int i = 0; i < 10; i ++)
	{
		viewingMatrix[i] = glm::lookAt(eyePos, eyePos + glm::vec3(1.0f * glm::sin(angleRad), 0.0f, 1.0f * glm::cos(angleRad)), glm::vec3(0.0f, 1.0f, 0.0f));
	}
	// Skybox
	//viewingMatrix[0] = glm::mat4(1);
	//viewingMatrix[0] = glm::lookAt(eyePos, eyePos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	modelingMatrix[0] = glm::mat4(1.0f);

	// Car body
	//viewingMatrix[1] = glm::lookAt(eyePos, eyePos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 matT = glm::translate(glm::mat4(1.0), glm::vec3(-0.1f, -0.2f, -7.0f));
	glm::mat4 matRx = glm::rotate<float>(glm::mat4(1.0), (0. / 180.) * M_PI, glm::vec3(1.0, 0.0, 0.0));
	glm::mat4 matRy = glm::rotate<float>(glm::mat4(1.0), (-180. / 180.) * M_PI, glm::vec3(0.0, 1.0, 0.0));
	glm::mat4 matRz = glm::rotate<float>(glm::mat4(1.0), angleRad, glm::vec3(0.0, 0.0, 1.0));
	modelingMatrix[1] = matT * matRz * matRy * matRx;

	// Ground
	//viewingMatrix[2] = glm::mat4(1);
	//viewingMatrix[2] = glm::lookAt(eyePos, eyePos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 matS = glm::scale(glm::mat4(1.0), glm::vec3(100.0f, 100.0f, 100.0f));
	matT = glm::translate(glm::mat4(1.0), glm::vec3(0.0f, -2.0f, 0.0f));
	matRx = glm::rotate<float>(glm::mat4(1.0), (90. / 180.) * M_PI, glm::vec3(1.0, 0.0, 0.0));
	modelingMatrix[2] = matT * matRx * matS;
	//modelingMatrix[2] = glm::mat4(1.0f);

	// Statue
	matT = glm::translate(glm::mat4(1.0), glm::vec3(0.0f, 0.0f, 5.0f));
	modelingMatrix[3] = matT;

	// Skybox
	glDepthMask(GL_FALSE);
	//glDepthFunc(GL_LEQUAL);
	setShader(0);
	glBindVertexArray(vao[0]);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, gTexCube);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gSkyIndexBuffer);
	//glDrawElements(GL_TRIANGLES, 12 * 3, GL_UNSIGNED_INT, 0);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	//glDepthFunc(GL_LESS);
	glDepthMask(GL_TRUE);

	// Car body
	setShader(1);
	glBindVertexArray(vao[1]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gBodyIndexBuffer);
	//glDrawElements(GL_TRIANGLES, gBodyFaceCount * 3, GL_UNSIGNED_INT, 0);

	// Ground
	setShader(2);
	glBindVertexArray(vao[2]);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gTexGround);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gGroundIndexBuffer);
	glDrawElements(GL_TRIANGLES, gGroundFaceCount * 3, GL_UNSIGNED_INT, 0);

	// Statue
	setShader(3);
	glBindVertexArray(vao[3]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gStatueIndexBuffer);
	glDrawElements(GL_TRIANGLES, gStatueFaceCount * 3, GL_UNSIGNED_INT, 0);
}

void reshape(GLFWwindow* window, int w, int h)
{
	w = w < 1 ? 1 : w;
	h = h < 1 ? 1 : h;

	gWidth = w;
	gHeight = h;

	glViewport(0, 0, w, h);

	//glMatrixMode(GL_PROJECTION);
	//glLoadIdentity();
	//glOrtho(-10, 10, -10, 10, -10, 10);
	//gluPerspective(45, 1, 1, 100);

	// Use perspective projection

	float fovyRad = (float)(60.0 / 180.0) * M_PI;
	//fovyRad = (float)(80.0 / 180.0) * M_PI;
	for (int i = 0; i < 10; i ++)
	{
		projectionMatrix[i] = glm::perspective(fovyRad, w/(float) h, 0.1f, 100.0f);
	}

	// Assume default camera position and orientation (camera is at
	// (0, 0, 0) with looking at -z direction and its up vector pointing
	// at +y direction)

	//viewingMatrix[1] = glm::lookAt(eyePos, glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	//glm::mat4 view = glm::mat4(glm::mat3(camera.GetViewMatrix()));  

	//glMatrixMode(GL_MODELVIEW);
	//glLoadIdentity();
}

void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_A && action == GLFW_PRESS)
	{
		angle += 10;
	}
	else if (key == GLFW_KEY_D && action == GLFW_PRESS)
	{
		angle -= 10;
	} 
	else if (key == GLFW_KEY_Q && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}
	else if (key == GLFW_KEY_G && action == GLFW_PRESS)
	{
		//glShadeModel(GL_SMOOTH);
		activeProgramIndex = 0;
	}
	else if (key == GLFW_KEY_P && action == GLFW_PRESS)
	{
		//glShadeModel(GL_SMOOTH);
		activeProgramIndex = 1;
	}
	else if (key == GLFW_KEY_F && action == GLFW_PRESS)
	{
		//glShadeModel(GL_FLAT);
	}
}

void mainLoop(GLFWwindow* window)
{
	while (!glfwWindowShouldClose(window))
	{
		display();
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}

int main(int argc, char** argv)   // Create Main Function For Bringing It All Together
{
	GLFWwindow* window;
	if (!glfwInit())
	{
		exit(-1);
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	int width = 640, height = 480;
	window = glfwCreateWindow(width, height, "Simple Example", NULL, NULL);

	if (!window)
	{
		glfwTerminate();
		exit(-1);
	}

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	// Initialize GLEW to setup the OpenGL Function pointers
	if (GLEW_OK != glewInit())
	{
		std::cout << "Failed to initialize GLEW" << std::endl;
		return EXIT_FAILURE;
	}

	char rendererInfo[512] = { 0 };
	strcpy(rendererInfo, (const char*)glGetString(GL_RENDERER));
	strcat(rendererInfo, " - ");
	strcat(rendererInfo, (const char*)glGetString(GL_VERSION));
	glfwSetWindowTitle(window, rendererInfo);

	init();

	glfwSetKeyCallback(window, keyboard);
	glfwSetWindowSizeCallback(window, reshape);

	reshape(window, width, height); // need to call this once ourselves
	mainLoop(window); // this does not return unless the window is closed

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}
