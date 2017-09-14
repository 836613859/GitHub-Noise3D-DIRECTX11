#include "Noise3D.h"
#include <sstream>

BOOL Init3D(HWND hwnd);
void MainLoop();
void Cleanup();
void	InputProcess();

using namespace Noise3D;

IRoot* pRoot;
IRenderer* pRenderer;
IScene* pScene;
ICamera* pCamera;
IAtmosphere* pAtmos;
IModelLoader* pModelLoader;
IMeshManager* pMeshMgr;
IMesh* pMesh1;
IMesh* pMesh2;
IMaterialManager*	pMatMgr;
ITextureManager*	pTexMgr;
IGraphicObjectManager*	pGraphicObjMgr;
IGraphicObject*	pGraphicObjBuffer;
ILightManager* pLightMgr;
IDirLightD*		pDirLight1;
IPointLightD*	pPointLight1;
ISpotLightD*	pSpotLight1;
IFontManager* pFontMgr;
IDynamicText* pMyText_fps;


Ut::ITimer NTimer(NOISE_TIMER_TIMEUNIT_MILLISECOND);
Ut::IInputEngine inputE;

//Main Entry
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
{

	//get Root interface
	pRoot = GetRoot();

	//create a window (using default window creation function)
	HWND windowHWND;
	windowHWND = pRoot->CreateRenderWindow(960, 720, L"Hahaha Render Window", hInstance);

	//initialize input engine (detection for keyboard and mouse input)
	inputE.Initialize(hInstance, windowHWND);

	//D3D and scene object init
	Init3D(windowHWND);

	//register main loop function
	pRoot->SetMainLoopFunction(MainLoop);

	//enter main loop
	pRoot->Mainloop();

	//program end
	Cleanup();

	//..
	return 0;
}

