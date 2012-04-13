/*
 *This C program is an exercise in texture loading
 *user rotation and translation enabled
 * Nathan Gossett
 *Spring 2012
 **/

#include <stdio.h>
#include <GL/Angel.h>
#include <stdlib.h>
#include <math.h>
#include <IL/il.h> //notice the OpenIL include

#pragma comment(lib, "glew32.lib")
//We have additional libraries to link to now as well
#pragma comment(lib,"ILUT.lib")
#pragma comment(lib,"DevIL.lib")
#pragma comment(lib,"ILU.lib")

GLuint location_permTexture,location_simplexTexture,location_gradTexture;

#define WIDTH 800
#define HEIGHT 700
#define TRUE 1
#define FALSE 0


int right_button_down = FALSE;
int left_button_down = FALSE;
int middle_button_down = FALSE;

int prevMouseX;
int prevMouseY;

double view_rotx = 180.0;
double view_roty = 0.0;
double view_rotz = 0.0;
double z_distance;

//We need three texture files
static GLuint texName[5];


GLuint * vao;
GLuint * vbo;

// EARTH 
GLuint * spherevao;
GLuint * spherevbo;

// CLOUD
GLuint * cloudvao;
GLuint * cloudvbo;


//our modelview and perspective matrices
mat4 mv, p;

//and we'll need pointers to our shader variables
GLuint model_view;
GLuint projection;
GLuint vPosition;
GLuint texCoord;
GLuint texMap;

GLuint program;

int multiflag = 0;




/////// EARTH
int space = 10; //10; // 259200
int VertexCount = (180 / space) * (360 / space) * 4; // 2592

vec4 squareverts[6];
vec2 texcoords[6];


int rotateYEarth;
int rotateXEarth;


///////////////
// CLOUD
//////////////

int perm[256]= {151,160,137,91,90,15,
  131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
  190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
  88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
  77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
  102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
  135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
  5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
  223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
  129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
  251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
  49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
  138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180};

/* These are Ken Perlin's proposed gradients for 3D noise. I kept them for
   better consistency with the reference implementation, but there is really
   no need to pad this to 16 gradients for this particular implementation.
   If only the "proper" first 12 gradients are used, they can be extracted
   from the grad4[][] array: grad3[i][j] == grad4[i*2][j], 0<=i<=11, j=0,1,2
*/
int grad3[16][3] = {{0,1,1},{0,1,-1},{0,-1,1},{0,-1,-1},
                   {1,0,1},{1,0,-1},{-1,0,1},{-1,0,-1},
                   {1,1,0},{1,-1,0},{-1,1,0},{-1,-1,0}, // 12 cube edges
                   {1,0,-1},{-1,0,-1},{0,-1,1},{0,1,1}}; // 4 more to make 16

