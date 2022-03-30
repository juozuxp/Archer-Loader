#include "GUIManager.h"

#include <d3d11.h>
#include <dwmapi.h>
#include "IMGUI/imgui.h"
#include "IMGUI/imgui_internal.h"
#include "IMGUI/imgui_impl_win32.h"
#include "IMGUI/imgui_impl_dx11.h"
#include <BasicUtilities.h>

#include "Assets.h"
#include "LodePNG/Lodepng.h"
#include "CryptString.h"

#include <time.h>

#ifndef _DEBUG
#include <VirtualizerSDK.h>
#endif

#pragma comment(lib, "d3d11.lib")

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

GUIManager* GUIManager::Instance;
ID3D11ShaderResourceView* GUIManager::ARKLogoTexture = 0;

void GUIManager::Attach()
{


	Instance = this;

	InitResources();

	ImGui::CreateContext();
	ImGui_ImplWin32_Init(Window);
	ImGui_ImplDX11_Init(Device, Context);

	SetImGuiStyle();

	WndProcFunction = ImGui_ImplWin32_WndProcHandler;


}

GameWindowReserve* GUIManager::PromptGameWindow(unsigned long long SubTimeStamp)
{
	static GameWindowReserve Reserved;

	memset(&Reserved, 0, sizeof(Reserved));

	Reserved.UnixTimestamp = SubTimeStamp;

	Instance->CurrentReserve = &Reserved;
	Instance->RenderFunction = (bool(*)(void* Reserved))GameWindow;

	while (Reserved.LaunchOption == GameLaunchOptions_None)
		Sleep(500);

	return &Reserved;
}

bool GUIManager::Stub()
{
	return true;
}

void GUIManager::EndFrameImGui()
{
	ImGui::PopFont();

	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void GUIManager::StartFrameImGui()
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ImGui::PushFont(Font);
}

void GUIManager::InitResources()
{
	ID3D11Texture2D* Texture;
	D3D11_TEXTURE2D_DESC TextureDesc;
	D3D11_SUBRESOURCE_DATA ResourceData;
	D3D11_SHADER_RESOURCE_VIEW_DESC ResourceDesc;

	void* Image;

	unsigned long Width;
	unsigned long Height;

	memset(&TextureDesc, 0, sizeof(TextureDesc));
	memset(&ResourceData, 0, sizeof(ResourceData));
	memset(&ResourceDesc, 0, sizeof(ResourceDesc));

	TextureDesc.MipLevels = 1;
	TextureDesc.ArraySize = 1;
	TextureDesc.CPUAccessFlags = 0;
	TextureDesc.SampleDesc.Count = 1;
	TextureDesc.Usage = D3D11_USAGE_DEFAULT;
	TextureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	TextureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	ResourceDesc.Texture2D.MostDetailedMip = 0;
	ResourceDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	ResourceDesc.Texture2D.MipLevels = TextureDesc.MipLevels;
	ResourceDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;

	lodepng_decode32((unsigned char**)&Image, (unsigned int*)&Width, (unsigned int*)&Height, ArkLogoPNG, sizeof(ArkLogoPNG));

	TextureDesc.Width = Width;
	TextureDesc.Height = Height;

	ResourceData.pSysMem = Image;
	ResourceData.SysMemPitch = Width * sizeof(unsigned long);
	ResourceData.SysMemSlicePitch = 0;

	while (!Font)
		Sleep(100);

	Device->CreateTexture2D(&TextureDesc, &ResourceData, &Texture);
	Device->CreateShaderResourceView(Texture, &ResourceDesc, &ARKLogoTexture);

	Texture->Release();
	free(Image);
}

void GUIManager::ResizeWindow(unsigned long SizeX, unsigned long SizeY)
{
	RECT WindowRect;

	GetWindowRect(Window, &WindowRect);
	if (WindowRect.right - WindowRect.left == SizeX && WindowRect.bottom - WindowRect.top == SizeY)
		return;

	SetWindowPos(Window, 0, (GetSystemMetrics(SM_CXSCREEN) - SizeX) / 2, (GetSystemMetrics(SM_CYSCREEN) - SizeY) / 2, SizeX, SizeY, 0);
}

