/*
  CSCI 420 Computer Graphics, USC
  Assignment 1: Height Fields
  C++ starter code

  Student username: rhiremat
*/

#include <iostream>
#include <cstring>
#include "openGLHeader.h"
#include "glutHeader.h"

#include "imageIO.h"
#include "openGLMatrix.h"
#include "basicPipelineProgram.h"

#ifdef WIN32
  #ifdef _DEBUG
    #pragma comment(lib, "glew32d.lib")
  #else
    #pragma comment(lib, "glew32.lib")
  #endif
#endif

#ifdef WIN32
  char shaderBasePath[1024] = SHADER_BASE_PATH;
#else
  char shaderBasePath[1024] = "../openGLHelper-starterCode";
#endif

using namespace std;

int mousePos[2]; // x,y coordinate of the mouse position

int leftMouseButton = 0; // 1 if pressed, 0 if not
int middleMouseButton = 0; // 1 if pressed, 0 if not
int rightMouseButton = 0; // 1 if pressed, 0 if not

typedef enum { ROTATE, TRANSLATE, SCALE } CONTROL_STATE;
CONTROL_STATE controlState = ROTATE;

typedef enum {POINTS, LINES, TRIANGLES, TRIANGLE_STRIP, WIREFRAME} GL_MODE;
GL_MODE displayMode = POINTS;
GL_MODE oldDisplayMode = POINTS;

// state of the world
float landRotate[3] = { 0.0f, 0.0f, 0.0f };
float landTranslate[3] = { 0.0f, 0.0f, 0.0f };
float landScale[3] = { 1.0f, 1.0f, 1.0f };

int windowWidth = 1280;
int windowHeight = 720;
char windowTitle[512] = "CSCI 420 homework I";

ImageIO * heightmapImage;
OpenGLMatrix * matrix;
BasicPipelineProgram * pipelineProgram;
GLuint vBuffer, lBuffer, tBuffer, tsBuffer;
GLuint vVAO, lVAO, tVAO, tsVAO;

int numVertices[4] = {0,0,0,0};
float sizeVertices[4] = {0,0,0,0};
float sizeColors[4] = {0,0,0,0};
float * pVertices;
float * pColors;
float * lVertices;
float * lColors;
float * tVertices;
float * tColors;
float * tsVertices;
float * tsColors;
float FOV = 90.0;
float eye[3] = {0, 0, 0};
bool takeSS=true;
int ssCount=0;

// write a screenshot to the specified filename
void saveScreenshot(const char * filename)
{
  unsigned char * screenshotData = new unsigned char[windowWidth * windowHeight * 3];
  glReadPixels(0, 0, windowWidth, windowHeight, GL_RGB, GL_UNSIGNED_BYTE, screenshotData);

  ImageIO screenshotImg(windowWidth, windowHeight, 3, screenshotData);

  if (screenshotImg.save(filename, ImageIO::FORMAT_JPEG) == ImageIO::OK)
    cout << "File " << filename << " saved successfully." << endl;
  else cout << "Failed to save file " << filename << '.' << endl;

  delete [] screenshotData;
}

// read the height information from the image
float getHeightFromImage(int x, int y)
{
  return float(1.0*heightmapImage->getPixel(x,y,1));
}

// save the coordinates of the vertices for POINTS view
void initVerticesBuffer()
{
  if (numVertices[0]==0)
  {
    int imageHeight = heightmapImage->getHeight();
    int imageWidth = heightmapImage->getWidth();
    int index = 0;
    int index2 = 0;
    float scale = 0.5/255;

    numVertices[0]= imageHeight * imageWidth;
    sizeVertices[0] = sizeof(float) * numVertices[0] * 3;
    pVertices = (float * )malloc(sizeVertices[0]);
    sizeColors[0] = sizeof(float) * numVertices[0] * 4;
    pColors = (float * )malloc(sizeColors[0]);

    for (int i = 0; i< imageHeight; i++)
    {
      for (int j = 0; j< imageWidth; j++)
      {
        pVertices[index] = i*1.0/imageHeight;
        pVertices[index+1] = scale*getHeightFromImage(i,j);
        pVertices[index+2] =  -j*1.0/imageWidth;
        index+=3;
        pColors[index2] = 1.0;
        pColors[index2+1] = 1.0;
        pColors[index2+2] = 1.0;
        pColors[index2+3] = 1.0;
        index2+=4;
      }
    }
  }
}

