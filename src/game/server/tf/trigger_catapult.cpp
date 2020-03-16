//============ Copyright Valve Corporation, All rights reserved. ===============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "trigger_catapult.h"
#include "tf_player.h"
#include "tf_gamerules.h"
#include "ai_basenpc.h"
#include "tf_team.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=============================================================================
//
// Trigger catapult tables.
//

LINK_ENTITY_TO_CLASS( trigger_catapult, CTriggerCatapult );

BEGIN_DATADESC( CTriggerCatapult )
	DEFINE_KEYFIELD( m_flPlayerVelocity, FIELD_FLOAT, "playerSpeed" ),
	DEFINE_KEYFIELD( m_flPhysicsVelocity, FIELD_FLOAT, "physicsSpeed" ),
	DEFINE_KEYFIELD( m_vecLaunchAngles, FIELD_VECTOR, "launchDirection" ),
	DEFINE_KEYFIELD( m_strLaunchTarget, FIELD_STRING, "launchTarget" ),
	DEFINE_KEYFIELD( m_bUseThresholdCheck, FIELD_BOOLEAN, "useThresholdCheck" ),
	DEFINE_KEYFIELD( m_bUseExactVelocity, FIELD_BOOLEAN, "useExactVelocity" ),
	DEFINE_KEYFIELD( m_flLowerThreshold, FIELD_FLOAT, "lowerThreshold" ),
	DEFINE_KEYFIELD( m_flUpperThreshold, FIELD_FLOAT, "upperThreshold" ),
	DEFINE_KEYFIELD( m_ExactVelocityChoice, FIELD_INTEGER, "exactVelocityChoiceType" ),
	DEFINE_KEYFIELD( m_bOnlyVelocityCheck, FIELD_BOOLEAN, "onlyVelocityCheck" ),
	DEFINE_KEYFIELD( m_bApplyAngularImpulse, FIELD_BOOLEAN, "applyAngularImpulse" ),
	DEFINE_KEYFIELD( m_flEntryAngleTolerance, FIELD_FLOAT, "EntryAngleTolerance" ),
	DEFINE_KEYFIELD( m_flAirControlSupressionTime, FIELD_FLOAT, "AirCtrlSupressionTime" ),
	DEFINE_KEYFIELD( m_bDirectionSuppressAirControl, FIELD_BOOLEAN, "DirectionSuppressAirControl" ),
	DEFINE_FIELD( m_hLaunchTarget, FIELD_EHANDLE ),
	DEFINE_FIELD( m_flRefireDelay, FIELD_FLOAT ),
	DEFINE_FIELD( m_hAbortedLaunchees, FIELD_EHANDLE ),

	DEFINE_INPUTFUNC( FIELD_STRING, "SetLaunchTarget", InputSetLaunchTarget ),
	DEFINE_INPUTFUNC( FIELD_FLOAT, "SetPlayerSpeed", InputSetPlayerSpeed ),
	DEFINE_INPUTFUNC( FIELD_FLOAT, "SetPhysicsSpeed", InputSetPhysicsSpeed ),
	DEFINE_INPUTFUNC( FIELD_INTEGER, "SetExactVelocityChoiceType", InputSetExactVelocityChoiceType ),

	DEFINE_OUTPUT( m_OnCatapulted,	"OnCatapulted" ),
END_DATADESC()

void CTriggerCatapult::Spawn( void )
{
	InitTrigger();
	BaseClass::Spawn();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTriggerCatapult::StartTouch( CBaseEntity *pOther )
{
	CBaseEntity	*pTarget = NULL;

	if ( m_bDisabled )
		return;

	if ( !PassesTriggerFilters( pOther ) )
		return;

	if ( m_strLaunchTarget != NULL_STRING )
	{
		pTarget = gEntList.FindEntityByName( pTarget, m_strLaunchTarget, NULL, pOther, pOther );
		if ( pTarget )
		{
			m_hLaunchTarget = pTarget;
		}
		else
		{
			Warning("Catapult trigger '%s' cannot find destination named '%s'!\n", GetEntityName(), m_strLaunchTarget );
		}
	}

	pOther->SetGroundEntity( NULL );

	Vector vecTargetOrigin = pTarget->GetAbsOrigin();

	float flVelocity = m_flPlayerVelocity;
	if ( pOther->GetMoveType() == MOVETYPE_VPHYSICS )
		flVelocity = m_flPhysicsVelocity;

	Vector vecNewVelocity = ( m_vecLaunchAngles * flVelocity );
	if ( pTarget )
		vecNewVelocity = ( vecTargetOrigin - pOther->GetAbsOrigin() - m_vecLaunchAngles * flVelocity );

	pOther->ApplyAbsVelocityImpulse( vecNewVelocity );

	AngularImpulse angVelocity( ( 600, random->RandomInt( -1200, 1200 ), 0 ) );

	IPhysicsObject *pPhys = pOther->VPhysicsGetObject();
	if ( pPhys )
	{
		if ( m_bApplyAngularImpulse )
			pPhys->AddVelocity( &vecNewVelocity, &angVelocity );
		else
			pPhys->AddVelocity( &vecNewVelocity, NULL );
	}

	m_OnCatapulted.FireOutput( this, this );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTriggerCatapult::InputSetPlayerSpeed( inputdata_t &inputdata )
{
	m_flPlayerVelocity = inputdata.value.Float();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTriggerCatapult::InputSetPhysicsSpeed( inputdata_t &inputdata )
{
	m_flPhysicsVelocity = inputdata.value.Float();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTriggerCatapult::InputSetLaunchTarget( inputdata_t &inputdata )
{
	KeyValue( "launchTarget", inputdata.value.String() );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTriggerCatapult::InputSetExactVelocityChoiceType( inputdata_t &inputdata )
{
	m_ExactVelocityChoice = inputdata.value.Int();
}
