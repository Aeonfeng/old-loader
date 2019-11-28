//model loader program Dom Ralphs November 2019
#include "GL/glew.h"
#include "GL/freeglut.h"
#include "GLFW/glfw3.h"
#include "LoadShaders.h"  
#include <glm/glm.hpp> //includes GML
#include <glm/ext/matrix_transform.hpp> // GLM: translate, rotate
#include <glm/ext/matrix_clip_space.hpp> // GLM: perspective and ortho 
#include <glm/gtc/type_ptr.hpp> // GLM: access to the value_ptr
#define STB_IMAGE_IMPLEMENTATION

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include "stb_image.h"



using namespace std;
using namespace glm;


#define KEY_DOWN(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 1 : 0) //define key down for keyboard input

enum VAO_IDs { object1, NumVAOs = 1 };  //name of game object is object1
enum Buffer_IDs { Triangles, Colours, Normals, Textures, Indices, NumBuffers = 5 };

GLuint  VAOs[NumVAOs];
GLuint  Buffers[NumBuffers];
GLuint texture1;

GLuint shader;

#define BUFFER_OFFSET(a) ((void*)(a))


//struct for storing the data from the file
struct Vertex
{
	vec3 position; //postion vector of 3 which is x,y z postion of vertices
	vec2 textcord; //textcord vector of 2 which is x,y  postion of textcords
	vec3 normal; //normals vector of 3 whihc is the x,y and z postion of normals
};

//struct for storing the temp data from the file
struct verTexRef
{
	verTexRef(int v, int vt, int vn) //vert ref for v = vertices, vt =  vertex textcords, vn = vertex normals
		: v(v), vt(vt), vn(vn) // used to store the data in each int.
	{ }
	int v, vt, vn;
};





