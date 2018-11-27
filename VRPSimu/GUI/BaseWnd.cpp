#include <random>
#include <functional>
#include "BaseWnd.h"
#include "../Model/GraphModel.h"
#include "../Model/Solution.h"
#pragma comment(lib, "d2d1")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "windowscodecs.lib")

static UINT32 time_slice = 5000;
static float animate_time;
static int brush_number;
static vector<bool> showlocus;


D2D1_POINT_2F BaseWnd::GC_To_D2Point(GraphCoor gc)
{
	D2D1_SIZE_F rtSize = pRenderTarget->GetSize();
	GraphModel gm = std::get<0>(*model);
	int width = static_cast<int>(rtSize.width);
	int height = static_cast<int>(rtSize.height);
	float x = gc.xcoor * min(width, height) / gm.geo_size + width / 2;
	float y = -gc.ycoor * min(width, height) / gm.geo_size + height / 2;
	return D2D1::Point2F(x, y);
}

HRESULT BaseWnd::Initialize(std::tuple<GraphModel&, Solution&>* model)
//HRESULT BaseWnd::Initialize()
{
	this->model = model;
	Solution sln = std::get<1>(*model);
	timer_time = sln.graph_model.vertices.begin()->strt_time;

	HRESULT hr;

	// Initialize device-indpendent resources, such
	// as the Direct2D factory.
	hr = CreateDeviceIndependentResources();

	if (SUCCEEDED(hr))
	{
		// Register the window class.
		WNDCLASSEX wcex = { sizeof(WNDCLASSEX) };
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = BaseWnd::WndProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = sizeof(LONG_PTR);
		wcex.hInstance = HINST_THISCOMPONENT;
		wcex.hbrBackground = NULL;

		wcex.lpszMenuName = NULL;
		wcex.hCursor = LoadCursor(NULL, IDI_APPLICATION);
		wcex.lpszClassName = L"BaseWnd";

		RegisterClassEx(&wcex);


		// Because the CreateWindow function takes its size in pixels,
		// obtain the system DPI and use it to scale the window size.
		FLOAT dpiX, dpiY;

		// The factory returns the current system DPI. This is also the value it will use
		// to create its own windows.
		pDirect2dFactory->GetDesktopDpi(&dpiX, &dpiY);

		TCHAR lpClassName[] = TEXT("BaseWnd");



		// Create the window.
		hwnd = CreateWindow(
			lpClassName,
			L"VRPSimulation App",
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			static_cast<UINT>(ceil(640.f * dpiX / 96.f)),
			static_cast<UINT>(ceil(480.f * dpiY / 96.f)),
			NULL,
			NULL,
			HINST_THISCOMPONENT,
			this
		);
		DWORD error = GetLastError();
		hr = hwnd ? S_OK : E_FAIL;
		if (SUCCEEDED(hr))
		{
			ShowWindow(hwnd, SW_SHOWNORMAL);
			UpdateWindow(hwnd);
		}
	}

	return hr;
}

BaseWnd::BaseWnd() :
	hwnd(NULL),
	pCarBitmap(NULL),
	pShopBitmap(NULL),
	pHouseBitmap(NULL),
	pDirect2dFactory(NULL),
	pIWICFactory(NULL),
	pRenderTarget(NULL),
	pBitmapRenderTarget(NULL),
	pLightSlateGrayBrush(NULL),
	pCornflowerBlueBrush(NULL),
	pPaleVioletRedBrush(NULL),
	pLightSeaGreenBrush(NULL),
	pDWriteFactory(NULL),
	pTextFormat(NULL),
	pMouse(D2D1::Point2F()),
	model(NULL)
{
}

HRESULT BaseWnd::CreateDeviceIndependentResources()
{
	HRESULT hr = S_OK;

	// Create a Direct2D factory.
	hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pDirect2dFactory);
	hr = DWriteCreateFactory(
		DWRITE_FACTORY_TYPE_SHARED,
		__uuidof(IDWriteFactory),
		reinterpret_cast<IUnknown**>(&pDWriteFactory)
	);

	if (SUCCEEDED(hr))
	{
		hr = pDWriteFactory->CreateTextFormat(
			L"Serif",                // Font family name.
			NULL,                       // Font collection (NULL sets it to use the system font collection).
			DWRITE_FONT_WEIGHT_REGULAR,
			DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL,
			10.0f,
			L"en-us",
			&pTextFormat
		);
	}
	if (SUCCEEDED(hr))
	{
		hr = pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	}

	if (SUCCEEDED(hr))
	{
		hr = pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
	}
	return hr;
}