// save the coordinates of the vertices for LINES view
void initLinesBuffer()
{
  if (numVertices[1]==0)
  {
    int imageHeight = heightmapImage->getHeight();
    int imageWidth = heightmapImage->getWidth();
    int index = 0;
    int index2 = 0;
    float scale = 0.5/255;

    numVertices[1]= 2*(imageWidth*(imageHeight-1) + (imageWidth-1)*imageHeight);
    sizeVertices[1] = sizeof(float) * numVertices[1] * 3;
    lVertices = (float * )malloc(sizeVertices[1]);
    sizeColors[1] = sizeof(float) * numVertices[1] * 4;
    lColors = (float * )malloc(sizeColors[1]);

    //horizontal lines
    for (int i = 0; i< imageHeight; i++)
    {
      for (int j = 0; j< imageWidth-1; j++)
      {
        lVertices[index] = i*1.0/imageHeight;
        lVertices[index+1] = scale*getHeightFromImage(i,j);
        lVertices[index+2] =  -j*1.0/imageWidth;
        index+=3;
        lColors[index2] = 1.0*getHeightFromImage(i,j)/255;
        lColors[index2+1] = 1.0*getHeightFromImage(i,j)/255;
        lColors[index2+2] = 1.0*getHeightFromImage(i,j)/255;
        lColors[index2+3] = 1.0;
        index2+=4;

        lVertices[index] = i*1.0/imageHeight;
        lVertices[index+1] = scale*getHeightFromImage(i,j+1);
        lVertices[index+2] =  -(j+1)*1.0/imageWidth;
        index+=3;
        lColors[index2] = 1.0*getHeightFromImage(i,j+1)/255;
        lColors[index2+1] = 1.0*getHeightFromImage(i,j+1)/255;
        lColors[index2+2] = 1.0*getHeightFromImage(i,j+1)/255;
        lColors[index2+3] = 1.0;
        index2+=4;
      }
    }

    //vertical lines
    for (int j = 0; j< imageWidth; j++)
    {
      for (int i = 0; i< imageHeight-1; i++)
      {
        lVertices[index] = i*1.0/imageHeight;
        lVertices[index+1] = scale*getHeightFromImage(i,j);
        lVertices[index+2] =  -j*1.0/imageWidth;
        index+=3;
        lColors[index2] = 1.0*getHeightFromImage(i,j)/255;
        lColors[index2+1] = 1.0*getHeightFromImage(i,j)/255;
        lColors[index2+2] = 1.0*getHeightFromImage(i,j)/255;
        lColors[index2+3] = 1.0;
        index2+=4;

        lVertices[index] = (i+1)*1.0/imageHeight;
        lVertices[index+1] = scale*getHeightFromImage(i+1,j);
        lVertices[index+2] =  -j*1.0/imageWidth;
        index+=3;
        lColors[index2] = 1.0*getHeightFromImage(i+1,j)/255;
        lColors[index2+1] = 1.0*getHeightFromImage(i+1,j)/255;
        lColors[index2+2] = 1.0*getHeightFromImage(i+1,j)/255;
        lColors[index2+3] = 1.0;
        index2+=4;
      }
    }
  }
}

