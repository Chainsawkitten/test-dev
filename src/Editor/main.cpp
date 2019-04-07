#include <Engine/Engine.hpp>
#include <Engine/MainWindow.hpp>
#include "Editor.hpp"
#include "Util/EditorSettings.hpp"
#include <Engine/Util/Input.hpp>
#include <Engine/Util/FileSystem.hpp>
#include <Utility/Log.hpp>
#include <Engine/Input/Input.hpp>
#include <Engine/Manager/Managers.hpp>
#include <Engine/Manager/ProfilingManager.hpp>
#include <Engine/Manager/RenderManager.hpp>
#include <Engine/Util/Profiling.hpp>
#include <Engine/Util/GPUProfiling.hpp>
#include <Engine/Hymn.hpp>
#include "ImGui/OpenGLImplementation.hpp"
#include <imgui.h>
#include "GUI/ProfilingWindow.hpp"
#include <iostream>

int main() {
    Log().SetupStreams(&std::cout, &std::cout, &std::cout, &std::cerr);

    // Enable logging if requested.
    if (EditorSettings::GetInstance().GetBool("Logging")) {
        FILE* file = freopen(FileSystem::DataPath("Hymn to Beauty", "log.txt").c_str(), "a", stderr);
        if (file == nullptr)
            Log() << "Could not open logging file!\n";
    }

    Log() << "Editor started - " << time(nullptr) << "\n";

    // Configure engine.
    Engine engine;
    engine.configuration.width = EditorSettings::GetInstance().GetLong("Width");
    engine.configuration.height = EditorSettings::GetInstance().GetLong("Height");
    engine.configuration.fullscreen = false;
    engine.configuration.borderless = false;
    engine.configuration.debug = EditorSettings::GetInstance().GetBool("Debug Context");

    // Start engine.
    if (!engine.Start())
        return -1;

    Editor* editor = new Editor();
    // Setup imgui implementation.
    MainWindow* window = engine.GetMainWindow();
    ImGuiImplementation::Init(window->GetGLFWWindow());

    bool profiling = false;
    GUI::ProfilingWindow profilingWindow;

    // Main loop.
    while (!engine.ShouldClose() || !editor->ReadyToClose()) {
        Managers().profilingManager->SetActive(profiling);

        // Begin new profiling frame.
        if (Managers().profilingManager->Active())
            Managers().profilingManager->BeginFrame();

        {
            PROFILE("Frame");
            GPUPROFILE("Frame", Video::Query::Type::TIME_ELAPSED);

            if (Input()->Triggered(InputHandler::PROFILE))
                profiling = !profiling;

            // Start new frame.
            ImGuiImplementation::NewFrame();

            // Update engine.
            engine.configuration.paused = editor->IsVisible();
            engine.Update();

            if (editor->IsVisible()) {
                Hymn().Render(editor->GetCamera(), EditorSettings::GetInstance().GetBool("Sound Source Icons"), EditorSettings::GetInstance().GetBool("Particle Emitter Icons"), EditorSettings::GetInstance().GetBool("Light Source Icons"), EditorSettings::GetInstance().GetBool("Camera Icons"), EditorSettings::GetInstance().GetBool("Physics Volumes"), EditorSettings::GetInstance().GetBool("Lighting"), EditorSettings::GetInstance().GetBool("Light Volumes"));

                if (window->ShouldClose())
                    editor->Close();

                editor->Show(engine.GetDeltaTime());

                if (window->ShouldClose() && !editor->isClosing())
                    window->CancelClose();
            } else {
                engine.Render();

                if (Input()->Triggered(InputHandler::PLAYTEST)) {
                    // Rollback to the editor state.
                    editor->LoadSceneState();

                    // Turn editor back on.
                    editor->SetVisible(true);
                }
            }
        }

        if (Managers().profilingManager->Active()) {
            Managers().profilingManager->EndFrame();
            profilingWindow.Show();
        }

        ImGui::Render();

        engine.Present();
    }

    // Save editor settings.
    EditorSettings::GetInstance().Save();

    // Shut down and cleanup.
    ImGuiImplementation::Shutdown();
    delete editor;
    engine.Shutdown();

    Log() << "Editor ended - " << time(nullptr) << "\n";

    return 0;
}
