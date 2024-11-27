#include "LevelCreationIMGUI.h"

#include "../MenuEnums.h"
#include <EWEngine/imgui/imgui.h>
#include "EWEngine/Graphics/Texture/Texture_Manager.h"

namespace EWE {
    LevelCreationIMGUI::LevelCreationIMGUI(std::queue<uint16_t>& clickReturns, float screenWidth, float screenHeight) : clickReturns{ clickReturns }, screenWidth{ screenWidth }, screenHeight{ screenHeight } {

    }

    void LevelCreationIMGUI::render() {

        ShowMainMenuBar();
        ShowGridControl();
        ShowTileSet();

        ShowSaveLevelPrompt();
        ShowLoadLevelPrompt();
        ShowNewPrompt();
    }

    void LevelCreationIMGUI::ShowMainMenuBar() {
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                ShowMenuFile();
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Edit")) {
                if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
                if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
                ImGui::Separator();
                if (ImGui::MenuItem("Cut", "CTRL+X")) {}
                if (ImGui::MenuItem("Copy", "CTRL+C")) {}
                if (ImGui::MenuItem("Paste", "CTRL+V")) {}
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("View")) {
                ImGui::MenuItem("Show Grid", "", &showGrid);
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
    }
    void LevelCreationIMGUI::ShowMenuFile() {
        //IMGUI_DEMO_MARKER("Examples/Menu");
        ImGui::MenuItem("(demo menu)", NULL, false, false);
        if (ImGui::MenuItem("New")) {
            printf("new file requested \n");
            showCreateLevelMenu = true;
        }
        if (ImGui::MenuItem("Open", "Ctrl+O")) {
            showLoadLevelMenu = true;
        }
        if (ImGui::BeginMenu("Open Recent")) {
            ImGui::MenuItem("fish_hat.c");
            ImGui::MenuItem("fish_hat.inl");
            ImGui::MenuItem("fish_hat.h");
            if (ImGui::BeginMenu("More.."))
            {
                ImGui::MenuItem("Hello");
                ImGui::MenuItem("Sailor");
                if (ImGui::BeginMenu("Recurse.."))
                {
                    ShowMenuFile();
                    ImGui::EndMenu();
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }
        if (ImGui::MenuItem("Save", "Ctrl+S")) {
		
        }
        if (ImGui::MenuItem("Save As..")) {
            showSaveLevelMenu = true;
        }

        ImGui::Separator();
        //IMGUI_DEMO_MARKER("Examples/Menu/Options");
        if (ImGui::BeginMenu("Options")) {
            static bool enabled = true;
            ImGui::MenuItem("Enabled", "", &enabled);
            ImGui::BeginChild("child", ImVec2(0, 60), ImGuiChildFlags_Border);
            for (int i = 0; i < 10; i++)
                ImGui::Text("Scrolling Text %d", i);
            ImGui::EndChild();
            static float f = 0.5f;
            static int n = 0;
            ImGui::SliderFloat("Value", &f, 0.0f, 1.0f);
            ImGui::InputFloat("Input", &f, 0.1f);
            ImGui::Combo("Combo", &n, "Yes\0No\0Maybe\0\0");
            ImGui::EndMenu();
        }

        //IMGUI_DEMO_MARKER("Examples/Menu/Colors");
        if (ImGui::BeginMenu("Colors")) {
            float sz = ImGui::GetTextLineHeight();
            for (int i = 0; i < ImGuiCol_COUNT; i++) {
                const char* name = ImGui::GetStyleColorName((ImGuiCol)i);
                ImVec2 p = ImGui::GetCursorScreenPos();
                ImGui::GetWindowDrawList()->AddRectFilled(p, ImVec2(p.x + sz, p.y + sz), ImGui::GetColorU32((ImGuiCol)i));
                ImGui::Dummy(ImVec2(sz, sz));
                ImGui::SameLine();
                ImGui::MenuItem(name);
            }
            ImGui::EndMenu();
        }

        // Here we demonstrate appending again to the "Options" menu (which we already created above)
        // Of course in this demo it is a little bit silly that this function calls BeginMenu("Options") twice.
        // In a real code-base using it would make senses to use this feature from very different code locations.
        if (ImGui::BeginMenu("Options")) {// <-- Append!
            //IMGUI_DEMO_MARKER("Examples/Menu/Append to an existing menu");
            static bool b = true;
            ImGui::Checkbox("SomeOption", &b);
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Disabled", false)) {// Disabled
            IM_ASSERT(0);
        }
        if (ImGui::MenuItem("Checked", NULL, true)) {}
        ImGui::Separator();
        if (ImGui::MenuItem("Quit", "Alt+F4")) {

            clickReturns.push(MCR_swapToMainMenu);
        }
    }

    void LevelCreationIMGUI::ShowGridControl() {
        if (!ImGui::Begin("Grid Control", &gridControlOpen, 0)) {
            // Early out if the window is collapsed, as an optimization.
            ImGui::End();
            return;
        }

        ImGui::SliderScalar("Push X Zoom", ImGuiDataType_Float, &gridZoom->x, &scaleLow, &scaleHigh);
        ImGui::SliderScalar("Push Y Zoom", ImGuiDataType_Float, &gridZoom->y, &scaleLow, &scaleHigh);
        ImGui::SliderScalar("Push X Trans", ImGuiDataType_Float, &gridTrans->x, &transLow, &transHigh);
        ImGui::SliderScalar("Push Y Trans", ImGuiDataType_Float, &gridTrans->y, &transLow, &transHigh);
        if (ImGui::SliderScalar("Push X Grid Scale", ImGuiDataType_Float, &gridScale->x, &scaleLow, &scaleHigh)) {

        }
        ImGui::SliderScalar("Push Y Grid Scale", ImGuiDataType_Float, &gridScale->y, &scaleLow, &scaleHigh);



        //printf("tileSetID : %d \n", tileSetID);
        ImGui::End();
    }
    void LevelCreationIMGUI::ShowTileSet() {
        if (!ImGui::Begin("TileSet Control", &gridControlOpen, 0)) {
            // Early out if the window is collapsed, as an optimization.
            ImGui::End();
            return;
        }
        ImGui::SliderScalar("tileSet scaling", ImGuiDataType_Float, &tileSetScale, &tileSetScaleLow, &tileSetScaleHigh);
        ImGui::SliderScalar("tileSet ratio", ImGuiDataType_Float, &tileSetRatio, &scaleLow, &scaleHigh);

        ImGui::Text("Selected Tile %u", selectedTile);
        ImGui::Image(tileSetID, ImVec2(64, 64), selectedTileUVTL, selectedTileUVBR);

        ImGui::SameLine();
        ShowToolControls();

        ImGuiChildFlags child_flags = ImGuiChildFlags_Border | ImGuiChildFlags_ResizeX | ImGuiChildFlags_ResizeY;
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_HorizontalScrollbar;
        ImGui::BeginChild("Child FULL", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, 260), child_flags, window_flags);
        float texW = tileSetScale * tileUVScaling.y / tileUVScaling.x;
        float texH = tileSetScale;

        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImGuiIO& io = ImGui::GetIO();
            ImGui::Image(tileSetID, ImVec2(texW, texH));
            if (ImGui::BeginItemTooltip()) {
                hoveringTileSet = true;

                float region_sz = 32.0f;
                float region_x = io.MousePos.x - pos.x;
                float region_y = io.MousePos.y - pos.y;
                ImGui::Text("mouse coords : (%.2f:%.2f)", region_x, region_y);
                ImGui::Text("hovered Tile : (%.1f:%.1f)", std::floor(region_x / texW * 64.f), std::floor(region_y / texH * 19.f));
                //region_y = region_y - glm::mod(region_y, 32.f);
                //region_x = region_x - glm::mod(region_x, 32.f);

                toolSelectedTile = static_cast<TileID>(std::floor(region_x / texW * 64.f) + std::floor(region_y / texH * 19.f) * 64.f);

                toolUV.x = std::floor(region_x / texW * 64.f) / 64.f;
                toolUV.y = std::floor(region_y / texH * 19.f) / 19.f;
                toolUVBR.x = toolUV.x + tileUVScaling.x;
                toolUVBR.y = toolUV.y + tileUVScaling.y;
                ImGui::Text("toolUV : (%.5f:%.5f)", toolUV.x, toolUV.y);

                float zoom = 4.0f;
                if (region_x < 0.0f) { region_x = 0.0f; }
                else if (region_x > texW - region_sz) { region_x = texW - region_sz; }
                if (region_y < 0.0f) { region_y = 0.0f; }
                else if (region_y > texH - region_sz) { region_y = texH - region_sz; }
                ImGui::Text("Selected Tile tool: (%d)", toolSelectedTile);
                ImVec2 uv0 = ImVec2((region_x) / texW, (region_y) / texH);
                ImVec2 uv1 = ImVec2((region_x + region_sz) / texW, (region_y + region_sz) / texH);
                ImGui::Image(tileSetID, ImVec2(64.f, 64.f), toolUV, toolUVBR);
                ImGui::EndTooltip();
            }
            else {
                hoveringTileSet = false;
            }
            ImGui::EndChild();
            ImGui::End();
    }
    void LevelCreationIMGUI::mouseCallback(int button, int action) {
        //printf("level creation imgui getting the clalback \n");
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
            if (hoveringTileSet) {
                selectedTile = toolSelectedTile;
                selectedTileUVTL = toolUV;
                selectedTileUVBR = toolUVBR;
            }
        }
    }
    void LevelCreationIMGUI::scrollCallback(double yOffset) {
        if (glm::abs(yOffset) > 0.001) {
            if (hoveringTileSet) {
                tileSetScale *= 1.f + 0.1f * static_cast<float>(yOffset);
            }
        }
    }