// save the coordinates of the vertices for TRIANGLES view
void initTrianglesBuffer()
{
  if (numVertices[2]==0)
  {
    int imageHeight = heightmapImage->getHeight();
    int imageWidth = heightmapImage->getWidth();
    int index = 0;
    int index2 = 0;
    float scale = 0.5/255;

    numVertices[2]= (imageHeight-1) * (imageWidth-1) * 2 * 3;
    sizeVertices[2] = sizeof(float) * numVertices[2] * 3;
    tVertices = (float * )malloc(sizeVertices[2]);
    sizeColors[2] = sizeof(float) * numVertices[2] * 4;
    tColors = (float * )malloc(sizeColors[2]);

    for (int i = 0; i< imageHeight-1; i++)
    {
      for (int j = 0; j< imageWidth-1; j++)
      {
        //quad
        //triangle1
        tVertices[index] = i*1.0/imageHeight;
        tVertices[index+1] = scale*getHeightFromImage(i,j);
        tVertices[index+2] =  -j*1.0/imageWidth;
        index+=3;
        tColors[index2] = 1.0*getHeightFromImage(i,j)/255;
        tColors[index2+1] = 1.0*getHeightFromImage(i,j)/255;
        tColors[index2+2] = 1.0*getHeightFromImage(i,j)/255;
        tColors[index2+3] = 1.0;
        index2+=4;


        tVertices[index] = i*1.0/imageHeight;
        tVertices[index+1] = scale*getHeightFromImage(i,j+1);
        tVertices[index+2] =  -(j+1)*1.0/imageWidth;
        index+=3;
        tColors[index2] = 1.0*getHeightFromImage(i,j+1)/255;
        tColors[index2+1] = 1.0*getHeightFromImage(i,j+1)/255;
        tColors[index2+2] = 1.0*getHeightFromImage(i,j+1)/255;
        tColors[index2+3] = 1.0;
        index2+=4;

        tVertices[index] = (i+1)*1.0/imageHeight;
        tVertices[index+1] = scale*getHeightFromImage(i+1,j);
        tVertices[index+2] =  -(j)*1.0/imageWidth;
        index+=3;
        tColors[index2] = 1.0*getHeightFromImage(i+1,j)/255;
        tColors[index2+1] = 1.0*getHeightFromImage(i+1,j)/255;
        tColors[index2+2] = 1.0*getHeightFromImage(i+1,j)/255;
        tColors[index2+3] = 1.0;
        index2+=4;

        //triangle2
        tVertices[index] = (i+1)*1.0/imageHeight;
        tVertices[index+1] = scale*getHeightFromImage(i+1,j);
        tVertices[index+2] =  -j*1.0/imageWidth;
        index+=3;
        tColors[index2] = 1.0*getHeightFromImage(i+1,j)/255;
        tColors[index2+1] = 1.0*getHeightFromImage(i+1,j)/255;
        tColors[index2+2] = 1.0*getHeightFromImage(i+1,j)/255;
        tColors[index2+3] = 1.0;
        index2+=4;

        tVertices[index] = (i+1)*1.0/imageHeight;
        tVertices[index+1] = scale*getHeightFromImage(i+1,j+1);
        tVertices[index+2] =  -(j+1)*1.0/imageWidth;
        index+=3;
        tColors[index2] = 1.0*getHeightFromImage(i+1,j+1)/255;
        tColors[index2+1] = 1.0*getHeightFromImage(i+1,j+1)/255;
        tColors[index2+2] = 1.0*getHeightFromImage(i+1,j+1)/255;
        tColors[index2+3] = 1.0;
        index2+=4;

        tVertices[index] = i*1.0/imageHeight;
        tVertices[index+1] = scale*getHeightFromImage(i,j+1);
        tVertices[index+2] =  -(j+1)*1.0/imageWidth;
        index+=3;
        tColors[index2] = 1.0*getHeightFromImage(i,j+1)/255;
        tColors[index2+1] = 1.0*getHeightFromImage(i,j+1)/255;
        tColors[index2+2] = 1.0*getHeightFromImage(i,j+1)/255;
        tColors[index2+3] = 1.0;
        index2+=4;
      }
    }
  }
}

