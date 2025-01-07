#include <windows.h>
#include <windowsx.h>
#include <xaudio2.h>
#include "khr/khrplatform.h"
#include <gl/gl.h>
#include "gl/glext.h"
#include "gl/wglext.h"
#include <stdio.h>
#include <math.h>

#if 0
{
#include "glad/glad.c"
#define GLFW_DLL
#include "glfw/glfw3.h"
#define GLFW_EXPOSE_NATIVE_WIN32
#include "glfw/glfw3native.h"
    
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui/imgui.h"
#include "imgui/imgui.cpp"
#include "imgui/imgui_draw.cpp"
#include "imgui/imgui_tables.cpp"
#include "imgui/imgui_widgets.cpp"
#include "imgui/imgui_demo.cpp"
#define IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#include "imgui/backends/imgui_impl_glfw.cpp"
#include "imgui/backends/imgui_impl_opengl3.cpp"
}
#endif

#include "base.h"
#include "log.h"
#include "game_loader.h"

#ifndef TEMPEST_RELEASE
#define GAME_DLL_PATH "game.dll"
#define GAME_DLL_COPY_PATH "game_temp_copy.dll"
#else
#define GAME_DLL_PATH "tempest.dll"
#endif

#define FAKE_WINDOW_NAME ("Fake Window (Indestructible!!!!)")

// Later: more extensive error logging on all of these

void
log_windows_error(DWORD error_code)
{
    LPSTR message_buffer;
    FormatMessage(
                  FORMAT_MESSAGE_ALLOCATE_BUFFER | 
                  FORMAT_MESSAGE_FROM_SYSTEM |
                  FORMAT_MESSAGE_IGNORE_INSERTS,
                  NULL,
                  error_code,
                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                  (LPTSTR) &message_buffer,
                  0, NULL );
    log(message_buffer);
}

F64
get_time()
{
    LARGE_INTEGER ticks;
    LARGE_INTEGER freq;
    if (!QueryPerformanceCounter(&ticks))
    {
        log_windows_error(GetLastError());
        ASSERT(false);
    }
    if (!QueryPerformanceFrequency(&freq))
    {
        log_windows_error(GetLastError());
        ASSERT(false);
    }
    return ((F64)(ticks.QuadPart)) / ((F64)(freq.QuadPart));
}

Void
sleep(F64 seconds)
{
    //ASSERT(timeBeginPeriod(1) == TIMERR_NOERROR);
    //ASSERT(timeEndPeriod(1) == TIMERR_NOERROR);
    
    //Sleep((DWORD)(seconds * 1000.0));
    //Sleep((DWORD)16);
    // NOTE: EVIL sleep
    // Later: make this not devour cpu cycles
    
#if 1
    F64 start_time = get_time();
    F64 current_time;
    F64 time_diff;
    do
    {
        current_time = get_time();
        time_diff = current_time - start_time;
    }
    while(time_diff < seconds);
#endif
}

U64 get_file_last_write_time(const Char *path)
{
    WIN32_FILE_ATTRIBUTE_DATA Data;
    if(GetFileAttributesEx(path, GetFileExInfoStandard, &Data))
    {
        U64 result = Data.ftLastWriteTime.dwLowDateTime;
        result = result | ((U64)(Data.ftLastWriteTime.dwHighDateTime) << 32);
        return result;
    }
    
    return 0;
}

Int
get_game_dll_last_write_time(LPFILETIME time)
{
    WIN32_FILE_ATTRIBUTE_DATA Data;
    if(GetFileAttributesEx(GAME_DLL_PATH, GetFileExInfoStandard, &Data))
    {
        *time = Data.ftLastWriteTime;
    }
    else
    {
        return 1;
    }
    
    return 0;
}


Int
unload_game_dll(GameCode *game_code, HINSTANCE *module_handle)
{
    int result = 0;
    
    result = FreeLibrary(*module_handle);
    if(result)
    {
        game_code->loaded = false;
    }
    
    return result;
}

