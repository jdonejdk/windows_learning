/*--------------------------------------
   CLOCK.C -- Analog Clock Program
			  (c) Charles Petzold, 1998
  --------------------------------------*/

#include <windows.h>
#include <math.h>

#define ID_TIMER    1
#define TWOPI       (2 * 3.14159)

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	PSTR szCmdLine, int iCmdShow)
{
	static TCHAR szAppName[] = TEXT("Clock");
	HWND         hwnd;
	MSG          msg;
	WNDCLASS     wndclass;

	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = WndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = hInstance;
	wndclass.hIcon = NULL;
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = szAppName;

	if (!RegisterClass(&wndclass))
	{
		MessageBox(NULL, TEXT("Program requires Windows NT!"),
			szAppName, MB_ICONERROR);
		return 0;
	}

	hwnd = CreateWindow(szAppName, TEXT("Analog Clock"),
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

void SetIsotropic(HDC hdc, int cxClient, int cyClient)
{
	/*SetMapMode函数设置指定设备环境的映射方式，映射方式定义了将逻辑单位转换为设备单位的度量单位，
	   并定义了设备的X、Y轴的方向  逻辑单位转换成具有均等比例轴的任意单位，即沿X轴的一个单位等于沿Y轴的一个单位 */
	SetMapMode(hdc, MM_ISOTROPIC);
	//用指定的值来设置指定设备环境坐标的X轴、Y轴范围
	SetWindowExtEx(hdc, 1000, 1000, NULL);
	//设置设备环境的窗口原点
	SetViewportExtEx(hdc, cxClient / 2, -cyClient / 2, NULL);
	//SetViewportOrgEx说明哪个设备点映射到窗口原点
	SetViewportOrgEx(hdc, cxClient / 2, cyClient / 2, NULL);

}

//pt是含有一个或多个点的数组，iNum为点的数目，iAngle为每个点的旋转角度
void RotatePoint(POINT pt[], int iNum, int iAngle)
{
	int   i;
	POINT ptTemp;

	for (i = 0; i < iNum; i++)
	{
		ptTemp.x = (int)(pt[i].x * cos(TWOPI * iAngle / 360) +
			pt[i].y * sin(TWOPI * iAngle / 360));//转换成旋转后的x坐标

		ptTemp.y = (int)(pt[i].y * cos(TWOPI * iAngle / 360) -
			pt[i].x * sin(TWOPI * iAngle / 360));//转换成旋转后的y坐标

		pt[i] = ptTemp; //新的点结构体替换原来的点结构体
	}
}

void DrawClock(HDC hdc)
{
	int   iAngle;
	POINT pt[3];
    /* 画点程序，360度60个点，一个点旋转6度 */
	for (iAngle = 0; iAngle < 360; iAngle += 6)
	{
		pt[0].x = 0;
		pt[0].y = 900;

		RotatePoint(pt, 1, iAngle); //用来设计旋转后的点的坐标信息

		pt[2].x = pt[2].y = iAngle % 5 ? 33 : 100; //判断钟表周围的点的直径大小

		pt[0].x -= pt[2].x / 2;
		pt[0].y -= pt[2].y / 2; //获取钟表周围点的外围矩形的左上角坐标值

		pt[1].x = pt[0].x + pt[2].x;
		pt[1].y = pt[0].y + pt[2].y; //获取钟表周围点的外围矩形的右下角坐标值

		SelectObject(hdc, GetStockObject(BLACK_BRUSH)); // 设置句柄
		//描绘一个椭圆，由指定的矩形围绕。椭圆用当前选择的画笔描绘，并用当前选择的刷子填充
		Ellipse(hdc, pt[0].x, pt[0].y, pt[1].x, pt[1].y);
	}
}

// 此函数用来绘画时针，分针和秒针
void DrawHands(HDC hdc, SYSTEMTIME* pst, BOOL fChange)
{
	static POINT pt[3][5] = { 0, -150, 100, 0, 0, 600, -100, 0, 0, -150, // 时钟5个点的坐标
							  0, -200,  50, 0, 0, 800,  -50, 0, 0, -200, // 分钟5个点的坐标
							  0,    0,   0, 0, 0,   0,    0, 0, 0,  800 }; // 秒钟5个点的坐标
	int          i, iAngle[3];
	// 存储着时分秒
	POINT        ptTemp[3][5];

	iAngle[0] = (pst->wHour * 30) % 360 + pst->wMinute / 2; //用来获取当前时间时针顺时针偏离竖直方向的角度
	iAngle[1] = pst->wMinute * 6;//用来获取当前时间分针顺时针偏离竖直方向的角度
	iAngle[2] = pst->wSecond * 6;//用来获取当前时间秒针顺时针偏离竖直方向的角度

	memcpy(ptTemp, pt, sizeof(pt));

	for (i = fChange ? 0 : 2; i < 3; i++)//fChange的作用就是当为真时，重绘时针分针和秒针
	{
		RotatePoint(ptTemp[i], 5, iAngle[i]);

		Polyline(hdc, ptTemp[i], 5);
	}
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static int        cxClient, cyClient;
	static SYSTEMTIME stPrevious;
	BOOL              fChange;
	HDC               hdc;
	PAINTSTRUCT       ps;
	SYSTEMTIME        st;

	switch (message)
	{
	case WM_CREATE:
		SetTimer(hwnd, ID_TIMER, 1000, NULL);
		GetLocalTime(&st);
		stPrevious = st;
		return 0;

	case WM_SIZE:
		cxClient = LOWORD(lParam);
		cyClient = HIWORD(lParam);
		return 0;

	case WM_TIMER:
		GetLocalTime(&st);
		//判断什么时候需要重绘时针和分针，需要重绘的时候为一
		fChange = st.wHour != stPrevious.wHour ||
			st.wMinute != stPrevious.wMinute;

		hdc = GetDC(hwnd);

		SetIsotropic(hdc, cxClient, cyClient);

		SelectObject(hdc, GetStockObject(WHITE_PEN));
		DrawHands(hdc, &stPrevious, fChange);

		SelectObject(hdc, GetStockObject(BLACK_PEN));
		DrawHands(hdc, &st, TRUE);

		ReleaseDC(hwnd, hdc);

		stPrevious = st;
		return 0;

	case WM_PAINT:
		hdc = BeginPaint(hwnd, &ps);

		SetIsotropic(hdc, cxClient, cyClient);
		DrawClock(hdc);
		DrawHands(hdc, &stPrevious, TRUE);

		EndPaint(hwnd, &ps);
		return 0;

	case WM_DESTROY:
		KillTimer(hwnd, ID_TIMER);
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}