vector< Vertex > LoadOBJ(istream& in) //class of vector, LoadOBJ struct for storing the data from the file
{

	vector< Vertex > verts; //set up a vector for the verts

	vector< vec4 > vertices(1, vec4(0, 0, 0, 0)); //set up the vertices as a vec of 4
	vector< vec3 > textcords(1, vec3(0, 0, 0)); // set up textcords as a vector of 3
	vector< vec3 > normals(1, vec3(0, 0, 0)); // set the normals as a vector of 3


	//read the obj file
	string line; //set line as a string
	while (getline(in, line)) //while there is data, read each line.
	{
		istringstream lineStuff(line); // set up the line as a string
		string lineType; //set up the linetype as a string, used for checking the first letter of the line.
		lineStuff >> lineType; //line into line type

		//check for vertex 
		if (lineType == "v") //check if the start of the line begains with a V
		{
			float vdx = 0, vdy = 0, vdz = 0, w = 1; //set up the floats to store each data cord in
			lineStuff >> vdx >> vdy >> vdz >> w;// add each cord data to the floats vdx = vertex data x
			vertices.push_back(vec4(vdx, vdy, vdz, w));//add the data of vdx,vdy,vdz, w into the vertices vector
		}

		//check for texture cords
		if (lineType == "vt")  //check if the start of the line begains with a vt
		{
			float tdx = 0, tdy = 0, tdz = 0; //set up the floats to store each text data cord in
			lineStuff >> tdx >> tdy >> tdz; // add each text cord data to the floats tdx = texture data x
			textcords.push_back(vec3(tdx, tdy, tdz));//add the data of vdx,vdy,vdz into the textcords vector
		}

		//check for normals cords
		if (lineType == "vn") //check if the start of the line begains with a vn
		{
			float ndx = 0, ndy = 0, ndz = 0; //set up the floats ndx = normal data x to store each normal data cord in
			lineStuff >> ndx >> ndy >> ndz; // add each text cord data to the floats
			normals.push_back(vec3(ndx, ndy, ndz)); //add the data of ndx,ndy,ndz into the normals vector
		}

		//check for faces
		if (lineType == "f") //check if the start of the line begains with a f
		{
			vector<verTexRef> FaceRefs; //set up a struct to store the refs
			string lineStRef; //set up a string called line string ref
			while (lineStuff >> lineStRef) //while there is data to read
			{
				istringstream faceRef(lineStRef); // set string object as a stream
				string verStr, textStr, norStr; //set up strings for vertext String, texture string, normal string to split the faces data up

				getline(faceRef, verStr, '/'); // split the faces up with / vertex string
				getline(faceRef, textStr, '/');  // split the faces up with / texture string
				getline(faceRef, norStr, '/');  // split the faces up with / normal string

				int v = atoi(verStr.c_str()); //copy verStr into a c-string and use atoi to convert to int
				int vt = atoi(textStr.c_str());//copy textStr into a c-string and use atoi to convert to int
				int vn = atoi(norStr.c_str());//copy norStr into a c-string and use atoi to convert to int

				v = (v >= 0 ? v : vertices.size() + v); //ternary operator, if v >= 0 then v // else  size of vertices + v
				vt = (vt >= 0 ? vt : textcords.size() + vt);//ternary operator, if vt >= 0 then vt // else  size of vertices + vt
				vn = (vn >= 0 ? vn : normals.size() + vn);//ternary operator, if vn >= 0 then vn // else  size of vertices + vn
				FaceRefs.push_back(verTexRef(v, vt, vn)); //add element v,vt and vn to end of vector verTexRef

			}

			if (FaceRefs.size() < 3) //check if face ref is less than 3 
			{
				//if its less than 3 it isn't in the correct format or isn't a face ref so skip
				printf("faces can't be read \n");
				continue;
			}

			// triangulate, assuming n>3 poloygons are convex (all interier angles are less than 180 degrees) and coplanar ()
			verTexRef* p[3] = { &FaceRefs[0],NULL,NULL }; //store data in p 
			for (size_t i = 1; i + 1 < FaceRefs.size(); ++i) //size_t unasigned int, for loop incrementimg before the expression evaluated 
			{
				p[1] = &FaceRefs[i + 0]; //point to the memory location of the first FaceRefs
				p[2] = &FaceRefs[i + 1];//point to the memory location of the second FaceRefs

				vec3 U(vertices[p[1]->v] - vertices[p[0]->v]); //add data to the vector
				vec3 V(vertices[p[2]->v] - vertices[p[0]->v]); //add data to the vector
				vec3 faceNormal = normalize(cross(U, V));  //check for crossed correlation between the vectors of U and V

				for (size_t j = 0; j < 3; ++j) //for loop to add the data
				{
					Vertex vert; //set vert in the vertex
					vert.position = vec3(vertices[p[j]->v]); //vert position = vector of 3
					vert.textcord = vec2(textcords[p[j]->vt]); //vert textcords = vector of 2
					vert.normal = (p[j]->vn != 0 ? normals[p[j]->vn] : faceNormal); //if p is less or grester than  not 0 then normals p is less or greater then vn i.e the plain is 0 else face normal.
					verts.push_back(vert); //add data from vert back to the verts.
				}
			}
		}
	}
	return verts;
}




