// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include <Windows.h>

constexpr int EventStackSize = 100;

struct aeWindow
{
	HWND hwnd{};
    int index{ 0 };
    unsigned window{ 0 };
    unsigned width{ 0 };
    unsigned height{ 0 };
};

struct aeWindowEvent
{
    enum class Type
    {
        Empty, Close, Mouse1Down, Mouse1Up, Mouse2Down, Mouse2Up, MouseMiddleDown, MouseMiddleUp,
        MouseWheelScrollDown, MouseWheelScrollUp, MouseMove, KeyDown, KeyUp,
    };

    enum class KeyCode
    {
        A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
        Left, Right, Up, Down, Space, Escape, Enter, None
    };

    Type type = Type::Empty;
    KeyCode keyCode = KeyCode::None;
    int x = 0;
    int y = 0;
};

struct aeWindowImpl
{
    aeWindowEvent events[ EventStackSize ];
    int eventIndex = -1;
    unsigned windowWidth = 0;
    unsigned windowHeight = 0;
    unsigned windowHeightWithoutTitleBar = 0;
    HWND hwnd = nullptr;
    aeWindowEvent::KeyCode keyMap[ 256 ] = {};
};

aeWindowImpl windows[ 1 ];

static LONGLONG PCFreq;

long GetMilliseconds()
{
    LARGE_INTEGER li;
    QueryPerformanceCounter( &li );
    return long( li.QuadPart / PCFreq );
}

double GetCounter( LONGLONG counterStart )
{
    LARGE_INTEGER li;
    QueryPerformanceCounter( &li );
    return double( li.QuadPart - counterStart ) / PCFreq;
}

static void IncEventIndex()
{
    if (windows[ 0 ].eventIndex < EventStackSize - 1)
    {
        ++windows[ 0 ].eventIndex;
    }
}

static void InitKeyMap()
{
    for (unsigned keyIndex = 0; keyIndex < 256; ++keyIndex)
    {
        windows[ 0 ].keyMap[ keyIndex ] = aeWindowEvent::KeyCode::None;
    }

    windows[ 0 ].keyMap[13] = aeWindowEvent::KeyCode::Enter;
    windows[ 0 ].keyMap[37] = aeWindowEvent::KeyCode::Left;
    windows[ 0 ].keyMap[38] = aeWindowEvent::KeyCode::Up;
    windows[ 0 ].keyMap[39] = aeWindowEvent::KeyCode::Right;
    windows[ 0 ].keyMap[40] = aeWindowEvent::KeyCode::Down;
    windows[ 0 ].keyMap[27] = aeWindowEvent::KeyCode::Escape;
    windows[ 0 ].keyMap[32] = aeWindowEvent::KeyCode::Space;

    windows[ 0 ].keyMap[ 65 ] = aeWindowEvent::KeyCode::A;
    windows[ 0 ].keyMap[ 66 ] = aeWindowEvent::KeyCode::B;
    windows[ 0 ].keyMap[ 67 ] = aeWindowEvent::KeyCode::C;
    windows[ 0 ].keyMap[ 68 ] = aeWindowEvent::KeyCode::D;
    windows[ 0 ].keyMap[ 69 ] = aeWindowEvent::KeyCode::E;
    windows[ 0 ].keyMap[ 70 ] = aeWindowEvent::KeyCode::F;
    windows[ 0 ].keyMap[ 71 ] = aeWindowEvent::KeyCode::G;
    windows[ 0 ].keyMap[ 72 ] = aeWindowEvent::KeyCode::H;
    windows[ 0 ].keyMap[ 73 ] = aeWindowEvent::KeyCode::I;
    windows[ 0 ].keyMap[ 74 ] = aeWindowEvent::KeyCode::J;
    windows[ 0 ].keyMap[ 75 ] = aeWindowEvent::KeyCode::K;
    windows[ 0 ].keyMap[ 76 ] = aeWindowEvent::KeyCode::L;
    windows[ 0 ].keyMap[ 77 ] = aeWindowEvent::KeyCode::M;
    windows[ 0 ].keyMap[ 78 ] = aeWindowEvent::KeyCode::N;
    windows[ 0 ].keyMap[ 79 ] = aeWindowEvent::KeyCode::O;
    windows[ 0 ].keyMap[ 80 ] = aeWindowEvent::KeyCode::P;
    windows[ 0 ].keyMap[ 81 ] = aeWindowEvent::KeyCode::Q;
    windows[ 0 ].keyMap[ 82 ] = aeWindowEvent::KeyCode::R;
    windows[ 0 ].keyMap[ 83 ] = aeWindowEvent::KeyCode::S;
    windows[ 0 ].keyMap[ 84 ] = aeWindowEvent::KeyCode::T;
    windows[ 0 ].keyMap[ 85 ] = aeWindowEvent::KeyCode::U;
    windows[ 0 ].keyMap[ 86 ] = aeWindowEvent::KeyCode::V;
    windows[ 0 ].keyMap[ 87 ] = aeWindowEvent::KeyCode::W;
    windows[ 0 ].keyMap[ 88 ] = aeWindowEvent::KeyCode::X;
    windows[ 0 ].keyMap[ 89 ] = aeWindowEvent::KeyCode::Y;
    windows[ 0 ].keyMap[ 90 ] = aeWindowEvent::KeyCode::Z;
}