HRESULT BaseWnd::CreateDeviceDependentResources()
{
	HRESULT hr = S_OK;

	if (!pRenderTarget)
	{
		RECT rc;
		GetClientRect(hwnd, &rc);

		//D2D1_SIZE_U size = D2D1::SizeU(
		//	rc.bottom - rc.top,
		//	rc.bottom - rc.top
		//);
		D2D1_SIZE_U size = D2D1::SizeU(
			rc.right - rc.left,
			rc.bottom - rc.top
		);

		// Create a Direct2D render target.
		hr = pDirect2dFactory->CreateHwndRenderTarget(
			D2D1::RenderTargetProperties(),
			D2D1::HwndRenderTargetProperties(hwnd, size),
			&pRenderTarget
		);

		//D2D1_SIZE_U size = D2D1::SizeU(
		//	rc.bottom - rc.top,
		//	rc.bottom - rc.top
		//);

		if (SUCCEEDED(hr))
		{
			// create bitmap render target
		}

		if (SUCCEEDED(hr))
		{
			// Create a gray brush.
			hr = pRenderTarget->CreateSolidColorBrush(
				D2D1::ColorF(D2D1::ColorF::LightSlateGray),
				&pLightSlateGrayBrush
			);
		}
		if (SUCCEEDED(hr))
		{
			// Create a blue brush.
			hr = pRenderTarget->CreateSolidColorBrush(
				D2D1::ColorF(D2D1::ColorF::CornflowerBlue),
				&pCornflowerBlueBrush
			);
		}
		if (SUCCEEDED(hr))
		{
			hr = pRenderTarget->CreateSolidColorBrush(
				D2D1::ColorF(D2D1::ColorF::PaleVioletRed),
				&pPaleVioletRedBrush
			);
		}
		if (SUCCEEDED(hr))
		{
			hr = pRenderTarget->CreateSolidColorBrush(
				D2D1::ColorF(D2D1::ColorF::LightSeaGreen),
				&pLightSeaGreenBrush
			);
		}
		if (SUCCEEDED(hr))
		{
			// 0 red
			ID2D1SolidColorBrush* brush;
			hr = pRenderTarget->CreateSolidColorBrush(
				D2D1::ColorF(D2D1::ColorF::Red),
				&brush
			);
			brush_pool.push_back(brush);
		}
		if (SUCCEEDED(hr))
		{
			// 1 orange
			ID2D1SolidColorBrush* brush;
			hr = pRenderTarget->CreateSolidColorBrush(
				D2D1::ColorF(D2D1::ColorF::Orange),
				&brush
			);
			brush_pool.push_back(brush);
		}
		if (SUCCEEDED(hr))
		{
			// 2 yellow
			ID2D1SolidColorBrush* brush;
			hr = pRenderTarget->CreateSolidColorBrush(
				D2D1::ColorF(D2D1::ColorF::Yellow),
				&brush
			);
			brush_pool.push_back(brush);
		}
		if (SUCCEEDED(hr))
		{
			// 3 green
			ID2D1SolidColorBrush* brush;
			hr = pRenderTarget->CreateSolidColorBrush(
				D2D1::ColorF(D2D1::ColorF::Green),
				&brush
			);
			brush_pool.push_back(brush);
		}
		if (SUCCEEDED(hr))
		{
			// 4 blue
			ID2D1SolidColorBrush* brush;
			hr = pRenderTarget->CreateSolidColorBrush(
				D2D1::ColorF(D2D1::ColorF::Blue),
				&brush
			);
			brush_pool.push_back(brush);
		}
		if (SUCCEEDED(hr))
		{
			// 5 indigo
			ID2D1SolidColorBrush* brush;
			hr = pRenderTarget->CreateSolidColorBrush(
				D2D1::ColorF(D2D1::ColorF::Indigo),
				&brush
			);
			brush_pool.push_back(brush);
		}
		if (SUCCEEDED(hr))
		{
			// 6 violet
			ID2D1SolidColorBrush* brush;
			hr = pRenderTarget->CreateSolidColorBrush(
				D2D1::ColorF(D2D1::ColorF::Violet),
				&brush
			);
			brush_pool.push_back(brush);
		}
		brush_number = brush_pool.size();
		if (SUCCEEDED(hr))
		{
			HRESULT hr = CoCreateInstance(
				CLSID_WICImagingFactory,
				NULL,
				CLSCTX_INPROC_SERVER,
				IID_IWICImagingFactory,
				(LPVOID*)&pIWICFactory
			);
		}
		//if (SUCCEEDED(hr))
		//{
		//	pBitmapRenderTarget->BeginDraw();
		//	pBitmapRenderTarget->
		//		pBitmapRenderTarget->EndDraw();
		//}
		if (SUCCEEDED(hr))
		{
			std::wstring str = L"img\\car.png";
			PCWSTR cstr = str.c_str();
			HRESULT hr = loadBitmapFromFile(pRenderTarget, pIWICFactory, cstr, 20, 20, &pCarBitmap);
			auto bitsize = pCarBitmap->GetSize();
			if (SUCCEEDED(hr))
			{
				std::wstring str = L"img\\shop.png";
				PCWSTR cstr = str.c_str();
				HRESULT hr = loadBitmapFromFile(pRenderTarget, pIWICFactory, cstr, 20, 20, &pShopBitmap);
				if (SUCCEEDED(hr))
				{
					std::wstring str = L"img\\house.png";
					PCWSTR cstr = str.c_str();
					HRESULT hr = loadBitmapFromFile(pRenderTarget, pIWICFactory, cstr, 20, 20, &pHouseBitmap);
					D2D1_BITMAP_BRUSH_PROPERTIES brushProperties =
						D2D1::BitmapBrushProperties(D2D1_EXTEND_MODE_WRAP, D2D1_EXTEND_MODE_WRAP);
					hr = pRenderTarget->CreateBitmapBrush(pCarBitmap, brushProperties, &pCarBitmapBrush);
					hr = pRenderTarget->CreateBitmapBrush(pShopBitmap, brushProperties, &pShopBitmapBrush);
					hr = pRenderTarget->CreateBitmapBrush(pHouseBitmap, brushProperties, &pHouseBitmapBrush);

				}
			}
		}
		//if (SUCCEEDED(hr))
		//{
		//	HRESULT hr = pDirect2dFactory->CreatePathGeometry(&pShopGeometry);
		//}
		//if (SUCCEEDED(hr))
		//{
		//	HRESULT hr = pDirect2dFactory->CreatePathGeometry(&pLeftCarGeometry);
		//}
		//if (SUCCEEDED(hr))
		//{
		//	HRESULT hr = pDirect2dFactory->CreatePathGeometry(&pUpCarGeometry);
		//}
	}

	return hr;
}

void BaseWnd::RunMessageLoop()
{
	MSG msg;

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return;
}