Int
load_game_dll(GameCode *game_code, HINSTANCE *module_handle)
{
#ifndef TEMPEST_RELEASE
    CopyFile(GAME_DLL_PATH, GAME_DLL_COPY_PATH, false);
    
    *module_handle = LoadLibrary(TEXT(GAME_DLL_COPY_PATH));
#else
    *module_handle = LoadLibrary(TEXT(GAME_DLL_PATH));
#endif
    
    if(module_handle == NULL)
    {
        print_error("dll wasn't loaded properly");
        return 1;
    }
    
    game_code->update_and_render = (void (*) (GameMemory*)) GetProcAddress(*module_handle, "update_and_render");
    
    if(game_code->update_and_render != NULL)
    {
        game_code->loaded = true;
    }
    else
    {
        print_error("functions didn't load correctly");
        
        log_windows_error(GetLastError());
        
        FreeLibrary(*module_handle);
        return 1;
    }
    
    return 0;
}

FileContents
read_file_contents(const Char *path)
{
    FileContents result = {0};
    
    HANDLE file_handle = CreateFileA(path,
                                     GENERIC_READ,
                                     FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
                                     NULL,
                                     OPEN_EXISTING,
                                     FILE_ATTRIBUTE_NORMAL,
                                     NULL);
    
    if(file_handle == INVALID_HANDLE_VALUE)
    {
        log_warning("couldn't open file");
        return result;
    }
    
    result.file_found = true;
    
    LARGE_INTEGER file_size;
    Bool get_file_size_result = GetFileSizeEx(file_handle,
                                              &file_size);
    if(!get_file_size_result)
    {
        log_warning("couldn't get file size");
        return result;
    }
    
    result.size = file_size.QuadPart;
    result.data = (U8 *)mem_alloc(result.size);
    result.allocated = true;
    
    DWORD bytes_read;
    Bool read_file_result = ReadFile(file_handle,
                                     (Void *)result.data,
                                     result.size,
                                     &bytes_read,
                                     NULL);
    
    if(!read_file_result)
    {
        print_warning("couldn't read file");
        return result;
    }
    
    result.contains_proper_data = true;
    return result;
};

Bool
write_file_contents(const Char *path, U8 *data, U64 size)
{
    if(size == 0 || !data)
    {
        log_warning("size == 0 or data is null");
        return false;
    }
    
    HANDLE file_handle = CreateFileA(path,
                                     GENERIC_WRITE,
                                     FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
                                     NULL,
                                     OPEN_ALWAYS,
                                     FILE_ATTRIBUTE_NORMAL,
                                     NULL);
    
    if(file_handle == INVALID_HANDLE_VALUE)
    {
        log_warning("couldn't open file");
        return false;
    }
    
    DWORD bytes_written;
    Bool write_result = WriteFile(file_handle,
                                  data,
                                  size,
                                  &bytes_written,
                                  NULL);
    if(!write_result)
    {
        log_warning("couldn's write to file");
        return false;
    }
    
    return true;
}


Void error_callback( Int error, const Char *msg ) {
    print_error("%d: %s", error, msg);
    ASSERT(false);
}

Float width_over_height = 1000.0f/1000.0f;

#if 0
Void
initialize_directsound()
{
    // load the library
    HMODULE directsound_library = LoadLibraryA("dsound.dll");
    
    if(directsound_library)
    {
    }
    
    // get directsound object
}
#endif

struct XAudio2Data
{
    IXAudio2 *instance;
    IXAudio2MasteringVoice *mastering_voice;
    IXAudio2SourceVoice *source_voice;
};