// save the coordinates of the vertices for TRIANGLE STRIP view
void initTriangleStripBuffer()
{
  if (numVertices[3]==0)
  {
    int imageHeight = heightmapImage->getHeight();
    int imageWidth = heightmapImage->getWidth();
    int index = 0;
    int index2 = 0;
    float scale = 0.5/255;

    numVertices[3]= (((imageHeight-1) * (imageWidth) * 2) + (imageHeight-1) * 3);
    sizeVertices[3] = sizeof(float) * numVertices[3] * 3;
    tsVertices = (float * )malloc(sizeVertices[3]);
    sizeColors[3] = sizeof(float) * numVertices[3] * 4;
    tsColors = (float * )malloc(sizeColors[3]);

    int i, j;
    for (i = 0; i< imageHeight-1; i++)
    {
      for (j = 0; j< imageWidth; j++)
      {
        tsVertices[index] = i*1.0/imageHeight;
        tsVertices[index+1] = scale*getHeightFromImage(i,j);
        tsVertices[index+2] =  -j*1.0/imageWidth;
        index+=3;
        tsColors[index2] = 1.0*getHeightFromImage(i,j)/255;
        tsColors[index2+1] = 0;
        tsColors[index2+2] = 0;
        tsColors[index2+3] = 1.0;
        index2+=4;

        tsVertices[index] = (i+1)*1.0/imageHeight;
        tsVertices[index+1] = scale*getHeightFromImage(i+1,j);
        tsVertices[index+2] =  -j*1.0/imageWidth;
        index+=3;
        tsColors[index2] = 1.0*getHeightFromImage(i+1,j)/255;
        tsColors[index2+1] = 0;
        tsColors[index2+2] = 0;
        tsColors[index2+3] = 1.0;
        index2+=4;
      }

      tsVertices[index] = (i+1)*1.0/imageHeight;
      tsVertices[index+1] = scale*getHeightFromImage(i+1,imageWidth-1);
      tsVertices[index+2] =  -(imageWidth-1)*1.0/imageWidth;
      index+=3;
      tsColors[index2] = 1.0*getHeightFromImage(i+1,imageWidth-1)/255;
      tsColors[index2+1] = 0;
      tsColors[index2+2] = 0;
      tsColors[index2+3] = 1.0;
      index2+=4;

      tsVertices[index] = i*1.0/imageHeight;
      tsVertices[index+1] = scale*getHeightFromImage(i,0);
      tsVertices[index+2] =  0;
      index+=3;
      tsColors[index2] = 1.0*getHeightFromImage(i,0)/255;
      tsColors[index2+1] = 0;
      tsColors[index2+2] = 0;
      tsColors[index2+3] = 1.0;
      index2+=4;

      tsVertices[index] = i*1.0/imageHeight;
      tsVertices[index+1] = scale*getHeightFromImage(i,0);
      tsVertices[index+2] =  0;
      index+=3;
      tsColors[index2] = 1.0*getHeightFromImage(i,0)/255;
      tsColors[index2+1] = 0;
      tsColors[index2+2] = 0;
      tsColors[index2+3] = 1.0;
      index2+=4;
    }
  }
}

// initialize VBO for POINTS view
void initVertexVBO()
{
  glGenBuffers(1, &vBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, vBuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeVertices[0] + sizeColors[0], NULL, GL_STATIC_DRAW);
  // upload position data
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeVertices[0], pVertices);
  // upload color data
  glBufferSubData(GL_ARRAY_BUFFER, sizeVertices[0], sizeColors[0], pColors);
}

// initialize VBO for LINES view
void initLineVBO()
{
  glGenBuffers(1, &lBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, lBuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeVertices[1] + sizeColors[1], NULL, GL_STATIC_DRAW);
  // upload position data
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeVertices[1], lVertices);
  // upload color data
  glBufferSubData(GL_ARRAY_BUFFER, sizeVertices[1], sizeColors[1], lColors);
}

// initialize VBO for TRIANGLES view
void initTriangleVBO()
{
  glGenBuffers(1, &tBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, tBuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeVertices[2] + sizeColors[2], NULL, GL_STATIC_DRAW);
  // upload position data
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeVertices[2], tVertices);
  // upload color data
  glBufferSubData(GL_ARRAY_BUFFER, sizeVertices[2], sizeColors[2], tColors);
}

// initialize VBO for TRIANGLE STRIP view
void initTriangleStripVBO()
{
  glGenBuffers(1, &tsBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, tsBuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeVertices[3] + sizeColors[3], NULL, GL_STATIC_DRAW);
  // upload position data
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeVertices[3], tsVertices);
  // upload color data
  glBufferSubData(GL_ARRAY_BUFFER, sizeVertices[3], sizeColors[3], tsColors);
}

