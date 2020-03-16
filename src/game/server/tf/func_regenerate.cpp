//======= Copyright © 1996-2005, Valve Corporation, All rights reserved. ======//
//
// Purpose: CTF Regenerate Zone.
//
//=============================================================================//

#include "cbase.h"
#include "tf_player.h"
#include "tf_item.h"
#include "tf_team.h"
#include "func_regenerate.h"
#include "tf_gamerules.h"
#include "eventqueue.h"
#include "ai_basenpc.h"

LINK_ENTITY_TO_CLASS( func_regenerate, CRegenerateZone );

#define TF_REGENERATE_SOUND				"Regenerate.Touch"
#define TF_REGENERATE_NEXT_USE_TIME		3.0f

//=============================================================================
//
// CTF Regenerate Zone tables.
//

BEGIN_DATADESC( CRegenerateZone )
	DEFINE_FIELD( m_hAssociatedModel, FIELD_EHANDLE ),
	DEFINE_KEYFIELD( m_iszAssociatedModel, FIELD_STRING, "associatedmodel" ),

	// Functions.
	DEFINE_FUNCTION( Touch ),
END_DATADESC();

//=============================================================================
//
// CTF Regenerate Zone functions.
//

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CRegenerateZone::CRegenerateZone()
{
	m_bDisabled = false;
}

//-----------------------------------------------------------------------------
// Purpose: Spawn function for the entity
//-----------------------------------------------------------------------------
void CRegenerateZone::Spawn( void )
{
	Precache();
	InitTrigger();
	AddSpawnFlags( SF_TRIGGER_ALLOW_CLIENTS | SF_TRIGGER_ALLOW_NPCS );
	SetTouch( &CRegenerateZone::Touch );
}

//-----------------------------------------------------------------------------
// Purpose: Precache function for the entity
//-----------------------------------------------------------------------------
void CRegenerateZone::Precache( void )
{
	PrecacheScriptSound( TF_REGENERATE_SOUND );
}

//-----------------------------------------------------------------------------
// Purpose: Precache function for the entity
//-----------------------------------------------------------------------------
void CRegenerateZone::Activate( void )
{
	BaseClass::Activate();

	if ( m_iszAssociatedModel != NULL_STRING )
	{
		CBaseEntity *pEnt = gEntList.FindEntityByName( NULL, STRING(m_iszAssociatedModel) );
		if ( !pEnt )
		{
			Warning("%s(%s) unable to find associated model named '%s'.\n", GetClassname(), GetDebugName(), STRING(m_iszAssociatedModel) );
		}
		else
		{
			m_hAssociatedModel = dynamic_cast<CDynamicProp*>(pEnt);
			if ( !m_hAssociatedModel )
			{
				Warning("%s(%s) tried to use associated model named '%s', but it isn't a dynamic prop.\n", GetClassname(), GetDebugName(), STRING(m_iszAssociatedModel) );
			}
		}	
	}
	else
	{
		Warning("%s(%s) has no associated model.\n", GetClassname(), GetDebugName() );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CRegenerateZone::Touch( CBaseEntity *pOther )
{
	if ( !IsDisabled() )
	{
		CTFPlayer *pPlayer = ToTFPlayer( pOther );
		if ( pPlayer )
		{
			if ( pPlayer->GetNextRegenTime() > gpGlobals->curtime )
				return;

			int iTeam = GetTeamNumber();

			if ( TFGameRules()->State_Get() != GR_STATE_TEAM_WIN )
			{
				if ( iTeam && ( pPlayer->GetTeamNumber() != iTeam ) )
					return;
			}
			else
			{
				// no health for the losing team, but all zones work for the winning team
				if ( TFGameRules()->GetWinningTeam() != pPlayer->GetTeamNumber() )
					return;
			}

			if ( TFGameRules()->InStalemate() )
				return;

			Regenerate( pPlayer );
		}

		CAI_BaseNPC *pNPC = dynamic_cast<CAI_BaseNPC *>( pOther );
		if ( pNPC )
		{
			if ( pNPC->GetNextRegenTime() > gpGlobals->curtime )
				return;

			int iTeam = GetTeamNumber();

			if ( TFGameRules()->State_Get() != GR_STATE_TEAM_WIN )
			{
				if ( iTeam && ( pNPC->GetTeamNumber() != iTeam ) )
					return;
			}
			else
			{
				// no health for the losing team, but all zones work for the winning team
				if ( TFGameRules()->GetWinningTeam() != pNPC->GetTeamNumber() )
					return;
			}

			if ( TFGameRules()->InStalemate() )
				return;

			Regenerate( pNPC );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CRegenerateZone::InputEnable( inputdata_t &inputdata )
{
	SetDisabled( false );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CRegenerateZone::InputDisable( inputdata_t &inputdata )
{
	SetDisabled( true );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CRegenerateZone::IsDisabled( void )
{
	return m_bDisabled;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CRegenerateZone::InputToggle( inputdata_t &inputdata )
{
	if ( m_bDisabled )
	{
		SetDisabled( false );
	}
	else
	{
		SetDisabled( true );
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CRegenerateZone::SetDisabled( bool bDisabled )
{
	m_bDisabled = bDisabled;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CRegenerateZone::Regenerate( CBaseEntity *pEntity )
{
	CTFPlayer *pPlayer = ToTFPlayer( pEntity );
	if ( pPlayer )
	{
		pPlayer->Regenerate();
		pPlayer->SetNextRegenTime( gpGlobals->curtime + TF_REGENERATE_NEXT_USE_TIME );

		CSingleUserRecipientFilter filter( pPlayer );
		EmitSound( filter, pPlayer->entindex(), TF_REGENERATE_SOUND );
	}

	CAI_BaseNPC *pNPC = dynamic_cast<CAI_BaseNPC *>( pEntity );
	if ( pNPC )
	{
		pNPC->Regenerate();
		pNPC->SetNextRegenTime( gpGlobals->curtime + TF_REGENERATE_NEXT_USE_TIME );
	}

	if ( m_hAssociatedModel )
	{
		variant_t tmpVar;
		tmpVar.SetString( MAKE_STRING("open") );
		m_hAssociatedModel->AcceptInput( "SetAnimation", this, this, tmpVar, 0 );

		tmpVar.SetString( MAKE_STRING("close") );
		g_EventQueue.AddEvent( m_hAssociatedModel, "SetAnimation", tmpVar, TF_REGENERATE_NEXT_USE_TIME - 1.0, this, this );
	}
}