    void LevelCreationIMGUI::ShowNewPrompt() {
        if (showCreateLevelMenu) {
            ImGuiWindowFlags flags = ImGuiWindowFlags_MenuBar;
            if (!ImGui::Begin("New Level", &showCreateLevelMenu, 0)) {
                ImGui::End();
                return;
            }
            ImGui::InputInt("Level Width", &widthBuffer, 1, 10);
            if (widthBuffer > UINT16_MAX) {
                widthBuffer = UINT16_MAX;
            }
            else if (widthBuffer < 0) {
                widthBuffer = 0;
            }

            ImGui::InputInt("Level Height", &heightBuffer, 1, 10);
            if (heightBuffer > UINT16_MAX){
                heightBuffer = UINT16_MAX;
			}
            else if (heightBuffer < 0){
                heightBuffer = 0;
			}

            if (ImGui::Button("Create Level", ImVec2(100, 50))) {
                showCreateLevelMenu = false;
                
                gridScale->x = static_cast<float>(widthBuffer);
                gridScale->y = static_cast<float>(heightBuffer);

                createButtonPtr(static_cast<uint16_t>(widthBuffer), static_cast<uint16_t>(heightBuffer));
            }

            ImGui::End();
        }
    }
    void LevelCreationIMGUI::ShowSaveLevelPrompt() {
        if (showSaveLevelMenu) {
            if (!ImGui::Begin("Save Level", &showSaveLevelMenu, 0)) {
                ImGui::End();
                return;
            }
            ImGui::InputText("Save Location", saveLocation, 128);

            if (ImGui::Button("Save File", ImVec2(100, 50))) {
                showSaveLevelMenu = false;

                tileMapD->saveMap(saveLocation);
            }

            ImGui::End();
        }
    }
    void LevelCreationIMGUI::ShowLoadLevelPrompt() {
        if (showLoadLevelMenu) {

            if (!ImGui::Begin("Load Level", &showLoadLevelMenu, 0)) {
                ImGui::End();
                return;
            }
            ImGui::InputText("Load Location", loadLocation, 128);

            if (ImGui::Button("Load File", ImVec2(100, 50))) {
                showLoadLevelMenu = false;

                tileMapD->loadMap(loadLocation);
            }

            ImGui::End();
        }
    }