XAudio2Data
initialize_xaudio2(AudioBuffer *buffer)
{
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    // Later: log this and run without audio
    ASSERT(!FAILED(hr));
    
    IXAudio2 *xaudio2_instance;
    ASSERT(XAudio2Create(&xaudio2_instance, 0, XAUDIO2_DEFAULT_PROCESSOR) == S_OK);
    
    IXAudio2MasteringVoice *xaudio2_mastering_voice;
    xaudio2_instance->CreateMasteringVoice(&xaudio2_mastering_voice);
    
    WAVEFORMATEX wave_format;
    wave_format.wFormatTag = WAVE_FORMAT_PCM;
    wave_format.nChannels = 2; // 2 channels
    wave_format.nSamplesPerSec = AUDIO_FRAMES_PER_SECOND;
    wave_format.wBitsPerSample = AUDIO_BYTES_PER_SAMPLE*8;
    wave_format.nBlockAlign = wave_format.nChannels * AUDIO_BYTES_PER_SAMPLE;
    wave_format.nAvgBytesPerSec = wave_format.nSamplesPerSec * wave_format.nBlockAlign;
    wave_format.cbSize = 0;
    
    IXAudio2SourceVoice *xaudio2_source_voice;
    ASSERT(SUCCEEDED(xaudio2_instance->CreateSourceVoice(&xaudio2_source_voice, &wave_format)));
    
    U32 buffer_index = 0;
    while (buffer_index < AUDIO_BUFFER_SAMPLE_COUNT)
    {
        buffer->samples[buffer_index++] = 0;
    }
    buffer->play_cursor_absolute = 0;
    buffer->write_cursor_absolute = 0;
    
    XAUDIO2_BUFFER xaudio2_buffer{};
    xaudio2_buffer.Flags = XAUDIO2_END_OF_STREAM;
    xaudio2_buffer.AudioBytes = sizeof(buffer->samples);
    xaudio2_buffer.pAudioData = (U8 *)buffer->samples;
    xaudio2_buffer.PlayBegin = 0;
    xaudio2_buffer.PlayLength = 0;
    xaudio2_buffer.LoopBegin = 0;
    xaudio2_buffer.LoopLength = 0;
    xaudio2_buffer.LoopCount = XAUDIO2_LOOP_INFINITE;
    
    ASSERT(SUCCEEDED(xaudio2_source_voice->SubmitSourceBuffer(&xaudio2_buffer)));
    ASSERT(SUCCEEDED(xaudio2_source_voice->Start()));
    
    return {xaudio2_instance, xaudio2_mastering_voice, xaudio2_source_voice};
}


Void
update_input(Input *input, Float d_time)
{
#if 0
    input->left_mouse_down = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    input->right_mouse_down = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
    
    
    Keys *keys = &input->keys;
    for(int i = 0; i < sizeof(keys->data) / sizeof(KeyData); i++)
    {
        KeyData *key = &(keys->data[i]);
        
        if(glfwGetKey(window, key->key_code) == GLFW_PRESS)
        {
            key->is_down = true;
            if(key->press_time == 0)
                key->just_pressed = true;
            else
                key->just_pressed = false;
            
            key->press_time += d_time;
        }
        else
        {
            key->is_down = false;
            key->just_pressed = false;
            key->press_time = 0;
            key->time_till_next_repeat = 0;
        }
        
        if(key->just_pressed || key->press_time >= input->key_first_repeat_time)
        {
            if(key->time_till_next_repeat <= 0)
            {
                key->time_till_next_repeat = input->key_repeat_speed;
            }
            else
            {
                key->time_till_next_repeat -= d_time;
            }
        }
    }
#endif
}


