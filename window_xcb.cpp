#include <sys/time.h>
#include <dirent.h>
#include <fcntl.h>
#include <linux/joystick.h>
#include <unistd.h>
#include <stdio.h>
#include <xcb/xcb_ewmh.h>
#include <xcb/xcb_icccm.h>
#include <xcb/xcb_keysyms.h>

struct aeWindow
{
    struct xcb_connection_t* connection = nullptr;
    int index = 0;
    unsigned window = 0;
    unsigned width = 0;
    unsigned height = 0;
};

struct aeWindowEvent
{
    enum class Type
    {
        Empty, Close, Mouse1Down, Mouse1Up, Mouse2Down, Mouse2Up, MouseMiddleDown, MouseMiddleUp,
        MouseWheelScrollDown, MouseWheelScrollUp, MouseMove, KeyDown, KeyUp,
        GamePadButtonA, GamePadButtonB, GamePadButtonX,
        GamePadButtonY, GamePadButtonDPadUp, GamePadButtonDPadDown,
        GamePadButtonDPadLeft, GamePadButtonDPadRight, GamePadButtonStart,
        GamePadButtonBack, GamePadButtonLeftShoulder, GamePadButtonRightShoulder,
        GamePadLeftThumbState, GamePadRightThumbState
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
    float gamePadThumbX = 0;
    /// Gamepad's thumb y in range [-1, 1]. Event type indicates left or right thumb.
    float gamePadThumbY = 0;
};

struct aeWindowImpl
{
    aeWindowEvent events[ 15 ];
    xcb_key_symbols_t* keySymbols = nullptr;
    int eventIndex = -1;
};

struct GamePad
{
    bool isActive = false;
    int fd;
    int buttonA;
    int buttonB;
    int buttonX;
    int buttonY;
    int buttonBack;
    int buttonStart;
    int buttonLeftShoulder;
    int buttonRightShoulder;
    int dpadXaxis;
    int dpadYaxis;
    int leftThumbX;
    int leftThumbY;
    int rightThumbX;
    int rightThumbY;
    short deadZone;
    float lastLeftThumbX;
    float lastLeftThumbY;
    float lastRightThumbX;
    float lastRightThumbY;
};

GamePad gamePad;
aeWindowImpl windows[ 10 ];

long GetMilliseconds()
{
    timeval time;
    gettimeofday( &time, nullptr );
    return (time.tv_sec * 1000) + (time.tv_usec / 1000);
}

static unsigned mystrlen( const char* str )
{
    const char *p =str;

    while (*p != '\0')
    {
        ++p;
    }
    
    return p - str;
}

static void IncEventIndex()
{
    if (windows[ 0 ].eventIndex < 15 - 1)
    {
        ++windows[ 0 ].eventIndex;
    }
}

static float ProcessGamePadStickValue( short value, short deadZoneThreshold )
{
    float result = 0;
        
    if (value < -deadZoneThreshold)
    {
        result = (value + deadZoneThreshold) / (32768.0f - deadZoneThreshold);
    }
    else if (value > deadZoneThreshold)
    {
        result = (value - deadZoneThreshold) / (32767.0f - deadZoneThreshold);
    }

    return result;
}

static void InitGamePad()
{
    DIR* dir = opendir( "/dev/input" );
    dirent* result = readdir( dir );

    while (result != nullptr)
    {
        dirent& entry = *result;
        
        if ((entry.d_name[0] == 'j') && (entry.d_name[1] == 's'))
        {
            char full_device_path[ 267 ];
            snprintf( full_device_path, sizeof( full_device_path ), "%s/%s", "/dev/input", entry.d_name );
            int fd = open( full_device_path, O_RDONLY );

            if (fd < 0)
            {
                // Permissions could cause this code path.
                continue;
            }

            char name[ 128 ];
            ioctl( fd, JSIOCGNAME( 128 ), name);

            int version;
            ioctl( fd, JSIOCGVERSION, &version );
            uint8_t axes;
            ioctl( fd, JSIOCGAXES, &axes );
            uint8_t buttons;
            ioctl( fd, JSIOCGBUTTONS, &buttons );
            gamePad.fd = fd;
            gamePad.isActive = true;
            // XBox One Controller values. Should also work for 360 Controller.
            gamePad.buttonA = 0;
            gamePad.buttonB = 1;
            gamePad.buttonX = 2;
            gamePad.buttonY = 3;
            gamePad.buttonStart = 7;
            gamePad.buttonBack = 6;
            gamePad.buttonLeftShoulder = 4;
            gamePad.buttonRightShoulder = 5;
            gamePad.dpadXaxis = 6;
            gamePad.dpadYaxis = 7;
            gamePad.leftThumbX = 0;
            gamePad.leftThumbY = 1;
            gamePad.rightThumbX = 3;
            gamePad.rightThumbY = 4;
            gamePad.deadZone = 7849;
            
            fcntl( fd, F_SETFL, O_NONBLOCK );
        }

        result = readdir( dir );
    }

    closedir( dir );
}

static aeWindowEvent::KeyCode GetKeycode( uint32_t xcbKey )
{
    switch( xcbKey )
    {
    case 97: return aeWindowEvent::KeyCode::A;
    case 98: return aeWindowEvent::KeyCode::B;
    case 99: return aeWindowEvent::KeyCode::C;
    case 100: return aeWindowEvent::KeyCode::D;
    case 101: return aeWindowEvent::KeyCode::E;
    case 102: return aeWindowEvent::KeyCode::F;
    case 103: return aeWindowEvent::KeyCode::G;
    case 104: return aeWindowEvent::KeyCode::H;
    case 105: return aeWindowEvent::KeyCode::I;
    case 106: return aeWindowEvent::KeyCode::J;
    case 107: return aeWindowEvent::KeyCode::K;
    case 108: return aeWindowEvent::KeyCode::L;
    case 109: return aeWindowEvent::KeyCode::M;
    case 110: return aeWindowEvent::KeyCode::N;
    case 111: return aeWindowEvent::KeyCode::O;
    case 112: return aeWindowEvent::KeyCode::P;
    case 113: return aeWindowEvent::KeyCode::Q;
    case 114: return aeWindowEvent::KeyCode::R;
    case 115: return aeWindowEvent::KeyCode::S;
    case 116: return aeWindowEvent::KeyCode::T;
    case 117: return aeWindowEvent::KeyCode::U;
    case 118: return aeWindowEvent::KeyCode::V;
    case 119: return aeWindowEvent::KeyCode::W;
    case 120: return aeWindowEvent::KeyCode::X;
    case 121: return aeWindowEvent::KeyCode::Y;
    case 122: return aeWindowEvent::KeyCode::Z;
    case 32: return aeWindowEvent::KeyCode::Space;
    case 65293: return aeWindowEvent::KeyCode::Enter;
    case 65361: return aeWindowEvent::KeyCode::Left;
    case 65362: return aeWindowEvent::KeyCode::Up;
    case 65363: return aeWindowEvent::KeyCode::Right;
    case 65364: return aeWindowEvent::KeyCode::Down;
    case 65307: return aeWindowEvent::KeyCode::Escape;
    default: return aeWindowEvent::KeyCode::A;
    }
}

aeWindow aeCreateWindow( unsigned width, unsigned height, const char* title )
{
    aeWindow outWindow;
    outWindow.index = 0;

    outWindow.connection = xcb_connect( nullptr, nullptr );

    if (xcb_connection_has_error( outWindow.connection ))
    {
        outWindow.index = -1;
        return outWindow;
    }

    xcb_screen_t* s = xcb_setup_roots_iterator( xcb_get_setup( outWindow.connection ) ).data;
    outWindow.window = xcb_generate_id( outWindow.connection );
    outWindow.width = width == 0 ? s->width_in_pixels : width;
    outWindow.height = height == 0 ? s->height_in_pixels : height;
    windows[ outWindow.index ].keySymbols = xcb_key_symbols_alloc( outWindow.connection );
    
    const unsigned mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
    const unsigned eventMask = XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE |
                               XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE | XCB_EVENT_MASK_POINTER_MOTION;
    const unsigned values[ 2 ] { s->white_pixel, eventMask };
    
    xcb_create_window( outWindow.connection, s->root_depth, outWindow.window, s->root,
                       10, 10,
                       outWindow.width,
                       outWindow.height,
                       1,
                       XCB_WINDOW_CLASS_INPUT_OUTPUT, s->root_visual,
                       mask, values );

    xcb_map_window( outWindow.connection, outWindow.window );
    xcb_flush( outWindow.connection );

    xcb_size_hints_t hints = {};
    xcb_icccm_size_hints_set_min_size( &hints, outWindow.width, outWindow.height );
    xcb_icccm_size_hints_set_max_size( &hints, outWindow.width, outWindow.height );
    xcb_icccm_set_wm_size_hints( outWindow.connection, outWindow.window, XCB_ATOM_WM_NORMAL_HINTS, &hints );

    xcb_change_property( outWindow.connection, XCB_PROP_MODE_REPLACE, outWindow.window, XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8, mystrlen( title ), title );

    if (width == 0 && height == 0)
    {
        xcb_ewmh_connection_t EWMH;
        xcb_intern_atom_cookie_t* EWMHCookie = xcb_ewmh_init_atoms( outWindow.connection, &EWMH );

        if (!xcb_ewmh_init_atoms_replies( &EWMH, EWMHCookie, nullptr ))
        {
            outWindow.index = -1;
            return outWindow;
        }

        xcb_change_property( outWindow.connection, XCB_PROP_MODE_REPLACE, outWindow.window, EWMH._NET_WM_STATE, XCB_ATOM_ATOM, 32, 1, &(EWMH._NET_WM_STATE_FULLSCREEN) );
        
        xcb_generic_error_t* error;
        xcb_get_window_attributes_reply_t* reply = xcb_get_window_attributes_reply( outWindow.connection,
                                                                                    xcb_get_window_attributes( outWindow.connection,
                                                                                                               outWindow.window ), &error );

        if (!reply)
        {
            outWindow.index = -1;
            return outWindow;
        }

        free( reply );
    }

    InitGamePad();
    return outWindow;
}

void aeDestroyWindow( aeWindow window )
{
    xcb_destroy_window( window.connection, window.window ); 
    xcb_disconnect( window.connection );
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

void aePumpWindowEvents( const aeWindow& window )
{
    xcb_generic_event_t* event;
    
    while ((event = xcb_poll_for_event( window.connection )))
    {
        if (windows[ window.index ].eventIndex >= 14)
        {
            free( event );
            return;
        }
        
        const uint8_t responseType = event->response_type & ~0x80;

        if (responseType == XCB_BUTTON_PRESS || responseType == XCB_BUTTON_RELEASE)
        {
            xcb_button_press_event_t* bp = (xcb_button_press_event_t *)event;
            IncEventIndex();
            
            if (bp->detail == 2)
            {
                windows[ window.index ].events[ windows[ window.index ].eventIndex ].type = responseType == XCB_BUTTON_RELEASE ? aeWindowEvent::Type::MouseMiddleUp : aeWindowEvent::Type::MouseMiddleDown;
            }
            else if (bp->detail == 3)
            {
                windows[ window.index ].events[ windows[ window.index ].eventIndex ].type = responseType == XCB_BUTTON_RELEASE ? aeWindowEvent::Type::Mouse2Up : aeWindowEvent::Type::Mouse2Down;
            }
            else
            {
                windows[ window.index ].events[ windows[ window.index ].eventIndex ].type = responseType == XCB_BUTTON_RELEASE ? aeWindowEvent::Type::Mouse1Up : aeWindowEvent::Type::Mouse1Down;
            }
            
            windows[ window.index ].events[ windows[ window.index ].eventIndex ].x = bp->event_x;
            windows[ window.index ].events[ windows[ window.index ].eventIndex ].y = bp->event_y;
        }
        else if (responseType == XCB_KEY_PRESS)
        {
            IncEventIndex();
            xcb_key_press_event_t* kp = (xcb_key_press_event_t *)event;
            const xcb_keysym_t keysym = xcb_key_symbols_get_keysym( windows[ window.index ].keySymbols, kp->detail, 0 );
            
            windows[ window.index ].events[ windows[ window.index ].eventIndex ].type = aeWindowEvent::Type::KeyDown;
            windows[ window.index ].events[ windows[ window.index ].eventIndex ].keyCode = GetKeycode( keysym );
        }
        else if (responseType == XCB_KEY_RELEASE)
        {
            IncEventIndex();
            xcb_key_press_event_t* kp = (xcb_key_press_event_t *)event;
            const xcb_keysym_t keysym = xcb_key_symbols_get_keysym( windows[ window.index ].keySymbols, kp->detail, 0 );

            windows[ window.index ].events[ windows[ window.index ].eventIndex ].type = aeWindowEvent::Type::KeyUp;
            windows[ window.index ].events[ windows[ window.index ].eventIndex ].keyCode = GetKeycode( keysym );
        }
        else if (responseType == XCB_MOTION_NOTIFY)
        {
            IncEventIndex();
            xcb_motion_notify_event_t* motion = (xcb_motion_notify_event_t *)event;
            windows[ window.index ].events[ windows[ window.index ].eventIndex ].type = aeWindowEvent::Type::MouseMove;
            windows[ window.index ].events[ windows[ window.index ].eventIndex ].x = motion->event_x;
            windows[ window.index ].events[ windows[ window.index ].eventIndex ].y = motion->event_y;
        }
        
        free( event );
    }

    aeWindowImpl& win = windows[ window.index ];

    if (!gamePad.isActive)
    {
        return;
    }
    
    js_event j;

    while (read( gamePad.fd, &j, sizeof( js_event ) ) == sizeof( js_event ))
    {
        j.type &= ~JS_EVENT_INIT;
            
        if (j.type == JS_EVENT_BUTTON)
        {
            IncEventIndex();

            if (j.number == gamePad.buttonA && j.value > 0)
            {
                win.events[ win.eventIndex ].type = aeWindowEvent::Type::GamePadButtonA;
            }
            else if (j.number == gamePad.buttonB && j.value > 0)
            {
                win.events[ win.eventIndex ].type = aeWindowEvent::Type::GamePadButtonB;
            }
            else if (j.number == gamePad.buttonX && j.value > 0)
            {
                win.events[ win.eventIndex ].type = aeWindowEvent::Type::GamePadButtonX;
            }
            else if (j.number == gamePad.buttonY && j.value > 0)
            {
                win.events[ win.eventIndex ].type = aeWindowEvent::Type::GamePadButtonY;
            }
            else if (j.number == gamePad.buttonStart && j.value > 0)
            {
                win.events[ win.eventIndex ].type = aeWindowEvent::Type::GamePadButtonStart;
            }
            else if (j.number == gamePad.buttonBack && j.value > 0)
            {
                win.events[ win.eventIndex ].type = aeWindowEvent::Type::GamePadButtonBack;
            }
        }
        else if (j.type == JS_EVENT_AXIS)
        {
            IncEventIndex();

            if (j.number == gamePad.leftThumbX)
            {
                const float x = ProcessGamePadStickValue( j.value, gamePad.deadZone );
                win.events[ win.eventIndex ].type = aeWindowEvent::Type::GamePadLeftThumbState;
                win.events[ win.eventIndex ].gamePadThumbX = x;
                win.events[ win.eventIndex ].gamePadThumbY = gamePad.lastLeftThumbY;
                gamePad.lastLeftThumbX = x;
            }
            else if (j.number == gamePad.leftThumbY)
            {
                const float y = ProcessGamePadStickValue( j.value, gamePad.deadZone );
                win.events[ win.eventIndex ].type = aeWindowEvent::Type::GamePadLeftThumbState;
                win.events[ win.eventIndex ].gamePadThumbX = gamePad.lastLeftThumbX;
                win.events[ win.eventIndex ].gamePadThumbY = -y;
                gamePad.lastLeftThumbY = -y;
            }
            else if (j.number == gamePad.rightThumbX)
            {
                const float x = ProcessGamePadStickValue( j.value, gamePad.deadZone );
                win.events[ win.eventIndex ].type = aeWindowEvent::Type::GamePadRightThumbState;
                win.events[ win.eventIndex ].gamePadThumbX = x;
                win.events[ win.eventIndex ].gamePadThumbY = gamePad.lastRightThumbY;
                gamePad.lastRightThumbX = x;
            }
            else if (j.number == gamePad.rightThumbY)
            {
                const float y = ProcessGamePadStickValue( j.value, gamePad.deadZone );
                win.events[ win.eventIndex ].type = aeWindowEvent::Type::GamePadRightThumbState;
                win.events[ win.eventIndex ].gamePadThumbX = gamePad.lastRightThumbX;
                win.events[ win.eventIndex ].gamePadThumbY = -y;
                gamePad.lastRightThumbY = -y;
            }
            else if (j.number == gamePad.dpadXaxis)
            {
                if (j.value > 0)
                {
                    win.events[ win.eventIndex ].type = aeWindowEvent::Type::GamePadButtonDPadRight;
                }
                if (j.value < 0)
                {
                    win.events[ win.eventIndex ].type = aeWindowEvent::Type::GamePadButtonDPadLeft;
                }
            }
            else if (j.number == gamePad.dpadYaxis)
            {
                if (j.value < 0)
                {
                    win.events[ win.eventIndex ].type = aeWindowEvent::Type::GamePadButtonDPadUp;
                }
                if (j.value > 0)
                {
                    win.events[ win.eventIndex ].type = aeWindowEvent::Type::GamePadButtonDPadDown;
                }
            }
        }
    }
}