    void LevelCreationIMGUI::ShowToolControls() {
        //ImGui::Image(*EWETexture::getDescriptorSets(tileSetID, 0), ImVec2(64.f, 64.f), toolUV, toolUVBR);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(1.0f, 1.0f));
        for (uint16_t i = 0; i < tools.size(); i++) {
            ImGui::PushID(i);

            ImVec2 size{ 32.f, 32.f };
            ImVec2 uv0{ 0.f, 0.f };
            ImVec2 uv1{ 1.f,1.f };
            ImVec4 tint_col{ 1.0f, 1.0f, 1.0f, 1.0f };

            

            if (ImGui::ImageButton("", tools[i].texture, size, uv0, uv1, tools[i].bgColor, tint_col)) {
                tools[i].bgColor = selectedColor;
                for (int j = 0; j < tools.size(); j++) {
                    if (i == j) {
                        continue;
                    }
                    tools[j].bgColor = idleColor;
                }
                selectedTool = (Tool_Enum)i;
                printf("selecting tool : %d \n", i);
            }

            ImGui::PopID();
            ImGui::SameLine();
        }
        ImGui::PopStyleVar();
        ImGui::NewLine();
    }

    void LevelCreationIMGUI::loadTextures(EWEDevice& device) {

        tileSetID = Texture_Builder::createSimpleTexture( "tileSet.png", false, false, VK_SHADER_STAGE_FRAGMENT_BIT);


        tools[Tool_pencil].texture = Texture_Builder::createSimpleTexture( "tileCreation/pencil.png", false, false, VK_SHADER_STAGE_FRAGMENT_BIT);
        tools[Tool_eraser].texture = Texture_Builder::createSimpleTexture( "tileCreation/eraser.png", false, false, VK_SHADER_STAGE_FRAGMENT_BIT);
        tools[Tool_colorSelection].texture = Texture_Builder::createSimpleTexture( "tileCreation/colorSelection.png", false, false, VK_SHADER_STAGE_FRAGMENT_BIT);
        tools[Tool_bucketFill].texture = Texture_Builder::createSimpleTexture( "tileCreation/bucketFill.png", false, false, VK_SHADER_STAGE_FRAGMENT_BIT);
    }

    void LevelCreationIMGUI::toolLeft(uint32_t clickedTilePosition, bool shiftKey, bool ctrlKey) {
        switch (selectedTool) {
        case LevelCreationIMGUI::Tool_pencil: {
            //if(tileMapD->)
            tileMapD->changeTile(clickedTilePosition, selectedTile);
            break;
        }
        case LevelCreationIMGUI::Tool_eraser: {
            tileMapD->removeTile(clickedTilePosition);
            break;
        }
        case LevelCreationIMGUI::Tool_colorSelection: {
            if (!shiftKey) {
                tileMapD->clearSelection();
            }
            tileMapD->colorSelection(clickedTilePosition);
            break;
        }
        case LevelCreationIMGUI::Tool_bucketFill: {
            printf("attempting bucket fill : %u \n", selectedTile);
            tileMapD->bucketFill(clickedTilePosition, selectedTile);
            break;
        }
        default: {
            printf("Default tool? : %d \n", selectedTool);
            break;
        }

        }
    }
}