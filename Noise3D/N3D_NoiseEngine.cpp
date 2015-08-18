
/***********************************************************************

                           类：NOISE Engine

			简述：主要负责管理全局接口和变量，初始化等

************************************************************************/

#include "Noise3D.h"

//构造函数
NoiseEngine::NoiseEngine()
{
	mRenderWindowTitle = L"Noise 3D - Render Window";
	m_pMainLoopFunction = nullptr;
}

NoiseEngine::~NoiseEngine()
{
	ReleaseAll();
}

HWND NoiseEngine::CreateRenderWindow(UINT pixelWidth, UINT pixelHeight, LPCWSTR windowTitle, HINSTANCE hInstance)
{

	WNDCLASS wndclass;
	HWND outHWND;//句柄
	mRenderWindowHINSTANCE = hInstance;
	mRenderWindowTitle = windowTitle;
	mRenderWindowClassName = L"Noise Engine Render Window";

	//窗体类注册
	if (mFunction_InitWindowClass(&wndclass) == FALSE)
	{
		DEBUG_MSG1("Window Class 创建失败");
		return FALSE;
	};

	//创建窗体

	outHWND = mFunction_InitWindow();
	if (outHWND == 0)
	{
		DEBUG_MSG1("窗体创建失败");
		return FALSE;
	};

	return outHWND;
};

BOOL NoiseEngine::InitD3D(HWND RenderHWND, UINT BufferWidth, UINT BufferHeight, BOOL IsWindowed)
{

	mRenderBufferPixelWidth = BufferWidth;
	mRenderBufferPixelHeight = BufferHeight;

	HRESULT hr = S_OK;
#pragma region InitDevice
	//用来做判断及返回结果

	//硬件驱动类型
	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,	//HAL 硬件驱动
		D3D_DRIVER_TYPE_REFERENCE,	//REF参考设备
		D3D_DRIVER_TYPE_WARP,		//Windows Advanced Rasterization Platform只支持DX10.1
	};
	UINT numDriverTypes = ARRAYSIZE(driverTypes);

	//D3D特性的版本
	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	//设备创建标签 
	UINT createDeviceFlags = 0;
#if defined(DEBUG)||defined(_DEBUG)		//D3D调试模式
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif


	//用列举出来的硬件方式 尝试初始化 直到成功
	UINT driverTypeIndex = 0;
	for (driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
	{
		//D3D_DRIVER_TYPE 
		g_Device_driverType = driverTypes[driverTypeIndex];
		hr = D3D11CreateDevice(
			NULL,				//null表示使用主显示器
			g_Device_driverType,		//驱动类型 HAL/REF
			NULL,
			D3D11_CREATE_DEVICE_DEBUG,//createDeviceFlags,	//是不是调试模式	
			featureLevels,		//让D3D选择的特性的版本
			numFeatureLevels,
			D3D11_SDK_VERSION,
			&g_pd3dDevice,		//返回D3D设备指针
			&g_Device_featureLevel,	//返回最终使用的特性的版本
			&g_pImmediateContext//返回
			);
		//创建成功了就不用继续尝试创建
		if (SUCCEEDED(hr))
		{
			break;
		};
	};
	//尝试创建设备失败
	HR_DEBUG(hr, "d3d设备创建失败");


	//检测多重采样
	g_pd3dDevice->CheckMultisampleQualityLevels(
		DXGI_FORMAT_R8G8B8A8_UNORM, 4, &g_Device_MSAA4xQuality);//4x坑锯齿一般都支持，这个返回值一般情况下都大于0
	if (g_Device_MSAA4xQuality > 0)
	{
		g_Device_MSAA4xEnabled = TRUE;	//4x抗锯齿可以开了
	};



	/*填充交换链的属性
	交换链，用于管理BUFEER的交换，主要处理back与front
	可以用于多窗口渲染
	DESC = Description*/
	DXGI_SWAP_CHAIN_DESC SwapChainParam;
	ZeroMemory(&SwapChainParam, sizeof(SwapChainParam));
	SwapChainParam.BufferCount = 1;
	SwapChainParam.BufferDesc.Width = mRenderBufferPixelWidth;
	SwapChainParam.BufferDesc.Height = mRenderBufferPixelHeight;
	SwapChainParam.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	SwapChainParam.BufferDesc.RefreshRate.Numerator = 60;//	分子= =？
	SwapChainParam.BufferDesc.RefreshRate.Denominator = 1;//分母
	SwapChainParam.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;//BACKBUFFER怎么被使用
	SwapChainParam.OutputWindow = RenderHWND;
	SwapChainParam.Windowed = IsWindowed;

	SwapChainParam.SampleDesc.Count = (g_Device_MSAA4xEnabled = TRUE ? 4 : 1);//多重采样倍数
	SwapChainParam.SampleDesc.Quality = (g_Device_MSAA4xEnabled = TRUE ? g_Device_MSAA4xQuality - 1 : 0);//quality之前获取了

	 //下面的COM的QueryInterface 用一个接口查询另一个接口
	IDXGIDevice *dxgiDevice = 0;
	g_pd3dDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice);
	IDXGIAdapter *dxgiAdapter = 0;
	dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&dxgiAdapter);
	IDXGIFactory *dxgiFactory = 0;
	dxgiAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&dxgiFactory);

	//终于创建了一个交换链
	hr = dxgiFactory->CreateSwapChain(
		g_pd3dDevice,		//设备的指针
		&SwapChainParam,	//交换链的描述
		&g_pSwapChain);		//返回的交换链指针
	HR_DEBUG(hr, "SwapChain创建失败！");

	dxgiFactory->Release();
	dxgiDevice->Release();
	dxgiAdapter->Release();
