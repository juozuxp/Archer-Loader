#include "GUIManager.h"
#include "PasswordManager.h"

#include <d3d11.h>
#include <dwmapi.h>
#include "IMGUI/imgui.h"
#include "IMGUI/imgui_impl_win32.h"
#include "IMGUI/imgui_impl_dx11.h"
#include "Randomizer.h"

#include "CryptString.h"
#include "Assets.h"

#ifndef _DEBUG
#include <ThemidaSDK.h>
#endif

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "Dwmapi.lib")

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

GUIManager GUIManager::Instance;

GUIManager* GUIManager::GetInterface()
{
	return &Instance;
}

void GUIManager::ResizeWindow(unsigned long SizeX, unsigned long SizeY)
{
	RECT WindowRect;

	GetWindowRect(Window, &WindowRect);
	if (WindowRect.right - WindowRect.left == SizeX && WindowRect.bottom - WindowRect.top == SizeY)
		return;

	SetWindowPos(Window, 0, (GetSystemMetrics(SM_CXSCREEN) - SizeX) / 2, (GetSystemMetrics(SM_CYSCREEN) - SizeY) / 2, SizeX, SizeY, 0);
}

void GUIManager::Render(unsigned long SizeX, unsigned long SizeY)
{


	float ClearColor[] = { 0, 0, 0, 0 };
	bool StayOpen;

	MSG Message;

	Window = InitializeWindow(SizeX, SizeY);

	BringWindowToTop(Window);

	InitializeD3D11(Window, &RenderView);

	ImGui::CreateContext();
	ImGui_ImplWin32_Init(Window);
	ImGui_ImplDX11_Init(Device, Context);

	Font = ImGui::GetIO().Fonts->AddFontFromMemoryTTF(RobotoRegular, sizeof(RobotoRegular), 13);
	ImGui::GetIO().Fonts->Build();

	SetImGuiStyle();

	StayOpen = true;
	while (StayOpen)
	{
		while (PeekMessageA(&Message, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&Message);
			DispatchMessageA(&Message);

			if (Message.message == WM_QUIT)
				exit(0);
		}

		Context->OMSetRenderTargets(1, &RenderView, 0);
		Context->ClearRenderTargetView(RenderView, ClearColor);

		StayOpen = RenderFunction(CurrentReserve);

		Swapchain->Present(1, 0);
	}

	DestroyWindow(Window);



	exit(0);
}