// set the Projection and ModelView matrices
void bindProgram()
{
  GLuint program = pipelineProgram->GetProgramHandle();
  //projection matrix
  GLint h_projectionMatrix = glGetUniformLocation(program, "projectionMatrix");
  matrix->SetMatrixMode(OpenGLMatrix::Projection);
  matrix->LoadIdentity();
  matrix->Perspective(FOV, (1.0*windowWidth/windowHeight), 0.01, 1000.0);

  float p[16];
  matrix->GetMatrix(p);
  pipelineProgram->Bind();
  glUniformMatrix4fv(h_projectionMatrix, 1, GL_FALSE, p);

  //modelview matrix
  GLint h_modelViewMatrix = glGetUniformLocation(program, "modelViewMatrix");
  matrix->SetMatrixMode(OpenGLMatrix::ModelView);
  matrix->LoadIdentity();
  matrix->LookAt(eye[0], eye[1], eye[2], 0, 0, -1, 0, 1, 0); // eye, focus, up
  matrix->Translate(landTranslate[0],landTranslate[1],landTranslate[2]);
  matrix->Rotate(landRotate[0],1,0,0);
  matrix->Rotate(landRotate[1],0,1,0);
  matrix->Rotate(landRotate[2],0,0,1);
  matrix->Scale(landScale[0],landScale[1],landScale[2]);

  float m[16];
  matrix->GetMatrix(m);
  pipelineProgram->Bind();
  glUniformMatrix4fv(h_modelViewMatrix, 1, GL_FALSE, m);
}

// set the vertices and colours for POINTS view
void displayVertices()
{
  initVerticesBuffer();
  initVertexVBO();
  glGenVertexArrays(1, &vVAO);

  GLuint program = pipelineProgram->GetProgramHandle();
  glBindVertexArray(vVAO);
  glBindBuffer(GL_ARRAY_BUFFER, vBuffer);
  GLuint loc = glGetAttribLocation(program, "position");
  glEnableVertexAttribArray(loc);
  const void * offset = (const void*) 0;
  glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, offset);

  GLuint loc2 = glGetAttribLocation(program, "color");
  glEnableVertexAttribArray(loc2);
  const void * offset2 = (const void*) (size_t)sizeVertices[0];
  glVertexAttribPointer(loc2, 4, GL_FLOAT, GL_FALSE, 0, offset2);
  glBindVertexArray(0);

  glBindVertexArray(vVAO);
  GLint first = 0;
  GLsizei numberOfVertices = numVertices[0];
  glDrawArrays(GL_POINTS, first, numberOfVertices);
}

// set the vertices and colours for LINES view
void displayLines()
{
  initLinesBuffer();
  initLineVBO();
  glGenVertexArrays(1, &lVAO);

  GLuint program = pipelineProgram->GetProgramHandle();
  glBindVertexArray(lVAO);
  glBindBuffer(GL_ARRAY_BUFFER, lBuffer);
  GLuint loc = glGetAttribLocation(program, "position");
  glEnableVertexAttribArray(loc);
  const void * offset = (const void*) 0;
  glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, offset);

  GLuint loc2 = glGetAttribLocation(program, "color");
  glEnableVertexAttribArray(loc2);
  const void * offset2 = (const void*) (size_t)sizeVertices[1];
  glVertexAttribPointer(loc2, 4, GL_FLOAT, GL_FALSE, 0, offset2);
  glBindVertexArray(0);

  glBindVertexArray(lVAO);
  GLint first = 0;
  GLsizei numberOfVertices = numVertices[1];
  glDrawArrays(GL_LINES, first, numberOfVertices);
}

// set the vertices and colours for TRIANGLES view
void displayTriangles()
{
  initTrianglesBuffer();
  initTriangleVBO();
  glGenVertexArrays(1, &tVAO);

  GLuint program = pipelineProgram->GetProgramHandle();
  glBindVertexArray(tVAO);
  glBindBuffer(GL_ARRAY_BUFFER, tBuffer);
  GLuint loc = glGetAttribLocation(program, "position");
  glEnableVertexAttribArray(loc);
  const void * offset = (const void*) 0;
  glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, offset);

  GLuint loc2 = glGetAttribLocation(program, "color");
  glEnableVertexAttribArray(loc2);
  const void * offset2 = (const void*) (size_t)sizeVertices[2];
  glVertexAttribPointer(loc2, 4, GL_FLOAT, GL_FALSE, 0, offset2);
  glBindVertexArray(0);

  glBindVertexArray(tVAO);
  GLint first = 0;
  GLsizei numberOfVertices = numVertices[2];
  glDrawArrays(GL_TRIANGLES, first, numberOfVertices);
}

