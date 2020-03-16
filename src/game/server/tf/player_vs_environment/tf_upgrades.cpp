//======= Copyright 1996-2020, Valve Corporation, All rights reserved. ========//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "tf_upgrades.h"
#include "tf_team.h"
#include "ndebugoverlay.h"
#include "tf_player.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

BEGIN_DATADESC( CUpgrades )
END_DATADESC()

LINK_ENTITY_TO_CLASS( func_upgradestation, CUpgrades );

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CUpgrades::CUpgrades()
{
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CUpgrades::~CUpgrades()
{
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CUpgrades::Spawn( void )
{
	BaseClass::Spawn();

	AddSpawnFlags( SF_TRIGGER_ALLOW_CLIENTS );

	InitTrigger();

	if ( m_bDisabled )
		SetDisabled( true );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CUpgrades::SetDisabled( bool bDisabled )
{
	m_bDisabled = bDisabled;

	if ( bDisabled )
	{
		BaseClass::Disable();
		//SetTouch( NULL );
	}
	else
	{
		BaseClass::Enable();
		//SetTouch( &CUpgrades::Touch );
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CUpgrades::InputEnable( inputdata_t &inputdata )
{
	SetDisabled( false );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CUpgrades::InputDisable( inputdata_t &inputdata )
{
	SetDisabled( true );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CUpgrades::UpgradeTouch( CBaseEntity *pOther )
{
	if ( IsDisabled() || !pOther )
		return;

	CTFPlayer *pPlayer = ToTFPlayer( pOther );
	if ( pPlayer && !pPlayer->m_Shared.InUpgradeZone() )
		pPlayer->m_Shared.SetInUpgradeZone( true );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CUpgrades::EndTouch( CBaseEntity *pOther )
{
	if ( !pOther )
		return;

	CTFPlayer *pPlayer = ToTFPlayer( pOther );
	if ( pPlayer && pPlayer->m_Shared.InUpgradeZone() )
		pPlayer->m_Shared.SetInUpgradeZone( false );

	BaseClass::EndTouch( pOther );
}
