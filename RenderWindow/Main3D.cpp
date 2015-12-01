

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
NoiseGUIManager		GUIMgr;
NoiseGUIButton			GUIButton1;
NoiseGUIScrollBar			GUIScrollBar1(TRUE);
NoiseGraphicObject	GraphicObjBuffer;
NoiseFontManager		fontMgr;
Noise2DTextStatic			myText1;
Noise2DTextDynamic	myText2;
Noise2DTextDynamic	myText_fps;

NoiseUtTimer NTimer(NOISE_TIMER_TIMEUNIT_MILLISECOND);
NoiseUtSlicer Slicer;
NoiseUtInputEngine inputE;
N_DirectionalLight DirLight1;
N_PointLight	 PointLight1;
N_SpotLight	SpotLight1;

float moveForwardVelocity;
std::vector<NVECTOR3>	 lineSegmentBuffer;
std::vector<N_LineStrip>	lineStrip;


//Main Entry
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
{
	HWND windowHWND;
	windowHWND = Engine.CreateRenderWindow(640, 480, L"Hahaha Render Window", hInstance);
	inputE.Initialize(hInstance, windowHWND);
	Init3D(windowHWND);
	Engine.SetMainLoopFunction(MainLoop);
	Engine.Mainloop();
	Cleanup();
	return 0;
}