// set the vertices and colours for TRIANGLE STRIP view
void displayTriangleStrip()
{
  initTriangleStripBuffer();
  initTriangleStripVBO();
  glGenVertexArrays(1, &tsVAO);

  GLuint program = pipelineProgram->GetProgramHandle();
  glBindVertexArray(tsVAO);
  glBindBuffer(GL_ARRAY_BUFFER, tsBuffer);
  GLuint loc = glGetAttribLocation(program, "position");
  glEnableVertexAttribArray(loc);
  const void * offset = (const void*) 0;
  glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, offset);

  GLuint loc2 = glGetAttribLocation(program, "color");
  glEnableVertexAttribArray(loc2);
  const void * offset2 = (const void*) (size_t)sizeVertices[3];
  glVertexAttribPointer(loc2, 4, GL_FLOAT, GL_FALSE, 0, offset2);
  glBindVertexArray(0);

  glBindVertexArray(tsVAO);
  GLint first = 0;
  GLsizei numberOfVertices = numVertices[3];
  glDrawArrays(GL_TRIANGLE_STRIP, first, numberOfVertices);
}

// set the vertices and colours for WIREFRAME view
void displayWireFrame()
{
  initLinesBuffer();
  initTriangleStripBuffer();
  initLineVBO();
  initTriangleStripVBO();

  GLuint program = pipelineProgram->GetProgramHandle();
  glGenVertexArrays(1, &tsVAO);
  glBindVertexArray(tsVAO);
  glBindBuffer(GL_ARRAY_BUFFER, tsBuffer);
  GLuint loc = glGetAttribLocation(program, "position");
  glEnableVertexAttribArray(loc);
  const void * offset = (const void*) 0;
  glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, offset);

  GLuint loc2 = glGetAttribLocation(program, "color");
  glEnableVertexAttribArray(loc2);
  const void * offset2 = (const void*) (size_t)sizeVertices[3];
  glVertexAttribPointer(loc2, 4, GL_FLOAT, GL_FALSE, 0, offset2);
  glBindVertexArray(0);

  glBindVertexArray(tsVAO);
  glEnable(GL_POLYGON_OFFSET_FILL);
  glPolygonOffset(1.0f, 1.0f);
  GLint first = 0;
  GLsizei numberOfVertices = numVertices[3];
  glDrawArrays(GL_TRIANGLE_STRIP, first, numberOfVertices);
  glBindVertexArray(0);
  glDisable(GL_POLYGON_OFFSET_FILL);

  glGenVertexArrays(1, &lVAO);
  glBindVertexArray(lVAO);
  glBindBuffer(GL_ARRAY_BUFFER, lBuffer);
   loc = glGetAttribLocation(program, "position");
  glEnableVertexAttribArray(loc);
  const void * offset3 = (const void*) 0;
  glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, offset3);

   loc2 = glGetAttribLocation(program, "color");
  glEnableVertexAttribArray(loc2);
  const void * offset4 = (const void*) (size_t)sizeVertices[1];
  glVertexAttribPointer(loc2, 4, GL_FLOAT, GL_FALSE, 0, offset4);
  glBindVertexArray(0);

  glBindVertexArray(lVAO);
   first = 0;
   numberOfVertices = numVertices[1];
  glDrawArrays(GL_LINES, first, numberOfVertices);
  glBindVertexArray(0);

}

void clearBuffers()
{
  if (oldDisplayMode != displayMode)
  {
    if (numVertices[0]!=0)
    {
      numVertices[0] = 0;
      delete(pVertices);
      delete(pColors);
    }
    if (numVertices[1]!=0)
    {
      numVertices[1] = 0;
      delete(lVertices);
      delete(lColors);
    }
    if (numVertices[2]!=0)
    {
      numVertices[2] = 0;
      delete(tVertices);
      delete(tColors);
    }
    if (numVertices[3]!=0)
    {
      numVertices[3] = 0;
      delete(tsVertices);
      delete(tsColors);
    }
  }
}

