#include <windows.h>
#include <stdio.h>
/*
This is the main file for the GUI for the cipher code.
*/

/*
Define the elements id's
*/
#define ID_INPUT1 101
#define ID_INPUT2 102
#define ID_ADD 201
#define ID_SUB 202
#define ID_MUL 203
#define ID_DIV 204
#define ID_RESULT 301

HWND hInput1, hInput2, hAddBtn, hSubBtn, hMulBtn, hDivBtn, hResult;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// Making the hInput1/2 vissible only if it has somekind of length.
void CheckInputs(HWND hwnd);
// The calculating function itself.
void Calculate(HWND hwnd, int oper);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    const char CLASS_NAME[] = "aSimpleApp";
    char appName[] = "A simple app";
    /*
    window class -> window type and other parameter related to a 'window'
    */
    WNDCLASS wc = {0};

    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    // Registering a address of a window class
    RegisterClass(&wc);

    /*
    HWND -> 'Handler' of the 'Window'.
    */

    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        appName,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL,
        NULL,
        hInstance,
        NULL);

    if (hwnd == NULL)
    {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);
    // Updating when clicking
    UpdateWindow(hwnd);

    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

/*
Windows procedure -> Clicks, mouse movemenet and etc.
*/
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {

    case WM_CREATE:
        /*
         Creting the sub-items in the main window
        */
        hInput1 = CreateWindow("EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER,
                               50, 30, 200, 25, hwnd, (HMENU)ID_INPUT1, NULL, NULL);
        hInput2 = CreateWindow("EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER,
                               50, 70, 200, 25, hwnd, (HMENU)ID_INPUT2, NULL, NULL);
        hAddBtn = CreateWindow("BUTTON", "+", WS_CHILD | WS_BORDER,
                               50, 110, 40, 25, hwnd, (HMENU)ID_ADD, NULL, NULL);
        hSubBtn = CreateWindow("BUTTON", "-", WS_CHILD | WS_BORDER,
                               100, 110, 40, 25, hwnd, (HMENU)ID_SUB, NULL, NULL);
        hMulBtn = CreateWindow("BUTTON", "*", WS_CHILD | WS_BORDER,
                               150, 110, 40, 25, hwnd, (HMENU)ID_MUL, NULL, NULL);
        hDivBtn = CreateWindow("BUTTON", "/", WS_CHILD | WS_BORDER,
                               200, 110, 40, 25, hwnd, (HMENU)ID_DIV, NULL, NULL);

        hResult = CreateWindow("EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY,
                               50, 150, 200, 25, hwnd, (HMENU)ID_RESULT, NULL, NULL);
        ShowWindow(hAddBtn, SW_HIDE);
        ShowWindow(hSubBtn, SW_HIDE);
        ShowWindow(hMulBtn, SW_HIDE);
        ShowWindow(hDivBtn, SW_HIDE);
        break;

    case WM_COMMAND:
        /*
        Manipulating
        */
        if (LOWORD(wParam) == ID_INPUT1 || LOWORD(wParam) == ID_INPUT2)
        {
            CheckInputs(hwnd);
        }
        else if (LOWORD(wParam) >= ID_ADD && LOWORD(wParam) <= ID_DIV)
        {
            Calculate(hwnd, LOWORD(wParam));
        }

        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        TextOut(hdc, 10, 10, "Hello, GUI World!", 17);
        EndPaint(hwnd, &ps);
    }
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void CheckInputs(HWND hwnd)
{
    char text1[100], text2[100];
    GetWindowText(hInput1, text1, 100);
    GetWindowText(hInput2, text2, 100);

    if (strlen(text1) > 0 && strlen(text2) > 0)
    {
        // Show buttons
        ShowWindow(hAddBtn, SW_SHOW);
        ShowWindow(hSubBtn, SW_SHOW);
        ShowWindow(hMulBtn, SW_SHOW);
        ShowWindow(hDivBtn, SW_SHOW);
    }
}

void Calculate(HWND hwnd, int oper)
{
    char text1[100], text2[100];
    GetWindowText(hInput1, text1, 100);
    GetWindowText(hInput2, text2, 100);

    double a = atof(text1);
    double b = atof(text2);
    double result;

    switch (oper)
    {
    case ID_ADD:
        result = a + b;
        break;
    case ID_SUB:
        result = a - b;
        break;
    case ID_MUL:
        result = a * b;
        break;
    case ID_DIV:
        if (b == 0)
        {
            SetWindowText(hResult, "Error: divide by zero");
            return;
        }
        result = a / b;
        break;
    }

    char resStr[100];
    sprintf(resStr, "Result: %.2f", result);
    SetWindowText(hResult, resStr);
}
