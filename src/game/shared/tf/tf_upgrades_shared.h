//====== Copyright ? 1996-2020, Valve Corporation, All rights reserved. =======
//
// Purpose: 
//
//=============================================================================
#ifndef TF_UPGRADES_SHARED_H
#define TF_UPGRADES_SHARED_H
#ifdef _WIN32
#pragma once
#endif

#include "KeyValues.h"
#include "tf_shareddefs.h"

struct CMannVsMachineUpgrades
{
	char m_szAttribute[128];
	char m_szIcon[MAX_PATH];
	float m_flIncrement;
	float m_flCap;
	int m_nCost;
	int m_iUIGroup;
	int m_iQuality;
	int m_iTier;
};

class CMannVsMachineUpgradeManager : public CAutoGameSystem
{
public:
	CMannVsMachineUpgradeManager( void ) : CAutoGameSystem( "CMannVsMachineUpgradeManager" ) {}

	virtual void	LevelInitPostEntity( void );
	virtual void	LevelShutdownPostEntity( void );
	
	void			LoadUpgradesFile();
	void			LoadUpgradesFileFromPath( const char *pszPath );
	
private:
	//int		GetAttributeIndexByName( const char *name );
	//ParseUpgradeBlockForUIGroup(KeyValues *kv, int ui_group);

	CUtlVector<CMannVsMachineUpgrades> m_Upgrades;
	//CUtlMap<const char *, int> m_UpgradeMap;
};

extern CMannVsMachineUpgradeManager g_MannVsMachineUpgrades;

//void GetUpgradeStepData(CTFPlayer *player, int i1, int i2, int& i3, bool& b1);

#endif // TF_UPGRADES_SHARED_H