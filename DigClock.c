/*-----------------------------------------
   DIGCLOCK.c -- Digital Clock
				 (c) Charles Petzold, 1998
  -----------------------------------------*/

#include <windows.h>

#define ID_TIMER    1

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	PSTR szCmdLine, int iCmdShow)
{
	static TCHAR szAppName[] = TEXT("DigClock");
	HWND         hwnd;
	MSG          msg;
	WNDCLASS     wndclass;

	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = WndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = hInstance;
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);//标准的鼠标样式
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH); //该函数检索预定义的备用笔、刷子、字体或者调色板的句柄。
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = szAppName;

	if (!RegisterClass(&wndclass))
	{
		MessageBox(NULL, TEXT("Program requires Windows NT!"),
			szAppName, MB_ICONERROR);
		return 0;
	}

	hwnd = CreateWindow(szAppName, TEXT("Digital Clock"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT,
		NULL, NULL, hInstance, NULL);

	ShowWindow(hwnd, iCmdShow);
	UpdateWindow(hwnd);

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}

void DisplayDigit(HDC hdc, int iNumber)
{
	/* 10个数字, 7段数码管 */
	static BOOL  fSevenSegment[10][7] = {
						1, 1, 1, 0, 1, 1, 1,     // 0
						0, 0, 1, 0, 0, 1, 0,     // 1
						1, 0, 1, 1, 1, 0, 1,     // 2
						1, 0, 1, 1, 0, 1, 1,     // 3
						0, 1, 1, 1, 0, 1, 0,     // 4
						1, 1, 0, 1, 0, 1, 1,     // 5
						1, 1, 0, 1, 1, 1, 1,     // 6
						1, 0, 1, 0, 0, 1, 0,     // 7
						1, 1, 1, 1, 1, 1, 1,     // 8
						1, 1, 1, 1, 0, 1, 1 };  // 9
   /* 7段数码管,每段都是六角菱形, 6个xy轴的坐标点 */
	static POINT ptSegment[7][6] = {
						  7,  6,  11,  2,  31,  2,  35,  6,  31, 10,  11, 10,
						  6,  7,  10, 11,  10, 31,   6, 35,   2, 31,   2, 11,
						 36,  7,  40, 11,  40, 31,  36, 35,  32, 31,  32, 11,
						  7, 36,  11, 32,  31, 32,  35, 36,  31, 40,  11, 40,
						  6, 37,  10, 41,  10, 61,   6, 65,   2, 61,   2, 41,
						 36, 37,  40, 41,  40, 61,  36, 65,  32, 61,  32, 41,
						  7, 66,  11, 62,  31, 62,  35, 66,  31, 70,  11, 70 };
	int          iSeg;

	for (iSeg = 0; iSeg < 7; iSeg++)
		if (fSevenSegment[iNumber][iSeg])
			Polygon(hdc, ptSegment[iSeg], 6);
}

void DisplayTwoDigits(HDC hdc, int iNumber, BOOL fSuppress)
{
	// 前面是否需要补零
	if (!fSuppress || (iNumber / 10 != 0))
		DisplayDigit(hdc, iNumber / 10);
	// 右移42个单位长度
	OffsetWindowOrgEx(hdc, -42, 0, NULL);
	DisplayDigit(hdc, iNumber % 10);
	OffsetWindowOrgEx(hdc, -42, 0, NULL);
}
// 画两个点
void DisplayColon(HDC hdc)
{
	// 两个四边形的坐标
	POINT ptColon[2][4] = { 2,  21,  6,  17,  10, 21,  6, 25,
							 2,  51,  6,  47,  10, 51,  6, 55 };

	Polygon(hdc, ptColon[0], 4);
	Polygon(hdc, ptColon[1], 4);

	OffsetWindowOrgEx(hdc, -12, 0, NULL);
}

void DisplayTime(HDC hdc, BOOL f24Hour, BOOL fSuppress)
{
	SYSTEMTIME st;
	//	获得本地时间
	GetLocalTime(&st);

	if (f24Hour)
		DisplayTwoDigits(hdc, st.wHour, fSuppress);
	else
		DisplayTwoDigits(hdc, (st.wHour %= 12) ? st.wHour : 12, fSuppress);

	DisplayColon(hdc);
	DisplayTwoDigits(hdc, st.wMinute, FALSE);
	DisplayColon(hdc);
	DisplayTwoDigits(hdc, st.wSecond, FALSE);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static BOOL   f24Hour, fSuppress;
	static HBRUSH hBrushRed;
	static int    cxClient, cyClient;
	HDC           hdc;
	PAINTSTRUCT   ps;

	switch (message)
	{
	case WM_CREATE:
		// 产生红色画刷
		hBrushRed = CreateSolidBrush(RGB(255, 0, 0));
		SetTimer(hwnd, ID_TIMER, 1000, NULL);// fall through
		//没有break, return.所以会继续执行

	case WM_SETTINGCHANGE:
		/* 如果用户改变任何系统设置，Windows 会广播一个 WM_SETTINGCHANGE 消息，
		  当前正在运行的所有应用程序都会接收到该消息。DIGCLOCK 调用 GetLocaleInfo 来处理这样的消息。
		  这样，你就可以试验不同的地区设置。判断是否为24时制*/
		GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_ITIME, szBuffer, 2);
		f24Hour = (szBuffer[0] == '1');
		/* 是否补零 */
		GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_ITLZERO, szBuffer, 2);
		fSuppress = (szBuffer[0] == '0');
		fSuppress = 0;
		/* 设置无效区 */
		InvalidateRect(hwnd, NULL, TRUE);
		return 0;

	case WM_SIZE:
		/* 在窗口大小发生变化后,会受到WM_SIZE消息 */
		cxClient = LOWORD(lParam);
		cyClient = HIWORD(lParam);
		return 0;

	case WM_TIMER:
		/* 设置整个客户端为无效区,触发PAINT消息 */
		InvalidateRect(hwnd, NULL, TRUE);
		return 0;

	case WM_PAINT:
		/* 无效区的时候产生消息 */
		hdc = BeginPaint(hwnd, &ps);
		// 设置映射模式
		SetMapMode(hdc, MM_ISOTROPIC);
		// 窗口范围
		SetWindowExtEx(hdc, 276, 72, NULL);
		// 视口范围
		SetViewportExtEx(hdc, cxClient, cyClient, NULL);
		// 把逻辑中心移到了原来逻辑坐标系的中心
		SetWindowOrgEx(hdc, 138, 36, NULL);
		// 把显示的中心设在了客户区的中心
		SetViewportOrgEx(hdc, cxClient / 2, cyClient / 2, NULL);

		SelectObject(hdc, GetStockObject(NULL_PEN));
		SelectObject(hdc, hBrushRed);

		DisplayTime(hdc, f24Hour, fSuppress);

		EndPaint(hwnd, &ps);
		return 0;

	case WM_DESTROY:
		KillTimer(hwnd, ID_TIMER);
		DeleteObject(hBrushRed);
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}