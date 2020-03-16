//Copyright, Valve Corporation,Bad Robot,Escalation Studios, All rights reserved//
//
// Purpose: CTF Passtime Goal Zone.
//
//=============================================================================//
#include "cbase.h"
#include "func_passtime_goal.h"
#include "tf_player.h"
#include "ai_basenpc.h"
#include "tf_item.h"
#include "tf_team.h"
#include "tf_gamerules.h"

//=============================================================================
//
// CTF Passtime Goal Zone tables.
//

BEGIN_DATADESC( CFuncPasstimeGoal )

	// Keyfields.
	DEFINE_KEYFIELD( m_iPoints, FIELD_INTEGER, "points" ),

	// Functions.
	DEFINE_FUNCTION( CFuncPasstimeGoalShim::StartTouch ),
	DEFINE_FUNCTION( CFuncPasstimeGoalShim::EndTouch ),

	// Inputs.
	DEFINE_INPUTFUNC( FIELD_VOID, "Enable", InputEnable ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Disable", InputDisable ),

	// Outputs.
	DEFINE_OUTPUT( m_onScoreAny, "OnScoreAny" ),
	DEFINE_OUTPUT( m_onScoreRed, "OnScoreRed" ),
	DEFINE_OUTPUT( m_onScoreBlu, "OnScoreBlu" ),

END_DATADESC()

LINK_ENTITY_TO_CLASS( func_passtime_goal, CFuncPasstimeGoal );


IMPLEMENT_SERVERCLASS_ST( CFuncPasstimeGoal, DT_FuncPasstimeGoal )
END_SEND_TABLE()

//=============================================================================
//
// CTF Passtime Goal Zone functions.
//

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CFuncPasstimeGoal::Spawn()
{
	AddSpawnFlags( SF_TRIGGER_ALLOW_ALL | SF_TRIG_TOUCH_DEBRIS );

	InitTrigger();
	SetTouch( &CFuncPasstimeGoalShim::StartTouch );

	if ( m_bTriggerDisabled )
	{
		SetDisabled( true );
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CFuncPasstimeGoal::ShimStartTouch( CBaseEntity *pOther )
{
	// Is the zone enabled?
	if ( IsDisabled() )
		return;

	if ( HasSpawnFlags( SF_PASSGOAL_DONTLETBALLSCORE ) )
		return;

	// Get the TF player.
	CTFPlayer *pPlayer = ToTFPlayer( pOther );
	if ( pPlayer )
	{
		if ( HasSpawnFlags( SF_PASSGOAL_CARRYBALLSCORE ) )
		{
			CTFWeaponBase *pWpn = pPlayer->Weapon_OwnsThisID( TF_WEAPON_PASSTIME_GUN );
			if ( pWpn )
			{
				// Output.
				m_onScoreAny.FireOutput( this, this );

				if ( pPlayer->GetTeamNumber() == TF_TEAM_RED )
					m_onScoreRed.FireOutput( this, this );

				if ( pPlayer->GetTeamNumber() == TF_TEAM_BLUE )
					m_onScoreBlu.FireOutput( this, this );

				IGameEvent *event = gameeventmanager->CreateEvent( "pass_score" );
				if ( event )
				{
					event->SetInt( "points", m_iPoints );
					event->SetInt( "scorer", pPlayer->GetUserID() );
					//event->SetInt( "assister", pAssister->GetUserID() );
					event->SetInt( "priority", 9 ); // HLTV priority

					gameeventmanager->FireEvent( event );
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CFuncPasstimeGoal::ShimEndTouch( CBaseEntity *pOther )
{
}

//-----------------------------------------------------------------------------
// Purpose: The timer is always transmitted to clients
//-----------------------------------------------------------------------------
int CFuncPasstimeGoal::UpdateTransmitState()
{
	// ALWAYS transmit to all clients.
	return SetTransmitState( FL_EDICT_ALWAYS );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CFuncPasstimeGoal::IsDisabled( void )
{
	return m_bTriggerDisabled;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CFuncPasstimeGoal::InputEnable( inputdata_t &inputdata )
{
	SetDisabled( false );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CFuncPasstimeGoal::InputDisable( inputdata_t &inputdata )
{
	SetDisabled( true );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CFuncPasstimeGoal::SetDisabled( bool bDisabled )
{
	m_bTriggerDisabled = bDisabled;

	if ( bDisabled )
	{
		BaseClass::Disable();
		SetTouch( NULL );
	}
	else
	{
		BaseClass::Enable();
		SetTouch( &CFuncPasstimeGoal::StartTouch );
	}
}