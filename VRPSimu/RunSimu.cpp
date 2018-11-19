#include "GUI/gui.h"
#include "GUI/BaseWnd.h"
#include "Model/GraphModel.h"
#include "Model/Solution.h"

GraphModel graph_model;
Solution solution;
std::tuple<GraphModel&, Solution&> model{ graph_model, solution };

DWORD WINAPI StartAnimation(LPVOID lpThreadParam);
DWORD WINAPI StartAlgorithm(LPVOID lpThreadParam);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	const WCHAR *s = L"b";
	printf("whcar%d", sizeof(s));
	int geo_size = 10000, order_size = 10,
		// int geo_size = 7000, order_size = 50,
		time_span = 20, tolerate_time = 30, speed = 300;
	int time_seed = 100, vrtc_seed = 0, vhcl_seed = 5;
	int wait_time = 0;
	// wait_time = 5;

	graph_model = GraphModel{ geo_size, order_size, time_span, tolerate_time, speed };

	// generate timer_time sequence
	vector<int> time_seq;
	time_seq.reserve(100);
	for (int i = 0; i < order_size; ++i)
	{
		static std::default_random_engine e(time_seed);
		static std::normal_distribution<> u(0, time_span / 6);
		int t = u(e);
		time_seq.emplace_back(t);
	}
	std::sort(time_seq.begin(), time_seq.end());

	// generate vehicle start position sequence
	vector<GraphCoor> strt_pos_seq;
	for (int i = 0; i < graph_model.vehicle_size; ++i) {
		static std::default_random_engine e(vhcl_seed);
		static std::normal_distribution<> u(0, geo_size / 6);
		int x = u(e);
		int y = u(e);
		strt_pos_seq.emplace_back(x, y);
	}
	InitializePDVertices(graph_model, time_seq, strt_pos_seq, vrtc_seed);

	solution = Solution(graph_model);

	LPVOID lpThreadParam = (void *)(&model);

	DWORD AlgorThreadID, GUIThreadID;
	HANDLE hAlgorThread = CreateThread(
		NULL,
		0,
		StartAlgorithm,
		lpThreadParam, //input data
		0,
		&AlgorThreadID
	);

	WaitForSingleObject(hAlgorThread, INFINITE);

	HANDLE hGUIThread = CreateThread(
		NULL,
		0,
		StartAnimation,
		lpThreadParam, //input data
		0,
		&GUIThreadID
	);
	HANDLE thread_pool[] = { hAlgorThread, hGUIThread };
	//WaitForMultipleObjectsEx(2, thread_pool, true, INFINITE, true);
	WaitForSingleObject(hGUIThread, INFINITE);
	return 0;

}

int main() {
	return _tWinMain(GetModuleHandle(NULL), NULL, GetCommandLine(), SW_SHOW);
}

DWORD WINAPI StartAlgorithm(LPVOID lpThreadParam)
{
	HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 3);

	DynamicTabuSolve(graph_model, solution, 0);
	return 0;
}

DWORD WINAPI StartAnimation(LPVOID lpThreadParam)
{
	HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);
	HRESULT hr = CoInitialize(NULL);
	if (SUCCEEDED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED)))
	{
		{
			BaseWnd map;

			if (SUCCEEDED(map.Initialize(&model)))
			{
				map.RunMessageLoop();
			}

		}
		return 0;
	}

	auto e = GetLastError();
	return 0;
}