void BaseWnd::DiscardDeviceResources()
{
	SafeRelease(&pRenderTarget);
	SafeRelease(&pLightSlateGrayBrush);
	SafeRelease(&pCornflowerBlueBrush);
	SafeRelease(&pPaleVioletRedBrush);
	SafeRelease(&pLightSeaGreenBrush);
	SafeRelease(&pDWriteFactory);
	SafeRelease(&pTextFormat);
	for (auto pbrush : brush_pool)
	{
		SafeRelease(&pbrush);
	}
}

BaseWnd::~BaseWnd()
{
	SafeRelease(&pDirect2dFactory);
	SafeRelease(&pRenderTarget);
	SafeRelease(&pLightSlateGrayBrush);
	SafeRelease(&pCornflowerBlueBrush);
	SafeRelease(&pPaleVioletRedBrush);
	SafeRelease(&pLightSeaGreenBrush);
	SafeRelease(&pDWriteFactory);
	SafeRelease(&pTextFormat);
	for (auto pbrush : brush_pool)
	{
		SafeRelease(&pbrush);
	}
}

HRESULT BaseWnd::StartTimer(HWND hwnd)
{
	SetTimer(hwnd, 1, time_slice, NULL);
	return 0;
}

LRESULT CALLBACK BaseWnd::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = 0;

	if (message == WM_CREATE)
	{
		LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
		BaseWnd *pBaseWnd = (BaseWnd *)pcs->lpCreateParams;

		// store this wndobject in hwnd
		::SetWindowLongPtrW(
			hwnd,
			GWLP_USERDATA,
			PtrToUlong(pBaseWnd)
		);
		pBaseWnd->StartTimer(hwnd);
		result = 1;
	}
	else
	{
		BaseWnd *pBaseWnd = reinterpret_cast<BaseWnd *>(static_cast<LONG_PTR>(
			::GetWindowLongPtrW(
				hwnd,
				GWLP_USERDATA
			)));

		bool wasHandled = false;

		if (pBaseWnd)
		{
			switch (message)
			{
			case WM_SIZE:
			{
				UINT width = LOWORD(lParam);
				UINT height = HIWORD(lParam);
				pBaseWnd->OnResize(width, height);
				result = 0;
				wasHandled = true;
				break;
			}

			case WM_DISPLAYCHANGE:
			{
				InvalidateRect(hwnd, NULL, FALSE);
				result = 0;
				wasHandled = true;
				break;
			}

			case WM_PAINT:
			{
				pBaseWnd->OnRender();
				ValidateRect(hwnd, NULL);
				result = 0;
				wasHandled = true;
				break;
			}

			case WM_DESTROY:
			{
				print(std::get<1>(*pBaseWnd->model));
				PostQuitMessage(0);
				result = 1;
				wasHandled = true;
				break;
			}

			case WM_TIMER:
			{
				pBaseWnd->OnTimer();
				result = 0;
				wasHandled = true;
				break;
			}

			case WM_LBUTTONDOWN:
				pBaseWnd->OnLButtonDown(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), (DWORD)wParam);
				wasHandled = true;
				return 0;

			case WM_LBUTTONUP:
				pBaseWnd->OnLButtonUp();
				wasHandled = true;
				return 0;

			case WM_MOUSEMOVE:
				pBaseWnd->OnMouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), (DWORD)wParam);
				wasHandled = true;
				return 0;
			}
		}

		if (!wasHandled)
		{
			result = DefWindowProc(hwnd, message, wParam, lParam);
		}
	}

	return result;
}

void BaseWnd::OnResize(UINT width, UINT height)
{
	if (pRenderTarget)
	{
		// Note: This method can fail, but it's okay to ignore the
		// error here, because the error will be returned again
		// the next time EndDraw is called.
		pRenderTarget->Resize(D2D1::SizeU(width, height));
	}
}

void __stdcall BaseWnd::OnTimer()
{
	Solution solution = std::get<1>(*model);
	vector<Route>& routes = solution.vehicle_routes;
	TmSpot tm_hrz = solution.tm_hrz;

	if (timer_time == tm_hrz)
	{
		return;
	}
	//if (timer_time == INT_MAX)
	//{
	//	return;
	//}

	if (timer_time != INT_MAX)
	{
		int slice = 50;
		for (int i = 0; i != slice; i++)
		{
			float fi = i;
			animate_time = timer_time + fi / slice;
			OnRender(animate_time);
		}
		timer_time++;
	}
}