bool GUIManager::Stub()
{


	Instance.StartFrameImGui();
	Instance.EndFrameImGui();



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

bool GUIManager::LoginWindow(LoginReserve* LoginObject)
{


	static char RegRegistrationKey[REGISRATIONKEYSIZE + 1] = { 0 };
	static char RegUsername[USERNAMESIZE + 1] = { 0 };
	static char RegPassword[PASSWORDSIZE + 1] = { 0 };

	static char Username[USERNAMESIZE + 1] = { 0 };
	static char Password[PASSWORDSIZE + 1] = { 0 };

	float StartLocation;
	bool IsOpen;

	ImGuiStyle* Style;
	ImVec2 Size;

	IsOpen = true;

	Instance.ResizeWindow(LoginWindowSize[0], LoginWindowSize[1]);

	Instance.StartFrameImGui();

	Style = &ImGui::GetStyle();

	ImGui::Begin(__CS("Login"), &IsOpen, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

	Size = ImGui::GetMainViewport()->Size;

	ImGui::SetWindowPos({ (Size.x - LoginWindowSize[0]) / 2, (Size.y - LoginWindowSize[1]) / 2 });
	ImGui::SetWindowSize({ LoginWindowSize[0], LoginWindowSize[1] });

	StartLocation = ImGui::GetCursorPosY();

	ImGui::BeginChild(__CS("##Login Screen"), { (LoginWindowSize[0] - Style->WindowPadding.x) / 2, LoginWindowSize[1] - StartLocation - Style->WindowPadding.y });

	Size = ImGui::CalcTextSize(__CS("Password:"));

	ImGui::Text(__CS("Username:"));
	ImGui::SameLine();

	ImGui::SetCursorPosX(Size.x + Style->ItemSpacing.x);

	ImGui::PushItemWidth(ImGui::GetWindowWidth() - ImGui::GetCursorPosX() - Style->ItemSpacing.x * 2);
	if (ImGui::InputText(__CS("##Username Input"), Username, sizeof(Username), ImGuiInputTextFlags_EnterReturnsTrue))
	{
		if (LoginObject->Login == LoginType_None)
		{
			memcpy(LoginObject->Username, Username, USERNAMESIZE);
			memcpy(LoginObject->Password, Password, PASSWORDSIZE);

			LoginObject->Login = LoginType_Login;
		}
	}

	ImGui::PopItemWidth();

	ImGui::Text(__CS("Password:"));
	ImGui::SameLine();

	ImGui::SetCursorPosX(Size.x + Style->ItemSpacing.x);

	ImGui::PushItemWidth(ImGui::GetWindowWidth() - ImGui::GetCursorPosX() - Style->ItemSpacing.x * 2);
	if (ImGui::InputText(__CS("##Password Input"), Password, sizeof(Password), ImGuiInputTextFlags_Password | ImGuiInputTextFlags_EnterReturnsTrue))
	{
		if (LoginObject->Login == LoginType_None)
		{
			memcpy(LoginObject->Username, Username, USERNAMESIZE);
			memcpy(LoginObject->Password, Password, PASSWORDSIZE);

			LoginObject->Login = LoginType_Login;
		}
	}

	ImGui::PopItemWidth();

	Size = ImGui::CalcTextSize(__CS("Login"));

	ImGui::SetCursorPosY(ImGui::GetWindowHeight() - Size.y * 2 - (Style->FramePadding.y * 4) - Style->ItemSpacing.y);

	if (ImGui::Button(__CS("Forgot password?"), { ImGui::GetWindowWidth() - (Style->ItemSpacing.x * 2), Size.y + (Style->FramePadding.y * 2) }))
		LoginObject->Login = LoginType_ForgotPassword;
	else if (LoginObject->Login == LoginType_None)
	{
		Size = ImGui::CalcTextSize(__CS("Login"));

		if (ImGui::Button(__CS("Login"), { ImGui::GetWindowWidth() - (Style->ItemSpacing.x * 2), Size.y + (Style->FramePadding.y * 2) }))
		{
			memcpy(LoginObject->Username, Username, USERNAMESIZE);
			memcpy(LoginObject->Password, Password, PASSWORDSIZE);

			LoginObject->Login = LoginType_Login;
		}
	}
	else
	{
		Size = ImGui::CalcTextSize(__CS("Please Wait..."));

		ImGui::SetCursorPos({ (ImGui::GetWindowWidth() - Size.x - (Style->ItemSpacing.x * 2.0f)) / 2, ImGui::GetCursorPosY() + ((ImGui::GetWindowHeight() - ImGui::GetCursorPosY() - Size.y - (Style->ItemSpacing.y * 2.0f)) / 2) });
		ImGui::Text(__CS("Please Wait..."));
	}

	ImGui::EndChild();

	ImGui::SameLine();

	ImGui::BeginChild(__CS("##Register Screen"), { (LoginWindowSize[0] - Style->WindowPadding.x) / 2, LoginWindowSize[1] - StartLocation - Style->WindowPadding.y });

	Size = ImGui::CalcTextSize(__CS("Registration key:"));

	ImGui::Text(__CS("Username:"));
	ImGui::SameLine();

	ImGui::SetCursorPosX(Size.x + Style->ItemSpacing.x);

	ImGui::PushItemWidth(ImGui::GetWindowWidth() - ImGui::GetCursorPosX() - Style->ItemSpacing.x * 2);
	if (ImGui::InputText(__CS("##RegUsername Input"), RegUsername, sizeof(RegUsername), ImGuiInputTextFlags_EnterReturnsTrue))
	{
		if (LoginObject->Login == LoginType_None)
		{
			memcpy(LoginObject->RegistrationKey, RegRegistrationKey, REGISRATIONKEYSIZE);
			memcpy(LoginObject->Username, RegUsername, USERNAMESIZE);
			memcpy(LoginObject->Password, RegPassword, PASSWORDSIZE);

			LoginObject->Login = LoginType_Register;
		}
	}

	ImGui::PopItemWidth();

	ImGui::Text(__CS("Password:"));
	ImGui::SameLine();

	ImGui::SetCursorPosX(Size.x + Style->ItemSpacing.x);

	ImGui::PushItemWidth(ImGui::GetWindowWidth() - ImGui::GetCursorPosX() - Style->ItemSpacing.x * 2);
	if (ImGui::InputText(__CS("##RegPassword Input"), RegPassword, sizeof(RegPassword), ImGuiInputTextFlags_Password | ImGuiInputTextFlags_EnterReturnsTrue))
	{
		if (LoginObject->Login == LoginType_None)
		{
			memcpy(LoginObject->RegistrationKey, RegRegistrationKey, REGISRATIONKEYSIZE);
			memcpy(LoginObject->Username, RegUsername, USERNAMESIZE);
			memcpy(LoginObject->Password, RegPassword, PASSWORDSIZE);

			LoginObject->Login = LoginType_Register;
		}
	}

	ImGui::PopItemWidth();

	ImGui::Text(__CS("Registration key:"));
	ImGui::SameLine();

	ImGui::SetCursorPosX(Size.x + Style->ItemSpacing.x);

	ImGui::PushItemWidth(ImGui::GetWindowWidth() - ImGui::GetCursorPosX() - Style->ItemSpacing.x * 2);
	if (ImGui::InputText(__CS("##RegRegistration Input"), RegRegistrationKey, sizeof(RegRegistrationKey), ImGuiInputTextFlags_EnterReturnsTrue))
	{
		if (LoginObject->Login == LoginType_None)
		{
			memcpy(LoginObject->RegistrationKey, RegRegistrationKey, REGISRATIONKEYSIZE);
			memcpy(LoginObject->Username, RegUsername, USERNAMESIZE);
			memcpy(LoginObject->Password, RegPassword, PASSWORDSIZE);

			LoginObject->Login = LoginType_Register;
		}
	}

	ImGui::PopItemWidth();

	Size = ImGui::CalcTextSize(__CS("Register"));

	ImGui::SetCursorPosY(ImGui::GetWindowHeight() - Size.y - (Style->FramePadding.y * 2.0f));
	if (LoginObject->Login == LoginType_None)
	{
		if (ImGui::Button(__CS("Register"), { ImGui::GetWindowWidth() - (Style->ItemSpacing.x * 2.0f), Size.y + (Style->FramePadding.y * 2.0f) }))
		{
			memcpy(LoginObject->RegistrationKey, RegRegistrationKey, REGISRATIONKEYSIZE);
			memcpy(LoginObject->Username, RegUsername, USERNAMESIZE);
			memcpy(LoginObject->Password, RegPassword, PASSWORDSIZE);

			LoginObject->Login = LoginType_Register;
		}
	}
	else
	{
		Size = ImGui::CalcTextSize(__CS("Please Wait..."));

		ImGui::SetCursorPos({ (ImGui::GetWindowWidth() - Size.x - (Style->ItemSpacing.x * 2.0f)) / 2, ImGui::GetCursorPosY() + ((ImGui::GetWindowHeight() - ImGui::GetCursorPosY() - Size.y - (Style->ItemSpacing.y * 2.0f)) / 2) });
		ImGui::Text(__CS("Please Wait..."));
	}

	ImGui::EndChild();

	ImGui::End();
	Instance.EndFrameImGui();



	return IsOpen;
}

bool GUIManager::ResetWindow(ResetReserve* ResetObject)
{
	static char RegistrationBuffer[REGISRATIONKEYSIZE + 1];
	static char PasswordBuffer[PASSWORDSIZE + 1];

	bool IsOpen;

	ImGuiStyle* Style;
	ImVec2 Size;

	IsOpen = true;

	Instance.ResizeWindow(ResetWindowSize[0], ResetWindowSize[1]);

	Instance.StartFrameImGui();

	Style = &ImGui::GetStyle();

	ImGui::Begin(__CS("Reset password"), &IsOpen, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

	Size = ImGui::GetMainViewport()->Size;

	ImGui::SetWindowPos({ (Size.x - ResetWindowSize[0]) / 2, (Size.y - ResetWindowSize[1]) / 2 });
	ImGui::SetWindowSize({ ResetWindowSize[0], ResetWindowSize[1] });

	Size = ImGui::CalcTextSize(__CS("Registration key: "));

	ImGui::Text(__CS("Registration key: "));
	
	ImGui::SameLine();

	ImGui::PushItemWidth(ResetWindowSize[0] - Style->WindowPadding.x * 2 - Style->ItemSpacing.x - Size.x);

	if (ImGui::InputText(__CS("##ResRegistration"), RegistrationBuffer, sizeof(RegistrationBuffer), ImGuiInputTextFlags_EnterReturnsTrue))
	{
		memcpy(ResetObject->RegistrationKey, RegistrationBuffer, REGISRATIONKEYSIZE);
		memcpy(ResetObject->Password, PasswordBuffer, PASSWORDSIZE);

		ResetObject->Type = ResetType_ResetPassword;
	}

	ImGui::PopItemWidth();

	ImGui::Text(__CS("Password: "));

	ImGui::SameLine();

	ImGui::SetCursorPosX(Size.x + Style->WindowPadding.x + Style->ItemSpacing.x);

	ImGui::PushItemWidth(ResetWindowSize[0] - Style->WindowPadding.x * 2 - Style->ItemSpacing.x - Size.x);

	if (ImGui::InputText(__CS("##ResPassword"), PasswordBuffer, sizeof(PasswordBuffer), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_Password))
	{
		memcpy(ResetObject->RegistrationKey, RegistrationBuffer, REGISRATIONKEYSIZE);
		memcpy(ResetObject->Password, PasswordBuffer, PASSWORDSIZE);

		ResetObject->Type = ResetType_ResetPassword;
	}

	ImGui::PopItemWidth();

	if (ImGui::Button(__CS("Reset"), { ResetWindowSize[0] - Style->WindowPadding.x * 2, 0 }))
	{
		memcpy(ResetObject->RegistrationKey, RegistrationBuffer, REGISRATIONKEYSIZE);
		memcpy(ResetObject->Password, PasswordBuffer, PASSWORDSIZE);

		ResetObject->Type = ResetType_ResetPassword;
	}

	if (ImGui::Button(__CS("Login/Register"), { ResetWindowSize[0] - Style->WindowPadding.x * 2, 0 }))
		ResetObject->Type = ResetType_Login;

	ImGui::End();
	Instance.EndFrameImGui();

	return IsOpen;
}

LoginType GUIManager::PromptLogin(char* Username, char* Password, char* RegistrationKey)
{
	static LoginReserve Object;

	Object.Login = LoginType_None;

	Object.Password = Password;
	Object.Username = Username;
	Object.RegistrationKey = RegistrationKey;

	Instance.RenderFunction = (bool(*)(void*))LoginWindow;
	Instance.CurrentReserve = &Object;

	while (Object.Login == LoginType_None)
		Sleep(100);

	return Object.Login;
}

ResetType GUIManager::PromptReset(char* Password, char* RegistrationKey)
{
	static ResetReserve Object;

	Object.Type = ResetType_None;

	Object.Password = Password;
	Object.RegistrationKey = RegistrationKey;

	Instance.RenderFunction = (bool(*)(void*))ResetWindow;
	Instance.CurrentReserve = &Object;

	while (Object.Type == ResetType_None)
		Sleep(100);

	return Object.Type;
}

void GUIManager::Initialize(unsigned long SizeX, unsigned long SizeY)
{
	Instance.WndProcFunction = ImGui_ImplWin32_WndProcHandler;
	CreateThread(0, 0, (LPTHREAD_START_ROUTINE)RenderThread, ((void*)(((unsigned long long)SizeX) | (((unsigned long long)SizeY) << 32))), 0, 0);
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

void GUIManager::InitializeD3D11(HWND Window, ID3D11RenderTargetView** RenderView)
{
	DXGI_SWAP_CHAIN_DESC sd;

	ZeroMemory(&sd, sizeof(sd));

	sd.BufferCount = 2;
	sd.BufferDesc.Width = 0;
	sd.BufferDesc.Height = 0;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 144;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = Window;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	UINT createDeviceFlags = 0;

	D3D_FEATURE_LEVEL featureLevel;
	const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
	if (D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &Swapchain, &Device, &featureLevel, &Context) != S_OK)
		return;

	ID3D11Texture2D* pBackBuffer;

	Swapchain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
	Device->CreateRenderTargetView(pBackBuffer, NULL, RenderView);
	pBackBuffer->Release();

	return;
}

void GUIManager::RandomString(char* String, unsigned long Length)
{
	for (unsigned long i = 0; i < Length; i++, String++)
		*String = 'A' + (Randomizer::RandomNumber() % (('Z' - 'A') + 1));

	*String = '\0';
}

LRESULT CALLBACK GUIManager::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return Instance.WindowProcedure(hwnd, uMsg, wParam, lParam);
}

void GUIManager::RenderThread(unsigned long long WindowSize)
{
	Instance.Render(WindowSize & ((1 << 32) - 1), WindowSize >> 32);
}

LRESULT CALLBACK GUIManager::WindowProcedure(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (Instance.WndProcFunction(hwnd, uMsg, wParam, lParam))
		return true;

	switch (uMsg)
	{
	case WM_SIZE:
	{
		if (Device)
		{
			ID3D11Texture2D* BackBuffer;

			if (RenderView)
				RenderView->Release();

			Swapchain->ResizeBuffers(0, lParam & ((1 << 16) - 1), (lParam >> 16) & ((1 << 16) - 1), DXGI_FORMAT_UNKNOWN, 0);

			Swapchain->GetBuffer(0, IID_PPV_ARGS(&BackBuffer));
			Device->CreateRenderTargetView(BackBuffer, NULL, &RenderView);
			BackBuffer->Release();
		}
	} break;
	case WM_DESTROY:
	{
		PostQuitMessage(0);
		return 0;
	} break;
	}

	return DefWindowProcA(hwnd, uMsg, wParam, lParam);
}

HWND GUIManager::InitializeWindow(unsigned long SizeX, unsigned long SizeY)
{
	MARGINS DWMMargin;
	WNDCLASSEXA Class;
	HWND WindowHandle;

	char ClassName[40];

	srand(GetTickCount());

	RandomString(ClassName, sizeof(ClassName) - 1);

	Class = { sizeof(WNDCLASSEXA), 0, WindowProc, 0L, 0L, GetModuleHandleA(0), 0, LoadCursor(0, IDC_ARROW), 0, 0, ClassName, 0 };

	RegisterClassExA(&Class);

	WindowHandle = CreateWindowExA(WS_EX_LAYERED, ClassName, 0, WS_POPUP, (GetSystemMetrics(SM_CXSCREEN) - SizeX) / 2, (GetSystemMetrics(SM_CYSCREEN) - SizeY) / 2, SizeX, SizeY, 0, 0, GetModuleHandleA(0), 0);

	SetLayeredWindowAttributes(WindowHandle, 0, 255, LWA_ALPHA);

	DWMMargin = { -1 };
	DwmExtendFrameIntoClientArea(WindowHandle, &DWMMargin);

	ShowWindow(WindowHandle, 1);

	return WindowHandle;
}