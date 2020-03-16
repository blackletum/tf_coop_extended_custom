//=============================================================================
//
// Purpose: capture the flag behavior
//
//=============================================================================

#ifndef LFE_NPC_FETCH_FLAG_H
#define LFE_NPC_FETCH_FLAG_H
#ifdef _WIN32
#pragma once
#endif

#include "simtimer.h"
#include "ai_behavior.h"
#include "func_capture_zone.h"
#include "entity_capture_flag.h"

#define AI_LFCTF_BEHAVIOR_BUSYTIME		10.0f

enum
{
	FETCHFLAG_SENTENCE_MOVE_TO_FLAG = SENTENCE_BASE_BEHAVIOR_INDEX,
	FETCHFLAG_SENTENCE_JUST_PICKUP,
};

class CAI_LFCTFBehavior : public CAI_SimpleBehavior
{
	DECLARE_CLASS( CAI_LFCTFBehavior, CAI_SimpleBehavior );
	DEFINE_CUSTOM_SCHEDULE_PROVIDER;
	DECLARE_DATADESC();
	
public:
	// Contructor/Deconstructor
	CAI_LFCTFBehavior();
	~CAI_LFCTFBehavior();

	void UpdateOnRemove();
	
	// Identifier
	const char *GetName() {	return "LFCTF"; }
	
	// Schedule
	bool 		CanSelectSchedule();
	void		BeginScheduleSelection();
	void		EndScheduleSelection();
	void		PrescheduleThink();

	Activity	NPC_TranslateActivity( Activity activity );

	// Conditions:
	virtual void GatherConditions();
	
	enum
	{
		SCHED_MOVE_TO_FLAG = BaseClass::NEXT_SCHEDULE,
		SCHED_MOVE_TO_CAPZONE,
		SCHED_FAIL_MOVE_TO_FLAG,
	};

	// Tasks
	void		StartTask( const Task_t *pTask );
	void		RunTask( const Task_t *pTask );

	enum
	{
		TASK_GET_PATH_TO_FLAG = BaseClass::NEXT_TASK,
		TASK_GET_PATH_TO_CAPZONE,
		TASK_REACH_FLAG,
		TASK_REACH_CAPZONE,
		TASK_FORGET_ABOUT_FLAG,
	};

	enum
	{
		COND_LFCTF_FLAG_DROP = BaseClass::NEXT_CONDITION,
		NEXT_CONDITION,
	};

	// Combat.
	CBaseEntity *BestEnemy( void );
	void Event_Killed( const CTakeDamageInfo &info );

	void DropFlag( void );

	int	 OnTakeDamage_Alive( const CTakeDamageInfo &info );

	// Time.
	void SetBusy( float flTime )		{ m_flBusyTime = flTime; }
	bool IsBusy( void )					{ return ( gpGlobals->curtime < m_flBusyTime ); }

	bool HasTheFlag( void )				{ return m_bHasTheFlag; }

private:
	
	// Schedule 
	int			SelectSchedule();
	
private:

	EHANDLE				m_hTeamFlag;
	EHANDLE				m_hCaptureZone;
	bool				m_bHasTheFlag;
	float				m_flBusyTime;
};

#endif // LFE_NPC_FETCH_FLAG_H
