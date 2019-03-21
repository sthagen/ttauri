
#include "Application_win32.hpp"

#include "GUI/Instance.hpp"
#include "GUI/Window_vulkan_win32.hpp"

#include <vulkan/vulkan.hpp>

#include <thread>

namespace TTauri {

Application_win32::Application_win32(const std::shared_ptr<Delegate> &delegate, HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) :
    Application(delegate),
    hInstance(hInstance),
    hPrevInstance(hPrevInstance),
    pCmdLine(pCmdLine),
    nCmdShow(nCmdShow)
{

    // Resource path, is the same directory as where the executable lives.
    wchar_t modulePathWChar[MAX_PATH];
    if (GetModuleFileNameW(nullptr, modulePathWChar, MAX_PATH) == 0) {
        BOOST_THROW_EXCEPTION(Application::ResourceDirError());
    }

    auto const modulePath = boost::filesystem::path(modulePathWChar);

    resourceDir = modulePath.parent_path();
}

int Application_win32::loop()
{
    startingLoop();

    // Run the message loop.
    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
}