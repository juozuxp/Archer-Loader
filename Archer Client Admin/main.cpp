#include "ClientManager.h"
#include <Imports.h>
#include <WindowsEntrys.h>
#include <Utilities.h>
#include "DynamicImage.h"
#include "GUIManager.h"
#include "ConfigManager.h"
#include "ProcessManager.h"
#include "Pair.h"
#include "CryptString.h"

#ifndef _DEBUG
#include <VirtualizerSDK.h>
#endif

void LoadThread(Pair<ClientManager*, GUIManager*>* LoadInfo)
{


	GameWindowReserve* GameWindowInfo;
	WSAPROTOCOL_INFOA ProtocolInfo;
	PROCESSENTRY32 Process;
	DynamicImage Image;
	ArcherType Type;

	unsigned long long SubscriptionTimeStamp;
	char Path[MAX_PATH];

	LoadInfo->First->Attach();
	LoadInfo->Second->Attach();

	if (!ClientManager::Send(ArcherType_Setup, 0, 0))
		return;

	Type = ArcherType_None;
	while (Type != ArcherType_Key)
	{
		Type = ClientManager::Receive(&SubscriptionTimeStamp, sizeof(SubscriptionTimeStamp), 0);
		if (Type == ArcherType_Disconnect)
			return;
	}

	while (true)
	{
		GameWindowInfo = GUIManager::PromptGameWindow(SubscriptionTimeStamp);

		if (GameWindowInfo->LaunchOption == GameLaunchOptions_License)
		{
			if (!ClientManager::Send(ArcherType_Key, GameWindowInfo->LicenseKey, LICENSEKEYSIZE))
				return;

			Type = ArcherType_None;
			while (Type != ArcherType_Key)
			{
				Type = ClientManager::Receive(&SubscriptionTimeStamp, sizeof(SubscriptionTimeStamp), 0);
				if (Type == ArcherType_Disconnect)
					return;
			}
		}
		else if (GameWindowInfo->LaunchOption == GameLaunchOptions_Admin)
		{
			unsigned long long UserCount;
			const UserInfo* Users;

			Users = 0;
			AdminManager::SyncUsers(&Users, &UserCount);
			while (true)
			{
				AdminPanelReserve* AdminWindowInfo = GUIManager::PromptAdminPanel(Users, UserCount);
				if (AdminWindowInfo->Action == AdminAction_Leave)
					break;

				switch (AdminWindowInfo->Action)
				{
				case AdminAction_SyncUsers:
				{
					AdminManager::SyncUsers(&AdminWindowInfo->Users, &AdminWindowInfo->UserCount);
					Users = AdminWindowInfo->Users;
				} break;
				case AdminAction_ResetPassword:
				{
					AdminManager::ResetPassword(AdminWindowInfo->RegistrationKey, AdminWindowInfo->NewPassword);
				} break;
				case AdminAction_NewRegistration:
				{
					AdminManager::CreateRegistration(AdminWindowInfo->RegistrationKey);
				} break;
				case AdminAction_NewLicense:
				{
					AdminManager::CreateLicense(AdminWindowInfo->LicenseLength, AdminWindowInfo->LicenseKey);
				} break;
				};
			}

			free((void*)Users);
		}
		else
			break;
	}

	if (NT_SUCCESS(GENERAL_ERROR_NTSTATUS(FindProcessByNameA(__CS("ShooterGame.exe"), &Process))))
	{
		if (!ProcessManager::WaitForProcessTermination(Process.th32ProcessID))
			return;
	}

	GameWindowInfo->Progress += 0.0434;

	ProcessManager::TerminateService(__CS("BEService"));

	GameWindowInfo->Progress += 0.0434;

	if (NT_SUCCESS(GENERAL_ERROR_NTSTATUS(FindProcessByNameA(__CS("ShooterGame_BE.exe"), &Process))))
	{
		if (!ProcessManager::WaitForProcessTermination(Process.th32ProcessID))
			return;
	}

	GameWindowInfo->Progress += 0.0434;

	switch (GameWindowInfo->LaunchOption)
	{
	case GameLaunchOptions_Steam:
	{
		constexpr bool IsSteam = true;

		if (!ConfigManager::GetPaths(Path, 0) || !ProcessManager::FileExists(Path))
		{
			GameWindowInfo->Progress += 0.0352625;

			if (((unsigned long long)ShellExecuteA(0, 0, __CS("SteamExec"), 0, 0, SW_SHOW)) <= 32)
				return;

			GameWindowInfo->Progress += 0.0352625;

			if (!ProcessManager::WaitForProcess(__CS("ShooterGame.exe"), &Process))
				return;

			GameWindowInfo->Progress += 0.0352625;

			if (!ProcessManager::GetImagePath(Process.th32ProcessID, Path, sizeof(Path)))
				return;

			GameWindowInfo->Progress += 0.0352625;

			if (!ProcessManager::WaitForProcessTermination(Process.th32ProcessID))
				return;

			GameWindowInfo->Progress += 0.0352625;

			ProcessManager::TerminateService(__CS("BEService"));

			GameWindowInfo->Progress += 0.0352625;

			if (NT_SUCCESS(GENERAL_ERROR_NTSTATUS(FindProcessByNameA(__CS("ShooterGame_BE.exe"), &Process))))
			{
				if (!ProcessManager::WaitForProcessTermination(Process.th32ProcessID))
					return;
			}

			GameWindowInfo->Progress += 0.0352625;

			memcpy(Path + strlen(Path) - (sizeof(".exe") - 1), __CS("_BE.exe"), sizeof("_BE.exe"));

			ConfigManager::UpdateConfig(Path, 0);

			GameWindowInfo->Progress += 0.0352625;
		}
		else
			GameWindowInfo->Progress += 0.2821;

		if (!ClientManager::Send(ArcherType_Setup, &IsSteam, sizeof(IsSteam)))
			return;

		GameWindowInfo->Progress += 0.0352625;

		Image = DynamicImage(true);
		if (!Image.GetImageSize())
			return;

		GameWindowInfo->Progress += 0.0352625;

		if (!ProcessManager::ReplaceTheProcess(Path, Image.GetImage(), Image.GetImageSize()))
			return;

		GameWindowInfo->Progress += 0.0352625;

		if (((unsigned long long)ShellExecuteA(0, 0, __CS("SteamExec"), 0, 0, SW_SHOW)) <= 32)
		{
			ProcessManager::RestoreTheProcess(Path);
			return;
		}

		GameWindowInfo->Progress += 0.0352625;

		if (!ProcessManager::WaitForProcess(__CS("ShooterGame.exe"), &Process))
		{
			ProcessManager::RestoreTheProcess(Path);
			return;
		}

		GameWindowInfo->Progress += 0.0352625;

		ProcessManager::RestoreTheProcess(Path);

		GameWindowInfo->Progress += 0.0352625;

		if (!ProcessManager::PatchSteam(Process.th32ProcessID))
			return;

		GameWindowInfo->Progress += 0.0352625;

		if (!ProcessManager::ResumeProcess(Process.th32ProcessID))
			return;
	} break;
	case GameLaunchOptions_Epic:
	{
		constexpr bool IsSteam = false;

		if (!ConfigManager::GetPaths(0, Path) || !ProcessManager::FileExists(Path))
		{
			GameWindowInfo->Progress += 0.0434;

			if (((unsigned long long)ShellExecuteA(0, 0, __CS("EpicExec"), 0, 0, SW_SHOW)) <= 32)
				return;

			GameWindowInfo->Progress += 0.0434;

			if (!ProcessManager::WaitForProcess(__CS("ShooterGame.exe"), &Process))
				return;

			GameWindowInfo->Progress += 0.0434;

			if (!ProcessManager::GetImagePath(Process.th32ProcessID, Path, sizeof(Path)))
				return;

			GameWindowInfo->Progress += 0.0434;

			if (!ProcessManager::WaitForProcessTermination(Process.th32ProcessID))
				return;

			GameWindowInfo->Progress += 0.0434;

			ProcessManager::TerminateService(__CS("BEService"));

			GameWindowInfo->Progress += 0.0434;

			if (NT_SUCCESS(GENERAL_ERROR_NTSTATUS(FindProcessByNameA(__CS("ShooterGame_BE.exe"), &Process))))
			{
				if (!ProcessManager::WaitForProcessTermination(Process.th32ProcessID))
					return;
			}

			GameWindowInfo->Progress += 0.0434;

			memcpy(Path + strlen(Path) - (sizeof(".exe") - 1), __CS("_BE.exe"), sizeof("_BE.exe"));

			ConfigManager::UpdateConfig(0, Path);

			GameWindowInfo->Progress += 0.0434;
		}
		else
			GameWindowInfo->Progress += 0.3472;

		if (!ClientManager::Send(ArcherType_Setup, &IsSteam, sizeof(IsSteam)))
			return;

		GameWindowInfo->Progress += 0.0434;

		Image = DynamicImage(true);
		if (!Image.GetImageSize())
			return;

		GameWindowInfo->Progress += 0.0434;

		if (!ProcessManager::ReplaceTheProcess(Path, Image.GetImage(), Image.GetImageSize()))
			return;

		GameWindowInfo->Progress += 0.0434;

		if (((unsigned long long)ShellExecuteA(0, 0, __CS("EpicExec"), 0, 0, SW_SHOW)) <= 32)
		{
			ProcessManager::RestoreTheProcess(Path);
			return;
		}

		GameWindowInfo->Progress += 0.0434;

		if (!ProcessManager::WaitForProcess(__CS("ShooterGame.exe"), &Process))
		{
			ProcessManager::RestoreTheProcess(Path);
			return;
		}

		GameWindowInfo->Progress += 0.0434;

		ProcessManager::RestoreTheProcess(Path);
	} break;
	}

	GameWindowInfo->Progress += 0.0434;

	if (!ProcessManager::WaitForProcessWindow(__CS("Game name")))
		return;

	GameWindowInfo->Progress += 0.0434;

	if (!ClientManager::Send(ArcherType_Setup, 0, 0))
		return;

	GameWindowInfo->Progress += 0.0434;

	Image = DynamicImage(true);
	if (!Image.GetImageSize())
		return;

	GameWindowInfo->Progress += 0.0434;

	ClientManager::Duplicate(Process.th32ProcessID, &ProtocolInfo);

	GameWindowInfo->Progress += 0.0434;

	ProcessManager::LoadDll(Process.th32ProcessID, &ProtocolInfo, sizeof(ProtocolInfo), Image.GetImage(), Image.GetImageSize());


}

DLL_ENTRY(Instance, Reason, Reserve)
{


	switch (Reason)
	{
	case DLL_PROCESS_ATTACH:
	{
		LoadThread((Pair<ClientManager*, GUIManager*>*)Reserve);
	} break;
	}



	return TRUE;
}