HRESULT BaseWnd::OnRender(float animate_time, float mouse_x, float mouse_y, bool click)
{
	HRESULT hr = S_OK;
	static bool first = true;
	if (first)
	{
		hr = CreateDeviceDependentResources();
		first = false;
	}
	//hr = CreateDeviceDependentResources();

	if (SUCCEEDED(hr))
	{
		pRenderTarget->BeginDraw();
		//pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
		pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));
		D2D1_SIZE_F rtSize = pRenderTarget->GetSize();
		int width = static_cast<int>(rtSize.width);
		int height = static_cast<int>(rtSize.height);
		float main_left = (width - height) / 2;
		float main_right = (width + height) / 2;
		float main_top = 0;
		float main_bottom = height;

		GraphModel gm = std::get<0>(*model);
		Solution sln = std::get<1>(*model);
		vector<Route>& routes = sln.vehicle_routes;
		int stride = gm.stride;

		for (int i = draw_locus.size(); i < routes.size(); i++)
		{
			draw_locus.push_back(true);
		}

		if (!end)
		{
			auto r = routes.begin();
			for (; r != routes.end(); r++)
			{
				// when timer_time larger than final complete, change timer_time to intmax to signal another display;
				// the latter is to prevent overflow, halfway in the project the timer_time is ini to intmin. although now it is not, I keep it.
				auto ve = r->move_chain.end(); ve--;
				if (ve->compl_time > timer_time - 5 && timer_time > INT_MIN + 4)
				{
					break;
				}
				else
				{
					auto re = routes.end(); re--;
					if (r == re)
					{
						end = true;
						for (int i = 0; i != routes.size(); i++) {
							draw_locus.push_back(true);
						}
					}
					continue;
				}
			}
		}


		// right panel
		std::wstring str = L"Draw Vehicle Route";
		const WCHAR* cstr = str.c_str();
		D2D1_RECT_F layoutRect = D2D1::RectF((width + height) / 2 + 20, 30, (width + height) / 2 + 80, 35);
		pRenderTarget->DrawTextW(cstr, wcslen(cstr), pTextFormat, layoutRect, pCornflowerBlueBrush, D2D1_DRAW_TEXT_OPTIONS_NONE, DWRITE_MEASURING_MODE_NATURAL);
		for (auto r = routes.begin(); r != routes.end(); r++)
		{
			float space = (width - height) / 2;
			int vhcl_num = r->vhcl_num;
			std::wstring str = L"Vehicle" + std::to_wstring(r->vhcl_num + 1);
			const WCHAR* cstr = str.c_str();
			float x = width - space + 10, y = 20 * vhcl_num + 60;
			D2D1_RECT_F layoutRect = D2D1::RectF(width - space + 20, 20 * vhcl_num + 50, width - space + 70, 20 * vhcl_num + 70);
			pRenderTarget->DrawTextW(cstr, wcslen(cstr), pTextFormat, layoutRect, pCornflowerBlueBrush, D2D1_DRAW_TEXT_OPTIONS_NONE, DWRITE_MEASURING_MODE_NATURAL);

			// radio button
			if (draw_locus[vhcl_num] == true)
			{
				pRenderTarget->FillEllipse(D2D1::Ellipse(D2D1::Point2F(x, y), 2, 2), pLightSeaGreenBrush);
			}
			else
			{
				pRenderTarget->DrawEllipse(D2D1::Ellipse(D2D1::Point2F(x, y), 2, 2), pLightSeaGreenBrush);
			}
		}


		if (!end)
			// not end
		{
			// start in x-axis of main screen
			float stridex = (width - height) / 2;
			for (int i = 0; i <= stride; i++)
			{
				pRenderTarget->DrawLine(
					D2D1::Point2F(stridex, 0),
					D2D1::Point2F(stridex, height),
					pCornflowerBlueBrush
				);
				stridex += height / stride;
			}
			float stridey = 0;
			for (int i = 0; i <= stride; i++)
			{
				pRenderTarget->DrawLine(
					D2D1::Point2F((width - height) / 2, stridey),
					D2D1::Point2F((width + height) / 2, stridey),
					pCornflowerBlueBrush
				);
				stridey += height / stride;
			}
			vector<VerticeState> vss = sln.vertice_states;
			for (VerticeState vs : vss)
			{
				if (vs.vertice.strt_time <= timer_time)
				{
					PDVertice v = vs.vertice;
					float x_origin = v.coor.xcoor * min(width, height) / gm.geo_size + height / 2;
					float y_origin = -v.coor.ycoor * min(width, height) / gm.geo_size + height / 2;
					int space = height / stride;
					int x_district = x_origin / space;
					int y_district = y_origin / space;
					float x = main_left + x_district * space + (float)space / 2;
					float y = main_top + y_district * space + (float)space / 2;
					std::wstring str;
					str = std::to_wstring(v.order_no + 1);
					const WCHAR* cstr = str.c_str();
					ID2D1SolidColorBrush* blackbrush;
					pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &blackbrush);
					//ID2D1Bitmap* p_bitmap;

					if (vs.vertice.strt_time <= timer_time)
					{
						if (vs.complete_time > timer_time)
						{
							pRenderTarget->DrawTextW(
								cstr,
								std::wcslen(cstr),
								pTextFormat,
								D2D1::RectF(x - 10, y - 5, x + 10, y + 15),
								blackbrush
							);
							if (vs.vertice.isDest())
							{
								drawHouse(pRenderTarget, x, y, 24, 24, pPaleVioletRedBrush);
							}
							else
							{
								drawShop(pRenderTarget, x, y, 24, 24, pPaleVioletRedBrush);
							}
						}
						else if (vs.complete_time > timer_time - 5)
						{
							pRenderTarget->DrawTextW(
								cstr,
								std::wcslen(cstr),
								pTextFormat,
								D2D1::RectF(x - 10, y - 5, x + 10, y + 15),
								blackbrush
							);
							if (vs.vertice.isDest())
							{
								drawHouse(pRenderTarget, x, y, 24, 24, pLightSeaGreenBrush);
							}
							else
							{
								drawShop(pRenderTarget, x, y, 24, 24, pLightSeaGreenBrush);
							}
						}
					}
				}
			}

			// #car
			for (Route r : routes)
			{
				GraphCoor* gc = &GraphCoor();
				getlocation(r, animate_time, gc);
				if (gc == NULL) {
					continue;
				}
				float x = gc->xcoor * min(width, height) / gm.geo_size + width / 2;
				float y = -gc->ycoor * min(width, height) / gm.geo_size + height / 2;
				//drawHouse(pRenderTarget, x, y, 24, 24, pLightSeaGreenBrush);
				//drawShop(pRenderTarget, x, y, 24, 24, pLightSeaGreenBrush);
				vhcl_state upv = up;
				int state = up * 100;
				//drawCar(pRenderTarget, x, y, 24, 24, pLightSeaGreenBrush, state);
				D2D1_ELLIPSE dot = D2D1::Ellipse(D2D1::Point2F(x, y), 3, 3);
				pRenderTarget->FillEllipse(dot, pCornflowerBlueBrush);
				//pRenderTarget->DrawBitmap(
				//	pCarBitmap,
				//	D2D1::RectF(x - 10, y - 10, x + 10, y + 10)
				//);


			}

		}
		else
		{
			// #distri panel
			int left_margin = 20, right_margin = 70;
			vector<int> time_distri;
			int interval = gm.time_span / brush_number + 1;
			for (int i = 0; i < brush_number; i++)
			{
				time_distri.push_back(0);
			}
			for (PDVertice v : gm.vertices)
			{
				int section = (v.strt_time + gm.time_span / 2) / interval;
				time_distri[section]++;
			}
			for (int i = 0; i < brush_number; i++)
			{
				int scale = 50 / gm.order_size + 1;
				int left_start = 45, top_start = 20;
				pRenderTarget->FillEllipse(D2D1::Ellipse(D2D1::Point2F(left_start, i * 20 + 30), 2, 2), brush_pool[i]);
				pRenderTarget->FillRectangle(D2D1::RectF(left_start + 5, i * 20 + top_start, left_start + 5 + time_distri[i] / 2 * scale, i * 20 + top_start + 20), brush_pool[i]);
				std::wstring str = std::to_wstring(i * interval) + L"-" + std::to_wstring((i + 1)*interval);
				const WCHAR* cstr = str.c_str();
				D2D1_RECT_F layoutRect = D2D1::RectF(5, i * 20 + 20, 45, i * 20 + 40);
				pRenderTarget->DrawTextW(cstr, wcslen(cstr), pTextFormat, layoutRect, pCornflowerBlueBrush);
				layoutRect = D2D1::RectF(left_start + 10 + time_distri[i] / 2 * scale, i * 20 + 20, left_start + time_distri[i] / 2 * scale + 30, i * 20 + 40);
				std::wstring num = std::to_wstring(time_distri[i] / 2);
				const WCHAR* cnum = num.c_str();
				pRenderTarget->DrawTextW(cnum, wcslen(cnum), pTextFormat, layoutRect, brush_pool[i]);
			}

			// routes
			for (auto ri = routes.begin(); ri != routes.end(); ri++)
			{
				Route r = *ri;
				if (!draw_locus[ri - routes.begin()])
				{
					continue;
				}

				// draw vehicle route and vertice
				auto ve = r.move_chain.begin();
				auto lst = ve->vertice;
				int lsttime = ve->compl_time;
				int lstvno = -r.vhcl_num;
				ve++;

				for (; ve != r.move_chain.end(); ve++)
				{
					// draw line between vertices
					std::shared_ptr<PDVertice> next = std::dynamic_pointer_cast<PDVertice>(ve->vertice);
					int nexttime = ve->compl_time;
					int nextvno = next->vrtc_no;
					auto lstpxel = GC_To_D2Point(lst->coor);
					auto nextpxel = GC_To_D2Point(next->coor);
					pRenderTarget->DrawLine(lstpxel, nextpxel, pLightSlateGrayBrush);

					// get infopopup
					float x1 = lstpxel.x, y1 = lstpxel.y, x2 = nextpxel.x, y2 = nextpxel.y, y3 = mouse_y;
					float x3 = (x2 - x1)*(y3 - y1) / (y2 - y1) + x1;
					if (mouse_x != 0 || mouse_y != 0)
					{
						if (abs(x3 - mouse_x) < 2 && (x1 - mouse_x)*(x2 - mouse_x) < 0 && abs(y1 - y3) > 5 && abs(y2 - y3) > 5)
						{
							std::wstring str = L"distance:" + std::to_wstring(distance(*lst, *next)) \
								+ L"\ntime:" + std::to_wstring(lsttime) + L"-" + std::to_wstring(nexttime);
							const WCHAR* cstr = str.c_str();
							//D2D1_RECT_F layoutRect;
							bool pos;
							if ((x1 - x2)*(y1 - y2) >= 0)
							{
								//                 /______
								//                /|      |
								//               / |______|

								//layoutRect = D2D1::RectF((x1 + x2) / 2, (y1 + y2) / 2 - 5, (x1 + x2) / 2 + 20, (y1 + y2) / 2 - 25);
								pos = true;
							}
							else
							{
								//                  ______
								//               \ |      |
								//                \|______|
								//                 \

								//layoutRect = D2D1::RectF((x1 + x2) / 2, (y1 + y2) / 2 + 25, (x1 + x2) / 2 + 20, (y1 + y2) / 2);
								pos = false;
							}
							int uid = lstvno * 1000 + nextvno;
						}
					}

					// draw point according to time section
					shared_ptr<PDVertice> v = std::dynamic_pointer_cast<PDVertice>(ve->vertice);
					float x = v->coor.xcoor * min(width, height) / gm.geo_size + width / 2;
					float y = -v->coor.ycoor * min(width, height) / gm.geo_size + height / 2;
					int timer_time = v->strt_time;
					int section = (timer_time + gm.time_span / 2) / (gm.time_span / brush_number);
					section = section % brush_pool.size();
					auto brush = brush_pool[section];
					pRenderTarget->FillEllipse(D2D1::Ellipse(D2D1::Point2F(x, y), 2, 2), brush);
					lst = next;
					lsttime = nexttime;
				}
			}
			// z1_layer
			/*for (InfoPopup info : z1_layer)
			{
				if (info.stay || info.show)
				{
					int uid = info.uid;
					int x1, x2, y1, y2;
					D2D1_RECT_F inforect;
					if (info.uid < 0)
					{
						uid = -info.uid;
						int lstvno = uid / 1000;
						x1 = gm.vertices[lstvno].coor.xcoor;
						y1 = gm.vertices[lstvno].coor.ycoor;
					}
					else
					{
						uid = info.uid;
						int lstvno = uid / 1000;
						x1 = gm.vertices[lstvno].coor.xcoor;
						y1 = gm.vertices[lstvno].coor.ycoor;
					}
					int nextvno = uid % 1000;
					x2 = gm.vertices[nextvno].coor.xcoor;
					y2 = gm.vertices[nextvno].coor.ycoor;
					int nextvrtc = info.uid % 1000;
					if (info.pos)
					{
						inforect = D2D1::RectF((x1 + x2) / 2, (y1 + y2) / 2 - 5, (x1 + x2) / 2 + 30, (y1 + y2) / 2 - 25);
					}
					else
					{
						inforect = D2D1::RectF((x1 + x2) / 2, (y1 + y2) / 2 + 25, (x1 + x2) / 2 + 30, (y1 + y2) / 2 + 5);
					}
					pRenderTarget->DrawTextW(info.text.c_str(), wcslen(info.text.c_str()), pTextFormat, layoutRect, pCornflowerBlueBrush);
				}
			}*/
		}
		hr = pRenderTarget->EndDraw();
	}

	if (hr == D2DERR_RECREATE_TARGET)
	{
		hr = S_OK;
		DiscardDeviceResources();
	}

	return hr;
}

