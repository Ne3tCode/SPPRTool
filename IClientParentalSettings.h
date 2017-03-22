//==========================  Open Steamworks  ================================
//
// This file is part of the Open Steamworks project. All individuals associated
// with this project do not claim ownership of the contents
// 
// The code, comments, and all related files, projects, resources, 
// redistributables included with this project are Copyright Valve Corporation.
// Additionally, Valve, the Valve logo, Half-Life, the Half-Life logo, the
// Lambda logo, Steam, the Steam logo, Team Fortress, the Team Fortress logo, 
// Opposing Force, Day of Defeat, the Day of Defeat logo, Counter-Strike, the
// Counter-Strike logo, Source, the Source logo, and Counter-Strike Condition
// Zero are trademarks and or registered trademarks of Valve Corporation.
// All other trademarks are property of their respective owners.
//
//=============================================================================

#ifndef ICLIENTPARENTALSETTINGS_H
#define ICLIENTPARENTALSETTINGS_H
#ifdef _WIN32
#pragma once
#endif

#include "SteamTypes.h"

abstract_class UNSAFE_INTERFACE IClientParentalSettings
{
public:
	virtual bool BIsParentalLockEnabled() = 0;
	virtual bool BIsParentalLockLocked() = 0;
	virtual bool BIsAppBlocked( AppId_t unAppID ) = 0;
	virtual bool BIsAppInBlockList( AppId_t unAppID ) = 0;
	virtual bool BIsFeatureBlocked( EParentalFeature eParentalFeature ) = 0;
	virtual bool BIsFeatureInBlockList( EParentalFeature eParentalFeature ) = 0;
	virtual bool BGetSerializedParentalSettings( CUtlBuffer * pBuffer ) = 0;
	virtual bool BGetRecoveryEmail(char*, int) = 0;
};

#endif // ICLIENTPARENTALSETTINGS_H