bool GUIManager::GameWindow(GameWindowReserve* GameObject)
{


	static ID3D11ShaderResourceView* GameWindows[] = { ARKLogoTexture };
	static GameWindowType ActiveWindow = GameWindowType_ARK;

	static char LicenseKey[LICENSEKEYSIZE + 1];

	unsigned long long SubLength;

	struct tm ExpirationTime;

	ImGuiStyle* ImGuiStyle;
	ImVec2 InitialCursor;
	ImVec2 WindowSize;
	ImVec2 TextSize;
	ImVec2 Cursor;

	bool IsOpen;

	IsOpen = true;

	ImGuiStyle = &ImGui::GetStyle();

	Instance->ResizeWindow(GameWindowSize[0], GameWindowSize[1]);

	Instance->StartFrameImGui();

	ImGui::Begin(__CS("Game Menu"), &IsOpen, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

	InitialCursor = ImGui::GetCursorPos();
	WindowSize = ImGui::GetMainViewport()->Size;

	ImGui::SetWindowPos({ (WindowSize.x - GameWindowSize[0]) / 2, (WindowSize.y - GameWindowSize[1]) / 2 });
	ImGui::SetWindowSize({ GameWindowSize[0], GameWindowSize[1] });

	if (ImGui::ListBoxHeader(__CS("##GameSelect"), { (GameWindowSize[0] - (ImGuiStyle->WindowPadding.x * 2)) / 4, GameWindowSize[1] - ImGuiStyle->WindowPadding.y - InitialCursor.y }))
	{
		for (unsigned long i = 0; i < GetArraySize(GameWindows); i++)
		{
			if (i == ActiveWindow)
				ImGuiStyle->Colors[ImGuiCol_Button] = ImVec4(0.6, 0.6, 0.6, 1);
			else
				ImGuiStyle->Colors[ImGuiCol_Button] = ImVec4(0.5, 0.5, 0.5, 1);

			if (ImGui::ImageButton(GameWindows[i], { ((GameWindowSize[0] - (ImGuiStyle->WindowPadding.x * 2)) / 4) - ImGuiStyle->FramePadding.x * 4, ((GameWindowSize[0] - (ImGuiStyle->WindowPadding.x * 2)) / 4) - ImGuiStyle->FramePadding.x * 4 }))
				ActiveWindow = (GameWindowType)i;
		}

		ImGui::ListBoxFooter();
	}

	ImGuiStyle->Colors[ImGuiCol_Button] = ImVec4(0.5, 0.5, 0.5, 1);

	ImGui::SameLine();

	ImGui::BeginChild("##GameWindow");

	Cursor = ImGui::GetCursorScreenPos();

	ImGui::Text(__CS("Game name"));

	if (GameObject->UnixTimestamp)
	{
		if (!localtime_s(&ExpirationTime, (const time_t*)&GameObject->UnixTimestamp))
			ImGui::Text(__CS("Your subscription expires at: %02u-%02u-%02u %02u:%02u:%02u"), 1900 + ExpirationTime.tm_year, ExpirationTime.tm_mon + 1, ExpirationTime.tm_mday, ExpirationTime.tm_hour, ExpirationTime.tm_min, ExpirationTime.tm_sec);
	}

	if (GameObject->UnixTimestamp > time(0))
	{
		SubLength = GameObject->UnixTimestamp - time(0);
		ImGui::Text(__CS("You have %02llu days %02llu hours %02llu mins %02llu secs left"), SubLength / (60 * 60 * 24), (SubLength % (60 * 60 * 24)) / (60 * 60), (SubLength % (60 * 60)) / 60, SubLength % 60);

		if (ImGui::ButtonEx(__CS("Launch [Steam]"), { (GameWindowSize[0] - (ImGuiStyle->WindowPadding.x * 2)) * 0.75f - ImGuiStyle->ItemSpacing.x, 0 }, GameObject->LaunchOption != GameLaunchOptions_None ? ImGuiButtonFlags_Disabled : 0))
		{
			if (GameObject->LaunchOption == GameLaunchOptions_None)
				GameObject->LaunchOption = GameLaunchOptions_Steam;
		}

		if (ImGui::ButtonEx(__CS("Launch [Epic]"), { (GameWindowSize[0] - (ImGuiStyle->WindowPadding.x * 2)) * 0.75f - ImGuiStyle->ItemSpacing.x, 0 }, GameObject->LaunchOption != GameLaunchOptions_None ? ImGuiButtonFlags_Disabled : 0))
		{
			if (GameObject->LaunchOption == GameLaunchOptions_None)
				GameObject->LaunchOption = GameLaunchOptions_Epic;
		}

		if (GameObject->LaunchOption == GameLaunchOptions_License)
			ImGui::Text(__CS("Please wait..."));
		else if (GameObject->LaunchOption != GameLaunchOptions_Epic && GameObject->LaunchOption != GameLaunchOptions_Steam)
		{
			ImGui::PushItemWidth(((GameWindowSize[0] - (ImGuiStyle->WindowPadding.x * 2)) * 0.75f - ImGuiStyle->ItemSpacing.x) / 2);
			ImGui::InputText(__CS("##License Key"), GameObject->LicenseKey, sizeof(GameObject->LicenseKey));
			ImGui::PopItemWidth();

			ImGui::SameLine();

			if (ImGui::Button(__CS("Redeem license"), { ((GameWindowSize[0] - (ImGuiStyle->WindowPadding.x * 2)) * 0.75f - ImGuiStyle->ItemSpacing.x) / 2, 20 }))
				GameObject->LaunchOption = GameLaunchOptions_License;
		}

		if (GameObject->LaunchOption != GameLaunchOptions_None)
		{
			if (GameObject->LaunchOption == GameLaunchOptions_Epic)
				TextSize = ImGui::CalcTextSize(__CS("[Epic] Loading..."));
			else if (GameObject->LaunchOption == GameLaunchOptions_Steam)
				TextSize = ImGui::CalcTextSize(__CS("[Steam] Loading..."));

			ImGui::SetCursorScreenPos({ Cursor.x, (GameWindowSize[1] - ImGuiStyle->WindowPadding.y) - TextSize.y - ImGui::GetCurrentContext()->FontSize - ImGuiStyle->FramePadding.y * 4 });

			if (GameObject->LaunchOption == GameLaunchOptions_Epic)
				ImGui::Text(__CS("[Epic] Loading..."));
			else if (GameObject->LaunchOption == GameLaunchOptions_Steam)
				ImGui::Text(__CS("[Steam] Loading..."));

			ImGui::SetCursorScreenPos({ Cursor.x, (GameWindowSize[1] - ImGuiStyle->WindowPadding.y) - ImGui::GetCurrentContext()->FontSize - ImGuiStyle->FramePadding.y * 2 });
			ImGui::ProgressBar(GameObject->Progress);
		}
	}
	else
	{
		if (GameObject->LaunchOption == GameLaunchOptions_License)
			ImGui::Text(__CS("Please wait..."));
		else
		{
			ImGui::PushItemWidth((GameWindowSize[0] - (ImGuiStyle->WindowPadding.x * 2)) * 0.75f - ImGuiStyle->ItemSpacing.x);
			ImGui::InputText(__CS("##License Key"), GameObject->LicenseKey, sizeof(GameObject->LicenseKey));
			ImGui::PopItemWidth();

			if (ImGui::Button(__CS("Redeem license"), { (GameWindowSize[0] - (ImGuiStyle->WindowPadding.x * 2)) * 0.75f - ImGuiStyle->ItemSpacing.x, 20 }))
				GameObject->LaunchOption = GameLaunchOptions_License;
		}
	}

	ImGui::EndChild();

	ImGui::End();

	Instance->EndFrameImGui();



	return IsOpen;
}

void GUIManager::SetImGuiStyle()
{


	ImGuiStyle* Style = &ImGui::GetStyle();

	Style->WindowRounding = 4;

	Style->Colors[ImGuiCol_TitleBgActive] = ImVec4(0.2, 0.2, 0.2, 1);
	Style->Colors[ImGuiCol_TitleBg] = ImVec4(0.1, 0.1, 0.1, 1);

	Style->Colors[ImGuiCol_ResizeGrip] = ImVec4(0.4, 0.4, 0.4, 1);
	Style->Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.6, 0.6, 0.6, 1);
	Style->Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.6, 0.6, 0.6, 1);

	Style->Colors[ImGuiCol_FrameBg] = ImVec4(0.4, 0.4, 0.4, 1);
	Style->Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.5, 0.5, 0.5, 1);
	Style->Colors[ImGuiCol_FrameBgActive] = ImVec4(0.5, 0.5, 0.5, 1);

	Style->Colors[ImGuiCol_SliderGrab] = ImVec4(0, 0, 0, 1);
	Style->Colors[ImGuiCol_SliderGrabActive] = ImVec4(0, 0, 0, 1);

	Style->Colors[ImGuiCol_PlotHistogram] = ImVec4(0.2, 0.2, 0.2, 1);

	Style->Colors[ImGuiCol_CheckMark] = ImVec4(0, 0, 0, 1);

	Style->Colors[ImGuiCol_Separator] = ImVec4(0.2, 0.2, 0.2, 1);
	Style->Colors[ImGuiCol_SeparatorActive] = ImVec4(0.2, 0.2, 0.2, 1);
	Style->Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.2, 0.2, 0.2, 1);

	Style->Colors[ImGuiCol_Header] = ImVec4(0.1, 0.1, 0.1, 1);
	Style->Colors[ImGuiCol_HeaderHovered] = ImVec4(0.3, 0.3, 0.3, 1);
	Style->Colors[ImGuiCol_HeaderActive] = ImVec4(0.3, 0.3, 0.3, 1);

	Style->Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.3, 0.3, 0.3, 1);

	Style->Colors[ImGuiCol_Tab] = ImVec4(0.3, 0.3, 0.3, 1);
	Style->Colors[ImGuiCol_TabActive] = ImVec4(0.7, 0.7, 0.7, 1);
	Style->Colors[ImGuiCol_TabHovered] = ImVec4(0.5, 0.5, 0.5, 1);
	Style->Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.5, 0.5, 0.5, 1);

	Style->Colors[ImGuiCol_DockingPreview] = ImVec4(0.5, 0.5, 0.5, 1);
	Style->Colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.7, 0.7, 0.7, 1);

	Style->Colors[ImGuiCol_Button] = ImVec4(0.5, 0.5, 0.5, 1);
	Style->Colors[ImGuiCol_ButtonHovered] = ImVec4(0.7, 0.7, 0.7, 1);

	Style->Colors[ImGuiCol_ButtonActive] = ImVec4(0.7, 0.7, 0.7, 1);


}