BOOL Init3D(HWND hwnd)
{

	//初始化失败
	if(!Engine.InitD3D( hwnd,640,480,TRUE))return FALSE;

	Scene.CreateMesh(Mesh1);
	Scene.CreateRenderer(Renderer);
	Scene.CreateCamera(Camera);
	Scene.CreateLightManager(LightMgr);
	Scene.CreateGraphicObject(GraphicObjBuffer);
	Scene.CreateMaterialManager(MatMgr);
	Scene.CreateTextureManager(TexMgr);
	Scene.CreateAtmosphere(Atmos);
	Scene.CreateGUI(GUIMgr, inputE, hwnd);
	Scene.CreateFontManager(fontMgr);

	//漫反射贴图
	//TexMgr.CreateTextureFromFile(L"Earth.jpg", "Earth", TRUE,0, 0, TRUE);
	//TexMgr.CreatePureColorTexture("myText", 300, 100, NVECTOR4(0.0f, 0.0f, 0.0f, 0.0f), TRUE);
	TexMgr.CreateTextureFromFile(L"Earth.jpg","Earth", TRUE,0, 0,FALSE);
	TexMgr.CreateTextureFromFile(L"Earth.jpg", "EarthNormalMap", TRUE, 0, 0, TRUE);
	TexMgr.CreateTextureFromFile(L"texture2.jpg", "Wood", TRUE, 0, 0, FALSE);
	TexMgr.CreateTextureFromFile(L"button.dds", "Button", TRUE, 0, 0, FALSE);
	TexMgr.CreateTextureFromFile(L"universe2.jpg", "Universe", TRUE, 0, 0, FALSE);
	TexMgr.CreateTextureFromFile(L"bottom-right-conner-title.jpg", "BottomRightTitle", TRUE, 0, 0, FALSE);
	TexMgr.CreateCubeMapFromDDS(L"UniverseEnv.dds", "EnvironmentMap",NOISE_CUBEMAP_SIZE_256x256);
	TexMgr.ConvertTextureToGreyMap(TexMgr.GetTextureID("EarthNormalMap"));
	TexMgr.ConvertHeightMapToNormalMap(TexMgr.GetTextureID("EarthNormalMap"),20.0f);

	//create font texture
	fontMgr.CreateFontFromFile("STXINWEI.ttf", "myFont", 36);

	fontMgr.CreateStaticTextW(0, L"Graphic Object在渲染前都会生成一个Subset List。。。这才对路嘛。。。", 300, 100, NVECTOR4(0, 1.0f, 0.5f, 1.0f), 0, 0, myText1);

	myText1.SetTextColor(NVECTOR4(1.0f, 0, 0, 0.5f));
	myText1.SetCenterPos(300.0f, 100.0f);
	fontMgr.CreateDynamicTextA(0, "abcdefghijklmnopqrstuvwxyz!@#$%^&*()_<>-+?/+ 1234567890<>?,./{}[]\\", 300, 100, NVECTOR4(0, 1.0f, 0.5f, 1.0f), 0, 0, myText2);
	myText2.SetTextColor(NVECTOR4(0.5f, 0.3f, 1.0f, 0.5f));
	myText2.SetCenterPos(300.0f,400.0f);
	fontMgr.CreateDynamicTextA(0, "fps:000", 200, 100, NVECTOR4(0,0,0,1.0f), 0, 0, myText_fps);
	myText_fps.SetTextColor(NVECTOR4(0,0.3f,1.0f,0.5f));
	myText_fps.SetDiagonal(NVECTOR2(20, 20),NVECTOR2(150, 50));

	Renderer.SetFillMode(NOISE_FILLMODE_SOLID);
	Renderer.SetCullMode(NOISE_CULLMODE_BACK);//NOISE_CULLMODE_BACK

	//Mesh1.LoadFile_STL("object.stl");
	Mesh1.CreateSphere(5.0f, 30, 30);
	//Mesh1.CreatePlane(10.0f, 10.0f, 5, 5);
	Mesh1.SetPosition(0,0,0);

	Camera.SetPosition(2.0f,0,0);
	Camera.SetLookAt(0,0,0);
	Camera.SetViewAngle(MATH_PI / 2.5f, 1.333333333f);
	Camera.SetViewFrustumPlane(1.0f, 500.f);
	//use bounding box of mesh to init camera pos
	NVECTOR3 AABB_MAX = Mesh1.GetBoundingBoxMax();
	float rotateRadius = sqrtf(AABB_MAX.x*AABB_MAX.x + AABB_MAX.z*AABB_MAX.z)*1.2f;
	float rotateY = Mesh1.GetBoundingBoxMax().y*1.3f;
	Camera.SetPosition(rotateRadius*0.7f, rotateY, rotateRadius*0.7f);
	Camera.SetLookAt(0, rotateY / 2, 0);

	Atmos.SetFogEnabled(FALSE);
	Atmos.SetFogParameter(7.0f, 8.0f, NVECTOR3(0, 0, 1.0f));
	Atmos.CreateSkyDome(4.0f, 4.0f, TexMgr.GetTextureID("Universe"));
	//Atmos.CreateSkyBox(10.0f, 10.0f, 10.0f, TexMgr.GetIndexByName("EnvironmentMap"));

	//——————灯光————————
	DirLight1.mAmbientColor = NVECTOR3(1.0f,1.0f,1.0f);
	DirLight1.mDiffuseColor	=	NVECTOR3(1.0f,1.0f,1.0f);
	DirLight1.mSpecularColor	=NVECTOR3(1.0f,1.0f,1.0f);
	DirLight1.mDirection = NVECTOR3(0.0f, -0.5f, 1.0f);
	DirLight1.mSpecularIntensity	=1.5f;
	DirLight1.mDiffuseIntensity = 1.0f;
	LightMgr.AddDynamicDirLight(DirLight1);

	N_Material Mat1;
	Mat1.baseMaterial.mBaseAmbientColor	= NVECTOR3(0.1f,  0.1f,0.1f);
	Mat1.baseMaterial.mBaseDiffuseColor		= NVECTOR3(1.0f,  1.0f, 1.0f);
	Mat1.baseMaterial.mBaseSpecularColor	=	NVECTOR3(1.0f, 1.0f,1.0f);
	Mat1.baseMaterial.mSpecularSmoothLevel	=	40;
	Mat1.baseMaterial.mNormalMapBumpIntensity = 0.3f;
	Mat1.baseMaterial.mEnvironmentMapTransparency = 0.05f;
	Mat1.diffuseMapID = TexMgr.GetTextureID("Earth");
	Mat1.normalMapID = TexMgr.GetTextureID("EarthNormalMap");
	Mat1.cubeMap_environmentMapID = TexMgr.GetTextureID("EnvironmentMap");
	UINT	 Mat1_ID = MatMgr.CreateMaterial(Mat1);

	//set material
	Mesh1.SetMaterial(Mat1_ID);

	GraphicObjBuffer.AddRectangle(NVECTOR2(340.0f, 430.0f), NVECTOR2(640.0f, 480.0f), NVECTOR4(0.3f, 0.3f, 1.0f, 1.0f),TexMgr.GetTextureID("BottomRightTitle"));
	
	//GUI System
	GUIMgr.SetWindowHWND(hwnd);
	GUIMgr.CreateButton(GUIButton1);
	GUIMgr.CreateScrollBar(GUIScrollBar1);
	GUIButton1.SetCenterPos(50.0f, 50.0f);
	GUIButton1.SetWidth(300.0f);
	GUIButton1.SetHeight(100.0f);
	GUIButton1.SetDragableX(TRUE);
	GUIButton1.SetDragableY(TRUE);
	GUIButton1.SetTexture(NOISE_GUI_BUTTON_STATE_COMMON,TexMgr.GetTextureID("Button"));
	GUIButton1.SetTexture(NOISE_GUI_BUTTON_STATE_MOUSEON,TexMgr.GetTextureID("Earth"));
	GUIButton1.SetTexture(NOISE_GUI_BUTTON_STATE_MOUSEBUTTONDOWN,TexMgr.GetTextureID("Wood"));
	GUIButton1.SetEventProcessCallbackFunction(Button1MsgProc);
	GUIScrollBar1.SetCenterPos(300.0f, 300.0f);
	GUIScrollBar1.SetWidth(30.0f);
	GUIScrollBar1.SetHeight(300.0f);
	GUIScrollBar1.SetTexture_ScrollButton(NOISE_GUI_BUTTON_STATE_COMMON, TexMgr.GetTextureID("Earth"));
	GUIScrollBar1.SetTexture_ScrollButton(NOISE_GUI_BUTTON_STATE_MOUSEON, TexMgr.GetTextureID("Wood"));
	GUIScrollBar1.SetTexture_ScrollButton(NOISE_GUI_BUTTON_STATE_MOUSEBUTTONDOWN, TexMgr.GetTextureID("Button"));
	GUIScrollBar1.SetTexture_ScrollGroove(TexMgr.GetTextureID("EarthNormalMap"));
	GUIScrollBar1.SetAlignment(FALSE);
	GUIScrollBar1.SetValue(0.5f);
	GUIMgr.Update();




	/*Slicer.Step1_LoadPrimitiveMeshFromSTLFile("object.stl");
	Slicer.Step2_Intersection(5);
	Slicer.Step3_GenerateLineStrip();// extremely neccessary for optimization
	Slicer.Step4_SaveLayerDataToFile("object.LayerOutput");*/
	//Slicer.Step3_LoadLineStripsFrom_NOISELAYER_File("object.LayerOutput");

	/*NVECTOR3 v1, v2, n;

	for (UINT i = 0;i < Slicer.GetLineStripCount();i++)
	{

		Slicer.GetLineStrip(lineStrip, i);
		for (UINT j = 0;j < lineStrip.at(i).pointList.size() - 1;j++)
		{
			v1 = lineStrip.at(i).pointList.at(j);
			v2 = lineStrip.at(i).pointList.at(j + 1);
			n = lineStrip.at(i).normalList.at(j);
			//vertex
			GraphicObjBuffer.AddLine3D(v1, v2);
			//normal
			GraphicObjBuffer.AddLine3D((v1 + v2) / 2, ((v1 + v2) / 2) + n / 4, NVECTOR4(1.0f, 1.0f, 1.0f, 1.0f), NVECTOR4(1.0f, 0.2f, 0.2f, 1.0f));
		}
	}*/
	
	return TRUE;
};