int BaseWnd::getChoosenRoutes(float x, float y)
{
	D2D1_SIZE_F rtSize = pRenderTarget->GetSize();
	int width = static_cast<int>(rtSize.width);
	int height = static_cast<int>(rtSize.height);

	// z = 0
	float choose_route_left_margin = (width + height) / 2 + 10;
	float choose_route_right_margin = choose_route_left_margin + 50;
	if (x >= choose_route_left_margin && x <= choose_route_right_margin)
	{
		return (y - 50) / 20;
	}
	else
	{
		return -1;
	}
}

void BaseWnd::OnLButtonDown(int pixelX, int pixelY, DWORD flags)
{
	D2D1_SIZE_F rtSize = pRenderTarget->GetSize();
	int width = static_cast<int>(rtSize.width);
	int height = static_cast<int>(rtSize.height);
	Solution sln = std::get<1>(*model);

	if (!end)
	{
		return;
	}
	else
	{
		// index = 0 ----> vhcl 0
		int index = getChoosenRoutes(pixelX, pixelY);
		if (index >= 0 && index < sln.vehicle_routes.size())
		{
			draw_locus[index] = !draw_locus[index];
		}
		else if (pixelX > (width - height) / 2 && pixelX < (width + height) / 2)
		{
			OnRender(animate_time, pixelX, pixelY, true);
		}
	}
}
void BaseWnd::OnLButtonUp() {}
void BaseWnd::OnMouseMove(int pixelX, int pixelY, DWORD flags)
{
}

