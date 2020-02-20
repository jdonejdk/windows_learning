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
	/*SetMapMode��������ָ���豸������ӳ�䷽ʽ��ӳ�䷽ʽ�����˽��߼���λת��Ϊ�豸��λ�Ķ�����λ��
	   ���������豸��X��Y��ķ���  �߼���λת���ɾ��о��ȱ���������ⵥλ������X���һ����λ������Y���һ����λ */
	SetMapMode(hdc, MM_ISOTROPIC);
	//��ָ����ֵ������ָ���豸���������X�ᡢY�᷶Χ
	SetWindowExtEx(hdc, 1000, 1000, NULL);
	//�����豸�����Ĵ���ԭ��
	SetViewportExtEx(hdc, cxClient / 2, -cyClient / 2, NULL);
	//SetViewportOrgEx˵���ĸ��豸��ӳ�䵽����ԭ��
	SetViewportOrgEx(hdc, cxClient / 2, cyClient / 2, NULL);

}

//pt�Ǻ���һ������������飬iNumΪ�����Ŀ��iAngleΪÿ�������ת�Ƕ�
void RotatePoint(POINT pt[], int iNum, int iAngle)
{
	int   i;
	POINT ptTemp;

	for (i = 0; i < iNum; i++)
	{
		ptTemp.x = (int)(pt[i].x * cos(TWOPI * iAngle / 360) +
			pt[i].y * sin(TWOPI * iAngle / 360));//ת������ת���x����

		ptTemp.y = (int)(pt[i].y * cos(TWOPI * iAngle / 360) -
			pt[i].x * sin(TWOPI * iAngle / 360));//ת������ת���y����

		pt[i] = ptTemp; //�µĵ�ṹ���滻ԭ���ĵ�ṹ��
	}
}

void DrawClock(HDC hdc)
{
	int   iAngle;
	POINT pt[3];
    /* �������360��60���㣬һ������ת6�� */
	for (iAngle = 0; iAngle < 360; iAngle += 6)
	{
		pt[0].x = 0;
		pt[0].y = 900;

		RotatePoint(pt, 1, iAngle); //���������ת��ĵ��������Ϣ

		pt[2].x = pt[2].y = iAngle % 5 ? 33 : 100; //�ж��ӱ���Χ�ĵ��ֱ����С

		pt[0].x -= pt[2].x / 2;
		pt[0].y -= pt[2].y / 2; //��ȡ�ӱ���Χ�����Χ���ε����Ͻ�����ֵ

		pt[1].x = pt[0].x + pt[2].x;
		pt[1].y = pt[0].y + pt[2].y; //��ȡ�ӱ���Χ�����Χ���ε����½�����ֵ

		SelectObject(hdc, GetStockObject(BLACK_BRUSH)); // ���þ��
		//���һ����Բ����ָ���ľ���Χ�ơ���Բ�õ�ǰѡ��Ļ�����棬���õ�ǰѡ���ˢ�����
		Ellipse(hdc, pt[0].x, pt[0].y, pt[1].x, pt[1].y);
	}
}

// �˺��������滭ʱ�룬���������
void DrawHands(HDC hdc, SYSTEMTIME* pst, BOOL fChange)
{
	static POINT pt[3][5] = { 0, -150, 100, 0, 0, 600, -100, 0, 0, -150, // ʱ��5���������
							  0, -200,  50, 0, 0, 800,  -50, 0, 0, -200, // ����5���������
							  0,    0,   0, 0, 0,   0,    0, 0, 0,  800 }; // ����5���������
	int          i, iAngle[3];
	// �洢��ʱ����
	POINT        ptTemp[3][5];

	iAngle[0] = (pst->wHour * 30) % 360 + pst->wMinute / 2; //������ȡ��ǰʱ��ʱ��˳ʱ��ƫ����ֱ����ĽǶ�
	iAngle[1] = pst->wMinute * 6;//������ȡ��ǰʱ�����˳ʱ��ƫ����ֱ����ĽǶ�
	iAngle[2] = pst->wSecond * 6;//������ȡ��ǰʱ������˳ʱ��ƫ����ֱ����ĽǶ�

	memcpy(ptTemp, pt, sizeof(pt));

	for (i = fChange ? 0 : 2; i < 3; i++)//fChange�����þ��ǵ�Ϊ��ʱ���ػ�ʱ����������
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
		//�ж�ʲôʱ����Ҫ�ػ�ʱ��ͷ��룬��Ҫ�ػ��ʱ��Ϊһ
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