void MainLoop()
{
	GUIMgr.Update();
	InputProcess();
	Renderer.ClearBackground();
	NTimer.NextTick();

	//update fps lable
	std::stringstream tmpS;
	tmpS <<"fps :" << NTimer.GetFPS()<<std::endl;//I wonder why this FPS is EXACTLY the half of Graphic Debug FPS
	myText_fps.SetTextAscii(tmpS.str());
	std::stringstream tmpS2;
	tmpS2 << GUIScrollBar1.GetValue();
	float aaaa = GUIScrollBar1.GetValue();
	myText2.SetTextAscii(tmpS2.str());

	//add to render list
	Renderer.AddOjectToRenderList(Mesh1);
	Renderer.AddOjectToRenderList(GraphicObjBuffer);
	Renderer.AddOjectToRenderList(Atmos);
	Renderer.AddOjectToRenderList(myText1);
	Renderer.AddOjectToRenderList(myText2);
	Renderer.AddOjectToRenderList(myText_fps);
	
	Renderer.AddOjectToRenderList(GUIScrollBar1);
	Renderer.AddOjectToRenderList(GUIButton1);

	//render
	Renderer.SetBlendingMode(NOISE_BLENDMODE_OPAQUE);
	Renderer.RenderMeshes();
	Renderer.SetBlendingMode(NOISE_BLENDMODE_OPAQUE);
	Renderer.RenderAtmosphere();
	Renderer.SetBlendingMode(NOISE_BLENDMODE_OPAQUE);
	Renderer.RenderGraphicObjects();
	Renderer.SetBlendingMode(NOISE_BLENDMODE_ALPHA);
	Renderer.RenderTexts();
	Renderer.SetBlendingMode(NOISE_BLENDMODE_ALPHA);
	Renderer.RenderGUIObjects();

	//present
	Renderer.RenderToScreen();
};

