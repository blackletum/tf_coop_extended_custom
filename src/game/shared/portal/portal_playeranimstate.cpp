//====== Copyright 1996-2003, Valve Corporation, All rights reserved. =======
//
// Purpose: 
//
//=============================================================================

#include "cbase.h"
#include "tier0/vprof.h"
#include "animation.h"
#include "studio.h"
#include "apparent_velocity_helper.h"
#include "utldict.h"
#include "portal_playeranimstate.h"
#include "base_playeranimstate.h"

#ifdef CLIENT_DLL
#include "C_Portal_Player.h"
#include "C_Weapon_Portalgun.h"
#else
#include "Portal_Player.h"
#include "Weapon_Portalgun.h"
#endif

#define PORTAL_RUN_SPEED			320.0f
#define PORTAL_CROUCHWALK_SPEED		110.0f

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pPlayer - 
// Output : CMultiPlayerAnimState*
//-----------------------------------------------------------------------------
CPortalPlayerAnimState* CreatePortalPlayerAnimState( CPortal_Player *pPlayer )
{
	// Setup the movement data.
	MultiPlayerMovementData_t movementData;
	movementData.m_flBodyYawRate = 720.0f;
	movementData.m_flRunSpeed = PORTAL_RUN_SPEED;
	movementData.m_flWalkSpeed = -1;
	movementData.m_flSprintSpeed = -1.0f;

	// Create animation state for this player.
	CPortalPlayerAnimState *pRet = new CPortalPlayerAnimState( pPlayer, movementData );

	// Specific Portal player initialization.
	pRet->InitPortal( pPlayer );

	return pRet;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  :  - 
//-----------------------------------------------------------------------------
CPortalPlayerAnimState::CPortalPlayerAnimState()
{
	m_pPortalPlayer = NULL;

	// Don't initialize Portal specific variables here. Init them in InitPortal()
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pPlayer - 
//			&movementData - 
//-----------------------------------------------------------------------------
CPortalPlayerAnimState::CPortalPlayerAnimState( CBasePlayer *pPlayer, MultiPlayerMovementData_t &movementData )
: CMultiPlayerAnimState( pPlayer, movementData )
{
	m_pPortalPlayer = NULL;

	// Don't initialize Portal specific variables here. Init them in InitPortal()
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  :  - 
//-----------------------------------------------------------------------------
CPortalPlayerAnimState::~CPortalPlayerAnimState()
{
}

//-----------------------------------------------------------------------------
// Purpose: Initialize Portal specific animation state.
// Input  : *pPlayer - 
//-----------------------------------------------------------------------------
void CPortalPlayerAnimState::InitPortal( CPortal_Player *pPlayer )
{
	m_pPortalPlayer = pPlayer;
	m_bInAirWalk = false;
	m_flHoldDeployedPoseUntilTime = 0.0f;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPortalPlayerAnimState::ClearAnimationState( void )
{
	m_bInAirWalk = false;

	BaseClass::ClearAnimationState();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : actDesired - 
// Output : Activity
//-----------------------------------------------------------------------------
Activity CPortalPlayerAnimState::TranslateActivity( Activity actDesired )
{
	Activity translateActivity = BaseClass::TranslateActivity( actDesired );

	if ( GetPortalPlayer()->GetActiveWeapon() )
	{
		translateActivity = GetPortalPlayer()->GetActiveWeapon()->ActivityOverride( translateActivity, false );
	}

	return translateActivity;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : event - 
//-----------------------------------------------------------------------------
void CPortalPlayerAnimState::DoAnimationEvent( PlayerAnimEvent_t event, int nData )
{
	Activity iWeaponActivity = ACT_INVALID;

	//CBaseCombatWeapon *pWeapon = GetPortalPlayer()->GetActiveWeapon();

	switch( event )
	{
	case PLAYERANIMEVENT_ATTACK_PRIMARY:
		{
			// Weapon primary fire.
			if ( GetBasePlayer()->GetFlags() & FL_DUCKING )
			{
				//if (pWeapon && pWeapon->m_iFireMode == FIREMODE_1)
				//	RestartGesture( GESTURE_SLOT_ATTACK_AND_RELOAD, ACT_MP_ATTACK_CROUCH_PRIMARYFIRE2 );
				//else if (pWeapon && pWeapon->m_iFireMode == FIREMODE_2)
				//	RestartGesture( GESTURE_SLOT_ATTACK_AND_RELOAD, ACT_MP_ATTACK_CROUCH_PRIMARYFIRE3 );
				//else					
					RestartGesture( GESTURE_SLOT_ATTACK_AND_RELOAD, ACT_MP_ATTACK_CROUCH_PRIMARYFIRE );
			}
			else
			{
				//if (pWeapon && pWeapon->m_iFireMode == FIREMODE_1)
				//	RestartGesture( GESTURE_SLOT_ATTACK_AND_RELOAD, ACT_MP_ATTACK_STAND_PRIMARYFIRE2 );	
				//else if (pWeapon && pWeapon->m_iFireMode == FIREMODE_2)
				//	RestartGesture( GESTURE_SLOT_ATTACK_AND_RELOAD, ACT_MP_ATTACK_STAND_PRIMARYFIRE3 );
				//else
					RestartGesture( GESTURE_SLOT_ATTACK_AND_RELOAD, ACT_MP_ATTACK_STAND_PRIMARYFIRE );
			}

			iWeaponActivity = ACT_VM_PRIMARYATTACK;
			break;
	}
	case PLAYERANIMEVENT_ATTACK_SECONDARY:
	{
		// Weapon secondary fire.
		if ( GetBasePlayer()->GetFlags() & FL_DUCKING )
			{
				//if (pWeapon && pWeapon->m_iFireMode == FIREMODE_1)
				//	RestartGesture( GESTURE_SLOT_ATTACK_AND_RELOAD, ACT_MP_ATTACK_CROUCH_PRIMARYFIRE2 );
				//else if (pWeapon && pWeapon->m_iFireMode == FIREMODE_2)
				//	RestartGesture( GESTURE_SLOT_ATTACK_AND_RELOAD, ACT_MP_ATTACK_CROUCH_PRIMARYFIRE3 );
				//else					
					RestartGesture( GESTURE_SLOT_ATTACK_AND_RELOAD, ACT_MP_ATTACK_CROUCH_PRIMARYFIRE );
			}
			else
			{
				//if (pWeapon && pWeapon->m_iFireMode == FIREMODE_1)
				//	RestartGesture( GESTURE_SLOT_ATTACK_AND_RELOAD, ACT_MP_ATTACK_STAND_PRIMARYFIRE2 );	
				//else if (pWeapon && pWeapon->m_iFireMode == FIREMODE_2)
				//	RestartGesture( GESTURE_SLOT_ATTACK_AND_RELOAD, ACT_MP_ATTACK_STAND_PRIMARYFIRE3 );
				//else
					RestartGesture( GESTURE_SLOT_ATTACK_AND_RELOAD, ACT_MP_ATTACK_STAND_PRIMARYFIRE );
			}

		iWeaponActivity = ACT_VM_SECONDARYATTACK;
		break;
	}
	case PLAYERANIMEVENT_RELOAD:
		{

			if (GetBasePlayer()->GetFlags() & FL_DUCKING)
			{
				//if (pWeapon && pWeapon->m_iFireMode == FIREMODE_1)
				//	RestartGesture(GESTURE_SLOT_ATTACK_AND_RELOAD, ACT_MP_RELOAD_CROUCH2);
				//else if (pWeapon && pWeapon->m_iFireMode == FIREMODE_2)
				//	RestartGesture(GESTURE_SLOT_ATTACK_AND_RELOAD, ACT_MP_RELOAD_CROUCH3);
				//else
					RestartGesture(GESTURE_SLOT_ATTACK_AND_RELOAD, ACT_MP_RELOAD_CROUCH);
			}
			else
			{
				//if (pWeapon && pWeapon->m_iFireMode == FIREMODE_1)
				//	RestartGesture(GESTURE_SLOT_ATTACK_AND_RELOAD, ACT_MP_RELOAD_STAND2);
				//else if (pWeapon && pWeapon->m_iFireMode == FIREMODE_2)
				//	RestartGesture(GESTURE_SLOT_ATTACK_AND_RELOAD, ACT_MP_RELOAD_STAND3);
				//else
					RestartGesture(GESTURE_SLOT_ATTACK_AND_RELOAD, ACT_MP_RELOAD_STAND);
			}

		iWeaponActivity = ACT_VM_RELOAD;
		break;
		}
	default:
		{
			BaseClass::DoAnimationEvent( event, nData );
			break;
		}
	}

#ifdef CLIENT_DLL
	CBaseCombatWeapon *pWeapon = GetPortalPlayer()->GetActiveWeapon();

	// Make the weapon play the animation as well
	if ( iWeaponActivity != ACT_INVALID )
	{
		if ( pWeapon )
		{
			pWeapon->SendWeaponAnim( iWeaponActivity );
		}
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CPortalPlayerAnimState::Teleport( const Vector *pNewOrigin, const QAngle *pNewAngles, CPortal_Player* pPlayer )
{
	QAngle absangles = pPlayer->GetAbsAngles();
	m_angRender = absangles;
	m_angRender.x = m_angRender.z = 0.0f;
	if ( pPlayer )
	{
		// Snap the yaw pose parameter lerping variables to face new angles.
		m_flCurrentFeetYaw = m_flGoalFeetYaw = m_flEyeYaw = pPlayer->EyeAngles()[YAW];
	}
}




//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *idealActivity - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CPortalPlayerAnimState::HandleMoving( Activity &idealActivity )
{
	float flSpeed = GetOuterXYSpeed();

	//CBaseCombatWeapon *pWeapon = GetPortalPlayer()->GetActiveWeapon();

	// If we move, cancel the deployed anim hold
	if ( flSpeed > MOVING_MINIMUM_SPEED )
	{
		m_flHoldDeployedPoseUntilTime = 0.0;

		//if (pWeapon && pWeapon->m_iFireMode == FIREMODE_1)
		//	idealActivity = ACT_MP_RUN2;
		//else if (pWeapon && pWeapon->m_iFireMode == FIREMODE_2)
		//	idealActivity = ACT_MP_RUN3;
		//else
			idealActivity = ACT_MP_RUN;
	}

	else if ( m_flHoldDeployedPoseUntilTime > gpGlobals->curtime )
	{
		// Unless we move, hold the deployed pose for a number of seconds after being deployed

	//	if (pWeapon && pWeapon->m_iFireMode == FIREMODE_1)
	//		idealActivity = ACT_MP_DEPLOYED_IDLE2;
	//	else if (pWeapon && pWeapon->m_iFireMode == FIREMODE_2)
	//		idealActivity = ACT_MP_DEPLOYED_IDLE3;
	//	else
			idealActivity = ACT_MP_DEPLOYED_IDLE;
	}
	else 
	{
		return BaseClass::HandleMoving( idealActivity );
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *idealActivity - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CPortalPlayerAnimState::HandleDucking( Activity &idealActivity )
{
//	CPortal_Player *pPlayer = GetPortalPlayer();

	//CBaseCombatWeapon *pWeapon = GetPortalPlayer()->GetActiveWeapon();

	if ( GetBasePlayer()->m_Local.m_bDucking || GetBasePlayer()->m_Local.m_bDucked )
	{
		if ( GetOuterXYSpeed() < MOVING_MINIMUM_SPEED )
		{
			//if (pWeapon && pWeapon->m_iFireMode == FIREMODE_1)
			//	idealActivity = ACT_MP_CROUCH_IDLE2;
			//else if (pWeapon && pWeapon->m_iFireMode == FIREMODE_2)
			//	idealActivity = ACT_MP_CROUCH_IDLE3;
			//else
				idealActivity = ACT_MP_CROUCH_IDLE;
		}
		else
		{
			//if (pWeapon && pWeapon->m_iFireMode == FIREMODE_1)
			//	idealActivity = ACT_MP_CROUCHWALK2;
			//else if (pWeapon && pWeapon->m_iFireMode == FIREMODE_2)
			//	idealActivity = ACT_MP_CROUCHWALK3;
			//else
				idealActivity = ACT_MP_CROUCHWALK;
		}

		return true;
	}
	
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
bool CPortalPlayerAnimState::HandleJumping( Activity &idealActivity )
{
	//CBaseCombatWeapon *pWeapon = GetPortalPlayer()->GetActiveWeapon();

	Vector vecVelocity;
	GetOuterAbsVelocity( vecVelocity );

	if ( ( vecVelocity.z > 300.0f || m_bInAirWalk ) )
	{
		// Check to see if we were in an airwalk and now we are basically on the ground.
		if ( GetBasePlayer()->GetFlags() & FL_ONGROUND )
		{				
			m_bInAirWalk = false;
			RestartMainSequence();

		//	if (pWeapon && pWeapon->m_iFireMode == FIREMODE_1)
		//		RestartGesture(GESTURE_SLOT_JUMP, ACT_MP_JUMP_LAND2);
		//	else if (pWeapon && pWeapon->m_iFireMode == FIREMODE_2)
		//		RestartGesture(GESTURE_SLOT_JUMP, ACT_MP_JUMP_LAND3);
		//	else
				RestartGesture(GESTURE_SLOT_JUMP, ACT_MP_JUMP_LAND);
		}
		else
		{
			// In an air walk.
			idealActivity = ACT_MP_AIRWALK;
			m_bInAirWalk = true;
		}
	}
	// Jumping.
	else
	{
		if ( m_bJumping )
		{
			if ( m_bFirstJumpFrame )
			{
				m_bFirstJumpFrame = false;
				RestartMainSequence();	// Reset the animation.
			}

			// Don't check if he's on the ground for a sec.. sometimes the client still has the
			// on-ground flag set right when the message comes in.
			else if ( gpGlobals->curtime - m_flJumpStartTime > 0.2f )
			{
				if ( GetBasePlayer()->GetFlags() & FL_ONGROUND )
				{
					m_bJumping = false;
					RestartMainSequence();

				//	if (pWeapon && pWeapon->m_iFireMode == FIREMODE_1)
				//		RestartGesture(GESTURE_SLOT_JUMP, ACT_MP_JUMP_LAND2);
				//	else if (pWeapon && pWeapon->m_iFireMode == FIREMODE_2)
				//		RestartGesture(GESTURE_SLOT_JUMP, ACT_MP_JUMP_LAND3);
				//	else
						RestartGesture(GESTURE_SLOT_JUMP, ACT_MP_JUMP_LAND);
				}
			}

			// if we're still jumping
			if ( m_bJumping )
			{
			//	if (pWeapon && pWeapon->m_iFireMode == FIREMODE_1)
			//		idealActivity = ACT_MP_JUMP_START3;
			//	else if (pWeapon && pWeapon->m_iFireMode == FIREMODE_2)
			//		idealActivity = ACT_MP_JUMP_START2;
			//	else
					idealActivity = ACT_MP_JUMP_START;
			}
		}	
	}	

	if ( m_bJumping || m_bInAirWalk )
		return true;

	return false;
}