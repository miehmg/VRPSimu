#include "gui.h"
#include "../Model/GraphModel.h"
#include "../Model/Solution.h"

class BaseWnd {
public:
	BaseWnd();
	~BaseWnd();
	HRESULT Initialize(std::tuple<GraphModel&, Solution&>*);
	void RunMessageLoop();
	GraphCoor* getlocation(Route r, float timer_time);
protected:
private:
	HRESULT StartTimer(HWND hwnd);

	void __stdcall OnTimer();

	// Initialize device-independent resources.
	HRESULT CreateDeviceIndependentResources();

	// Initialize device-dependent resources.
	HRESULT CreateDeviceDependentResources();

	// Release device-dependent resource.
	void DiscardDeviceResources();

	// Draw content.
	HRESULT OnRender(float animate_time = -1000000, float mouse_x = 0, float mouse_y = 0, bool click = false);

	void    OnLButtonDown(int pixelX, int pixelY, DWORD flags);
	void    OnLButtonUp();
	void    OnMouseMove(int pixelX, int pixelY, DWORD flags);

	// Resize the render target.
	void OnResize(
		UINT width,
		UINT height
	);

	D2D1_POINT_2F GC_To_D2Point(GraphCoor gc);

	int getChoosenRoutes(float, float);
	// The windows procedure.
	static LRESULT CALLBACK WndProc(
		HWND hWnd,
		UINT message,
		WPARAM wParam,
		LPARAM lParam
	);

	HRESULT loadBitmapFromFile(
		ID2D1RenderTarget* pRenderTarget,
		IWICImagingFactory* pIWICFactory,
		PCWSTR uri,
		UINT destinationWidth,
		UINT destinationHeight,
		ID2D1Bitmap **ppBitmap
	);
	void drawHouse(ID2D1RenderTarget* rt, float x, float y, float width, float height, ID2D1SolidColorBrush* brush);
	void drawShop(ID2D1RenderTarget* rt, float x, float y, float width, float height, ID2D1SolidColorBrush* brush);
	void drawCar(ID2D1RenderTarget* rt, float x, float y, float width, float height, ID2D1SolidColorBrush* brush, int state);
private:
	HWND hwnd;
	ID2D1Bitmap* pCarBitmap;
	ID2D1Bitmap* pShopBitmap;
	ID2D1Bitmap* pHouseBitmap;
	ID2D1BitmapBrush* pCarBitmapBrush;
	ID2D1BitmapBrush* pShopBitmapBrush;
	ID2D1BitmapBrush* pHouseBitmapBrush;
	ID2D1Factory* pDirect2dFactory;
	IWICImagingFactory* pIWICFactory;
	ID2D1HwndRenderTarget* pRenderTarget;
	ID2D1BitmapRenderTarget* pBitmapRenderTarget;
	ID2D1PathGeometry* pPathGeometry;
	ID2D1GeometrySink* pSink;				// PathGeometry Sink
	ID2D1SolidColorBrush* pLightSlateGrayBrush;
	ID2D1SolidColorBrush* pCornflowerBlueBrush;
	ID2D1SolidColorBrush* pPaleVioletRedBrush;
	ID2D1SolidColorBrush* pLightSeaGreenBrush;
	vector<ID2D1SolidColorBrush*> brush_pool;

	IDWriteFactory* pDWriteFactory;
	IDWriteTextFormat* pTextFormat;

	int timer_time;
	bool end = false;
	D2D1_POINT_2F pMouse;
	vector<bool> draw_locus;
	std::tuple<GraphModel&, Solution&>* model;
	};

