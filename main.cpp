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
int gWidth = 1280, gHeight = 800;
int dynDim = 600;

GLint modelingMatrixLoc[10];
GLint viewingMatrixLoc[10];
GLint projectionMatrixLoc[10];
GLint eyePosLoc[10];

glm::mat4 projectionMatrix;
glm::mat4 viewingMatrix[10];
glm::mat4 modelingMatrix[10];
glm::vec3 skyboxEyePos = glm::vec3(0, 0, 0);
glm::vec3 eyePos;

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
GLuint fbo;
GLuint rbo;
GLuint dynTex;
GLint dynTexLoc;

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

GLuint gTireVertexBuffer;
GLuint gTireIndexBuffer;
GLint gTireFaceCount;

GLuint gWindowVertexBuffer;
GLuint gWindowIndexBuffer;
GLint gWindowFaceCount;

GLint gInVertexLoc, gInNormalLoc;
int skyboxBufferSize = 3 * 6 * 6;

GLuint CubeSampler;

GLfloat angle = 0;
GLfloat carPosx = 0;
GLfloat carPosz = 10;
GLfloat speed = 0;
GLfloat tireRot = 0;

bool angleInc = false;
bool angleDec = false;
bool speedInc = false;
bool speedDec = false;

enum CameraDir {BACK, FRONT, LEFT, RIGHT};
CameraDir camDir = BACK;

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

	gProgram[0] = glCreateProgram(); // Skybox
	gProgram[1] = glCreateProgram(); // Body
	gProgram[2] = glCreateProgram(); // Ground
	gProgram[3] = glCreateProgram(); // Statue
	gProgram[4] = glCreateProgram(); // Tires
	gProgram[5] = glCreateProgram(); // Windows

	// Create the shaders for both programs

	GLuint vs0 = createVS("shaders/sky_vert.glsl");
	GLuint fs0 = createFS("shaders/sky_frag.glsl");

	GLuint vs1 = createVS("shaders/body_vert.glsl");
	GLuint fs1 = createFS("shaders/body_frag.glsl");

	GLuint vs2 = createVS("shaders/ground_vert.glsl");
	GLuint fs2 = createFS("shaders/ground_frag.glsl");

	GLuint vs3 = createVS("shaders/statue_vert.glsl");
	GLuint fs3 = createFS("shaders/statue_frag.glsl");

	GLuint vs4 = createVS("shaders/tire_vert.glsl");
	GLuint fs4 = createFS("shaders/tire_frag.glsl");

	GLuint vs5 = createVS("shaders/window_vert.glsl");
	GLuint fs5 = createFS("shaders/window_frag.glsl");

	// Attach the shaders to the programs

	glAttachShader(gProgram[0], vs0);
	glAttachShader(gProgram[0], fs0);

	glAttachShader(gProgram[1], vs1);
	glAttachShader(gProgram[1], fs1);

	glAttachShader(gProgram[2], vs2);
	glAttachShader(gProgram[2], fs2);

	glAttachShader(gProgram[3], vs3);
	glAttachShader(gProgram[3], fs3);

	glAttachShader(gProgram[4], vs4);
	glAttachShader(gProgram[4], fs4);

	glAttachShader(gProgram[5], vs5);
	glAttachShader(gProgram[5], fs5);

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

	glLinkProgram(gProgram[4]);
	glGetProgramiv(gProgram[4], GL_LINK_STATUS, &status);

	if (status != GL_TRUE)
	{
		cout << "Program link failed" << endl;
		exit(-1);
	}

	glLinkProgram(gProgram[5]);
	glGetProgramiv(gProgram[5], GL_LINK_STATUS, &status);

	if (status != GL_TRUE)
	{
		cout << "Program link failed" << endl;
		exit(-1);
	}


	// Get the locations of the uniform variables from both programs

	for (int i = 0; i < 6; ++ i)
	{
		modelingMatrixLoc[i] = glGetUniformLocation(gProgram[i], "modelingMatrix");
		viewingMatrixLoc[i] = glGetUniformLocation(gProgram[i], "viewingMatrix");
		projectionMatrixLoc[i] = glGetUniformLocation(gProgram[i], "projectionMatrix");
		eyePosLoc[i] = glGetUniformLocation(gProgram[i], "eyePos");
	}

	groundTexLoc = glGetUniformLocation(gProgram[2], "groundTex");
	skyTexLoc = glGetUniformLocation(gProgram[0], "skyTex");
	dynTexLoc = glGetUniformLocation(gProgram[1], "dynTex");
	//skyTexLoc = glGetUniformLocation(gProgram[5], "dynTex");
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

	glGenFramebuffers(1, &fbo);
	glGenRenderbuffers(1, &rbo);

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

	///////////////////////////////////////

	initObj(gBodyVertexBuffer, gBodyIndexBuffer, "hw2_support_files/obj/cybertruck/cybertruck_body.obj", 1, gBodyFaceCount);

	///////////////////////////////////////

	initObj(gGroundVertexBuffer, gGroundIndexBuffer, "hw2_support_files/obj/ground.obj", 2, gGroundFaceCount);

	////////////////////////////////////////

	initObj(gStatueVertexBuffer, gStatueIndexBuffer, "hw2_support_files/obj/armadillo.obj", 3, gStatueFaceCount);

	////////////////////////////////////////

	initObj(gTireVertexBuffer, gTireIndexBuffer, "hw2_support_files/obj/cybertruck/cybertruck_tires.obj", 4, gTireFaceCount);

	////////////////////////////////////////

	initObj(gWindowVertexBuffer, gWindowIndexBuffer, "hw2_support_files/obj/cybertruck/cybertruck_windows.obj", 5, gWindowFaceCount);

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

	//////////////////////////////////////////////////

	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glGenTextures(1, &dynTex);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_CUBE_MAP, dynTex);

	for (int i = 0; i < 6; i ++)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB8, dynDim, dynDim, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	}

	//glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	//glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	//glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glBindSampler(2, CubeSampler);

    //glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, dynTex, 0);

	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, dynDim, dynDim);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << endl;
	}

	GLenum drawBufs[] = {GL_COLOR_ATTACHMENT0};
	glDrawBuffers(1, drawBufs);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
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
	glUniformMatrix4fv(projectionMatrixLoc[index], 1, GL_FALSE, glm::value_ptr(projectionMatrix));
	glUniformMatrix4fv(viewingMatrixLoc[index], 1, GL_FALSE, glm::value_ptr(viewingMatrix[index]));
	glUniformMatrix4fv(modelingMatrixLoc[index], 1, GL_FALSE, glm::value_ptr(modelingMatrix[index]));

	switch (index)
	{
	case 0:
		glUniform3fv(eyePosLoc[index], 1, glm::value_ptr(skyboxEyePos));
		glUniform1i(skyTexLoc, 0);
		break;
	
	case 1:
		glUniform3fv(eyePosLoc[index], 1, glm::value_ptr(eyePos));
		glUniform1i(dynTexLoc, 2);
		break;
	case 2:
		glUniform3fv(eyePosLoc[index], 1, glm::value_ptr(eyePos));
		glUniform1i(groundTexLoc, 1);
		break;

	case 5:
		glUniform3fv(eyePosLoc[index], 1, glm::value_ptr(eyePos));
		glUniform1i(dynTexLoc, 2);
		break;

	default:
		glUniform3fv(eyePosLoc[index], 1, glm::value_ptr(eyePos));
		break;
	}
}

