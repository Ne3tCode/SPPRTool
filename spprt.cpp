/**
 * Steam Parental Pin Recovery Tool
 *
 * MIT License
 *
 * Copyright (c) 2017 Nephrite
 */

//#define STEAMWORKS_CLIENT_INTERFACES
//#define NO_CSTEAMID_STL

#include "./3rdparty/OSW/OpenSteamworks/Steamworks.h"
#include "IClientParentalSettings.h"
#include "utlbuffer_mini.h"
#include "protoreader.h"
#include "CSteamAPI.h"

extern "C" {
//#include "3rdparty/scrypt-jane/scrypt-jane.h"                // scrypt
//#include "3rdparty/scrypt-jane/code/scrypt-jane-portable.h"  // +
//#include "3rdparty/scrypt-jane/code/scrypt-jane-hash.h"      // scrypt_pbkdf2

// ↑ or ↓
#include "3rdparty/scrypt-jane/scrypt-jane.c"  // same shit, less size ¯\_(ツ)_/¯
}

CSteamAPI SteamApi;

uint32 brute_v4(uint8 *pubSalt, uint32 cubSaltSize, uint8 *pubPasswordHash, uint32 cubHashSize)
{
	uint8 digest[32];
	uint32 password;
	uint8 a, b, c, d;

	for (a = '0'; a <= '9'; a++)
	{
		for (b = '0'; b <= '9'; b++)
		{
			SteamApi.Tier0_Log("%d%% ", a * 10 + b - 528);
			for (c = '0'; c <= '9'; c++)
			{
				for (d = '0'; d <= '9'; d++)
				{
					password = a | (b << 8) | (c << 16) | (d << 24);
					scrypt_pbkdf2((uint8 *)&password, sizeof(password), pubSalt, cubSaltSize, 10000, digest, sizeof(digest));
					if (memcmp(digest, pubPasswordHash, cubHashSize) == 0)
					{
						SteamApi.Tier0_Log("Done!\n");
						return password;
					}
				}
			}
		}
	}

	return 0;
}

uint32 brute_v6(uint8 *pubSalt, uint32 cubSaltSize, uint8 *pubPasswordHash, uint32 cubHashSize)
{
	uint8 digest[32];
	uint32 password;
	uint8 a, b, c, d;

	for (a = '0'; a <= '9'; a++)
	{
		for (b = '0'; b <= '9'; b++)
		{
			SteamApi.Tier0_Log("%d%% ", a * 10 + b - 528);
			for (c = '0'; c <= '9'; c++)
			{
				for (d = '0'; d <= '9'; d++)
				{
					password = a | (b << 8) | (c << 16) | (d << 24);
					scrypt((uint8 *)&password, sizeof(password), pubSalt, cubSaltSize, 12, 3, 0, digest, sizeof(digest));
					if (memcmp(digest, pubPasswordHash, cubHashSize) == 0)
					{
						SteamApi.Tier0_Log("Done!\n");
						return password;
					}
				}
			}
		}
	}

	return 0;
}