/* This is a look-up table to speed up the decision on which simplex we
   are in inside a cube or hypercube "cell" for 3D and 4D simplex noise.
   It is used to avoid complicated nested conditionals in the GLSL code.
   The table is indexed in GLSL with the results of six pair-wise
   comparisons beween the components of the P=(x,y,z,w) coordinates
   within a hypercube cell.
   c1 = x>=y ? 32 : 0;
   c2 = x>=z ? 16 : 0;
   c3 = y>=z ? 8 : 0;
   c4 = x>=w ? 4 : 0;
   c5 = y>=w ? 2 : 0;
   c6 = z>=w ? 1 : 0;
   offsets = simplex[c1+c2+c3+c4+c5+c6];
   o1 = step(160,offsets);
   o2 = step(96,offsets);
   o3 = step(32,offsets);
   (For the 3D case, c4, c5, c6 and o3 are not needed.)
*/
  unsigned char simplex4[][4] = {{0,64,128,192},{0,64,192,128},{0,0,0,0},
  {0,128,192,64},{0,0,0,0},{0,0,0,0},{0,0,0,0},{64,128,192,0},
  {0,128,64,192},{0,0,0,0},{0,192,64,128},{0,192,128,64},
  {0,0,0,0},{0,0,0,0},{0,0,0,0},{64,192,128,0},
  {0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},
  {0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},
  {64,128,0,192},{0,0,0,0},{64,192,0,128},{0,0,0,0},
  {0,0,0,0},{0,0,0,0},{128,192,0,64},{128,192,64,0},
  {64,0,128,192},{64,0,192,128},{0,0,0,0},{0,0,0,0},
  {0,0,0,0},{128,0,192,64},{0,0,0,0},{128,64,192,0},
  {0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},
  {0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},
  {128,0,64,192},{0,0,0,0},{0,0,0,0},{0,0,0,0},
  {192,0,64,128},{192,0,128,64},{0,0,0,0},{192,64,128,0},
  {128,64,0,192},{0,0,0,0},{0,0,0,0},{0,0,0,0},
  {192,64,0,128},{0,0,0,0},{192,128,0,64},{192,128,64,0}};

  /* These are my own proposed gradients for 4D noise. They are the coordinates
   of the midpoints of each of the 32 edges of a tesseract, just like the 3D
   noise gradients are the midpoints of the 12 edges of a cube.
*/
int grad4[32][4]= {{0,1,1,1}, {0,1,1,-1}, {0,1,-1,1}, {0,1,-1,-1}, // 32 tesseract edges
                   {0,-1,1,1}, {0,-1,1,-1}, {0,-1,-1,1}, {0,-1,-1,-1},
                   {1,0,1,1}, {1,0,1,-1}, {1,0,-1,1}, {1,0,-1,-1},
                   {-1,0,1,1}, {-1,0,1,-1}, {-1,0,-1,1}, {-1,0,-1,-1},
                   {1,1,0,1}, {1,1,0,-1}, {1,-1,0,1}, {1,-1,0,-1},
                   {-1,1,0,1}, {-1,1,0,-1}, {-1,-1,0,1}, {-1,-1,0,-1},
                   {1,1,1,0}, {1,1,-1,0}, {1,-1,1,0}, {1,-1,-1,0},
                   {-1,1,1,0}, {-1,1,-1,0}, {-1,-1,1,0}, {-1,-1,-1,0}};

//Modified slightly from the OpenIL tutorials
ILuint loadTexFile(const char* filename){
	
	ILboolean success; /* ILboolean is type similar to GLboolean and can equal GL_FALSE (0) or GL_TRUE (1)
    it can have different value (because it's just typedef of unsigned char), but this sould be
    avoided.
    Variable success will be used to determine if some function returned success or failure. */


	/* Before calling ilInit() version should be checked. */
	  if (ilGetInteger(IL_VERSION_NUM) < IL_VERSION)
	  {
		/* wrong DevIL version */
		printf("Wrong IL version");
		exit(1);
	  }
 
	  success = ilLoadImage((LPTSTR)filename); /* Loading of image from file */
	if (success){ /* If no error occured: */
		//We need to figure out whether we have an alpha channel or not
		  if(ilGetInteger(IL_IMAGE_BPP) == 3){
			success = ilConvertImage(IL_RGB, IL_UNSIGNED_BYTE); /* Convert every color component into
		  unsigned byte. If your image contains alpha channel you can replace IL_RGB with IL_RGBA */
		  }else if(ilGetInteger(IL_IMAGE_BPP) == 4){
			  success = ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);
		  }else{
			  success = false;
		  }
		if (!success){
		  /* Error occured */
		 printf("failed conversion to unsigned byte");
		 exit(1);
		}
	}else{
		/* Error occured */
	   printf("Failed to load image ");
	   printf(filename);
		exit(1);
	}
}



void CreateSphere (vec4 verts[2592], vec2 texcoords[2592], double R, double H, double K, double Z) {
    
	int n;
    double a;
    double b;
    n = 0;
    
	for( b = 0; b <= 180 - space; b+=space){
       
		for( a = 0; a <= 360 - space; a+=space){
            
			verts[n] = vec4( R * sin((a) / 180 * M_PI) * sin((b) / 180 * M_PI) - H,
								   R * cos((a) / 180 * M_PI) * sin((b) / 180 * M_PI) + K,
								   R * cos((b) / 180 * M_PI) - Z, 
								   1);
            texcoords[n] = vec2((a) / 360 , (2 * b) / 360);
            

			n++;
            
			verts[n] = vec4(R * sin((a) / 180 * M_PI) * sin((b + space) / 180 * M_PI) - H,
					             R * cos((a) / 180 * M_PI) * sin((b + space) / 180 * M_PI) + K,
								 R * cos((b + space) / 180 * M_PI) - Z,
								 1);

			texcoords[n] = vec2((a) / 360 , 
							(2 *  (b + space)) / 360);
            
            
			n++;
            

            
			verts[n] = vec4(R * sin((a + space) / 180 * M_PI) * sin((b) / 180 * M_PI) - H,
								R * cos((a + space) / 180 * M_PI) * sin((b) / 180 * M_PI) + K,
								R * cos((b) / 180 * M_PI) - Z,
								1);
         
			texcoords[n] = vec2((a + space) / 360 ,  (2 *  (b)) / 360 );


			n++;
            
			verts[n] = vec4( R * sin((a + space) / 180 * M_PI) * sin((b + space) /180 * M_PI) - H,
									R * cos((a + space) / 180 * M_PI) * sin((b + space) / 180 * M_PI) + K,
									 R * cos((b + space) / 180 * M_PI) - Z,
									 1);
            
			texcoords[n] = vec2((a + space) / 360, (2 *  (b + space)) / 360);

			n++;
            

            
		}
    
	}
}