BOOL Init3D(HWND hwnd)
{
	const UINT bufferWidth = 960;
	const UINT bufferHeight = 720;

	//初始化失败
	if (!pRoot->InitD3D(hwnd))return FALSE;

	//query pointer to IScene
	pScene = pRoot->GetScenePtr();

	//retrieve meshMgr and Create new mesh
	pMeshMgr = pScene->GetMeshMgr();

	//use "myMesh1" string to initialize UID (unique-Identifier)
	pMesh1 = pMeshMgr->CreateMesh("liver");
	pMesh2 = pMeshMgr->CreateMesh("cancer");


	pRenderer = pScene->CreateRenderer(bufferWidth, bufferHeight, TRUE);
	pCamera = pScene->GetCamera();
	pLightMgr = pScene->GetLightMgr();
	pMatMgr = pScene->GetMaterialMgr();
	pTexMgr = pScene->GetTextureMgr();
	pAtmos = pScene->GetAtmosphere();
	pGraphicObjMgr = pScene->GetGraphicObjMgr();


	//漫反射贴图
	pTexMgr->CreateTextureFromFile("media/red.jpg", "Red", TRUE, 1024, 1024, FALSE);
	pTexMgr->CreateTextureFromFile("media/Jade.jpg", "Jade", FALSE, 256, 256, FALSE);
	pTexMgr->CreateTextureFromFile("media/universe2.jpg", "Universe", FALSE, 256, 256, FALSE);
	pTexMgr->CreateTextureFromFile("media/bottom-right-conner-title.jpg", "BottomRightTitle", TRUE, 0, 0, FALSE);


	//create font texture
	pFontMgr = pScene->GetFontMgr();
	pFontMgr->CreateFontFromFile("media/STXINWEI.ttf", "myFont", 24);
	pMyText_fps = pFontMgr->CreateDynamicTextA("myFont", "fpsLabel", "fps:000", 200, 100, NVECTOR4(0, 0, 0, 1.0f), 0, 0);
	pMyText_fps->SetTextColor(NVECTOR4(0, 0.3f, 1.0f, 0.5f));
	pMyText_fps->SetDiagonal(NVECTOR2(20, 20), NVECTOR2(150, 60));
	pMyText_fps->SetFont("myFont");

	pRenderer->SetFillMode(NOISE_FILLMODE_WIREFRAME);
	pRenderer->SetCullMode(NOISE_CULLMODE_NONE);//NOISE_CULLMODE_BACK

	//------------------MESH INITIALIZATION----------------
	pModelLoader->LoadFile_STL(pMesh1, "model/liver_mc.stl");//这个破模型的法线YZ没有翻转
	pModelLoader->LoadFile_STL(pMesh2, "model/liver_cancer_mc.stl");
	//pModelLoader->LoadSphere(pMesh1, 5.0f);
	//pModelLoader->LoadBox(pMesh1, 10.0f, 10.0f, 10.0f);
	pMesh1->SetPosition(10.8f, 0, 10.0f);
	pMesh1->SetScale(1.1f, 1.1f, 1.1f);
	pMesh2->SetPosition(10.0f, 0, 10.0f);
	IModelProcessor* pModelProc = pScene->GetModelProcessor();
	pModelProc->WeldVertices(pMesh1,1.0f);
	pModelProc->Smooth_Laplacian(pMesh1);
	pModelProc->WeldVertices(pMesh2, 1.0f);
	pModelProc->Smooth_Laplacian(pMesh2);

	const std::vector<N_DefaultVertex>* pTmpVB;
	pTmpVB = pMesh1->GetVertexBuffer();
	pGraphicObjBuffer = pGraphicObjMgr->CreateGraphicObj("Axis");
	pGraphicObjBuffer->AddLine3D({ 0,0,0 }, { 20.0f,0,0 }, { 1.0f,1.0f,1.0f,1.0f }, { 1.0f,0,0,0 });
	pGraphicObjBuffer->AddLine3D({ 0,0,0 }, { 0,20.0f,0 }, { 1.0f,1.0f,1.0f,1.0f }, { 0,1.0f,0,0 });
	pGraphicObjBuffer->AddLine3D({ 0,0,0 }, { 0,0,20.0f }, { 1.0f,1.0f,1.0f,1.0f }, { 0,0,1.0f,0 });
	/*for (auto v : tmpVB)
	{
	pGraphicObjBuffer->AddLine3D(v.Pos, v.Pos + 5.0f*v.Normal, NVECTOR4(1.0f, 0, 0, 1.0f), NVECTOR4(1.0f, 1.0f, 1.0f, 1.0f));//draw the normal
	pGraphicObjBuffer->AddLine3D(v.Pos, v.Pos + 5.0f*v.Tangent, NVECTOR4(0,0, 1.0f, 1.0f), NVECTOR4(1.0f, 1.0f, 1.0f, 1.0f));//draw the tangent
	}*/

	//----------------------------------------------------------

	pCamera->SetPosition(2.0f, 0, 0);
	pCamera->SetLookAt(0, 0, 0);
	pCamera->SetViewAngle(MATH_PI / 2.5f, 1.333333333f);
	pCamera->SetViewFrustumPlane(1.0f, 500.f);
	//use bounding box of mesh to init camera pos
	N_Box meshAABB = pMesh1->ComputeBoundingBox();
	float rotateRadius = sqrtf(meshAABB.max.x*meshAABB.max.x + meshAABB.max.z*meshAABB.max.z)*1.2f;
	float rotateY = meshAABB.max.y*1.3f;
	pCamera->SetPosition(rotateRadius*0.7f, rotateY, rotateRadius*0.7f);
	pCamera->SetLookAt(0, 0, 0);

	pAtmos->SetFogEnabled(FALSE);
	pAtmos->SetFogParameter(7.0f, 8.0f, NVECTOR3(0, 0, 1.0f));
	pAtmos->CreateSkyDome(4.0f, 4.0f, "Universe");


	//------------------灯光-----------------
	pDirLight1 = pLightMgr->CreateDynamicDirLight("myDirLight1");
	N_DirLightDesc dirLightDesc;
	dirLightDesc.ambientColor = NVECTOR3(1.0f, 1.0f, 1.0f);
	dirLightDesc.diffuseColor = NVECTOR3(1.0f, 1.0f, 1.0f);
	dirLightDesc.specularColor = NVECTOR3(1.0f, 1.0f, 1.0f);
	dirLightDesc.direction = NVECTOR3(0, 0, 0);//just init
	dirLightDesc.specularIntensity = 1.0f;
	dirLightDesc.diffuseIntensity = 1.0f;
	pDirLight1->SetDesc(dirLightDesc);

	//-------------------材质----------------

	//Material 1 : liver
	N_MaterialDesc mat;
	mat.ambientColor = NVECTOR3(0.3f, 0.3f, 0.3f);
	mat.diffuseColor = NVECTOR3(1.0f, 1.0f, 1.0f);
	mat.specularColor = NVECTOR3(1.0f, 1.0f, 1.0f);
	mat.specularSmoothLevel = 40;
	mat.normalMapBumpIntensity = 0.2f;
	mat.environmentMapTransparency = 0.05f;
	mat.transparency = 0.2f;
	mat.diffuseMapName = "Red";
	pMatMgr->CreateMaterial("meshMat1", mat);
	//set material
	pMesh1->SetMaterial("meshMat1");

	//Material 2: cancer
	N_MaterialDesc mat2;
	mat2.ambientColor = NVECTOR3(0.3f, 0.3f, 0.3f);
	mat2.diffuseColor = NVECTOR3(1.0f, 1.0f, 0.0f);
	mat2.specularColor = NVECTOR3(1.0f, 1.0f, 0.0f);
	mat2.specularSmoothLevel = 20;
	mat2.normalMapBumpIntensity = 0.2f;
	mat2.environmentMapTransparency = 0.05f;
	mat2.transparency = 1.0f;
	pMatMgr->CreateMaterial("cancerMat", mat2);
	//set material
	pMesh2->SetMaterial("cancerMat");

	//bottom right
	pGraphicObjBuffer->AddRectangle(NVECTOR2(800.0, 680.0f), NVECTOR2(960.0f, 720.0f), NVECTOR4(0.3f, 0.3f, 1.0f, 1.0f), "BottomRightTitle");

	return TRUE;
};