void displayFunc()
{
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  pipelineProgram->Bind();

  bindProgram();
  pipelineProgram->Bind();

  switch (displayMode)
  {
    case POINTS:
      clearBuffers();
      displayVertices();
      oldDisplayMode = POINTS;
      break;
    case LINES:
      clearBuffers();
      displayLines();
      oldDisplayMode = LINES;
      break;
    case TRIANGLES:
      clearBuffers();
      displayTriangles();
      oldDisplayMode = TRIANGLES;
      break;
    case TRIANGLE_STRIP:
      clearBuffers();
      displayTriangleStrip();
      oldDisplayMode = TRIANGLE_STRIP;
      break;
    case WIREFRAME:
      clearBuffers();
      displayWireFrame();
      oldDisplayMode = WIREFRAME;
      break;
  }

  glBindVertexArray(0);
  glutSwapBuffers();
}

void idleFunc()
{
  // for example, here, you can save the screenshots to disk (to make the animation)
  glutPostRedisplay();
}

void reshapeFunc(int w, int h)
{
  // setup perspective matrix...
  GLfloat aspect = (GLfloat) w / (GLfloat) h;
  glViewport(0, 0, w, h);
  matrix->SetMatrixMode(OpenGLMatrix::Projection);
  matrix->LoadIdentity();
  matrix->Perspective(FOV, aspect, 0.01, 1000.0);
  matrix->SetMatrixMode(OpenGLMatrix::ModelView);
}

void mouseMotionDragFunc(int x, int y)
{
  // mouse has moved and one of the mouse buttons is pressed (dragging)

  // the change in mouse position since the last invocation of this function
  int mousePosDelta[2] = { x - mousePos[0], y - mousePos[1] };

  switch (controlState)
  {
    // translate the landscape
    case TRANSLATE:
      if (leftMouseButton)
      {
        // control x,y translation via the left mouse button
        landTranslate[0] += mousePosDelta[0] * 0.01f;
        landTranslate[1] -= mousePosDelta[1] * 0.01f;
      }
      if (middleMouseButton)
      {
        // control z translation via the middle mouse button
        landTranslate[2] += mousePosDelta[1] * 0.01f;
      }
      break;

    // rotate the landscape
    case ROTATE:
      if (leftMouseButton)
      {
        // control x,y rotation via the left mouse button
        landRotate[0] += mousePosDelta[1];
        landRotate[1] += mousePosDelta[0];
      }
      if (middleMouseButton)
      {
        // control z rotation via the middle mouse button
        landRotate[2] += mousePosDelta[1];
      }
      break;

    // scale the landscape
    case SCALE:
      if (leftMouseButton)
      {
        // control x,y scaling via the left mouse button
        landScale[0] *= 1.0f + mousePosDelta[0] * 0.01f;
        landScale[1] *= 1.0f - mousePosDelta[1] * 0.01f;
      }
      if (middleMouseButton)
      {
        // control z scaling via the middle mouse button
        landScale[2] *= 1.0f - mousePosDelta[1] * 0.01f;
      }
      break;
  }

  // store the new mouse position
  mousePos[0] = x;
  mousePos[1] = y;
}

void mouseMotionFunc(int x, int y)
{
  // mouse has moved
  // store the new mouse position
  mousePos[0] = x;
  mousePos[1] = y;
}

void mouseButtonFunc(int button, int state, int x, int y)
{
  // a mouse button has has been pressed or depressed

  // keep track of the mouse button state, in leftMouseButton, middleMouseButton, rightMouseButton variables
  switch (button)
  {
    case GLUT_LEFT_BUTTON:
      leftMouseButton = (state == GLUT_DOWN);
    break;

    case GLUT_MIDDLE_BUTTON:
      middleMouseButton = (state == GLUT_DOWN);
    break;

    case GLUT_RIGHT_BUTTON:
      rightMouseButton = (state == GLUT_DOWN);
    break;
  }
  switch (glutGetModifiers())
  {
    case GLUT_ACTIVE_SHIFT:
      controlState = TRANSLATE;
    break;



    // if CTRL and SHIFT are not pressed, we are in rotate mode
    default:
      controlState = ROTATE;
    break;
  }

  // store the new mouse position
  mousePos[0] = x;
  mousePos[1] = y;
}