void display()
{
	if (speedInc /*&& speed < 0.04*/)
	{
		speed += 0.0004;
	}
	if (speedDec /*&& speed > -0.04*/)
	{
		speed -= 0.0004;
	}
	if (angleInc)
	{
		angle += 0.15;
	}
	if (angleDec)
	{
		angle -= 0.15;
	}

	float angleRad = (float)(angle / 180.0) * M_PI;
	float tireRotRad = (float)(tireRot / 180.0) * M_PI;

	//tireRot += 0.5;

	carPosx += speed * glm::sin(angleRad);
	carPosz += speed * glm::cos(angleRad);

	glClearColor(0, 0, 0, 1);
	glClearDepth(1.0f);
	glClearStencil(0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	glm::mat4 matT;
	glm::mat4 matS;
	glm::mat4 matRx;
	glm::mat4 matRy;
	glm::mat4 matRz;

	// Skybox
	//viewingMatrix[0] = glm::mat4(1);
	//viewingMatrix[0] = glm::lookAt(eyePos, eyePos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	matRy = glm::rotate<float>(glm::mat4(1.0), -angleRad, glm::vec3(0.0, 1.0, 0.0));
	modelingMatrix[0] = matRy;

	// Car body
	//viewingMatrix[1] = glm::lookAt(eyePos, eyePos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	matT = glm::translate(glm::mat4(1.0), glm::vec3(carPosx, 1.0f, carPosz));
	matRx = glm::rotate<float>(glm::mat4(1.0), (0. / 180.) * M_PI, glm::vec3(1.0, 0.0, 0.0));
	matRy = glm::rotate<float>(glm::mat4(1.0), angleRad, glm::vec3(0.0, 1.0, 0.0));
	matRz = glm::rotate<float>(glm::mat4(1.0), (0. / 180.) * M_PI, glm::vec3(0.0, 0.0, 1.0));
	modelingMatrix[1] = matT * matRz * matRy * matRx;

	// Ground
	//viewingMatrix[2] = glm::mat4(1);
	//viewingMatrix[2] = glm::lookAt(eyePos, eyePos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	matS = glm::scale(glm::mat4(1.0), glm::vec3(200.0f, 200.0f, 200.0f));
	matT = glm::translate(glm::mat4(1.0), glm::vec3(0.0f, 0.0f, 0.0f));
	matRx = glm::rotate<float>(glm::mat4(1.0), (90. / 180.) * M_PI, glm::vec3(1.0, 0.0, 0.0));
	modelingMatrix[2] = matT * matRx * matS;
	//modelingMatrix[2] = glm::mat4(1.0f);

	// Statue
	matS = glm::scale(glm::mat4(1.0), glm::vec3(4.0f, 4.0f, 4.0f));
	matT = glm::translate(glm::mat4(1.0), glm::vec3(5.0f, 4.0f, 15.0f));
	modelingMatrix[3] = matT * matS;

	// Tires
	//viewingMatrix[1] = glm::lookAt(eyePos, eyePos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	//glm::mat4 matW0 = glm::translate(glm::mat4(1.0), glm::vec3(0.0f, 0.37f, -2.585f));
	//glm::mat4 matW1 = glm::translate(glm::mat4(1.0), glm::vec3(0.0f, -0.37f, 2.585f));
	matT = glm::translate(glm::mat4(1.0), glm::vec3(carPosx, 1.0f, carPosz));
	matRx = glm::rotate<float>(glm::mat4(1.0), tireRotRad, glm::vec3(1.0, 0.0, 0.0));
	matRy = glm::rotate<float>(glm::mat4(1.0), angleRad, glm::vec3(0.0, 1.0, 0.0));
	matRz = glm::rotate<float>(glm::mat4(1.0), (0. / 180.) * M_PI, glm::vec3(0.0, 0.0, 1.0));
	//modelingMatrix[4] = matT * matW1 * matRz * matRy * matRx * matW0;
	modelingMatrix[4] = matT * matRz * matRy * matRx;

	// Windows
	//viewingMatrix[1] = glm::lookAt(eyePos, eyePos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	matT = glm::translate(glm::mat4(1.0), glm::vec3(carPosx, 1.0f, carPosz));
	matRx = glm::rotate<float>(glm::mat4(1.0), (0. / 180.) * M_PI, glm::vec3(1.0, 0.0, 0.0));
	matRy = glm::rotate<float>(glm::mat4(1.0), angleRad, glm::vec3(0.0, 1.0, 0.0));
	matRz = glm::rotate<float>(glm::mat4(1.0), (0. / 180.) * M_PI, glm::vec3(0.0, 0.0, 1.0));
	modelingMatrix[5] = matT * matRz * matRy * matRx;

	float fovyRad = (float)(90.0 / 180.0) * M_PI;

	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	glViewport(0, 0, dynDim, dynDim);
	projectionMatrix = glm::perspective(fovyRad, 1.0f, 0.1f, 100.0f);
	eyePos = glm::vec3(carPosx, 1.0f, carPosz);
	glm::vec3 look;
	for (int i = 0; i < 6; i ++)
	{
		glClearColor(0, 0, 0, 1);
		glClearDepth(1.0f);
		glClearStencil(0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		switch (i)
		{
		case 0:
			look = glm::vec3(1, 0, 0);
			break;
		
		case 1:
			look = glm::vec3(-1, 0, 0);
			break;
		
		case 2:
			look = glm::vec3(0, 1, 0);
			break;
		
		case 3:
			look = glm::vec3(0, -1, 0);
			break;
		
		case 4:
			look = glm::vec3(0, 0, 1);
			break;
		
		case 5:
			look = glm::vec3(0, 0, -1);
			break;
		}

		for (int i = 0; i < 10; i ++)
		{
			//viewingMatrix[i] = glm::lookAt(eyePos[i], eyePos[i] + glm::vec3(1.0f * glm::sin(angleRad), 0.0f, 1.0f * glm::cos(angleRad)), glm::vec3(0.0f, 1.0f, 0.0f));
			if (i == 0)
			{
				viewingMatrix[i] = glm::lookAt(skyboxEyePos, look, glm::vec3(0.0f, 1.0f, 0.0f));
			}
			else
			{
				viewingMatrix[i] = glm::lookAt(eyePos, look, glm::vec3(0.0f, 1.0f, 0.0f));
			}
		}

		glDepthMask(GL_FALSE);
		//glDepthFunc(GL_LEQUAL);
		setShader(0);
		glBindVertexArray(vao[0]);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, gTexCube);
		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gSkyIndexBuffer);
		//glDrawElements(GL_TRIANGLES, 12 * 3, GL_UNSIGNED_INT, 0);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		//glDepthFunc(GL_LESS);
		glDepthMask(GL_TRUE);

		// Ground
		setShader(2);
		glBindVertexArray(vao[2]);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, gTexGround);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gGroundIndexBuffer);
		//glDrawElements(GL_TRIANGLES, gGroundFaceCount * 3, GL_UNSIGNED_INT, 0);

		// Statue
		setShader(3);
		glBindVertexArray(vao[3]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gStatueIndexBuffer);
		//glDrawElements(GL_TRIANGLES, gStatueFaceCount * 3, GL_UNSIGNED_INT, 0);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, dynTex, 0);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	switch (camDir)
	{
		case BACK:
			eyePos = glm::vec3(carPosx - 6. * sin(angleRad), 3, carPosz - 6. * cos(angleRad));
			break;
		
		case FRONT:
			eyePos = glm::vec3(carPosx + 6. * sin(angleRad), 3, carPosz + 6. * cos(angleRad));
			break; 

		case LEFT:
			eyePos = glm::vec3(carPosx + 6. * cos(angleRad), 3, carPosz - 6. * sin(angleRad));
			break;

		case RIGHT:
			eyePos = glm::vec3(carPosx - 6. * cos(angleRad), 3, carPosz + 6. * sin(angleRad));
			break;
	}

	glViewport(0, 0, gWidth, gHeight);
	projectionMatrix = glm::perspective(fovyRad, gWidth/(float) gHeight, 0.1f, 100.0f);

	for (int i = 0; i < 10; i ++)
	{
		//viewingMatrix[i] = glm::lookAt(eyePos[i], eyePos[i] + glm::vec3(1.0f * glm::sin(angleRad), 0.0f, 1.0f * glm::cos(angleRad)), glm::vec3(0.0f, 1.0f, 0.0f));
		if (i == 0)
		{
			viewingMatrix[i] = glm::lookAt(skyboxEyePos, skyboxEyePos + glm::vec3(0.0f, -0.5f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		}
		else
		{
			viewingMatrix[i] = glm::lookAt(eyePos, glm::vec3(carPosx, 0.0f, carPosz), glm::vec3(0.0f, 1.0f, 0.0f));
		}
	}

	// Skybox
	glDepthMask(GL_FALSE);
	//glDepthFunc(GL_LEQUAL);
	setShader(0);
	glBindVertexArray(vao[0]);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, gTexCube);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gSkyIndexBuffer);
	//glDrawElements(GL_TRIANGLES, 12 * 3, GL_UNSIGNED_INT, 0);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	//glDepthFunc(GL_LESS);
	glDepthMask(GL_TRUE);

	// Car body
	setShader(1);
	glBindVertexArray(vao[1]);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, dynTex);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gBodyIndexBuffer);
	glDrawElements(GL_TRIANGLES, gBodyFaceCount * 3, GL_UNSIGNED_INT, 0);

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

	// Tires
	setShader(4);
	glBindVertexArray(vao[4]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gTireIndexBuffer);
	glDrawElements(GL_TRIANGLES, gTireFaceCount * 3, GL_UNSIGNED_INT, 0);

	// Windows
	setShader(5);
	glBindVertexArray(vao[5]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gWindowIndexBuffer);
	glDrawElements(GL_TRIANGLES, gWindowFaceCount * 3, GL_UNSIGNED_INT, 0);
}