void BaseWnd::getlocation(Route r, float time, GraphCoor* gc)
{
	GraphModel gm = std::get<0>(*this->model);
	GraphCoor *lst = NULL, *curr = NULL, *next = NULL;
	int lsttime = INT_MIN, nexttime = INT_MIN;
	VerticeEvent currve, nextve, lstve;

	auto pve = r.move_chain.begin();
	pve++;
	for (; pve != r.move_chain.end(); pve++)
	{
		VerticeEvent ve = *pve;
		if (ve.compl_time == time)
		{
			currve = ve;
			curr = &ve.vertice->getCoor();
			break;
		}
		else if (ve.compl_time > time)
		{
			nextve = ve;
			next = &ve.vertice->getCoor();
			nexttime = ve.compl_time;
			break;
		}
		else
		{
			lstve = ve;
			lst = &ve.vertice->getCoor();
			lsttime = ve.compl_time;
		}
	}
	if (curr != NULL)
	{
		*gc = *curr;
		return;
	}
	if (lst == NULL)
	{
		gc = NULL;
		return;
	}
	if (next == NULL)
	{
		*gc = *lst;
		return;
	}

	// the vehicle waited for some time
	if (nexttime - lsttime > distance(*lst, *next) / gm.speed)
	{
		int wait = nexttime - lsttime - distance(*lst, *next) / gm.speed;
		time = time - wait;
		time = time > lsttime ? time : lsttime;
	}
	int dis = (time - lsttime) * gm.speed;

	int xcoor, ycoor;
	vector<GraphCoor> graph = nextve.path;
	auto pgc = graph.begin();
	GraphCoor l = *pgc;
	pgc++;
	for (; pgc != graph.end(); pgc++)
	{
		GraphCoor n = *pgc;
		int d = distance(l, n);
		if (dis == d)
		{
			*gc = n;
			return;
		}
		else if (dis < d)
		{
			float x, y;
			// below is only true because l and n have the same x coor or y coor.
			x = l.xcoor + dis * (n.xcoor - l.xcoor) / d;
			y = l.ycoor + dis * (n.ycoor - l.ycoor) / d;
			*gc = GraphCoor(x, y);
			return;
		}
		else if (dis > d)
		{
			dis -= d;
		}
		l = n;
	}

	//if (xfirst)
	//{
	//	int xdist = abs(lst->xcoor - next->xcoor) > dis ? dis : abs(lst->xcoor - next->xcoor);
	//	int ydist = dis - xdist;
	//	xcoor = lst->xcoor < next->xcoor ? lst->xcoor - xdist : lst->xcoor - xdist;
	//	ycoor = lst->ycoor < next->ycoor ? lst->ycoor + ydist : lst->ycoor - ydist;
	//}
	//else
	//{
	//	int ydist = abs(lst->ycoor - next->ycoor) > dis ? dis : abs(lst->ycoor - next->ycoor);
	//	int xdist = dis - ydist;
	//	xcoor = lst->xcoor < next->xcoor ? lst->xcoor + xdist : lst->xcoor - xdist;
	//	ycoor = lst->ycoor < next->ycoor ? lst->ycoor + ydist : lst->ycoor - ydist;
	//}
	//return &GraphCoor(xcoor, ycoor);

}