void CreateCloudSphere(vec4 verts[2592], vec2 texcoords[2592], double R, double H, double K, double Z)
{
	CreateSphere (verts, texcoords, 5.5, 0, 0, 0);
}

void createSquare(vec4 squareverts[6], vec2 texcoords[6])
{
   
   squareverts[0] = vec4(-1, -1, 0, 1); 
   texcoords[0] = vec2(0, 0);
   squareverts[1] = vec4(1, -1, 0, 1);
   texcoords[1] = vec2(1, 0);
   squareverts[2] = vec4(1, 1, 0, 1);
   texcoords[2] = vec2(1, 1);
   squareverts[3] = vec4(1, 1, 0, 1);
   texcoords[3] = vec2(1, 1);
   squareverts[4] = vec4(-1, 1, 0, 1);
   texcoords[4] = vec2(0, 1);
   squareverts[5] = vec4(-1, -1, 0, 1);
   texcoords[5] = vec2(0, 0);
}

void SetupShader(GLuint vao[1], GLuint vbo[3], vec4 squareverts[6], vec2 texcoords[2],ILuint ilTexID[3],
					GLuint texName[3])
{
	// Create a vertex array object
    glGenVertexArrays( 1, &vao[0] );

    // Create and initialize any buffer objects
	glBindVertexArray( vao[0] );
	glGenBuffers( 2, &vbo[0] );
    glBindBuffer( GL_ARRAY_BUFFER, vbo[0] );
    glBufferData( GL_ARRAY_BUFFER, sizeof(squareverts), squareverts, GL_STATIC_DRAW);

	glBindBuffer( GL_ARRAY_BUFFER, vbo[1] );
    glBufferData( GL_ARRAY_BUFFER, sizeof(texcoords), texcoords, GL_STATIC_DRAW);

	


	ilBindImage(ilTexID[0]); /* Binding of IL image name */
	loadTexFile("images/Earth.png");
	glBindTexture(GL_TEXTURE_2D, texName[0]); //bind OpenGL texture name

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   //Note how we depend on OpenIL to supply information about the file we just loaded in
    glTexImage2D(GL_TEXTURE_2D, 0, ilGetInteger(IL_IMAGE_BPP), ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT),0,
	ilGetInteger(IL_IMAGE_FORMAT), ilGetInteger(IL_IMAGE_TYPE), ilGetData());
}


