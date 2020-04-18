// ConsoleApplication1.cpp : 此檔案包含 'main' 函式。程式會於該處開始執行及結束執行。
//

#include <GL/freeglut.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <cmath>

using namespace std;

#define WINDOW_TITLE_PREFIX "Advanced CG Example"

int CurrentWidth = 800, CurrentHeight = 600, WindowHandle = 0;
const aiScene *scene;

void ResizeFunction(int, int);
void RenderFunction();
void KeyboardFunction(unsigned char key, int x, int y);
void RenderScene(const aiScene* sc, const aiNode* nd);


// camera parameters
double CameraFOV = 45;
double CameraY = 1000;
bool dolly = false;
double dollyFactor = 0;

constexpr double pi = 3.1415926;

int main(int argc, char *argv[])
{

	printf("Loading scene...\n");
	Assimp::Importer importer;
	scene = importer.ReadFile("ElkInScene.ply", aiProcess_Triangulate | aiProcess_FlipUVs);


	//初始化 glut
	glutInit(&argc, argv);

	//設定 glut 畫布尺寸 與color / depth模式
	glutInitWindowSize(CurrentWidth, CurrentHeight);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);

	//根據已設定好的 glut (如尺寸,color,depth) 向window要求建立一個視窗，接著若失敗則退出程式
	WindowHandle = glutCreateWindow(WINDOW_TITLE_PREFIX);
	if (WindowHandle < 1) { fprintf(stderr, "ERROR: Could not create a new rendering window.\n"); exit(EXIT_FAILURE); }

	glutReshapeFunc(ResizeFunction); //設定視窗 大小若改變，則跳到"AResizeFunction"這個函數處理應變事項
	glutDisplayFunc(RenderFunction);  //設定視窗 如果要畫圖 則執行"RenderFunction"
	glutKeyboardFunc(KeyboardFunction);


	// Before we move on, enable necessary setting
	// since there's default color on each node in the scene
	// uncomment the following code if you want lighting
	/*
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	GLfloat MaterialAmbient[] = { 0.4,0.4,0.4,1.0f };
	GLfloat MaterialDiffuse[] = { 0.7,0.7,0.7,1.0f };
	GLfloat MaterialSpecular[] = { 1.2,1.2,1.2, 1.0f };
	glLightfv(GL_LIGHT0, GL_AMBIENT, MaterialAmbient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, MaterialDiffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, MaterialSpecular);
	
	glEnable(GL_COLOR_MATERIAL);
	*/
	
	glEnable(GL_DEPTH_TEST);
	

	glutMainLoop();

	exit(EXIT_SUCCESS);
}

void ResizeFunction(int Width, int Height)
{
	CurrentWidth = Width;
	CurrentHeight = Height;
	glViewport(0, 0, CurrentWidth, CurrentHeight);
}

void KeyboardFunction(unsigned char key, int x, int y)
{
	switch (tolower(key)) {
	case 'w':
		CameraY -= 10; break;
	case 's':
		CameraY += 10; break;
	case 'q':
		// toggle dolly zoom(on/off)
		dolly = !dolly;
		// if, currently, dolly is toggled to on
		if (dolly) {
			dollyFactor = 1 / tan(CameraFOV * pi / 180 / 2) / CameraY;
			printf("Dolly enabled\n");
		}
		else {
			printf("Dolly disabled\n");
		}
	}
}

void RenderFunction()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, CurrentWidth, CurrentHeight);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	if (dolly) {
		// update camera fov
		CameraFOV = atan(1 / (dollyFactor * CameraY)) * 2 * 180 / pi;
	}

	gluPerspective(CameraFOV, (double)CurrentWidth / CurrentHeight, 0.1, 100000.);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0, CameraY, 100, 0, CameraY-1, 100, 0, 0, 1);

	RenderScene(scene, scene->mRootNode);


	glutSwapBuffers();
	glutPostRedisplay();
}



void RenderScene(const aiScene* sc, const aiNode* nd)
{
	unsigned int i;
	unsigned int n = 0, t;
	aiMatrix4x4 m = nd->mTransformation;

	// update transform
	aiMatrix4x4 mT = m.Transpose();
	glPushMatrix();
	glMultMatrixf((float*)& mT);

	// draw all meshes assigned to this node
	for (n = 0; n < nd->mNumMeshes; n++) {
		const aiMesh* mesh = sc->mMeshes[nd->mMeshes[n]];


		for (t = 0; t < mesh->mNumFaces; ++t) {
			const aiFace* face = &mesh->mFaces[t];
			GLenum face_mode;

			switch (face->mNumIndices) {
			case 1: face_mode = GL_POINTS; break;
			case 2: face_mode = GL_LINES; break;
			case 3: face_mode = GL_TRIANGLES; break;
			default: face_mode = GL_POLYGON; break;
			}

			glBegin(face_mode);
			for (i = 0; i < face->mNumIndices; i++) {
				int index = face->mIndices[i];
				if (mesh->mColors[0] != NULL) glColor4fv((GLfloat*)& mesh->mColors[0][index]);
				if (mesh->mNormals != NULL)
				{
					glNormal3fv(&mesh->mNormals[index].x);
				}

				glVertex3fv(&mesh->mVertices[index].x);
			}

			glEnd();
		}

	}

	// draw all children
	for (n = 0; n < nd->mNumChildren; ++n) {
		RenderScene(sc, nd->mChildren[n]);
	}

	glPopMatrix();
}