#pragma endregion InitDevice

	//创建缓冲区和渲染视口，深度/模版 视口
	//这些Views是用来绑定到pipeline上
#pragma region CreateViews

	// 创建一个(可以多个)渲染视图RENDER TARGET VIEW
	ID3D11Texture2D* pBackBuffer = NULL;
	hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
	if (FAILED(hr))
		return FALSE;

	hr = g_pd3dDevice->CreateRenderTargetView(
		pBackBuffer,
		NULL,					//可以填充一个D3D11_RENDERTARGETVIEW_DESC
		&g_pRenderTargetView);	//返回一个渲染视口

	pBackBuffer->Release();		//已经用完了的临时接口- -

	HR_DEBUG(hr, "创建RENDER TARGET VIEW失败");



	//创建depth/stencil view
	D3D11_TEXTURE2D_DESC DSBufferDesc;
	DSBufferDesc.Width = BufferWidth;
	DSBufferDesc.Height = BufferHeight;
	DSBufferDesc.MipLevels = 1;
	DSBufferDesc.ArraySize = 1;
	DSBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	DSBufferDesc.SampleDesc.Count = (g_Device_MSAA4xEnabled = TRUE ? 4 : 1);
	DSBufferDesc.SampleDesc.Quality = (g_Device_MSAA4xEnabled = TRUE ? g_Device_MSAA4xQuality - 1 : 0);
	DSBufferDesc.Usage = D3D11_USAGE_DEFAULT;	//尽量避免DYNAMIC和STAGING
	DSBufferDesc.CPUAccessFlags = 0;	//CPU不能碰它 GPU才行 这样能够加快
	DSBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;//和PIPELINE的绑定
	DSBufferDesc.MiscFlags = 0;

	ID3D11Texture2D* pDepthStencilBuffer;
	g_pd3dDevice->CreateTexture2D(&DSBufferDesc, 0, &pDepthStencilBuffer);//创建一个缓冲区
	hr = g_pd3dDevice->CreateDepthStencilView(
		pDepthStencilBuffer,
		0,
		&g_pDepthStencilView);	//返回一个depth/stencil视口指针

	pDepthStencilBuffer->Release();
	if (FAILED(hr))
	{
		return FALSE;
	};


	//设置渲染对象：刚刚创建的渲染视口和depth/stencil的
	//这就是绑定到pipeline
	g_pImmediateContext->OMSetRenderTargets(
		1,
		&g_pRenderTargetView,
		g_pDepthStencilView);

#pragma endregion CreateViews


	//XY都是-1到1，深度Z是0到1，DX11不会默认创建视口，DX9就会
#pragma region CreateViewPort

	D3D11_VIEWPORT vp;
	vp.Width = (FLOAT)BufferWidth;		//视口WIDTH 跟后缓冲区一样
	vp.Height = (FLOAT)BufferHeight;	//视口Height
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	//SetViewport 参数1：视口的个数 参数2：视口数组的首地址
	g_pImmediateContext->RSSetViewports(1, &vp);

#pragma endregion CreateViewPort


	//创建预设的光栅化state
	//Create Raster State;If you want various Raster State,you should pre-Create all of them in the beginning
#pragma region CreateRasterState

	D3D11_RASTERIZER_DESC tmpRasterStateDesc;//光栅化设置
	ZeroMemory(&tmpRasterStateDesc, sizeof(D3D11_RASTERIZER_DESC));
	tmpRasterStateDesc.AntialiasedLineEnable = TRUE;//抗锯齿设置
	tmpRasterStateDesc.CullMode = D3D11_CULL_NONE;//剔除模式
	tmpRasterStateDesc.FillMode = D3D11_FILL_SOLID;
	hr = g_pd3dDevice->CreateRasterizerState(&tmpRasterStateDesc, &g_pRasterState_FillMode_Solid);
	HR_DEBUG(hr, "创建RASTERIZER STATE_solid失败");

	tmpRasterStateDesc.FillMode = D3D11_FILL_WIREFRAME;
	hr = g_pd3dDevice->CreateRasterizerState(&tmpRasterStateDesc, &g_pRasterState_FillMode_WireFrame);
	HR_DEBUG(hr, "创建RASTERIZER STATE_wireframe失败");

