#include "ResourceList.hpp"

#include <Engine/Geometry/RiggedModel.hpp>
#include <Engine/Geometry/StaticModel.hpp>
#include <Engine/Texture/Texture2D.hpp>
#include <Engine/Audio/SoundBuffer.hpp>

#include <Engine/Hymn.hpp>
#include <Engine/Entity/Entity.hpp>
#include <Engine/MainWindow.hpp>
#include <Engine/Util/FileSystem.hpp>
#include <imgui.h>

using namespace GUI;

void ResourceList::Show() {
    ImGui::Begin("Resources");
    
    // Scenes.
    if (ImGui::TreeNode("Scenes")) {
        if (ImGui::Button("Add scene"))
            Hymn().scenes.push_back("Scene #" + std::to_string(Hymn().scenes.size()));
        
        for (std::size_t i = 0; i < Hymn().scenes.size(); ++i) {
            if (ImGui::Selectable(Hymn().scenes[i].c_str())) {
                sceneEditor.Save();
                sceneEditor.SetVisible(true);
                sceneEditor.SetScene(i);
                std::string sceneFile = Hymn().GetPath() + FileSystem::DELIMITER + "Scenes" + FileSystem::DELIMITER + Hymn().scenes[i] + ".json";
                if (FileSystem::FileExists(sceneFile.c_str()))
                    Hymn().world.Load(sceneFile);
            }
            
            if (ImGui::BeginPopupContextItem(Hymn().scenes[i].c_str())) {
                if (ImGui::Selectable("Delete")) {
                    Hymn().scenes.erase(Hymn().scenes.begin() + i);
                    ImGui::EndPopup();
                    break;
                }
                ImGui::EndPopup();
            }
        }
        ImGui::TreePop();
    }

    // Models.
    bool modelPressed = false;
    if (ImGui::TreeNode("Models")) {
        if (ImGui::Button("Add rigged model")) {
            Geometry::Model* model = new Geometry::RiggedModel();
            model->name = "RiggedModel #" + std::to_string(Hymn().modelNumber++);
            Hymn().models.push_back(model);
        }
        if (ImGui::Button("Add static model")) {
            Geometry::Model* model = new Geometry::StaticModel();
            model->name = "StaticModel #" + std::to_string(Hymn().modelNumber++);
            Hymn().models.push_back(model);
        }
        
        for (auto it = Hymn().models.begin(); it != Hymn().models.end(); ++it) {
            Geometry::Model* model = *it;
            if (ImGui::Selectable(model->name.c_str())) {
                modelPressed = true;
                modelEditor.SetModel(model);
            }
            
            if (ImGui::BeginPopupContextItem(model->name.c_str())) {
                if (ImGui::Selectable("Delete")) {
                    delete model;
                    Hymn().models.erase(it);
                    ImGui::EndPopup();
                    break;
                }
                ImGui::EndPopup();
            }
        }
        ImGui::TreePop();
    }
        
    // Textures.
    bool texturePressed = false;
    if (ImGui::TreeNode("Textures")) {
        if (ImGui::Button("Add texture")) {
            Texture2D* texture = new Texture2D();
            texture->name = "Texture #" + std::to_string(Hymn().textureNumber++);
            Hymn().textures.push_back(texture);
        }
        
        for (auto it = Hymn().textures.begin(); it != Hymn().textures.end(); ++it) {
            Texture2D* texture = *it;
            if (ImGui::Selectable(texture->name.c_str())) {
                texturePressed = true;
                textureEditor.SetTexture(texture);
            }
            
            if (ImGui::BeginPopupContextItem(texture->name.c_str())) {
                if (ImGui::Selectable("Delete")) {
                    delete texture;
                    Hymn().textures.erase(it);
                    ImGui::EndPopup();
                    break;
                }
                ImGui::EndPopup();
            }
        }
        
        ImGui::TreePop();
    }
    
    // Sounds.
    bool soundPressed = false;
    if (ImGui::TreeNode("Sounds")) {
        if (ImGui::Button("Add sound")) {
            Audio::SoundBuffer* sound = new Audio::SoundBuffer();
            sound->name = "Sound #" + std::to_string(Hymn().soundNumber++);
            Hymn().sounds.push_back(sound);
        }
        
        for (auto it = Hymn().sounds.begin(); it != Hymn().sounds.end(); ++it) {
            Audio::SoundBuffer* sound = *it;
            if (ImGui::Selectable(sound->name.c_str())) {
                soundPressed = true;
                soundEditor.SetSound(sound);
            }
            
            if (ImGui::BeginPopupContextItem(sound->name.c_str())) {
                if (ImGui::Selectable("Delete")) {
                    delete sound;
                    Hymn().sounds.erase(it);
                    ImGui::EndPopup();
                    break;
                }
                ImGui::EndPopup();
            }
        }
        
        ImGui::TreePop();
    }


    if (sceneEditor.entityPressed || texturePressed || modelPressed || soundPressed) {

		sceneEditor.entityEditor.SetVisible(sceneEditor.entityPressed);
        textureEditor.SetVisible(texturePressed);
        modelEditor.SetVisible(modelPressed);
        soundEditor.SetVisible(soundPressed);

    }
	ImVec2 size(MainWindow::GetInstance()->GetSize().x, MainWindow::GetInstance()->GetSize().y);

	if (sceneEditor.IsVisible()) {
		
		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(250, size.y - 250));
		sceneEditor.Show();

	}

 	ImGui::SetNextWindowPos(ImVec2(size.x - 250, 0));
	ImGui::SetNextWindowSize(ImVec2(250, size.y));
   
	if (sceneEditor.entityEditor.IsVisible())
		sceneEditor.entityEditor.Show();
	if (textureEditor.IsVisible())
        textureEditor.Show();
    if (modelEditor.IsVisible())
        modelEditor.Show();
    if (soundEditor.IsVisible())
        soundEditor.Show();

    ImGui::End();
}

bool ResourceList::IsVisible() const {
    return visible;
}

void ResourceList::SetVisible(bool visible) {
    this->visible = visible;
}

void ResourceList::HideEditors() {
	
	sceneEditor.SetVisible(false);
	modelEditor.SetVisible(false);
    textureEditor.SetVisible(false);
    soundEditor.SetVisible(false);

}

void ResourceList::SaveScene() const {
    sceneEditor.Save();
}

void ResourceList::ResetScene() {
    sceneEditor.SetScene(0);
    sceneEditor.SetVisible(false);
}