void MainLoop()
{
	static float incrNum = 0.0;
	incrNum += 0.001f;
	pDirLight1->SetDirection(NVECTOR3(sin(incrNum), -1.0f, cos(incrNum)));

	//GUIMgr.Update();
	InputProcess();
	pRenderer->ClearBackground();
	NTimer.NextTick();

	//update fps lable
	std::stringstream tmpS;
	tmpS << "fps :" << NTimer.GetFPS() << std::endl;
	pMyText_fps->SetTextAscii(tmpS.str());


	//add to render list
	pRenderer->AddObjectToRenderList(pMesh2);
	pRenderer->AddObjectToRenderList(pMesh1);
	pRenderer->AddObjectToRenderList(pGraphicObjBuffer);
	pRenderer->AddObjectToRenderList(pAtmos);
	pRenderer->AddObjectToRenderList(pMyText_fps);

	//render
	pRenderer->SetBlendingMode(NOISE_BLENDMODE_OPAQUE);
	pRenderer->RenderAtmosphere();
	pRenderer->SetBlendingMode(NOISE_BLENDMODE_ALPHA);
	pRenderer->RenderMeshes();
	pRenderer->SetBlendingMode(NOISE_BLENDMODE_ADDITIVE);
	pRenderer->RenderGraphicObjects();
	pRenderer->SetBlendingMode(NOISE_BLENDMODE_ALPHA);
	pRenderer->RenderTexts();

	//present
	pRenderer->PresentToScreen();
};

void InputProcess()
{
	inputE.Update();

	if (inputE.IsKeyPressed(Ut::NOISE_KEY_A))
	{
		pCamera->fps_MoveRight(-0.1f, FALSE);
	}
	if (inputE.IsKeyPressed(Ut::NOISE_KEY_D))
	{
		pCamera->fps_MoveRight(0.1f, FALSE);
	}
	if (inputE.IsKeyPressed(Ut::NOISE_KEY_W))
	{
		pCamera->fps_MoveForward(0.1f, FALSE);
	}
	if (inputE.IsKeyPressed(Ut::NOISE_KEY_S))
	{
		pCamera->fps_MoveForward(-0.1f, FALSE);
	}
	if (inputE.IsKeyPressed(Ut::NOISE_KEY_SPACE))
	{
		pCamera->fps_MoveUp(0.1f);
	}
	if (inputE.IsKeyPressed(Ut::NOISE_KEY_LCONTROL))
	{
		pCamera->fps_MoveUp(-0.1f);
	}

	if (inputE.IsMouseButtonPressed(Ut::NOISE_MOUSEBUTTON_LEFT))
	{
		pCamera->RotateY_Yaw((float)inputE.GetMouseDiffX() / 200.0f);
		pCamera->RotateX_Pitch((float)inputE.GetMouseDiffY() / 200.0f);
	}

	//quit main loop and terminate program
	if (inputE.IsKeyPressed(Ut::NOISE_KEY_ESCAPE))
	{
		pRoot->SetMainLoopStatus(NOISE_MAINLOOP_STATUS_QUIT_LOOP);
	}

};


void Cleanup()
{
	pRoot->ReleaseAll();
};
