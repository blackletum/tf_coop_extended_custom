//====== Copyright ? 1996-2020, Valve Corporation, All rights reserved. =======
//
// Purpose: 
//
//=============================================================================

#include "cbase.h"
#include "tf_upgrades_shared.h"
#include "filesystem.h"

CMannVsMachineUpgradeManager g_MannVsMachineUpgrades;

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Inherited from IAutoServerSystem
//-----------------------------------------------------------------------------
void CMannVsMachineUpgradeManager::LevelInitPostEntity( void )
{
	//LoadUpgradesFile();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CMannVsMachineUpgradeManager::LevelShutdownPostEntity( void )
{
	m_Upgrades.RemoveAll();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CMannVsMachineUpgradeManager::LoadUpgradesFile( void )
{
	LoadUpgradesFileFromPath( "scripts/items/mvm_upgrades.txt" );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CMannVsMachineUpgradeManager::LoadUpgradesFileFromPath( const char *pszPath )
{
	KeyValues *pkvFile = new KeyValues( "Upgrades" );

	if ( pkvFile->LoadFromFile( filesystem, pszPath, "MOD" ) )
	{
		KeyValues *pItemUpgrades = pkvFile->FindKey( "ItemUpgrades" );
		if( pItemUpgrades )
		{
			FOR_EACH_SUBKEY( pItemUpgrades, pSubData )
			{
				//int iIndex = atoi( pSubData->GetName() );

				const char *pAttribute = "damage bonus";
				const char *pIcon = "achievements/tf_demoman_kill_x_with_directpipe";
				float	flIncrement = 1.0f;
				float	flCap = 1.0f;
				int		iCost = 10;
				int		iUIGroup = 0;
				int		iQuality = 2; // 1-low, 2-normal, 3-high
				int		iTier = 0;

				FOR_EACH_VALUE( pSubData, pValue )
				{
					if ( V_stricmp( pValue->GetName(), "attribute" ) == 0 )
					{
						pAttribute = pValue->GetString();
					}
					else if ( V_stricmp( pValue->GetName(), "icon" ) == 0 )
					{
						pIcon = pValue->GetString();
					}
					else if ( V_stricmp( pValue->GetName(), "increment" ) == 0 )
					{
						flIncrement = pValue->GetFloat();
					}
					else if ( V_stricmp( pValue->GetName(), "cap" ) == 0 )
					{
						flCap = pValue->GetFloat();
					}
					else if ( V_stricmp( pValue->GetName(), "cost" ) == 0 )
					{
						iCost = pValue->GetInt();
					}
					else if ( V_stricmp( pValue->GetName(), "ui_group" ) == 0 )
					{
						iUIGroup = pValue->GetInt();
					}
					else if ( V_stricmp( pValue->GetName(), "quality" ) == 0 )
					{
						iQuality = pValue->GetInt();
					}
				}

				CMannVsMachineUpgrades upgrades;
				Q_snprintf( upgrades.m_szAttribute, sizeof( upgrades.m_szAttribute ), pAttribute );
				Q_snprintf( upgrades.m_szIcon, sizeof( upgrades.m_szIcon ), pIcon );
				upgrades.m_flIncrement = flIncrement,
				upgrades.m_flCap = flCap,
				upgrades.m_nCost = iCost,
				upgrades.m_iUIGroup = iUIGroup,
				upgrades.m_iQuality = iQuality,
				upgrades.m_iTier = iTier,

				m_Upgrades.AddToTail( upgrades );
			}
		}

		KeyValues *pPlayerUpgrades = pkvFile->FindKey( "PlayerUpgrades" );
		if( pPlayerUpgrades )
		{
			FOR_EACH_SUBKEY( pPlayerUpgrades, pSubData )
			{
				//int iIndex = atoi( pSubData->GetName() );

				const char *pAttribute = "damage bonus";
				const char *pIcon = "achievements/tf_demoman_kill_x_with_directpipe";
				float	flIncrement = 1.0f;
				float	flCap = 1.0f;
				int		iCost = 10;
				int		iUIGroup = 1;
				int		iQuality = 2; // 1-low, 2-normal, 3-high
				int		iTier = 0;

				FOR_EACH_VALUE( pSubData, pValue )
				{
					if ( V_stricmp( pValue->GetName(), "attribute" ) == 0 )
					{
						pAttribute = pValue->GetString();
					}
					else if ( V_stricmp( pValue->GetName(), "icon" ) == 0 )
					{
						pIcon = pValue->GetString();
					}
					else if ( V_stricmp( pValue->GetName(), "increment" ) == 0 )
					{
						flIncrement = pValue->GetFloat();
					}
					else if ( V_stricmp( pValue->GetName(), "cap" ) == 0 )
					{
						flCap = pValue->GetFloat();
					}
					else if ( V_stricmp( pValue->GetName(), "cost" ) == 0 )
					{
						iCost = pValue->GetInt();
					}
					else if ( V_stricmp( pValue->GetName(), "ui_group" ) == 0 )
					{
						iUIGroup = pValue->GetInt();
					}
					else if ( V_stricmp( pValue->GetName(), "quality" ) == 0 )
					{
						iQuality = pValue->GetInt();
					}
				}

				CMannVsMachineUpgrades upgrades;
				Q_snprintf( upgrades.m_szAttribute, sizeof( upgrades.m_szAttribute ), pAttribute );
				Q_snprintf( upgrades.m_szIcon, sizeof( upgrades.m_szIcon ), pIcon );
				upgrades.m_flIncrement = flIncrement,
				upgrades.m_flCap = flCap,
				upgrades.m_nCost = iCost,
				upgrades.m_iUIGroup = iUIGroup,
				upgrades.m_iQuality = iQuality,
				upgrades.m_iTier = iTier,

				m_Upgrades.AddToTail( upgrades );
			}
		}
	}

	pkvFile->deleteThis();
}