void keyboardFunc(unsigned char key, int x, int y)
{
  switch (key)
  {
    case 27: // ESC key
      exit(0); // exit the program
    break;

    case ' ':
      cout << "You pressed the spacebar." << endl;
    break;

    case 'x':
      // take a screenshot
      saveScreenshot("screenshot.jpg");
    break;

      case 't':
        controlState = TRANSLATE;
        // store the new mouse position
        mousePos[0] = x;
        mousePos[1] = y;
      break;

      case 's':
        controlState = SCALE;
        // store the new mouse position
        mousePos[0] = x;
        mousePos[1] = y;
      break;

      case 'r':
        controlState = ROTATE;
        // store the new mouse position
        mousePos[0] = x;
        mousePos[1] = y;
      break;

      case '1':
        displayMode = POINTS;
      break;

      case '2':
        displayMode = LINES;
      break;

      case '3':
        displayMode = TRIANGLES;
      break;

      case '4':
        displayMode = TRIANGLE_STRIP;
      break;

      case '5':
        displayMode = WIREFRAME;
      break;

  }
}

void initScene(int argc, char *argv[])
{
  // load the image from a jpeg disk file to main memory
  heightmapImage = new ImageIO();
  if (heightmapImage->loadJPEG(argv[1]) != ImageIO::OK)
  {
    cout << "Error reading image " << argv[1] << "." << endl;
    exit(EXIT_FAILURE);
  }

  initVerticesBuffer();

  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

  glEnable (GL_DEPTH_TEST);

  pipelineProgram = new BasicPipelineProgram();
  pipelineProgram->Init("../openGLHelper-starterCode");
  matrix = new OpenGLMatrix();
  initVerticesBuffer();
}

void screenshotTimer(int value)
{
  switch(value){
    case 0:
      if(!takeSS) break;
      char fileName[40];
      sprintf(fileName, "screenshots/%03d.jpg", ssCount);
      saveScreenshot(fileName);
      ssCount++;
      if(ssCount == 300)
        takeSS = false;
      break;
    default:
      break;
  }
  glutTimerFunc(1000, screenshotTimer, 0);
}

int main(int argc, char *argv[])
{
  if (argc != 2)
  {
    cout << "The arguments are incorrect." << endl;
    cout << "usage: ./hw1 <heightmap file>" << endl;
    exit(EXIT_FAILURE);
  }
  cout << "Initializing GLUT..." << endl;
  glutInit(&argc,argv);

  cout << "Initializing OpenGL..." << endl;

  #ifdef __APPLE__
    glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
  #else
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
  #endif

  glutInitWindowSize(windowWidth, windowHeight);
  glutInitWindowPosition(0, 0);
  glutCreateWindow(windowTitle);

  cout << "OpenGL Version: " << glGetString(GL_VERSION) << endl;
  cout << "OpenGL Renderer: " << glGetString(GL_RENDERER) << endl;
  cout << "Shading Language Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;

  // tells glut to use a particular display function to redraw
  glutDisplayFunc(displayFunc);
  // perform animation inside idleFunc
  glutIdleFunc(idleFunc);
  // callback for mouse drags
  glutMotionFunc(mouseMotionDragFunc);
  // callback for idle mouse movement
  glutPassiveMotionFunc(mouseMotionFunc);
  // callback for mouse button changes
  glutMouseFunc(mouseButtonFunc);
  // callback for resizing the window
  glutReshapeFunc(reshapeFunc);
  // callback for pressing the keys on the keyboard
  glutKeyboardFunc(keyboardFunc);
  // callback for a timer to save screenshots
  glutTimerFunc(1000, screenshotTimer, 0);

  // init glew
  #ifdef __APPLE__
    // nothing is needed on Apple
  #else
    // Windows, Linux
    GLint result = glewInit();
    if (result != GLEW_OK)
    {
      cout << "error: " << glewGetErrorString(result) << endl;
      exit(EXIT_FAILURE);
    }
  #endif

  // do initialization
  initScene(argc, argv);

  // sink forever into the glut loop
  glutMainLoop();
}