void init(void)
{    
   glClearColor (0.0, 0.0, 0.0, 0.0);
   glEnable(GL_DEPTH_TEST);

	glEnable(GL_TEXTURE_1D); // Enable 1D texturing
    glEnable(GL_TEXTURE_2D); // Enable 2D texturing


	rotateYEarth = 0;
	rotateXEarth = 0;

     
   //squareverts = new vec4[6];

   //createSquare(squareverts, texcoords);

	////////////////////////////
	// Creating EARTH
	///////////////////////////
	vec4 sphereverts[2592];
	vec2 spheretexcoords[2592]; // 2592


	CreateSphere(sphereverts, spheretexcoords, 2, 0,0,0);

	/////////////////////////////////////////
	// Create a vertex array object
	spherevao = new GLuint[1];
	spherevbo = new GLuint[3];


	glGenVertexArrays( 1, &spherevao[0] );

	// Create and initialize any buffer objects
	glBindVertexArray( spherevao[0] );
	glGenBuffers( 2, &spherevbo[0] );
	glBindBuffer( GL_ARRAY_BUFFER, spherevbo[0] );
	glBufferData( GL_ARRAY_BUFFER, VertexCount*sizeof(vec4), sphereverts, GL_STATIC_DRAW);

	glBindBuffer( GL_ARRAY_BUFFER, spherevbo[1] );
	glBufferData( GL_ARRAY_BUFFER, VertexCount*sizeof(vec2), spheretexcoords, GL_STATIC_DRAW);

	

	//////////////////////////////////////
	// Initialize Shader
	//////////////////////////////////////
   program = InitShader( "vshader-texture.glsl", "fshader-texture.glsl" );

   location_permTexture = glGetUniformLocation( program, "permTexture" );
	
   location_simplexTexture = glGetUniformLocation( program, "simplexTexture" );
	
   location_gradTexture = glGetUniformLocation( program, "gradTexture" );


   ILuint ilTexID[5]; /* ILuint is a 32bit unsigned integer.

    //Variable texid will be used to store image name. */
   

	ilInit(); /* Initialization of OpenIL */
	ilGenImages(5, ilTexID); /* Generation of three image names for OpenIL image loading */
	glGenTextures(5, texName); //and we eventually want the data in an OpenGL texture
 
	/////////////////////////////////////////
	// EARTH BUFFERS
	////////////////////////////////////////////////////
	

	if ( true )
	{
		ilBindImage(ilTexID[0]); /* Binding of IL image name */
		loadTexFile("images/Earth.png");
		glBindTexture(GL_TEXTURE_2D, texName[0]); //bind OpenGL texture name

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	   //Note how we depend on OpenIL to supply information about the file we just loaded in
		glTexImage2D(GL_TEXTURE_2D, 0, ilGetInteger(IL_IMAGE_BPP), ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT),0,
		ilGetInteger(IL_IMAGE_FORMAT), ilGetInteger(IL_IMAGE_TYPE), ilGetData());


	}
	
	
	if (true)
	{
		ilBindImage(ilTexID[1]); /* Binding of IL image name */
		glBindTexture(GL_TEXTURE_2D, texName[1]); //bind OpenGL texture name
		loadTexFile("images/earthcloudmap.png");
		

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	   //Note how we depend on OpenIL to supply information about the file we just loaded in
		glTexImage2D(GL_TEXTURE_2D, 0, ilGetInteger(IL_IMAGE_BPP), ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT),0,
		ilGetInteger(IL_IMAGE_FORMAT), ilGetInteger(IL_IMAGE_TYPE), ilGetData());
	}

	// permutation
	if ( true )
	{
		char *pixels;
		  int i,j;
  
		  //glGenTextures(1, texID); // Generate a unique texture ID
		  //glBindTexture(GL_TEXTURE_2D, *texID); // Bind the texture to texture unit 0

		  ilBindImage(ilTexID[2]); /* Binding of IL image name */
		  glBindTexture(GL_TEXTURE_2D, texName[2]); //bind OpenGL texture name

		  pixels = (char*)malloc( 256*256*4 );
		  for(i = 0; i<256; i++)
			for(j = 0; j<256; j++) {
			  int offset = (i*256+j)*4;
			  char value = perm[(j+perm[i]) & 0xFF];
			  pixels[offset] = grad3[value & 0x0F][0] * 64 + 64;   // Gradient x
			  pixels[offset+1] = grad3[value & 0x0F][1] * 64 + 64; // Gradient y
			  pixels[offset+2] = grad3[value & 0x0F][2] * 64 + 64; // Gradient z
			  pixels[offset+3] = value;                     // Permuted index
			}
  
		  // GLFW texture loading functions won't work here - we need GL_NEAREST lookup.
		  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels );
		  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
		  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	}


	if (true)
	{
		glActiveTexture(GL_TEXTURE1); // Activate a different texture unit (unit 1)

		  //glGenTextures(1, texID); // Generate a unique texture ID
		  //glBindTexture(GL_TEXTURE_1D, *texID); // Bind the texture to texture unit 1
			
		  ilBindImage(ilTexID[3]); /* Binding of IL image name */
		  glBindTexture(GL_TEXTURE_1D, texName[3]); //bind OpenGL texture name

		  // GLFW texture loading functions won't work here - we need GL_NEAREST lookup.
		  glTexImage1D( GL_TEXTURE_1D, 0, GL_RGBA, 64, 0, GL_RGBA, GL_UNSIGNED_BYTE, simplex4 );
		  glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
		  glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

		  glActiveTexture(GL_TEXTURE0); // Switch active texture unit back to 0 again
	}

	if ( true )
	{
		char *pixels;
		  int i,j;
  
		  glActiveTexture(GL_TEXTURE2); // Activate a different texture unit (unit 2)

		  //glGenTextures(1, texID); // Generate a unique texture ID
		 // glBindTexture(GL_TEXTURE_2D, *texID); // Bind the texture to texture unit 2
		  ilBindImage(ilTexID[4]); /* Binding of IL image name */
		  glBindTexture(GL_TEXTURE_1D, texName[4]); //bind OpenGL texture name

		  pixels = (char*)malloc( 256*256*4 );
		  for(i = 0; i<256; i++)
			for(j = 0; j<256; j++) {
			  int offset = (i*256+j)*4;
			  char value = perm[(j+perm[i]) & 0xFF];
			  pixels[offset] = grad4[value & 0x1F][0] * 64 + 64;   // Gradient x
			  pixels[offset+1] = grad4[value & 0x1F][1] * 64 + 64; // Gradient y
			  pixels[offset+2] = grad4[value & 0x1F][2] * 64 + 64; // Gradient z
			  pixels[offset+3] = grad4[value & 0x1F][3] * 64 + 64; // Gradient z
			}
  
		  // GLFW texture loading functions won't work here - we need GL_NEAREST lookup.
		  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels );
		  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
		  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

		  glActiveTexture(GL_TEXTURE0); // Switch active texture unit back to 0 again
	}
	////////////////////////////////////////////////
    ilDeleteImages(3, ilTexID); //we're done with OpenIL, so free up the memory

	////////////////////////////////////////////////////

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	model_view = glGetUniformLocation(program, "model_view");
	projection = glGetUniformLocation(program, "projection");
	
	texMap = glGetUniformLocation(program, "permTexture"); // "texture");
	glUniform1i(texMap, 0);//assign this one to texture unit 0
				

	
	/////////////////////////////////////////
	// CLOUD BUFFERS
	////////////////////////////////////////////////////

	

	/////////////////////////////////////////
   ////////////////////////////////////////////////////
	//Now repeat the process for the second image
	/*ilBindImage(ilTexID[1]);
	
	glBindTexture(GL_TEXTURE_2D, texName[1]);
	loadTexFile("images/alpaca.png");

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, ilGetInteger(IL_IMAGE_BPP), ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT),0,
	   ilGetInteger(IL_IMAGE_FORMAT), ilGetInteger(IL_IMAGE_TYPE), ilGetData());
*/
	////////////////////////////////////////////////////
	//And the third image

	/*ilBindImage(ilTexID[2]);
	glBindTexture(GL_TEXTURE_2D, texName[2]);
	loadTexFile("images/opengl.png");

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, ilGetInteger(IL_IMAGE_BPP), ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT),0,
	   ilGetInteger(IL_IMAGE_FORMAT), ilGetInteger(IL_IMAGE_TYPE), ilGetData());
*/

	
	/////////////////////////////////////////////////////////////////
	///// bind VAO and VBO


	if ( false )
	{
		/*glBindBuffer( GL_ARRAY_BUFFER, vbo[0] );
		vPosition = glGetAttribLocation(program, "vPosition");
		glEnableVertexAttribArray(vPosition);
		glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);

		glBindBuffer( GL_ARRAY_BUFFER, vbo[1] );
		texCoord = glGetAttribLocation(program, "texCoord");
		glEnableVertexAttribArray(texCoord);
		glVertexAttribPointer(texCoord, 2, GL_FLOAT, GL_FALSE, 0, 0);*/
	}
	else
	{
		glBindBuffer( GL_ARRAY_BUFFER, spherevbo[0] );
		vPosition = glGetAttribLocation(program, "vPosition");
		glEnableVertexAttribArray(vPosition);
		glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);

		glBindBuffer( GL_ARRAY_BUFFER, spherevbo[1] );
		texCoord = glGetAttribLocation(program, "texCoord");
		glEnableVertexAttribArray(texCoord);
		glVertexAttribPointer(texCoord, 2, GL_FLOAT, GL_FALSE, 0, 0);
	}


	/*if (true)
	{ 
		 glBindBuffer( GL_ARRAY_BUFFER, cloudvbo[0] );
		vPosition = glGetAttribLocation(program, "vPosition");
		glEnableVertexAttribArray(vPosition);
		glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);

		glBindBuffer( GL_ARRAY_BUFFER, cloudvbo[1] );
		texCoord = glGetAttribLocation(program, "texCoord");
		glEnableVertexAttribArray(texCoord);
		glVertexAttribPointer(texCoord, 2, GL_FLOAT, GL_FALSE, 0, 0);
	}*/
}
////////////////////////////////
////////////////////////////////
void display(void)
{
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
    glUniform1i( location_permTexture, 0 ); // Texture unit 0
 	glUniform1i( location_simplexTexture, 1 ); // Texture unit 1
 	glUniform1i( location_gradTexture, 2 ); // Texture unit 2
	
    //mat4 camera = mv =  LookAt(vec4(0,0,5.0+z_distance,1),vec4(0,0,0,1),vec4(0,1,0,0))* RotateX(view_rotx) * RotateY(view_roty) * RotateZ(view_rotz);

    
	
	glActiveTexture(GL_TEXTURE0);

	mat4 camera = mv =  LookAt(vec4(0,0,5.0+z_distance,1),vec4(0,0,0,1),vec4(0,1,0,0)) * RotateX(-90.0); //RotateX(view_rotx) * RotateY(view_roty);

	// mv = mv * RotateZ(rotateYEarth);

	glUniformMatrix4fv(model_view, 1, GL_TRUE, mv * RotateZ(rotateYEarth)*Translate(0,0,0));
	glBindTexture(GL_TEXTURE_2D, texName[0]); //which texture do we want?
	glDrawArrays( GL_QUAD_STRIP, 0, VertexCount );



	glUniformMatrix4fv(model_view, 1, GL_TRUE, mv*RotateZ(rotateYEarth / 2)*Scale(1.02,1.02,1.02)*Translate(0,0,0));
	glBindTexture(GL_TEXTURE_2D, texName[1]); //which texture do we want?
	glDrawArrays( GL_QUAD_STRIP, 0, VertexCount );

	/*
	mv = camera * RotateY(90)* Translate(0,0,1);
	glUniformMatrix4fv(model_view, 1, GL_TRUE, mv);
	
	glBindTexture(GL_TEXTURE_2D, texName[1]); //which texture do we want?
	glDrawArrays( GL_TRIANGLES, 0, 6 );

	mv = camera* RotateY(-90) * Translate(0,0,1);
	glUniformMatrix4fv(model_view, 1, GL_TRUE, mv);
	glBindTexture(GL_TEXTURE_2D, texName[2]); //which texture do we want?
	glDrawArrays( GL_TRIANGLES, 0, 6 );
	*/

   glutSwapBuffers();


}

