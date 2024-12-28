#include <windows.h>
#include <xaudio2.h>
//#include <dsound.h>
#include <stdio.h>
#include <math.h>

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


#include "base.h"
#include "log.h"
#include "game_loader.h"

#ifndef TEMPEST_RELEASE
#define GAME_DLL_PATH "game.dll"
#define GAME_DLL_COPY_PATH "game_temp_copy.dll"
#else
#define GAME_DLL_PATH "tempest.dll"
#endif


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

Void framebuffer_size_callback(GLFWwindow* window, Int width, Int height)
{
    glViewport(0, 0, width, height * width_over_height);
}

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


Int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
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
        print_error("issue with dll loading");
        ASSERT(false);
        return 1;
    }
    GameMemory game_memory = {0};
    game_memory.game_running = true;
    game_memory.functions_loaded = false;
    game_memory.size = megabytes(10);
    game_memory.memory = mem_alloc(game_memory.size);
    zero_memory(game_memory.memory, game_memory.size);
    game_memory.global_log = global_log;
    game_memory.allocated = true;
    
    
    // INITIALIZE WINDOW AND OPENGL
    glfwSetErrorCallback( error_callback );
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    GLFWwindow* window = glfwCreateWindow(1920, 1080, "Tempest", NULL, NULL);
    if (window == NULL)
    {
        print_error("failed to create GLFW window");
        glfwTerminate();
        ASSERT(false);
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    
    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        print_error("failed to initialize GLAD");
        ASSERT(false);
    }
    
    framebuffer_size_callback(window, 1920, 1080);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    
    
    game_memory.window = window;
    game_memory.get_file_last_write_time = get_file_last_write_time;
    game_memory.read_file_contents = read_file_contents;
    game_memory.write_file_contents = write_file_contents;
    game_memory.get_time = get_time;
    game_memory.sleep = sleep;
    game_memory.d_time = 0.06f;
    
    XAudio2Data xaudio2_data = initialize_xaudio2(&game_memory.audio_buffer);
    
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    
    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);          // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
    ImGui_ImplOpenGL3_Init();
    
    
    if(0)
    {
        int present = glfwJoystickPresent(GLFW_JOYSTICK_1);
        
        int count;
        //const float* axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &count);
    }
    
    while(game_memory.game_running)
    {
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
        
        if(glfwWindowShouldClose(window))
            break;
        
        glfwPollEvents();
        
#if 1
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::ShowDemoWindow();
        ImGui::Render();
#endif
        
        (game_code.update_and_render)(&game_memory);
        
#if 1
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#endif
        
        glfwSwapBuffers(window);
        
        
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
    }
    
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    
    glfwTerminate();
    return 0;
}