void InputProcess()
{
	inputE.Update();

	if (inputE.IsKeyPressed(NOISE_KEY_A))
	{
		Camera.fps_MoveRight(-0.1f, FALSE);
	}
	if (inputE.IsKeyPressed(NOISE_KEY_D))
	{
		Camera.fps_MoveRight(0.1f, FALSE);
	}
	if (inputE.IsKeyPressed(NOISE_KEY_W))
	{
		Camera.fps_MoveForward(0.1f, FALSE);
	}
	if (inputE.IsKeyPressed(NOISE_KEY_S))
	{
		Camera.fps_MoveForward(-0.1f, FALSE);
	}
	if (inputE.IsKeyPressed(NOISE_KEY_SPACE))
	{
		Camera.fps_MoveUp(0.1f);
	}
	if (inputE.IsKeyPressed(NOISE_KEY_LCONTROL))
	{
		Camera.fps_MoveUp(-0.1f);
	}

	if (inputE.IsMouseButtonPressed(NOISE_MOUSEBUTTON_LEFT))
	{
		Camera.RotateY_Yaw((float)inputE.GetMouseDiffX() / 200.0f);
		Camera.RotateX_Pitch((float)inputE.GetMouseDiffY() / 200.0f);
	}

	//quit main loop and terminate program
	if (inputE.IsKeyPressed(NOISE_KEY_ESCAPE))
	{
		Engine.SetMainLoopStatus(NOISE_MAINLOOP_STATUS_QUIT_LOOP);
	}

};


void Cleanup()
{
	inputE.SelfDestruction();
	Slicer.SelfDestruction();
	NTimer.SelfDestruction();

	Engine.ReleaseAll();
	Scene.ReleaseAllChildObject();
};


UINT Button1MsgProc(UINT msg)
{
	switch (msg)
	{
	case NOISE_GUI_EVENTS_BUTTON_MOUSEON:
		break;

	case NOISE_GUI_EVENTS_BUTTON_MOUSEPRESSED:
		break;

	case NOISE_GUI_EVENTS_BUTTON_MOUSEMOVE:
		break;
	default:
		break;
	}
	return 0;
}