KeyData *
get_key_data_for_key_code(Input *input, WPARAM key_code)
{
    Keys *keys = &input->keys;
    
    if(key_code == VK_LBUTTON) return &keys->mouse_left;
    if(key_code == VK_MBUTTON) return &keys->mouse_middle;
    if(key_code == VK_RBUTTON) return &keys->mouse_right;
    
    if(key_code == VK_LEFT) return &keys->left;
    if(key_code == VK_RIGHT) return &keys->right;
    if(key_code == VK_UP) return &keys->up;
    if(key_code == VK_DOWN) return &keys->down;
    if(key_code == VK_PRIOR) return &keys->page_up;
    if(key_code == VK_NEXT) return &keys->page_down;
    
    if(key_code == 0x41) return &keys->a;
    if(key_code == 0x42) return &keys->b;
    if(key_code == 0x43) return &keys->c;
    if(key_code == 0x44) return &keys->d;
    if(key_code == 0x45) return &keys->e;
    if(key_code == 0x46) return &keys->f;
    if(key_code == 0x47) return &keys->g;
    if(key_code == 0x48) return &keys->h;
    if(key_code == 0x49) return &keys->i;
    if(key_code == 0x4A) return &keys->j;
    if(key_code == 0x4B) return &keys->k;
    if(key_code == 0x4C) return &keys->l;
    if(key_code == 0x4D) return &keys->m;
    if(key_code == 0x4E) return &keys->n;
    if(key_code == 0x4F) return &keys->o;
    if(key_code == 0x50) return &keys->p;
    if(key_code == 0x51) return &keys->q;
    if(key_code == 0x52) return &keys->r;
    if(key_code == 0x53) return &keys->s;
    if(key_code == 0x54) return &keys->t;
    if(key_code == 0x55) return &keys->u;
    if(key_code == 0x56) return &keys->v;
    if(key_code == 0x57) return &keys->w;
    if(key_code == 0x58) return &keys->x;
    if(key_code == 0x59) return &keys->y;
    if(key_code == 0x5A) return &keys->z;
    
    if(key_code == 0x30) return &keys->number_0;
    if(key_code == 0x31) return &keys->number_1;
    if(key_code == 0x32) return &keys->number_2;
    if(key_code == 0x33) return &keys->number_3;
    if(key_code == 0x34) return &keys->number_4;
    if(key_code == 0x35) return &keys->number_5;
    if(key_code == 0x36) return &keys->number_6;
    if(key_code == 0x37) return &keys->number_7;
    if(key_code == 0x38) return &keys->number_8;
    if(key_code == 0x39) return &keys->number_9;
    
    if(key_code == VK_OEM_3) return &keys->grave;
    if(key_code == VK_OEM_MINUS) return &keys->minus;
    if(key_code == VK_OEM_PLUS) return &keys->equal;
    if(key_code == VK_OEM_4) return &keys->left_bracket;
    if(key_code == VK_OEM_6) return &keys->right_bracket;
    if(key_code == VK_OEM_5) return &keys->backslash;
    if(key_code == VK_OEM_1) return &keys->semicolon;
    if(key_code == VK_OEM_7) return &keys->quote;
    if(key_code == VK_OEM_2) return &keys->slash;
    if(key_code == VK_OEM_COMMA) return &keys->comma;
    if(key_code == VK_OEM_PERIOD) return &keys->period;
    
    if(key_code == VK_SPACE) return &keys->space;
    if(key_code == VK_BACK) return &keys->backspace;
    if(key_code == VK_DELETE) return &keys->del;
    if(key_code == VK_TAB) return &keys->tab;
    if(key_code == VK_RETURN) return &keys->enter;
    if(key_code == VK_CAPITAL) return &keys->caps_lock;
    if(key_code == VK_ESCAPE) return &keys->escape;
    
    if(key_code == VK_SHIFT) return &keys->shift;
    if(key_code == VK_CONTROL) return &keys->control;
    if(key_code == VK_MENU) return &keys->alt;
    
    if(key_code == VK_F1) return &keys->f1;
    if(key_code == VK_F2) return &keys->f2;
    if(key_code == VK_F3) return &keys->f3;
    if(key_code == VK_F4) return &keys->f4;
    if(key_code == VK_F5) return &keys->f5;
    if(key_code == VK_F6) return &keys->f6;
    if(key_code == VK_F7) return &keys->f7;
    if(key_code == VK_F8) return &keys->f8;
    if(key_code == VK_F9) return &keys->f9;
    if(key_code == VK_F10) return &keys->f10;
    if(key_code == VK_F11) return &keys->f11;
    if(key_code == VK_F12) return &keys->f12;
    if(key_code == VK_F13) return &keys->f13;
    if(key_code == VK_F14) return &keys->f14;
    if(key_code == VK_F15) return &keys->f15;
    if(key_code == VK_F16) return &keys->f16;
    if(key_code == VK_F17) return &keys->f17;
    if(key_code == VK_F18) return &keys->f18;
    if(key_code == VK_F19) return &keys->f19;
    if(key_code == VK_F20) return &keys->f20;
    if(key_code == VK_F21) return &keys->f21;
    if(key_code == VK_F22) return &keys->f22;
    if(key_code == VK_F23) return &keys->f23;
    if(key_code == VK_F24) return &keys->f24;
    
    return NULL;
}


