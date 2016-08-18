#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <Engine/MainWindow.hpp>
#include "Editor.hpp"
#include "Util/EditorSettings.hpp"
#include <Engine/Util/FileSystem.hpp>
#include <Engine/Util/Log.hpp>
#include <Engine/Manager/Managers.hpp>

int main() {
    // Enable logging if requested.
    if (EditorSettings::GetInstance().GetBool("Logging"))
        freopen(FileSystem::DataPath("Hymn to Beauty", "log.txt").c_str(), "a", stderr);
    
    Log() << "Editor started - " << time(nullptr) << "\n";
    
    if (!glfwInit())
        return 1;
    
    MainWindow* window = new MainWindow(EditorSettings::GetInstance().GetLong("Width"), EditorSettings::GetInstance().GetLong("Height"), false, false, "Hymn to Beauty", EditorSettings::GetInstance().GetBool("Debug Context"));
    glewInit();
    window->Init(false);
    
    Managers().StartUp(window->GetSize());
    
    Editor* editor = new Editor();
    
    while (!window->ShouldClose()) {
        window->Update();
        editor->Update();
        editor->Render();
        glfwPollEvents();
    }
    
    editor->Save();
    delete editor;
    
    Managers().ShutDown();
    
    delete window;
    
    glfwTerminate();
    
    EditorSettings::GetInstance().Save();
    
    Log() << "Editor ended - " << time(nullptr) << "\n";
    
    return 0;
}