HRESULT BaseWnd::loadBitmapFromFile(
	ID2D1RenderTarget* pRenderTarget,
	IWICImagingFactory* pIWICFactory,
	PCWSTR uri,
	UINT destinationWidth,
	UINT destinationHeight,
	ID2D1Bitmap **ppBitmap
)
{
	IWICBitmapDecoder *pDecoder = NULL;
	IWICBitmapFrameDecode *pSource = NULL;
	IWICStream *pStream = NULL;
	IWICFormatConverter *pConverter = NULL;
	IWICBitmapScaler *pScaler = NULL;

	HRESULT hr = pIWICFactory->CreateDecoderFromFilename(
		uri,
		NULL,
		GENERIC_READ,
		WICDecodeMetadataCacheOnLoad,
		&pDecoder
	);
	if (SUCCEEDED(hr))
	{
		// Create the initial frame.
		hr = pDecoder->GetFrame(0, &pSource);
	}
	// Create the scaler.
	if (SUCCEEDED(hr))
	{
		hr = pIWICFactory->CreateBitmapScaler(&pScaler);
	}
	// Initialize the scaler to half the size of the original source.
	UINT x, y;
	UINT *w = &x, *h = &y;
	pSource->GetSize(w, h);
	if (SUCCEEDED(hr))
	{
		hr = pScaler->Initialize(
			pSource,									 // Bitmap source to scale.
			destinationWidth,							 // Scale width to half of original.
			destinationHeight,						     // Scale height to half of original.
			WICBitmapInterpolationModeFant);		     // Use Fant mode interpolation.
	}
	if (SUCCEEDED(hr))
	{

		// Convert the image format to 32bppPBGRA
		// (DXGI_FORMAT_B8G8R8A8_UNORM + D2D1_ALPHA_MODE_PREMULTIPLIED).
		hr = pIWICFactory->CreateFormatConverter(&pConverter);

	}
	if (SUCCEEDED(hr))
	{
		hr = pConverter->Initialize(
			pSource,
			GUID_WICPixelFormat32bppPBGRA,
			WICBitmapDitherTypeNone,
			NULL,
			0.f,
			WICBitmapPaletteTypeMedianCut
		);
	}
	if (SUCCEEDED(hr))
	{

		// Create a Direct2D bitmap from the WIC bitmap.
		hr = pRenderTarget->CreateBitmapFromWicBitmap(
			pConverter,
			NULL,
			ppBitmap
		);
	}

	SafeRelease(&pDecoder);
	SafeRelease(&pSource);
	SafeRelease(&pStream);
	SafeRelease(&pConverter);
	SafeRelease(&pScaler);

	return hr;
}

void BaseWnd::drawHouse(ID2D1RenderTarget* rt, float x, float y, float width, float height, ID2D1SolidColorBrush* brush)
{
	rt->DrawLine(
		D2D1::Point2F(x - width / 2, y),
		D2D1::Point2F(x, y - height / 2),
		brush
	);
	rt->DrawLine(
		D2D1::Point2F(x, y - height / 2),
		D2D1::Point2F(x + width / 4, y - height / 4),
		brush
	);
	rt->DrawLine(
		D2D1::Point2F(x + width / 4, y - height / 4),
		D2D1::Point2F(x + width / 4, y - height / 2),
		brush
	);
	rt->DrawLine(
		D2D1::Point2F(x + width / 4, y - height / 2),
		D2D1::Point2F(x + (width * 3) / 8, y - height / 2),
		brush
	);
	rt->DrawLine(
		D2D1::Point2F(x + (width * 3) / 8, y - height / 2),
		D2D1::Point2F(x + (width * 3) / 8, y - height / 8),
		brush
	);
	rt->DrawLine(
		D2D1::Point2F(x + (width * 3) / 8, y - height / 8),
		D2D1::Point2F(x + width / 2, y),
		brush
	);
	rt->DrawLine(
		D2D1::Point2F(x + width / 2, y),
		D2D1::Point2F(x + (width * 3) / 8, y),
		brush
	);
	rt->DrawLine(
		D2D1::Point2F(x + (width * 3) / 8, y),
		D2D1::Point2F(x + (width * 3) / 8, y + height / 2),
		brush
	);
	rt->DrawLine(
		D2D1::Point2F(x + (width * 3) / 8, y + height / 2),
		D2D1::Point2F(x - (width * 3) / 8, y + height / 2),
		brush
	);
	rt->DrawLine(
		D2D1::Point2F(x - (width * 3) / 8, y + height / 2),
		D2D1::Point2F(x - (width * 3) / 8, y),
		brush
	);
	rt->DrawLine(
		D2D1::Point2F(x - (width * 3) / 8, y),
		D2D1::Point2F(x - (width * 4) / 8, y),
		brush
	);
}