static LRESULT CALLBACK WindowProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
    switch (message)
    {
    case WM_SYSKEYUP:
    case WM_KEYUP:
        IncEventIndex();
        windows[ 0 ].events[ windows[ 0 ].eventIndex ].type = aeWindowEvent::Type::KeyUp;
        windows[ 0 ].events[ windows[ 0 ].eventIndex ].keyCode = windows[ 0 ].keyMap[ (unsigned)wParam ];
    break;
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
    {
        IncEventIndex();
        windows[ 0 ].events[ windows[ 0 ].eventIndex ].type = aeWindowEvent::Type::KeyDown;
        windows[ 0 ].events[ windows[ 0 ].eventIndex].keyCode = windows[ 0 ].keyMap[ (unsigned)wParam ];
    }
    break;
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    {
        IncEventIndex();
        windows[ 0 ].events[ windows[ 0 ].eventIndex ].type = message == WM_LBUTTONDOWN ? aeWindowEvent::Type::Mouse1Down : aeWindowEvent::Type::Mouse1Up;
        windows[ 0 ].events[ windows[ 0 ].eventIndex ].x = LOWORD( lParam );
        windows[ 0 ].events[ windows[ 0 ].eventIndex ].y = windows[ 0 ].windowHeightWithoutTitleBar - HIWORD( lParam );
    }
    break;
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    {
        IncEventIndex();
        windows[ 0 ].events[ windows[ 0 ].eventIndex ].type = message == WM_RBUTTONDOWN ? aeWindowEvent::Type::Mouse2Down : aeWindowEvent::Type::Mouse2Up;
        windows[ 0 ].events[ windows[ 0 ].eventIndex ].x = LOWORD( lParam );
        windows[ 0 ].events[ windows[ 0 ].eventIndex ].y = windows[ 0 ].windowHeightWithoutTitleBar - HIWORD( lParam );
    }
    break;
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
        IncEventIndex();
        windows[ 0 ].events[ windows[ 0 ].eventIndex ].type = message == WM_MBUTTONDOWN ? aeWindowEvent::Type::MouseMiddleDown : aeWindowEvent::Type::MouseMiddleUp;
        windows[ 0 ].events[ windows[ 0 ].eventIndex ].x = LOWORD( lParam );
        windows[ 0 ].events[ windows[ 0 ].eventIndex ].y = windows[ 0 ].windowHeightWithoutTitleBar - HIWORD( lParam );
    break;
	case WM_MOUSEWHEEL:
	{
        IncEventIndex();
		int delta = GET_WHEEL_DELTA_WPARAM( wParam );
        windows[ 0 ].events[ windows[ 0 ].eventIndex ].type = delta < 0 ? aeWindowEvent::Type::MouseWheelScrollDown : aeWindowEvent::Type::MouseWheelScrollUp;
        windows[ 0 ].events[ windows[ 0 ].eventIndex ].x = LOWORD( lParam );
        windows[ 0 ].events[ windows[ 0 ].eventIndex ].y = windows[ 0 ].windowHeightWithoutTitleBar - HIWORD( lParam );
	}
	break;
	case WM_MOUSEMOVE:
        IncEventIndex();
        windows[ 0 ].events[ windows[ 0 ].eventIndex ].type = aeWindowEvent::Type::MouseMove;
        windows[ 0 ].events[ windows[ 0 ].eventIndex ].x = LOWORD( lParam );
        windows[ 0 ].events[ windows[ 0 ].eventIndex ].y = windows[ 0 ].windowHeightWithoutTitleBar - HIWORD( lParam );
    break;
    case WM_CLOSE:
        IncEventIndex();
        windows[ 0 ].events[ windows[ 0 ].eventIndex ].type = aeWindowEvent::Type::Close;
        windows[ 0 ].events[ 1 ].type = aeWindowEvent::Type::Close;
    break;
    }

    return DefWindowProc( hWnd, message, wParam, lParam );
}

