#include "FrameCore.h"

// ImGui::Combo/ImGui::List
const char* BoxTypeList[] = { "Simple", "Cornered" };
const char* AimTypeList[] = { "FOV", "Distance" };
const char* AimBoneList[] = { "Head", "Chest" };
const char* AimKeyTypeList[] = { "and", "or" };
const char* CrosshairList[] = { "Cross", "Circle" };
std::vector<std::string> MenuTabs = { "Aimbot", "Visuals", "Misc", "Players", "System" };

void CFramework::RenderMenu()
{
    static int Index = 0;
    ImVec4* colors = ImGui::GetStyle().Colors;
    ImGuiStyle& style = ImGui::GetStyle();
    static float DefaultSpacing = style.ItemSpacing.y;
    static int BindingID = 0;

    ImGui::SetNextWindowBgAlpha(0.975f);
    ImGui::SetNextWindowSize(ImVec2(725.f, 450.f));
    ImGui::Begin("FiveM External Base", &g.ShowMenu,
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);

    // ==== Linkes Panel ====
    ImGui::BeginChild("##LeftPanel", ImVec2(150.f, ImGui::GetContentRegionAvail().y), true);

    // Text mittig setzen
    const char* title = "FiveM External Base";
    float textWidth = ImGui::CalcTextSize(title).x;
    float panelWidth = ImGui::GetWindowSize().x;

    ImGui::SetCursorPosX((panelWidth - textWidth) * 0.5f);
    ImGui::Text(title);

    ImGui::Separator();
    ImGui::Spacing();

    // Tabs rendern
    for (int i = 0; i < MenuTabs.size(); i++)
    {
        if (ImGui::CustomButton(MenuTabs[i].c_str(),
            ImVec2(ImGui::GetContentRegionAvail().x, 30.f), Index == i))
            Index = i;
    }

    // Exit Button unten
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 8));
    ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 40.0f);

    if (ImGui::Button("Exit Cheat",
        ImVec2(ImGui::GetContentRegionAvail().x, 30.f)))
        g.Run = false;

    ImGui::PopStyleVar();
    ImGui::EndChild();

    ImGui::SameLine();

    //  Content Panel 
    ImGui::BeginChild("##ContentPanel", ImVec2(ImGui::GetContentRegionAvail()), true);

    ImGui::Spacing();
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.9f, 1.f, 1.f));
    ImGui::Text(("-> " + MenuTabs[Index]).c_str());
    ImGui::PopStyleColor();
    ImGui::Separator();
    ImGui::Spacing();

    switch (Index)
    {
    case 0: // Aimbot
    {
        if (ImGui::BeginTabBar("##AimbotTabs", ImGuiTabBarFlags_NoCloseWithMiddleMouseButton))
        {
            //  MAIN OPTIONS 
            if (ImGui::BeginTabItem("Main"))
            {
                ImGui::Text("Main Aimbot Settings");  
                ImGui::Checkbox("AimBot", &g.AimBot); // Aimbot Is Broken in Vechiles
				ImGui::Checkbox("Prediction", &g.Aim_Prediction); // Idk if you want this but you can delete it if you dont

                ImGui::Spacing();
                ImGui::NewLine();

                ImGui::Text("Aim Options");
                ImGui::Separator();
                ImGui::Spacing();

                ImGui::Checkbox("Aim at NPC", &g.Aim_NPC);
                ImGui::Combo("AimBone", &g.Aim_Bone, AimBoneList, IM_ARRAYSIZE(AimBoneList));
                ImGui::EndTabItem();
            }

            //  ADVANCED SETTINGS 
            if (ImGui::BeginTabItem("Settings"))
            {
                ImGui::Text("Advanced Settings");
                ImGui::Separator();
                ImGui::Spacing();

                style.ItemSpacing.y = 3.f;
                ImGui::CustomSliderFloat("Smooth", "##aim_smt", &g.Aim_Smooth, 1.f, 20.f);
                style.ItemSpacing.y = DefaultSpacing;
                ImGui::CustomSliderFloat("Distance", "##aim_dist", &g.Aim_MaxDistance, 50.f, 300.f);

                ImGui::Spacing();
                ImGui::Spacing();
                ImGui::Combo("Aim Type", &g.Aim_Type, AimTypeList, IM_ARRAYSIZE(AimTypeList));

                ImGui::EndTabItem();
            }

            //  KEYBINDS 
            if (ImGui::BeginTabItem("Keybinds"))
            {
                ImGui::Text("KeyBind");
                ImGui::Separator();
                ImGui::Spacing();

                ImGui::Text("1st Key:");
                if (ImGui::Button(BindingID == 1 ? "< Press Any Key >" : KeyNames[g.AimKey0], ImVec2(215.f, 22.5f))) {
                    BindingID = 1;
                    std::thread([&]() {KeyBinder(g.AimKey0, BindingID); }).detach();
                }

                ImGui::PushItemWidth(215.f);
                ImGui::Combo("##KeyMode", &g.AimKeyType, AimKeyTypeList, IM_ARRAYSIZE(AimKeyTypeList));
                ImGui::PopItemWidth();
                ImGui::Spacing();

                ImGui::Text("2nd Key:");
                if (ImGui::Button(BindingID == 2 ? "< Press Any Key >" : KeyNames[g.AimKey1], ImVec2(215.f, 22.5f))) {
                    BindingID = 2;
                    std::thread([&]() {KeyBinder(g.AimKey1, BindingID); }).detach();
                }

                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }
    }
    break;

    case 1: // Visuals
    {
        if (ImGui::BeginTabBar("##VisualTabs", ImGuiTabBarFlags_NoCloseWithMiddleMouseButton))
        {
            //  ESP OPTIONS
            if (ImGui::BeginTabItem("Options"))
            {
                ImGui::Checkbox("ESP", &g.ESP);
                ImGui::Checkbox("NPC ESP", &g.ESP_NPC);
                ImGui::NewLine();
                ImGui::Text("ESP Options");
                ImGui::Separator();
                ImGui::Spacing();
                ImGui::Checkbox("Box", &g.ESP_Box);
                ImGui::Checkbox("BoxFilled", &g.ESP_BoxFilled);
                ImGui::Checkbox("Line", &g.ESP_Line);
                ImGui::Checkbox("Name", &g.ESP_Name);
                ImGui::Checkbox("Skeleton", &g.ESP_Skeleton);
                ImGui::Checkbox("Distance", &g.ESP_Distance);
                ImGui::Checkbox("HealthBar", &g.ESP_HealthBar);
                ImGui::CustomSliderFloat("Distance", "##Dist", &g.ESP_MaxDistance, 100.f, 2000.f);
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Colors"))
            {
                ImGui::Text("ESP Colors");
                ImGui::Separator();
                ImGui::Spacing();

                ImGui::BulletText("Box Colors");
                ImGui::ColorEdit4("NPC Box", &ESP_NPC.Value.x);
                ImGui::ColorEdit4("Player Box", &ESP_PLAYER.Value.x);
                ImGui::ColorEdit4("GodMode Box", &ESP_GOD.Value.x);

                ImGui::Spacing();
                ImGui::BulletText("Other");
                ImGui::ColorEdit4("BoxFilled", &ESP_Filled.Value.x);

                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }
    }
    break;

    case 2: // Misc
    {
        if (ImGui::BeginTabBar("##MiscTabs", ImGuiTabBarFlags_NoCloseWithMiddleMouseButton | ImGuiTabBarFlags_FittingPolicyResizeDown))
        {
            //  Player 
            if (ImGui::BeginTabItem("Player"))
            {
                float currentHealth = m.Read<float>(local.address + sdk.m_fHealth);
                float currentArmor = m.Read<float>(local.address + sdk.m_fArmor);

                // Health
                ImGui::Text("Health");
                ImGui::SameLine(100);
                ImGui::TextColored(ImVec4(0, 1, 0, 1), "%.0f/200", currentHealth);
                ImGui::SameLine(200);
                if (ImGui::SmallButton("Heal"))
                {
                    uintptr_t healthAddress = local.address + sdk.m_fHealth;
                    m.Write<float>(healthAddress, 200.0f);
                }

                // Armor
                ImGui::Text("Armor");
                ImGui::SameLine(100);
                ImGui::TextColored(ImVec4(0, 0.5f, 1, 1), "%.0f/100", currentArmor);
                ImGui::SameLine(200);
                if (ImGui::SmallButton("Fill"))
                {
                    uintptr_t armorAddress = local.address + sdk.m_fArmor;
                    m.Write<float>(armorAddress, 100.0f);
                }

                ImGui::Separator();
                ImGui::Checkbox("God Mode", &g.GodMode);
                ImGui::EndTabItem();
            }

            //  Weapon
            if (ImGui::BeginTabItem("Weapon"))
            {
                ImGui::TextColored(ImVec4(1, 1, 0, 1), "General");
                ImGui::Separator();
                ImGui::Checkbox("No Recoil", &g.NoRecoil);
                ImGui::Checkbox("No Spread", &g.NoSpread);
                ImGui::EndTabItem();
            }

            //  Vehicle 
            if (ImGui::BeginTabItem("Vehicle"))
            {
                ImGui::Text("Vehicle");
                ImGui::Separator();
                ImGui::Spacing();
                // Vehicle stuff 
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }
    }
    break;

    case 3: // Players
    {
        static char TargetName[64];
        ImGui::InputText("SearchPlayer", TargetName, IM_ARRAYSIZE(TargetName));

        if (ImGui::BeginTable("PlayerListTable", 5, ImGuiTableFlags_Borders))
        {
            ImGui::TableSetupColumn("Name");
            ImGui::TableSetupColumn("Alive");
            ImGui::TableSetupColumn("Vehicle");
            ImGui::TableSetupColumn("Position");
            ImGui::TableSetupColumn("Teleport");
            ImGui::TableHeadersRow();

            for (auto& ped : EntityList)
            {
                if (!ped.IsPlayer())
                    continue;

                char pN[32]{};
                m.ReadString(ped.PlayerInfo + 0xFC, pN, sizeof(pN));
                std::string pName = pN;

                if (TargetName[0] != '\0' && pName.find(TargetName) == std::string::npos)
                    continue;

                ImGui::TableNextColumn();
                ImGui::Text(pName.c_str());
                ImGui::TableNextColumn();
                ImGui::TextColored(!ped.IsDead() ? ImColor(0.f, 1.f, 0.f, 1.f) : ImColor(1.f, 0.f, 0.f, 1.f), !ped.IsDead() ? "Yes" : "No");
                ImGui::TableNextColumn();
                ImGui::TextColored(ped.InVehicle() ? ImColor(0.f, 1.f, 0.f, 1.f) : ImColor(1.f, 0.f, 0.f, 1.f), ped.InVehicle() ? "Yes" : "No");
                ImGui::TableNextColumn();
                ImGui::Text("%.f, %.f, %.f", ped.m_pVecLocation.x, ped.m_pVecLocation.y, ped.m_pVecLocation.z);
                ImGui::TableNextColumn();
                std::string button_string = "Teleport##" + pName;

                // Teleport
                // if (ImGui::Button(button_string.c_str()) && !ped.IsDead() && ped.m_pVecLocation.z > -30)
                //     local.set_position(ped.BoneList[HEAD] + Vector3(0.f, 0.f, 0.5f));
            }
            ImGui::EndTable();
        }
    }
    break;

    case 4: // System
    {
        if (ImGui::BeginTabBar("##SettingsTab", ImGuiTabBarFlags_NoCloseWithMiddleMouseButton | ImGuiTabBarFlags_FittingPolicyResizeDown))
        {
            //  General Settings 
            if (ImGui::BeginTabItem("General"))
            {
                ImGui::Text("Menu Settings");
                ImGui::Separator();

                // Menu Key
                static int menuKeyBinding = 0;
                ImGui::Text("Menu Key:");
                ImGui::SameLine();
                if (ImGui::Button(menuKeyBinding == 1 ? "< Press Any Key >" : KeyNames[g.MenuKey], ImVec2(120, 0))) {
                    menuKeyBinding = 1;
                    std::thread([&]() { KeyBinder(g.MenuKey, menuKeyBinding); }).detach();
                }

                ImGui::Spacing();
                ImGui::Separator();

                ImGui::Text("Performance");
                ImGui::Checkbox("Streamproof", &g.StreamProof);
                ImGui::Spacing();
                ImGui::Separator();

                ImGui::EndTabItem();
            }

            //  Info / About 
            if (ImGui::BeginTabItem("About"))
            {
                ImGui::Text("FiveM External Base");
                ImGui::Separator();
                ImGui::TextColored(ImVec4(0, 1, 1, 1), "Version: 1.0.0");
                ImGui::Spacing();
                ImGui::Text("Hotkeys:");
                ImGui::BulletText("INSERT - Show/Hide Menu");
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }
    }
    break;
    }

    ImGui::EndChild(); 
    ImGui::End();        
}