Void
process_window_messages(Bool *game_running, Input *input)
{
    Bool peek_message_result;
    MSG message = {0};
    Keys *keys = &input->keys;
    
    while((peek_message_result = PeekMessage(&message, NULL, 0, 0, PM_REMOVE)) != 0)
    { 
        if(message.message == WM_KEYDOWN)
        {
            KeyData *key = get_key_data_for_key_code(input, message.wParam);
            if(key)
                key->is_down = true;
        }
        else if(message.message == WM_KEYUP)
        {
            KeyData *key = get_key_data_for_key_code(input, message.wParam);
            if(key)
                key->is_down = false;
        }
        else if(message.message == WM_QUIT)
        {
            // Later: tell the game when the window is being closed so it can ask
            // the user if they want to save
            *game_running = false;
        }
        else if(message.message == WM_LBUTTONDOWN)
        {
            keys->mouse_left.is_down = true;
        }
        else if(message.message == WM_LBUTTONUP)
        {
            keys->mouse_left.is_down = false;
        }
        else if(message.message == WM_MBUTTONDOWN)
        {
            keys->mouse_middle.is_down = true;
        }
        else if(message.message == WM_MBUTTONUP)
        {
            keys->mouse_middle.is_down = false;
        }
        else if(message.message == WM_RBUTTONDOWN)
        {
            keys->mouse_right.is_down = true;
        }
        else if(message.message == WM_RBUTTONUP)
        {
            keys->mouse_right.is_down = false;
        }
        else if(message.message == WM_MOUSEMOVE)
        {
            input->mouse_pos[0] = float(GET_X_LPARAM(message.lParam));
            input->mouse_pos[1] = float(GET_Y_LPARAM(message.lParam));
        }
        else if(message.message == WM_MOUSEWHEEL)
        {
            input->d_scroll += GET_WHEEL_DELTA_WPARAM(message.wParam);
        }
        
        DispatchMessage(&message);
    } 
}