int main()
{
	if (!SteamApi.Load())
	{
		// Tier0 is not available here
		//printf("Unable to load steamclient.\n");
		return 1;
	}

	if (!SteamApi.Connect())
	{
		SteamApi.Tier0_Msg("Unable to connect to Steam or get the client engine interfaces.\n");
		return 2;
	}

	SteamApi.Tier0_Msg("Connected to Steam Client\n\n");

	if (!SteamApi.User->BLoggedOn())
	{
		SteamApi.Tier0_Msg("Not logged on.\n");
		return 2;
	}

	SteamApi.Tier0_Msg("Persona name: %s\n", SteamApi.Friends->GetPersonaName());
	SteamApi.Tier0_Msg("SteamID: %llu\n\n", SteamApi.User->GetSteamID().ConvertToUint64());

	char response;
	while (true)
	{
		SteamApi.Tier0_Msg("Is this your account? (y/n): ");
		response = getchar();
		fflush(stdin);
		if (response == 'Y' || response == 'y')
			break;
		if (response == 'N' || response == 'n')
		{
			SteamApi.Tier0_Msg("Okay, h4x0r... ");
			Sleep(1000);
			SteamApi.Tier0_Msg("Reported!");
			Sleep(3000);
			SteamApi.Tier0_Msg("\nJust kidding :D\n");
			return 0;
		}
		SteamApi.Tier0_Msg("Please type 'Y' or 'N'\n");
	}

	uint32 i = 0;
	CUtlBuffer MsgParentalSettings(0, 0);

	SteamApi.Tier0_Msg("Trying to get Parental Settings...\n");

	uint64 hHandle = SteamApi.UnifiedMessages->SendMethod("Parental.GetSignedParentalSettings#1", NULL, 0, 0);

	if (hHandle == 0)
	{
		SteamApi.Tier0_Msg("Cannot send Unified Message: handle == 0\n");
		return 3;
	}

	uint32 unResponseSize = 0;
	EResult eResult = k_EResultFail;
	bool res;

	do
	{
		Sleep(200);
		res = SteamApi.UnifiedMessages->GetMethodResponseInfo(hHandle, &unResponseSize, &eResult);
		i += 1;
	}
	while (!res && (i < 5));

	if (eResult != k_EResultOK)
	{
		SteamApi.Tier0_Msg("Error: response is not available, result = %u\n", eResult);
		SteamApi.UnifiedMessages->ReleaseMethod(hHandle);
		return 4;
	}

	SteamApi.Tier0_Msg("OK: response size = %u\n", unResponseSize);
	MsgParentalSettings.EnsureCapacity(unResponseSize);
	res = SteamApi.UnifiedMessages->GetMethodResponseData(hHandle, MsgParentalSettings.Base(), unResponseSize, true);
	if (!res)
	{
		SteamApi.Tier0_Msg("Error: no data available\n");
		return 5;
	}

	uint8 passwordhashtype = 0;
	uint8 salt[8] = {0};
	uint8 passwordhash[32] = {0};
	bool is_enabled = false;

	uint8 *msg_ptr = MsgParentalSettings.Base();
	uint32 msg_size = MsgParentalSettings.Size();

	// extract inner proto-message
	if (!GetFieldDataByTag(1, msg_ptr, msg_size, msg_ptr, &msg_size))
	{
		SteamApi.Tier0_Msg("Awww.. Why am I failing every fockin time??\n");
		return 6;
	}

#ifdef DEBUG
	SteamApi.Tier0_Log("[DEBUG] MsgSettings: %u ", msg_size);
	for (i = 0; i < msg_size; ++i)
		SteamApi.Tier0_Log("%02X", msg_ptr[i]);
	SteamApi.Tier0_Log("\n");
#endif

	uint32 size = sizeof(is_enabled);
	if (!GetFieldDataByTag(9, msg_ptr, msg_size, &is_enabled, &size))
	{
		SteamApi.Tier0_Warning("enabled - fail\n");
	}

	if (!is_enabled) // IClientUser->BIsParentalLockEnabled()
	{
		SteamApi.Tier0_Msg("Parental lock is not enabled\n");
		return 0;
	}

	CUtlBuffer SerializedParentalSettings(0, 0);
	uint32 value = 0;
	uint8 *serial_ptr;

	if (SteamApi.ParentalSettings)
	{
		if (SteamApi.ParentalSettings->BGetSerializedParentalSettings(&SerializedParentalSettings))
		{
			serial_ptr = SerializedParentalSettings.Base();

#ifdef DEBUG
			value = SerializedParentalSettings.Size();
			SteamApi.Tier0_Log("[DEBUG] Serialized Settings: %d ", value);
			for (i = 0; i < value; ++i)
			{
				SteamApi.Tier0_Log("%02X", serial_ptr[i]);
			}
			SteamApi.Tier0_Log("\n");
#endif

			value = *(uint32 *)serial_ptr;
#ifdef DEBUG
			SteamApi.Tier0_Msg("state: %d (%s)\n", value, (value == 1 ? "Locked" : (value == 2) ? "Unlocked" : "Unknown"));
#endif
		}
		else
		{
			SteamApi.Tier0_Msg("Error: cannot get serialized Parental Settings\n");
			value = 1;
		}
	}

	if (value == 1 || !SteamApi.ParentalSettings) // IClientUser->BIsParentalLockLocked()
	{
		size = sizeof(passwordhashtype);
		if (GetFieldDataByTag(6, msg_ptr, msg_size, &passwordhashtype, &size))
			SteamApi.Tier0_Msg("hash type: %u\n", passwordhashtype);
		else
			SteamApi.Tier0_Warning("hash type - fail\n");

		size = sizeof(salt);
		if (GetFieldDataByTag(7, msg_ptr, msg_size, salt, &size))
		{
			SteamApi.Tier0_Msg("salt: ");
			for (i = 0; i < sizeof(salt); ++i)
				SteamApi.Tier0_Msg("%02X", salt[i]);
			SteamApi.Tier0_Msg("\n");
		}
		else
			SteamApi.Tier0_Warning("salt - fail\n");

		size = sizeof(passwordhash);
		if (GetFieldDataByTag(8, msg_ptr, msg_size, passwordhash, &size))
		{
			SteamApi.Tier0_Msg("hash: ");
			for (i = 0; i < sizeof(passwordhash); ++i)
				SteamApi.Tier0_Msg("%02X", passwordhash[i]);
			SteamApi.Tier0_Msg("\n");
		}
		else
			SteamApi.Tier0_Warning("hash - fail\n");

		uint32 pin;
		uint64 tick_start, tick_stop;

		SteamApi.Tier0_Msg("Bruteforcing...\n");
		tick_start = SteamApi.Tier0_Plat_RelativeTicks();

		switch (passwordhashtype)
		{
			case 4:
				pin = brute_v4(salt, sizeof(salt), passwordhash, sizeof(passwordhash));
				break;
			case 6:
				pin = brute_v6(salt, sizeof(salt), passwordhash, sizeof(passwordhash));
				break;
			default:
				SteamApi.Tier0_Warning("Unsupported hash type (%u)\n", passwordhashtype);
				return -1;
		}

		tick_stop = SteamApi.Tier0_Plat_RelativeTicks();
		SteamApi.Tier0_Msg("Total time taken by CPU: %.2fs\n", (float)SteamApi.Tier0_Plat_TickDiffMilliSec(tick_start, tick_stop) / 1000.0f);

		if (pin != 0)
		{
			SteamApi.Tier0_Msg("Pin: \"%c%c%c%c\"\n", pin, pin >> 8, pin >> 16, pin >> 24);
			SteamApi.Tier0_Msg("Use it to unlock your Steam Client\n");
		}
		else
			SteamApi.Tier0_Msg("Pin not found\n");
	}
	else
	{
		SteamApi.Tier0_Msg("Parental lock is not locked\n");

		// pin is saved, parse the settings
		serial_ptr += 4;
		//value = *(uint32 *)serial_ptr;
		//printf("applist_base_id: %s (%d)\n", (value == 1 ? "Block all games" : "Allow all games"), value);
		serial_ptr += 4;
		value = *(uint32 *)serial_ptr;
		//printf("applist_base length: %d (skip)\n", value);
		serial_ptr += 4;
		for (i = 0; i < value; ++i)
		{
			serial_ptr += 8;
		}
		value = *(uint32 *)serial_ptr;
		//printf("applist_custom length: %d\n", value);
		serial_ptr += 4;
		for (i = 0; i < value; ++i)
		{
			//printf("> appid: %d, allowed: %d\n", *(uint32 *)serial_ptr, *(uint32 *)(serial_ptr + 4));
			serial_ptr += 8;
		}
		//value = *(uint32 *)serial_ptr;
		//printf("enabled_features: %d (0x%X)\n", value, value);
		serial_ptr += 4;
		value = *(uint32 *)serial_ptr;
		serial_ptr += 4;
		//printf("recovery_email: \"%s\" (length = %d)\n", serial_ptr, value);
		serial_ptr += value + 1; // length + \0 ??
		value = *(uint32 *)serial_ptr;
		serial_ptr += 4;
		SteamApi.Tier0_Msg("Pin: \"");
		for (i = 0; i < value; ++i)
		{
			SteamApi.Tier0_Msg("%c", serial_ptr[i]);
		}
		SteamApi.Tier0_Msg("\"\n");
	}

	SteamApi.Tier0_Msg("Press <Enter> to exit...");
	getchar();

	return 0;
}