void init(void)
{

	glGenVertexArrays(NumVAOs, VAOs); //generate vertext array
	glBindVertexArray(VAOs[object1]); //bind text array

	ShaderInfo  shaders[] =
	{
		{ GL_VERTEX_SHADER, "media/triangles.vert" },
		{ GL_FRAGMENT_SHADER, "media/triangles.frag" },
		{ GL_NONE, NULL }
	};

	shader = LoadShaders(shaders);
	glUseProgram(shader);


	//you can add lighting here ################################################# 


	//add colours 
	GLfloat  colours[][4] = { { 1.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f, 1.0f },
							  { 1.0f, 1.0f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f, 1.0f },
							  { 0.0f, 0.0f, 1.0f, 1.0f }, { 1.0f, 1.0f, 0.0f, 1.0f },
	};


	//read file and add data
	ifstream myFile("media/textures/Creeper-OBJ/Creeper.obj"); //file location of obj
	vector<Vertex> model = LoadOBJ(myFile);


	glGenBuffers(NumBuffers, Buffers);

	glBindBuffer(GL_ARRAY_BUFFER, Buffers[Triangles]);   //vertext buffer
	vector<GLfloat> temp_vert;
	vector<GLfloat> temp_norm; //temp vector storing the values
	vector<GLfloat> temp_tex;
	for (int i = 0; i < model.size(); i++) //for loop to read the data
	{
		temp_vert.push_back(model[i].position.x); //read the x postions of vertices
		temp_vert.push_back(model[i].position.y); //read the y postions of vertices
		temp_vert.push_back(model[i].position.z); //read the z postions of vertices

		temp_norm.push_back(model[i].normal.x); //read the x postions of normals
		temp_norm.push_back(model[i].normal.y); //read the y postions of normals
		temp_norm.push_back(model[i].normal.z); //read the z postions of normals

		temp_tex.push_back(model[i].textcord.x); //read the x postions of textcord
		temp_tex.push_back(model[i].textcord.y);//read the y postions of textcord



	}


	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Buffers[Indices]); //texture buffer
	glBufferData(GL_ARRAY_BUFFER, temp_vert.size() * sizeof(GLfloat), &temp_vert[0], GL_STATIC_DRAW);
	glVertexAttribPointer(Triangles, 3, GL_FLOAT,
		GL_FALSE, 0, BUFFER_OFFSET(0));

	//Colour Binding
	glBindBuffer(GL_ARRAY_BUFFER, Buffers[Colours]);
	glBufferStorage(GL_ARRAY_BUFFER, sizeof(colours), colours, 0);


	glVertexAttribPointer(Colours, 4, GL_FLOAT,
		GL_FALSE, 0, BUFFER_OFFSET(0));

	//Colour Binding
	glBindBuffer(GL_ARRAY_BUFFER, Buffers[Normals]);
	glBufferData(GL_ARRAY_BUFFER, temp_norm.size() * sizeof(GLfloat), &temp_norm[0], GL_STATIC_DRAW);

	glVertexAttribPointer(Normals, 3, GL_FLOAT,
		GL_FALSE, 0, BUFFER_OFFSET(0));

	//Texture Binding
	glBindBuffer(GL_ARRAY_BUFFER, Buffers[Textures]);
	glBufferData(GL_ARRAY_BUFFER, temp_tex.size() * sizeof(GLfloat), &temp_tex[0], GL_STATIC_DRAW);

	glVertexAttribPointer(Textures, 2, GL_FLOAT,
		GL_FALSE, 0, BUFFER_OFFSET(0));

	// load and create a texture 
	// -------------------------

	// texture 1
	// ---------
	glGenTextures(1, &texture1);
	glBindTexture(GL_TEXTURE_2D, texture1);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load image, create texture and generate mipmaps
	GLint width, height, nrChannels;
	stbi_set_flip_vertically_on_load(false); // tell stb_image.h to flip loaded texture's on the y-axis.
	unsigned char* data = stbi_load("media/textures/Creeper-OBJ/Texture.png", &width, &height, &nrChannels, 0); //file path for texture
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);


	glUniform1i(glGetUniformLocation(shader, "texture1"), 0);

	// creating the model matrix
	glm::mat4 model_matrix = glm::mat4(1.0f);
	model_matrix = glm::scale(model_matrix, glm::vec3(2.0f, 2.0f, 2.0f));


	// creating the view matrix
	glm::mat4 view = glm::mat4(1.0f);
	view = glm::translate(view, glm::vec3(0.0f, 0.0f, -4.0f));

	// creating the projection matrix
	glm::mat4 projection = glm::perspective(45.0f, 4.0f / 3, 0.1f, 20.0f);

	// Adding all matrices up to create combined matrix
	glm::mat4 mv = view * model_matrix;


	//adding the Uniform to the shader
	int mvLoc = glGetUniformLocation(shader, "mv_matrix");
	//glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mv));
	//adding the Uniform to the shader
	int pLoc = glGetUniformLocation(shader, "p_matrix");
	glUniformMatrix4fv(pLoc, 1, GL_FALSE, glm::value_ptr(projection));


	glEnableVertexAttribArray(Triangles);
	glEnableVertexAttribArray(Colours);
	glEnableVertexAttribArray(Textures);
	glEnableVertexAttribArray(Normals);

}