void BaseWnd::drawShop(ID2D1RenderTarget* rt, float x, float y, float width, float height, ID2D1SolidColorBrush* brush) {
	rt->DrawLine(
		D2D1::Point2F(x - width / 2, y - height / 6),
		D2D1::Point2F(x - width / 4, y - height / 2),
		brush
	);
	rt->DrawLine(
		D2D1::Point2F(x - width / 4, y - height / 2),
		D2D1::Point2F(x + width / 4, y - height / 2),
		brush
	);
	rt->DrawLine(
		D2D1::Point2F(x + width / 4, y - height / 2),
		D2D1::Point2F(x + width / 2, y - height / 6),
		brush
	);
	float ellibasey = y - height / 6;
	ID2D1GeometrySink *pSink = NULL;
	// Create a path geometry.
	ID2D1PathGeometry *pShopGeometry;
	HRESULT hr = pDirect2dFactory->CreatePathGeometry(&pShopGeometry);
	if (SUCCEEDED(hr))
	{
		// Write to the path geometry using the geometry sink.
		HRESULT hr = pShopGeometry->Open(&pSink);

		if (SUCCEEDED(hr))
		{
			pSink->BeginFigure(
				D2D1::Point2F(x - width / 2, ellibasey),
				D2D1_FIGURE_BEGIN_FILLED
			);

			pSink->AddArc(D2D1::ArcSegment(
				D2D1::Point2F(x - width / 4, ellibasey),
				D2D1::SizeF(width / 8, height / 8),
				180,
				D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE,
				D2D1_ARC_SIZE_SMALL
			));
			pSink->AddArc(D2D1::ArcSegment(
				D2D1::Point2F(x, ellibasey),
				D2D1::SizeF(width / 8, height / 8),
				180,
				D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE,
				D2D1_ARC_SIZE_SMALL
			));
			pSink->AddArc(D2D1::ArcSegment(
				D2D1::Point2F(x + width / 4, ellibasey),
				D2D1::SizeF(width / 8, height / 8),
				180,
				D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE,
				D2D1_ARC_SIZE_SMALL
			));
			pSink->AddArc(D2D1::ArcSegment(
				D2D1::Point2F(x + width / 2, ellibasey),
				D2D1::SizeF(width / 8, height / 8),
				180,
				D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE,
				D2D1_ARC_SIZE_SMALL
			));
			pSink->EndFigure(D2D1_FIGURE_END_OPEN);

			hr = pSink->Close();
		}
		SafeRelease(&pSink);
		rt->DrawGeometry(pShopGeometry, brush);
	}
	//rt->DrawEllipse(
	//	D2D1::Ellipse(D2D1::Point2F(x - (width * 3) / 8, ellibasey), width / 4, height / 6),
	//	brush
	//);
	//rt->DrawEllipse(
	//	D2D1::Ellipse(D2D1::Point2F(x - (width * 1) / 8, ellibasey), width / 4, height / 6),
	//	brush
	//);
	//rt->DrawEllipse(
	//	D2D1::Ellipse(D2D1::Point2F(x + (width * 1) / 8, ellibasey), width / 4, height / 6),
	//	brush
	//);
	//rt->DrawEllipse(
	//	D2D1::Ellipse(D2D1::Point2F(x + (width * 3) / 8, ellibasey), width / 4, height / 6),
	//	brush
	//);
	rt->DrawLine(
		D2D1::Point2F(x - width / 3, y + height / 2),
		D2D1::Point2F(x - width / 3, y),
		brush
	);
	rt->DrawLine(
		D2D1::Point2F(x + width / 3, y + height / 2),
		D2D1::Point2F(x + width / 3, y),
		brush
	);
	rt->DrawLine(
		D2D1::Point2F(x - width / 2, y + height / 2),
		D2D1::Point2F(x + width / 2, y + height / 2),
		brush
	);
}


void BaseWnd::drawCar(ID2D1RenderTarget* rt, float x, float y, float width, float height, ID2D1SolidColorBrush* brush, int state) {
	// Create a path geometry.
	ID2D1PathGeometry *pCarGeometry;
	HRESULT hr = pDirect2dFactory->CreatePathGeometry(&pCarGeometry);
	if (SUCCEEDED(hr))
	{
		ID2D1GeometrySink *pSink = NULL;
		// Write to the path geometry using the geometry sink.
		int direction = state / 100;
		float wheel = 2;
		HRESULT hr = pCarGeometry->Open(&pSink);
		if (SUCCEEDED(hr))
		{
			switch (direction)
			{
			case up:
				pSink->BeginFigure(
					D2D1::Point2F(x - width / 4, y + height / 2),
					D2D1_FIGURE_BEGIN_FILLED
				);
				pSink->AddArc(D2D1::ArcSegment(
					D2D1::Point2F(x - width / 4, y + height / 2 - wheel),
					D2D1::SizeF(wheel, wheel),
					180,
					D2D1_SWEEP_DIRECTION_CLOCKWISE,
					D2D1_ARC_SIZE_SMALL
				));
				pSink->AddLine(D2D1::Point2F(x - width / 4, y + height / 2));
				pSink->AddLine(D2D1::Point2F(x - width / 4, y - height / 4));
				pSink->AddArc(D2D1::ArcSegment(
					D2D1::Point2F(x - width / 4, y - height / 4 + wheel),
					D2D1::SizeF(wheel, wheel),
					180,
					D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE,
					D2D1_ARC_SIZE_SMALL
				));
				pSink->AddLine(D2D1::Point2F(x - width / 6, y - height / 2));
				pSink->AddLine(D2D1::Point2F(x + width / 6, y - height / 2));
				pSink->AddLine(D2D1::Point2F(x + width / 4, y - height / 4));
				pSink->AddArc(D2D1::ArcSegment(
					D2D1::Point2F(x + width / 4, y - height / 4 + wheel),
					D2D1::SizeF(wheel, wheel),
					180,
					D2D1_SWEEP_DIRECTION_CLOCKWISE,
					D2D1_ARC_SIZE_SMALL
				));
				pSink->AddLine(D2D1::Point2F(x - width / 4, y - height / 4));
				pSink->AddLine(D2D1::Point2F(x + width / 4, y - height / 4));
				pSink->AddLine(D2D1::Point2F(x + width / 4, y + height / 2));
				pSink->AddArc(D2D1::ArcSegment(
					D2D1::Point2F(x + width / 4, y + height / 2 - wheel),
					D2D1::SizeF(wheel, wheel),
					180,
					D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE,
					D2D1_ARC_SIZE_SMALL
				));
				pSink->AddLine(D2D1::Point2F(x - width / 4, y + height / 2));
				pSink->EndFigure(D2D1_FIGURE_END_OPEN);
				break;
			}
			hr = pSink->Close();
			SafeRelease(&pSink);
		}
		rt->DrawGeometry(pCarGeometry, brush);
	}
}