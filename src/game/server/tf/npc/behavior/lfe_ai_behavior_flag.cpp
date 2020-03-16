//=============================================================================
//
// Purpose: capture the flag behavior
//
//=============================================================================

#include "cbase.h"
#include "lfe_ai_behavior_flag.h"
#include "ai_navigator.h"
#include "ai_memory.h"
#include "ai_senses.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// How long to fire a func tank before running schedule selection again.
#define FUNCTANK_FIRE_TIME	5.0f

BEGIN_DATADESC( CAI_LFCTFBehavior )
	DEFINE_FIELD( m_hTeamFlag, FIELD_EHANDLE ),
	DEFINE_FIELD( m_bHasTheFlag, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_hCaptureZone, FIELD_EHANDLE ),
	DEFINE_FIELD( m_flBusyTime, FIELD_TIME ),
END_DATADESC();

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CAI_LFCTFBehavior::CAI_LFCTFBehavior()
{
	m_hTeamFlag = NULL;
	m_bHasTheFlag = false;
	m_hCaptureZone = NULL;
	m_flBusyTime = 0.0f;
}

//-----------------------------------------------------------------------------
// Purpose: Deconstructor
//-----------------------------------------------------------------------------
CAI_LFCTFBehavior::~CAI_LFCTFBehavior()
{
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CAI_LFCTFBehavior::CanSelectSchedule()
{
	// If we don't have a flag and a capture don't bother with conditions, schedules, etc.
	if ( !m_hTeamFlag && !m_hCaptureZone )
		return false;

	// Are you alive, in a script?
	if ( !GetOuter()->IsInterruptable() )
		return false;
	
	// Commander is giving you orders?
	if ( GetOuter()->HasCondition( COND_RECEIVED_ORDERS ) )
		return false;
	
	return true;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CAI_LFCTFBehavior::BeginScheduleSelection()
{
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CAI_LFCTFBehavior::EndScheduleSelection()
{
	if ( m_bHasTheFlag )
		DropFlag();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAI_LFCTFBehavior::PrescheduleThink()
{
	BaseClass::PrescheduleThink();

	CBaseEntity *pNearestIntel = gEntList.FindEntityByClassnameNearest( "item_teamflag", GetAbsOrigin(), 9999);
	if ( pNearestIntel && !pNearestIntel->InSameTeam( GetOuter() ) )
		m_hTeamFlag = pNearestIntel;

	CBaseEntity *pNearestCapZone = gEntList.FindEntityByClassnameNearest( "func_capturezone", GetAbsOrigin(), 9999);
	if ( m_hCaptureZone && m_hCaptureZone->InSameTeam( GetOuter() ) )
		m_hCaptureZone = pNearestCapZone;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
int	CAI_LFCTFBehavior::SelectSchedule()
{
	if ( !m_hTeamFlag )
		return BaseClass::SelectSchedule();

	// If we've been told to drop flag.
	if ( HasCondition( COND_LFCTF_FLAG_DROP ) )
	{
		if ( HasTheFlag() )
			DropFlag();

		return BaseClass::SelectSchedule();
	}

	if ( !HasTheFlag() )
		return SCHED_MOVE_TO_FLAG;		// If we don't have the flag look for one.
	else
		return SCHED_MOVE_TO_CAPZONE;	// Then we can look for func_capturezone.

	return BaseClass::SelectSchedule();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : activity - 
// Output : Activity
//-----------------------------------------------------------------------------
Activity CAI_LFCTFBehavior::NPC_TranslateActivity( Activity activity )
{
	return BaseClass::NPC_TranslateActivity( activity );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CAI_LFCTFBehavior::DropFlag( void )
{
	SetBusy( gpGlobals->curtime + AI_LFCTF_BEHAVIOR_BUSYTIME );

	Assert( m_hTeamFlag );

	if ( m_hTeamFlag )
	{
		GetOuter()->MyNPCPointer()->DropFlag();
		m_bHasTheFlag = false;
	}

	// Set this condition to force breakout of any ctf behavior schedules
	SetCondition( COND_LFCTF_FLAG_DROP );
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
int CAI_LFCTFBehavior::OnTakeDamage_Alive( const CTakeDamageInfo &info )
{
	int iResult = BaseClass::OnTakeDamage_Alive( info );
	if ( !iResult )
		return 0;

	return iResult;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CAI_LFCTFBehavior::StartTask( const Task_t *pTask )
{
	switch ( pTask->iTask )
	{
	case TASK_GET_PATH_TO_FLAG:
		{
			if ( !m_hTeamFlag )
			{
				TaskFail( FAIL_NO_TARGET );
				return;
			}

			AI_NavGoal_t goal( m_hTeamFlag->GetAbsOrigin() );
			if ( !GetOuter()->InSameTeam( m_hTeamFlag ) )
			{
				goal.pTarget = m_hCaptureZone;
				if ( GetNavigator()->SetGoal( goal ) )
				{
					GetNavigator()->SetArrivalDirection( m_hTeamFlag->GetAbsAngles() );
					TaskComplete();
				}
				else
				{
					TaskFail("NO PATH");

					// Don't try and use me again for a while
					SetBusy( gpGlobals->curtime + AI_LFCTF_BEHAVIOR_BUSYTIME );
				}
			}
			else
			{
				TaskFail("NO ENEMY FLAG");

				// Don't try and use me again for a while
				SetBusy( gpGlobals->curtime + AI_LFCTF_BEHAVIOR_BUSYTIME );
			}
			break;
		}
	case TASK_GET_PATH_TO_CAPZONE:
		{
			if ( !m_hCaptureZone && !m_hTeamFlag )
			{
				TaskFail( FAIL_NO_TARGET );
				return;
			}

			AI_NavGoal_t goal( m_hCaptureZone->GetAbsOrigin() );
			if ( GetOuter()->InSameTeam( m_hCaptureZone ) )
			{
				goal.pTarget = m_hCaptureZone;
				if ( GetNavigator()->SetGoal( goal ) )
				{
					GetNavigator()->SetArrivalDirection( m_hCaptureZone->GetAbsAngles() );
					TaskComplete();
				}
				else
				{
					TaskFail("NO PATH");

					// Don't try and use me again for a while
					SetBusy( gpGlobals->curtime + AI_LFCTF_BEHAVIOR_BUSYTIME );
				}
			}
			else
			{
				TaskFail("NO CAPTURE ZONE");

				// Don't try and use me again for a while
				SetBusy( gpGlobals->curtime + AI_LFCTF_BEHAVIOR_BUSYTIME );
			}
			break;
		}
	case TASK_REACH_FLAG:
		{
			if ( !m_hTeamFlag )
			{
				TaskFail( FAIL_NO_TARGET );
				return;
			}

			if ( GetAbsOrigin() == m_hTeamFlag->GetAbsOrigin() )
			{
				if ( GetOuter()->MyNPCPointer()->HasTheFlag() )
				{
					TaskComplete();
				}
			}

			GetMotor()->SetIdealYawToTarget( m_hTeamFlag->GetAbsOrigin() );
			GetOuter()->SetTurnActivity(); 
			break;
		}
	case TASK_REACH_CAPZONE:
		{
			if ( !m_hTeamFlag && !m_hCaptureZone )
			{
				TaskFail( FAIL_NO_TARGET );
				return;
			}

			if ( GetAbsOrigin() == m_hCaptureZone->GetAbsOrigin() )
			{
				TaskComplete();
			}

			GetMotor()->SetIdealYawToTarget( m_hCaptureZone->GetAbsOrigin() );
			GetOuter()->SetTurnActivity(); 
			break;
		}

	case TASK_FORGET_ABOUT_FLAG:
		{
			if ( !m_hTeamFlag )
			{
				TaskFail( FAIL_NO_TARGET );
				return;
			}
			break;
		}
	default:
		{
			BaseClass::StartTask( pTask );
			break;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CAI_LFCTFBehavior::RunTask( const Task_t *pTask )
{
	switch ( pTask->iTask )
	{
	case TASK_REACH_CAPZONE:
		{
			Assert( m_hTeamFlag );

			GetMotor()->UpdateYaw();

			if ( GetOuter()->FacingIdeal() )
			{
				TaskComplete();
			}
			break;
		}
	case TASK_FORGET_ABOUT_FLAG:
		{
			SetBusy( gpGlobals->curtime + AI_LFCTF_BEHAVIOR_BUSYTIME );
			TaskComplete();
			break;
		}
	default:
		{
			BaseClass::RunTask( pTask );
			break;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CAI_LFCTFBehavior::Event_Killed( const CTakeDamageInfo &info )
{
	if ( m_hTeamFlag )
		DropFlag();

	Assert( !m_hTeamFlag );

	BaseClass::Event_Killed( info );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CAI_LFCTFBehavior::UpdateOnRemove( void )
{
	if ( m_hTeamFlag )
		DropFlag();

	BaseClass::UpdateOnRemove();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CAI_LFCTFBehavior::GatherConditions()
{
	BaseClass::GatherConditions();
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CBaseEntity *CAI_LFCTFBehavior::BestEnemy( void )
{
	return BaseClass::BestEnemy();
}

//=============================================================================
//
// Custom AI schedule data
//

AI_BEGIN_CUSTOM_SCHEDULE_PROVIDER( CAI_LFCTFBehavior )

	DECLARE_TASK( TASK_GET_PATH_TO_FLAG )
	DECLARE_TASK( TASK_GET_PATH_TO_CAPZONE )
	DECLARE_TASK( TASK_REACH_FLAG )
	DECLARE_TASK( TASK_REACH_CAPZONE )
	DECLARE_TASK( TASK_FORGET_ABOUT_FLAG )

	DECLARE_CONDITION( COND_LFCTF_FLAG_DROP )

	//=========================================================
	//=========================================================
	DEFINE_SCHEDULE 
	(
		SCHED_MOVE_TO_FLAG,

		"	Tasks"
		"		TASK_SET_FAIL_SCHEDULE		SCHEDULE: SCHED_FAIL_MOVE_TO_FLAG"
		"		TASK_GET_PATH_TO_FLAG		1000"
		"		TASK_SPEAK_SENTENCE			1"
		"		TASK_RUN_PATH				0"
		"		TASK_WAIT_FOR_MOVEMENT		0"
		"		TASK_STOP_MOVING			0"
		"		TASK_REACH_FLAG				0"
		"	"
		"	Interrupts"
		"		COND_LFCTF_FLAG_DROP"
	)

	DEFINE_SCHEDULE 
	(
		SCHED_MOVE_TO_CAPZONE,

		"	Tasks"
		"		TASK_SET_FAIL_SCHEDULE		SCHEDULE: SCHED_FAIL_MOVE_TO_FLAG"
		"		TASK_GET_PATH_TO_CAPZONE	1000"
		"		TASK_SPEAK_SENTENCE			1"
		"		TASK_RUN_PATH				0"
		"		TASK_WAIT_FOR_MOVEMENT		0"
		"		TASK_STOP_MOVING			0"
		"		TASK_REACH_CAPZONE			0"
		"	"
		"	Interrupts"
		"		COND_LFCTF_FLAG_DROP"
	)


	DEFINE_SCHEDULE 
	(
		SCHED_FAIL_MOVE_TO_FLAG,

		"	Tasks"
		"		TASK_FORGET_ABOUT_FLAG		1000"
		""
		"	Interrupts"
	)	

AI_END_CUSTOM_SCHEDULE_PROVIDER()