void reshape(int w, int h)
{
   glViewport(0, 0, (GLsizei) w, (GLsizei) h);

	 p = Perspective(60.0, (float)w/(float)h, 1.0, 30.0);
	glUniformMatrix4fv(projection, 1, GL_TRUE, p);
}

void keyboard (unsigned char key, int x, int y)
{
   switch (key) {
      case 27:
         exit(0);
         break;
      default:
         break;
   }
}

void mouse_dragged(int x, int y) {
	double thetaY, thetaX;
	if (left_button_down) {

		rotateYEarth += 1;

		/*thetaY = 360.0 *(x-prevMouseX)/WIDTH;    
		thetaX = 360.0 *(prevMouseY - y)/HEIGHT;
		prevMouseX = x;
		prevMouseY = y;
		view_rotx += thetaX;
		view_roty += thetaY;*/
	}
	else if (right_button_down) {

		rotateXEarth += 1;
		// z_distance = 5.0*(prevMouseY-y)/HEIGHT;
	}

	else if (middle_button_down)
	{
		z_distance += 1;
	}
    glutPostRedisplay();
}


void mouse(int button, int state, int x, int y) {
  //establish point of reference for dragging mouse in window
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
      left_button_down = TRUE;
	  prevMouseX= x;
      prevMouseY = y;
    }

	else if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
      right_button_down = TRUE;
      prevMouseX = x;
      prevMouseY = y;
    }
    else if (state == GLUT_UP) {
      left_button_down = FALSE;
	  right_button_down = FALSE;
	}
}

int main(int argc, char** argv)
{
   glutInit(&argc, argv);
   glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
   glutInitWindowSize(WIDTH, HEIGHT);
   glutInitWindowPosition(100, 100);
   glutCreateWindow("Texturing");
   glewExperimental = GL_TRUE;

	glewInit();
   init();
   glutDisplayFunc(display);
   glutReshapeFunc(reshape);
   glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
	glutMotionFunc(mouse_dragged);    
   glutMainLoop();
   return 0; 
}