LRESULT CALLBACK
window_message_callback(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if(message == WM_DESTROY)
    {
        // I feel sinful writing this
        
        Char name_buffer[100];
        GetWindowTextA(hWnd, name_buffer, 100);
        
        // Later: move this into a macro
        const Char *fake_window_name = FAKE_WINDOW_NAME;
        Bool names_equal = true;
        for(Int i = 0; i < 100; i++)
        {
            if(!name_buffer[i] && !fake_window_name[i])
                break;
            if(name_buffer[i] != fake_window_name[i])
            {
                names_equal = false;
                break;
            }
        }
        
        if(!names_equal)
            PostQuitMessage(0);
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}

ATOM
register_window_class(HINSTANCE hInstance)
{
    WNDCLASSEX wcex;
    ZeroMemory(&wcex, sizeof(wcex));
    wcex.cbSize = sizeof(wcex);
    wcex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wcex.lpfnWndProc = window_message_callback;
    wcex.hInstance = hInstance;
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.lpszClassName = "Core";
    return RegisterClassEx(&wcex);
}



Int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    // Open window and initialize OpenGL
    
    register_window_class(hInstance);
    
    HWND fakeWND = CreateWindow("Core", FAKE_WINDOW_NAME,      // window class, title
                                WS_CLIPSIBLINGS | WS_CLIPCHILDREN, // style
                                0, 0,           // position x, y
                                1, 1,           // width, height
                                NULL, NULL,     // parent window, menu
                                hInstance, NULL);       // instance, param
    HDC fakeDC = GetDC(fakeWND);
    
    
    PIXELFORMATDESCRIPTOR fakePFD;
    ZeroMemory(&fakePFD, sizeof(fakePFD));
    fakePFD.nSize = sizeof(fakePFD);
    fakePFD.nVersion = 1;
    fakePFD.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    fakePFD.iPixelType = PFD_TYPE_RGBA;
    fakePFD.cColorBits = 32;
    fakePFD.cAlphaBits = 8;
    fakePFD.cDepthBits = 24;
    
    int fakePFDID = ChoosePixelFormat(fakeDC, &fakePFD);
    if (fakePFDID == 0) {
        log_error("ChoosePixelFormat() failed.");
        return 1;
    }
    
    if (SetPixelFormat(fakeDC, fakePFDID, &fakePFD) == false) {
        log_error("SetPixelFormat() failed.");
        return 1;
    }
    
    HGLRC fakeRC = wglCreateContext(fakeDC);    // Rendering Contex
    if (fakeRC == 0) {
        log_error("wglCreateContext() failed.");
        return 1;
    }
    if (wglMakeCurrent(fakeDC, fakeRC) == false) {
        log_error("wglMakeCurrent() failed.");
        return 1;
    }
    
    PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = nullptr;
    wglChoosePixelFormatARB = reinterpret_cast<PFNWGLCHOOSEPIXELFORMATARBPROC>(wglGetProcAddress("wglChoosePixelFormatARB"));
    if (wglChoosePixelFormatARB == nullptr) {
        log_error("wglGetProcAddress() failed.");
        return 1;
    }
    PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = nullptr;
    wglCreateContextAttribsARB = reinterpret_cast<PFNWGLCREATECONTEXTATTRIBSARBPROC>(wglGetProcAddress("wglCreateContextAttribsARB"));
    if (wglCreateContextAttribsARB == nullptr) {
        log_error("wglGetProcAddress() failed.");
        return 1;
    }
    
    // Now create real window
    
    HWND WND = CreateWindow(
                            "Core", "OpenGL Window",       // class name, window name
                            WS_CAPTION | WS_SYSMENU | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, // style
                            0, 0,      // posx, posy
                            500, 300,   // width, height
                            NULL, NULL,                    // parent window, menu
                            hInstance, NULL);              // instance, param
    HDC DC = GetDC(WND);
    
    const int pixelAttribs[] = {
        WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
        WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
        WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
        WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
        WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
        WGL_COLOR_BITS_ARB, 32,
        WGL_ALPHA_BITS_ARB, 8,
        WGL_DEPTH_BITS_ARB, 24,
        WGL_STENCIL_BITS_ARB, 8,
        WGL_SAMPLE_BUFFERS_ARB, GL_TRUE,
        WGL_SAMPLES_ARB, 4,
        0
    };
    int pixelFormatID; UINT numFormats;
    bool status = wglChoosePixelFormatARB(DC, pixelAttribs, NULL, 1, &pixelFormatID, &numFormats);
    if (status == false || numFormats == 0) {
        log_error("wglChoosePixelFormatARB() failed.");
        return 1;
    }
    
    PIXELFORMATDESCRIPTOR PFD;
    DescribePixelFormat(DC, pixelFormatID, sizeof(PFD), &PFD);
    SetPixelFormat(DC, pixelFormatID, &PFD);
    
    
    const int major_min = 4, minor_min = 5;
    int  contextAttribs[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, major_min,
        WGL_CONTEXT_MINOR_VERSION_ARB, minor_min,
        WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0
    };
    HGLRC RC = wglCreateContextAttribsARB(DC, 0, contextAttribs);
    if (RC == NULL) {
        log_error("wglCreateContextAttribsARB() failed.");
        return 1;
    }
    
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(fakeRC);
    ReleaseDC(fakeWND, fakeDC);
    DestroyWindow(fakeWND);
    if (!wglMakeCurrent(DC, RC)) {
        log_error("wglMakeCurrent() failed.");
        return 1;
    }
    
    SetWindowText(WND, (LPCSTR)glGetString(GL_VERSION));
    ShowWindow(WND, SW_SHOWNORMAL);
    
    // End of window and OpenGL setup
    
    Log _global_log;
    global_log = &_global_log;
    
    GameCode game_code = {0};
    HINSTANCE module_handle = 0;
    
    FILETIME dll_last_write_time;
    if(get_game_dll_last_write_time(&dll_last_write_time))
    {
        print_error("couldn't open DLL file for time reading");
        ASSERT(false);
        return 1;
    }
    
    Int dll_load_result = load_game_dll(&game_code, &module_handle);
    if(dll_load_result == 1)
    {
        //print_error("issue with dll loading");
        //ASSERT(false);
        //return 1;
    }
    GameMemory game_memory = {0};
    game_memory.game_running = true;
    game_memory.functions_loaded = false;
    game_memory.size = megabytes(10);
    game_memory.memory = mem_alloc(game_memory.size);
    zero_memory(game_memory.memory, game_memory.size);
    game_memory.global_log = global_log;
    game_memory.allocated = true;
    
    
    //game_memory.window = window;
    game_memory.get_file_last_write_time = get_file_last_write_time;
    game_memory.read_file_contents = read_file_contents;
    game_memory.write_file_contents = write_file_contents;
    game_memory.get_time = get_time;
    game_memory.sleep = sleep;
    game_memory.d_time = 0.06f;
    
    XAudio2Data xaudio2_data = initialize_xaudio2(&game_memory.audio_buffer);
    
    while(game_memory.game_running)
    {
        {
            game_memory.input.d_scroll = 0;
            Float prev_mouse_pos[2];
            prev_mouse_pos[0] = game_memory.input.mouse_pos[0];
            prev_mouse_pos[1] = game_memory.input.mouse_pos[1];
            
            process_window_messages(&game_memory.game_running, &game_memory.input);
            
            game_memory.input.d_mouse_pos[0] = game_memory.input.mouse_pos[0] - prev_mouse_pos[0];
            game_memory.input.d_mouse_pos[1] = game_memory.input.mouse_pos[1] - prev_mouse_pos[1];
        }
        
        if(game_memory.input.keys.mouse_left.is_down)
            glClearColor(0.129f, 0.586f, 0.949f, 1.0f); // rgb(33,150,243)
        else
            glClearColor(0, 0, 0, 1.0f); // rgb(33,150,243)
        
        glClear(GL_COLOR_BUFFER_BIT);
        SwapBuffers(DC);
        
        update_input(&game_memory.input, game_memory.d_time);
        
#if 0
#ifndef TEMPEST_RELEASE
        FILETIME old_dll_last_write_time = dll_last_write_time;
        get_game_dll_last_write_time(&dll_last_write_time);
        if(CompareFileTime(&dll_last_write_time, &old_dll_last_write_time) != 0)
        {
            unload_game_dll(&game_code, &module_handle);
            
            // TODO: use a better system, like a lock file
            Sleep(500);
            
            Int dll_load_result = load_game_dll(&game_code, &module_handle);
            if(dll_load_result == 1)
            {
                print_error("issue with dll loading");
                return 1;
            }
            
            game_memory.functions_loaded = false;
        }
#endif
        F64 frame_start_time = get_time();
        
        //(game_code.update_and_render)(&game_memory);
        
        
        
        // Update xaudio2
        {
            XAUDIO2_VOICE_STATE xaudio2_voice_state = {0};
            xaudio2_data.source_voice->GetState(&xaudio2_voice_state, 0);
            AudioBuffer *audio_buffer = &game_memory.audio_buffer;
            audio_buffer->total_frames_played = xaudio2_voice_state.SamplesPlayed;
            audio_buffer->play_cursor_absolute = audio_buffer->total_frames_played;
        }
        
        F64 frame_end_time = get_time();
        game_memory.d_time = frame_end_time - frame_start_time;
        if(game_memory.d_time < 0)
            game_memory.d_time = 0.001f;
#endif
    }
    
    return 0;
}