#pragma endregion CreateRasterState


	return TRUE;

};

void NoiseEngine::ReleaseAll()//考虑下在构造函数那弄个AddToReleaseList呗
{
	g_Debug_MsgString.clear();
	g_pImmediateContext->Flush();
	g_pImmediateContext->ClearState();

	ReleaseCOM(g_pRenderTargetView);
	ReleaseCOM(g_pSwapChain);
	ReleaseCOM(g_pVertexLayout_Default);
	ReleaseCOM(g_pVertexLayout_Simple);
	ReleaseCOM(g_pRenderTargetView);
	ReleaseCOM(g_pDepthStencilView);
	ReleaseCOM(g_pImmediateContext);
	ReleaseCOM(g_pRasterState_FillMode_Solid);
	ReleaseCOM(g_pRasterState_FillMode_WireFrame);
	//g_pd3dDevice->Release();
	ReleaseCOM(g_pd3dDevice);

}

void NoiseEngine::Mainloop()
{
	MSG msg;//消息体
	ZeroMemory(&msg, sizeof(msg));

	//在程序还没有接收到“退出”消息的时候
	while (msg.message != WM_QUIT)
	{
		/*PM_REMOVE  PeekMessage处理后，消息从队列里除掉。
		而函数PeekMesssge是以查看的方式从系统中获取消息，
		可以不将消息从系统中移除，是非阻塞函数；
		当系统无消息时，返回FALSE，继续执行后续代码。*/

		//有消息的时候赶紧处理了 即Peek返回TRUE的时候
		if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
		{
			TranslateMessage(&msg);//翻译消息并发送到windows消息队列
			DispatchMessage(&msg);//接收信息
		}
		else
			//消息队列没东西处理了 那就搞主循环
		{
			switch (mMainLoopStatus)
			{
			case NOISE_MAINLOOP_STATUS_BUSY:
				//如果主循环函数有效
				if (m_pMainLoopFunction)
				{
					(*m_pMainLoopFunction)();
				}
					break;

			case NOISE_MAINLOOP_STATUS_QUIT_LOOP:
				return;
				break;

			default:
				break;

			}//switch
		}
	};
	
	UnregisterClass(mRenderWindowClassName, mRenderWindowHINSTANCE);
}

void NoiseEngine::SetMainLoopFunction( void (*pFunction)(void) )
{
	m_pMainLoopFunction =pFunction;
}

void NoiseEngine::SetMainLoopStatus(NOISE_MAINLOOP_STATUS loopStatus)
{
	mMainLoopStatus = loopStatus;
};


/************************************************************************
										 PRIVATE                               
************************************************************************/

//windows message handler
static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)//消息的处理程序
{
	/*HDC			hdc ;
	PAINTSTRUCT ps ;
	RECT        rect ;*/
	switch (message)
	{
	case WM_CREATE:
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hwnd, message, wParam, lParam);

}

BOOL NoiseEngine::mFunction_InitWindowClass(WNDCLASS* wc)
{
	wc->style = CS_HREDRAW | CS_VREDRAW; //样式
	wc->cbClsExtra = 0;
	wc->cbWndExtra = 0;
	wc->hInstance = mRenderWindowHINSTANCE;//窗体实例名，由windows自动分发
	wc->hIcon = LoadIcon(NULL, IDI_APPLICATION);//显示上面的图标titlte
	wc->hCursor = LoadCursor(NULL, IDC_ARROW);//窗口光标
	wc->hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);//背景刷
	wc->lpszMenuName = NULL;
	wc->lpfnWndProc = WndProc;//设置窗体接收windws消息函数
	wc->lpszClassName = mRenderWindowClassName;//窗体类名

	if (!RegisterClass(wc))//注册窗体类
	{
		MessageBox(NULL, TEXT("This program requires Windows NT!"), TEXT("R"), MB_ICONERROR);
		return FALSE;
	}
	else
	{
		return TRUE;
	};

};

HWND NoiseEngine::mFunction_InitWindow()
{
	HWND hwnd;
	hwnd = CreateWindow(
		mRenderWindowClassName,      // window class name
		mRenderWindowTitle,   // window caption
		WS_OVERLAPPEDWINDOW, // window style
		CW_USEDEFAULT,// initial x position
		CW_USEDEFAULT,// initial y position
		640,// initial x size
		480,// initial y size
		NULL, // parent window handle
		NULL, // window menu handle
		mRenderWindowHINSTANCE, // program instance handle
		NULL);

	if (hwnd == 0)
	{
		//创建失败
		return 0;
	}
	else
	{
		//创建成功
		ShowWindow(hwnd, SW_RESTORE);//显示窗口
		UpdateWindow(hwnd);//更新窗体
		return hwnd;
	};

};
