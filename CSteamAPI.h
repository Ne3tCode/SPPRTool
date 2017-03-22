#ifndef CSTEAMAPI_H
#define CSTEAMAPI_H

#pragma once

#include "./3rdparty/OSW/OpenSteamworks/Steamworks.h"
#include "./3rdparty/OSW/OpenSteamAPI/src/Interface_OSW.h"

class CSteamAPI
{
public:
	CSteamAPI()
		: hPipe(0), hUser(0), m_pClientEngine(nullptr), m_pSteamClient(nullptr),
		  Friends(nullptr), User(nullptr), UnifiedMessages(nullptr), ParentalSettings(nullptr)
	{
		loader = new CSteamAPILoader();
	}

	~CSteamAPI()
	{
		if (hPipe && hUser)
		{
			m_pSteamClient->ReleaseUser(hPipe, hUser);
			m_pSteamClient->BReleaseSteamPipe(hPipe);
			m_pSteamClient->BShutdownIfAllPipesClosed();
		}
		delete loader;
	}

	bool Load()
	{
		if (!loader->Load())
			return false;

		DynamicLibrary m_pTier0("tier0_s"); // x64 ???
		Tier0_Log = (decltype(Tier0_Log)) m_pTier0.GetSymbol("Log");
		Tier0_Msg = (decltype(Tier0_Msg)) m_pTier0.GetSymbol("Msg");
		Tier0_Warning = (decltype(Tier0_Warning)) m_pTier0.GetSymbol("Warning");
		Tier0_Plat_RelativeTicks = (decltype(Tier0_Plat_RelativeTicks)) m_pTier0.GetSymbol("Plat_RelativeTicks");
		Tier0_Plat_TickDiffMilliSec = (decltype(Tier0_Plat_TickDiffMilliSec)) m_pTier0.GetSymbol("Plat_TickDiffMilliSec");

		return Tier0_Log && Tier0_Msg;
	}

	bool Connect()
	{
		CreateInterfaceFn factory = loader->GetSteam3Factory();
		if (!factory)
			return false;

		m_pClientEngine = (IClientEngine *)factory(CLIENTENGINE_INTERFACE_VERSION, NULL);
		m_pSteamClient = (ISteamClient017 *)factory(STEAMCLIENT_INTERFACE_VERSION_017, NULL);
		if (!m_pSteamClient)
			return false;

		hPipe = m_pSteamClient->CreateSteamPipe();
		hUser = m_pSteamClient->ConnectToGlobalUser(hPipe);
		if (!hPipe || !hUser)
			return false;

		Friends = (ISteamFriends015 *)m_pSteamClient->GetISteamFriends(hUser, hPipe, STEAMFRIENDS_INTERFACE_VERSION_015);
		User = (ISteamUser019 *)m_pSteamClient->GetISteamUser(hUser, hPipe, STEAMUSER_INTERFACE_VERSION_019);
		UnifiedMessages = (ISteamUnifiedMessages001 *)m_pSteamClient->GetISteamUnifiedMessages(hUser, hPipe, STEAMUNIFIEDMESSAGES_INTERFACE_VERSION_001);

		if (m_pClientEngine)
		{
			// ugly hack, but I am too lazy to fix it in the original repository
			ParentalSettings = (m_pClientEngine->*(IClientParentalSettings *(IClientEngine::*)(HSteamPipe, const char *))&m_pClientEngine->GetIClientParentalSettings)(hPipe, NULL);
			if (!ParentalSettings) // this mean vtable is changed (very old or very new steam client)
			{
				// fix the stack pointer
#if defined(_MSC_VER)
				__asm {subl $4,%esp}
#elif defined(__GNUC__)
				asm ("subl $4,%esp");
#else
				// ..or not. not my problem.
#endif
			}
		}

		return Friends && User && UnifiedMessages;
	}

private:
	CSteamAPILoader *loader;
	HSteamPipe hPipe;
	HSteamUser hUser;
	IClientEngine *m_pClientEngine;
	ISteamClient017 *m_pSteamClient;

public:
	ISteamFriends015 *Friends;
	ISteamUser019 *User;
	ISteamUnifiedMessages001 *UnifiedMessages;
	IClientParentalSettings *ParentalSettings;

	void (STEAM_CALL *Tier0_Log)(const char *szFormat, ...);
	void (STEAM_CALL *Tier0_Msg)(const char *szFormat, ...);
	void (STEAM_CALL *Tier0_Warning)(const char *szFormat, ...);
	uint64 (STEAM_CALL *Tier0_Plat_RelativeTicks)();
	int32 (STEAM_CALL *Tier0_Plat_TickDiffMilliSec)(uint64 tickStart, uint64 tickEnd);
};

#endif // CSTEAMAPI_H
