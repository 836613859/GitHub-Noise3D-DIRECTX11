

#include "Main3D.h"
#include <sstream>

NoiseEngine Engine;
NoiseRenderer Renderer;
NoiseScene Scene;
NoiseMesh Mesh1;
NoiseCamera Camera;
NoiseLightManager LightMgr;
NoiseMaterialManager	MatMgr;
NoiseTextureManager	TexMgr;
NoiseAtmosphere			Atmos;

NoiseGraphicObject	GraphicObjBuffer;
NoiseUtTimer NTimer(NOISE_TIMER_TIMEUNIT_MILLISECOND);
NoiseUtSlicer Slicer;
N_DirectionalLight DirLight1;
N_PointLight	 PointLight1;
N_SpotLight	SpotLight1;

float Angle;
std::vector<NVECTOR3>	 lineSegmentBuffer;
std::vector<N_LineStrip>	lineStrip;


//Main Entry
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
{
	HWND windowHWND;
	windowHWND = Engine.CreateRenderWindow(640, 480, L"Hahaha Render Window", hInstance);
	Init3D(windowHWND);
	Engine.SetMainLoopFunction(MainLoop);
	Engine.Mainloop();
	Cleanup();
	return 0;
}



BOOL Init3D(HWND hwnd)
{
	
	//��ʼ��ʧ��
	if(!Engine.InitD3D( hwnd,640,480,TRUE))return FALSE;

	Scene.CreateMesh(&Mesh1);
	Scene.CreateRenderer(&Renderer);
	Scene.CreateCamera(&Camera);
	Scene.CreateLightManager(&LightMgr);
	Scene.CreateGraphicObject(&GraphicObjBuffer);
	Scene.CreateMaterialManager(&MatMgr);
	Scene.CreateTextureManager(&TexMgr);
	Scene.CreateAtmosphere(&Atmos);

	//��������ͼ
	TexMgr.CreateTextureFromFile(L"Earth.jpg", "Earth", TRUE, 0, 0);
	TexMgr.CreateTextureFromFile(L"texture2.jpg", "Wood", TRUE, 0, 0);
	TexMgr.CreateTextureFromFile(L"button.dds", "Button", TRUE, 0, 0);
	TexMgr.CreateTextureFromFile(L"planet.png", "Planet", TRUE, 0, 0);
	TexMgr.CreateTextureFromFile(L"universe.jpg", "Universe", TRUE, 0, 0);
	TexMgr.CreateTextureFromFile(L"bottom-right-conner-title.jpg", "BottomRightTitle", TRUE, 0, 0);
	TexMgr.CreateTextureFromFile(L"Earth_Bump.bmp", "NormalMap_Earth", TRUE, 0, 0);
	TexMgr.CreateCubeMapFromDDS(L"UniverseEnv.dds", "EnvironmentMap",NOISE_CUBEMAP_SIZE_256x256);

	Renderer.SetFillMode(NOISE_FILLMODE_SOLID);
	Renderer.SetCullMode(NOISE_CULLMODE_BACK);//NOISE_CULLMODE_BACK

	Mesh1.CreateSphere(4.0f,30,30);
	Mesh1.SetPosition(0,0,0);

	Camera.SetPosition(2.0f,0,0);
	Camera.SetLookAt(0,0,0);
	Camera.SetViewAngle(MATH_PI / 2.0f, 1.333333333f);
	Camera.SetViewFrustumPlane(1.0f, 500.f);

	Atmos.SetFogEnabled(FALSE);
	Atmos.SetFogParameter(7.0f, 8.0f, NVECTOR3(0, 0, 1.0f));
	Atmos.CreateSkyDome(4.0f, 4.0f, TexMgr.GetIndexByName("Universe"));
	//Atmos.CreateSkyBox(10.0f, 10.0f, 10.0f, TexMgr.GetIndexByName("EnvironmentMap"));

	//�������������ƹ⡪��������������
	DirLight1.mAmbientColor = NVECTOR3(1.0f,1.0f,1.0f);
	DirLight1.mDiffuseColor	=	NVECTOR3(1.0f,1.0f,1.0f);
	DirLight1.mSpecularColor	=NVECTOR3(1.0f,1.0f,1.0f);
	DirLight1.mDirection = NVECTOR3(0.0f, -0.5f, 1.0f);
	DirLight1.mSpecularIntensity	=1.5f;
	DirLight1.mDiffuseIntensity = 1.0f;
	LightMgr.AddDynamicDirLight(&DirLight1);

	N_Material Mat1;
	Mat1.baseMaterial.mBaseAmbientColor	= NVECTOR3(0.1f,  0.1f,0.1f);
	Mat1.baseMaterial.mBaseDiffuseColor		= NVECTOR3(1.0f,  1.0f, 1.0f);
	Mat1.baseMaterial.mBaseSpecularColor	=	NVECTOR3(1.0f, 1.0f,1.0f);
	Mat1.baseMaterial.mSpecularSmoothLevel	=	40;
	Mat1.baseMaterial.mNormalMapBumpIntensity = 0.3f;
	Mat1.baseMaterial.mEnvironmentMapTransparency = 0.2f;
	Mat1.diffuseMapID = TexMgr.GetIndexByName("Earth");
	Mat1.normalMapID = TexMgr.GetIndexByName("NormalMap_Earth");
	//Mat1.specularMapID = TexMgr.GetIndexByName("Earth");
	Mat1.cubeMap_environmentMapID = TexMgr.GetIndexByName("EnvironmentMap");
	UINT	 Mat1_ID = MatMgr.CreateMaterial(Mat1);

	//set material
	Mesh1.SetMaterial(Mat1_ID);

	GraphicObjBuffer.AddLine2D(NVECTOR2(-1.0f, 0), NVECTOR2(1.0f, 0));
	GraphicObjBuffer.AddLine2D(NVECTOR2(0, 1.0f), NVECTOR2(0,-1.0f));
	GraphicObjBuffer.AddRectangle(NVECTOR2(340.0f, 430.0f), NVECTOR2(640.0f, 480.0f), NVECTOR4(0, 0, 0, 0), TexMgr.GetIndexByName("BottomRightTitle"));
	return TRUE;
};


void MainLoop()
{
	Angle += 0.0005f;
	Camera.SetPosition(7.0f*cos(Angle),4.0f,7.0f*sin(Angle));
	Camera.SetLookAt(0,0,0);
	//GraphicObjBuffer.SetEllipse(0, 100.0f, 100.0f, NVECTOR2(300+100 * sin(-Angle), 240+100 * cos(-Angle)), NVECTOR4(0, 1.0f, 0, 0.5f), TexMgr.GetIndexByName("Planet"));


	Renderer.ClearBackground();

	//add to render list
	Mesh1.AddToRenderList();
	GraphicObjBuffer.AddToRenderList();
	Atmos.AddToRenderList();

	//render
	Renderer.SetBlendingMode(NOISE_BLENDMODE_OPAQUE);
	Renderer.RenderMeshInList();
	Renderer.SetBlendingMode(NOISE_BLENDMODE_OPAQUE);
	Renderer.RenderAtmosphereInList();
	Renderer.SetBlendingMode(NOISE_BLENDMODE_ADDITIVE);
	Renderer.RenderGraphicObjectInList();

	//present
	Renderer.RenderToScreen();
};


void Cleanup()
{
	Engine.ReleaseAll();
	Scene.ReleaseAllChildObject();
};