void reshape(GLFWwindow* window, int w, int h)
{
	w = w < 1 ? 1 : w;
	h = h < 1 ? 1 : h;

	gWidth = w;
	gHeight = h;
}

void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_A && action == GLFW_PRESS)
	{
		//angle += 10;
		angleInc = true;
	}
	else if (key == GLFW_KEY_D && action == GLFW_PRESS)
	{
		//angle -= 10;
		angleDec = true;
	}
	else if (key == GLFW_KEY_W && action == GLFW_PRESS)
	{
		//speed += 0.01;
		speedInc = true;
	} 
	else if (key == GLFW_KEY_S && action == GLFW_PRESS)
	{
		//speed -= 0.01;
		speedDec = true;
	}
	if (key == GLFW_KEY_A && action == GLFW_RELEASE)
	{
		//angle += 10;
		angleInc = false;
	}
	else if (key == GLFW_KEY_D && action == GLFW_RELEASE)
	{
		//angle -= 10;
		angleDec = false;
	}
	else if (key == GLFW_KEY_W && action == GLFW_RELEASE)
	{
		//speed += 0.01;
		speedInc = false;
	} 
	else if (key == GLFW_KEY_S && action == GLFW_RELEASE)
	{
		//speed -= 0.01;
		speedDec = false;
	}
	
	else if (key == GLFW_KEY_Q && action == GLFW_PRESS)
	{
		camDir = LEFT;
	}
	else if (key == GLFW_KEY_E && action == GLFW_PRESS)
	{
		camDir = RIGHT;
	}
	else if (key == GLFW_KEY_R && action == GLFW_PRESS)
	{
		camDir = BACK;
	}
	else if (key == GLFW_KEY_T && action == GLFW_PRESS)
	{
		camDir = FRONT;
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

	window = glfwCreateWindow(gWidth, gHeight, "Simple Example", NULL, NULL);

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

	reshape(window, gWidth, gHeight); // need to call this once ourselves
	mainLoop(window); // this does not return unless the window is closed

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}