aeWindow aeCreateWindow( unsigned width, unsigned height, const char* title )
{
    aeWindow outWindow;
	outWindow.index = 0;

    windows[ outWindow.index ].windowWidth = width == 0 ? GetSystemMetrics( SM_CXSCREEN ) : width;
    windows[ outWindow.index ].windowHeight = height == 0 ? GetSystemMetrics( SM_CYSCREEN ) : height;

    const HINSTANCE hInstance = GetModuleHandle( nullptr );
    const bool fullscreen = (width == 0 && height == 0);

    WNDCLASSEX wc = {};

    wc.cbSize = sizeof( WNDCLASSEX );
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor( nullptr, IDC_ARROW );
    wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
    wc.lpszClassName = L"WindowClass1";
    wc.hIcon = static_cast<HICON>( LoadImage( nullptr, L"glider.ico", IMAGE_ICON, 32, 32, LR_LOADFROMFILE ) );

    RegisterClassEx( &wc );

    const int xPos = (GetSystemMetrics( SM_CXSCREEN ) - windows[ outWindow.index ].windowWidth) / 2;
    const int yPos = (GetSystemMetrics( SM_CYSCREEN ) - windows[ outWindow.index ].windowHeight) / 2;

    windows[ outWindow.index ].hwnd = CreateWindowExA( fullscreen ? WS_EX_TOOLWINDOW | WS_EX_TOPMOST : 0,
        "WindowClass1", "Window",
        fullscreen ? WS_POPUP : (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU),
        xPos, yPos,
        windows[ outWindow.index ].windowWidth, windows[ outWindow.index ].windowHeight,
        nullptr, nullptr, hInstance, nullptr );
    
    outWindow.hwnd = windows[ outWindow.index ].hwnd;

    SetWindowTextA( windows[ outWindow.index ].hwnd, title );
    ShowWindow( windows[ outWindow.index ].hwnd, SW_SHOW );

    RECT rect = {};
    GetClientRect( windows[ outWindow.index ].hwnd, &rect );
    windows[ outWindow.index ].windowHeightWithoutTitleBar = rect.bottom;

    InitKeyMap();

    LARGE_INTEGER li;
    QueryPerformanceFrequency( &li );
    PCFreq = li.QuadPart / 1000;

    return outWindow;
}

void aePumpWindowEvents( const aeWindow& window )
{
    MSG msg;

    while (PeekMessage( &msg, nullptr, 0, 0, PM_REMOVE ))
    {
        TranslateMessage( &msg );
        DispatchMessage( &msg );
    }
}

const aeWindowEvent& aePopWindowEvent( const aeWindow& window )
{
    aeWindowImpl& win = windows[ window.index ];

    if (win.eventIndex == -1)
    {
        win.events[ 0 ].type = aeWindowEvent::Type::Empty;
        return win.events[ 0 ];
    }

    --win.eventIndex;
    return win.events[ win.eventIndex + 1 ];
}

void aeDestroyWindow( aeWindow /*window*/ )
{

}