//// display
void display(GLfloat delta)
{
	static const float black[] = { 0.0f, 0.0f, 0.0f, 0.0f };

	glClearBufferfv(GL_COLOR, 0, black);
	//glClear(GL_COLOR_BUFFER_BIT);

	// bind textures on corresponding texture units
	//glFrontFace(GL_CW);
	//glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE); //check which one is the outside face
	// creating the model matrix
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::scale(model, glm::vec3(2.0f, 2.0f, 2.0f)); //scale of object1
	model = glm::rotate(model, glm::radians(delta), glm::vec3(1.0f, 0.0f, 0.0f)); //rotation of cude add - to spin the opp way
	//if (KEY_DOWN(VK_UP))model = glm::rotate(model, glm::radians(delta), glm::vec3(1.0f, 0.0f, 0.0f)); //rotation of cude add - to spin the opp way
	////																			 speed
	//if (KEY_DOWN(VK_RIGHT))model = glm::rotate(model, glm::radians(delta), glm::vec3(0.0f, 1.0f, 0.0f)); //rotation of cude RIGHT
	////																					  
	//if (KEY_DOWN(VK_LEFT))model = glm::rotate(model, glm::radians(-delta), glm::vec3(0.0f, 1.0f, 0.0f)); //rotation of cude LEFT
	////																				
	//if (KEY_DOWN(VK_DOWN))model = glm::rotate(model, glm::radians(delta), glm::vec3(0.0f, 0.0f, 1.0f)); //rotation of cude add Z ROTATION
	////																					        AXIS 

	// creating the view matrix
	glm::mat4 view = glm::mat4(1.0f);
	view = glm::translate(view, glm::vec3(0.0f, 0.0f, -4.0f));

	// creating the projection matrix   
	//camera distance lower = further distnce 10 
	glm::mat4 projection = glm::perspective(45.0f, 4.0f / 3, 0.1f, 20.0f);

	// Adding all matrices up to create combined matrix
	glm::mat4 mv = view * model;


	//adding the Uniform to the shader
	int mvLoc = glGetUniformLocation(shader, "mv_matrix");
	glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mv));
	//adding the Uniform to the shader
	int pLoc = glGetUniformLocation(shader, "p_matrix");
	glUniformMatrix4fv(pLoc, 1, GL_FALSE, glm::value_ptr(projection));


	glBindVertexArray(VAOs[object1]);
	glBindTexture(GL_TEXTURE_2D, texture1);
	glDrawArrays(GL_TRIANGLES, 0, 36);

}


//void keys(void)
//{
//	if (KEY_DOWN(VK_SPACE))glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); //press space for wire frame
//}










//void readFile()
//{
//	string path = "T";
//	printf("please enter file path...");
//	cin >> path;
//		
//
//		//ifstream myFile("media/textures/creeper-obj/Creeper.obj"); //Creeper
//		//ifstream myFile("media/textures/Pouf-obj/pouf.obj"); //Pouf
//		//ifstream myFile("media/textures/LowPolyBoat-obj/low_poly_boat.obj"); //Low Poly Boat
//
//	
//
//
//	
//
//}







// main
//


int main(int argc, char** argv)
{


	glfwInit();

	GLFWwindow* window = glfwCreateWindow(800, 600, "Model Loader", NULL, NULL);

	glfwMakeContextCurrent(window);
	glewInit();


	init();
	GLfloat timer = 0.0f;
	while (!glfwWindowShouldClose(window))
	{

		// uncomment to draw only wireframe 
		// glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		//if (KEY_DOWN(VK_SPACE))glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); //press space for wire frame

		//keys();
		display(timer);
		//myDisplay();


		glfwSwapBuffers(window);
		glfwPollEvents();
		timer += 0.1f;

		if (KEY_DOWN(VK_ESCAPE))glfwDestroyWindow(window);
	}

	glfwTerminate();
}