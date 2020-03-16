//=============================================================================//
//
// Purpose: 
//
//=============================================================================//

#ifndef HEADLESS_HATMAN_H
#define HEADLESS_HATMAN_H
#ifdef _WIN32
#pragma once
#endif

#include "ai_blended_movement.h"
#include "soundent.h"
#include "ai_behavior_follow.h"
#include "ai_behavior_assault.h"

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CHeadlessHatman : public CAI_BlendedNPC
{
public:
	DECLARE_CLASS( CHeadlessHatman, CAI_BlendedNPC );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	CHeadlessHatman( void );

	Class_T	Classify( void ) { return CLASS_ZOMBIE; }
	virtual int		GetSoundInterests( void ) { return (SOUND_WORLD|SOUND_COMBAT|SOUND_PLAYER|SOUND_DANGER); }

	virtual int		SelectFailSchedule( int failedSchedule, int failedTask, AI_TaskFailureCode_t taskFailCode );

	virtual int		TranslateSchedule( int scheduleType );
	virtual int		OnTakeDamage_Alive( const CTakeDamageInfo &info );
	virtual void	DeathSound( const CTakeDamageInfo &info );
	virtual void	Event_Killed( const CTakeDamageInfo &info );
	virtual int		SelectSchedule( void );

	virtual void	Precache( void );
	virtual void	Spawn( void );
	virtual void	Activate( void );
	virtual void	HandleAnimEvent( animevent_t *pEvent );
	virtual void	UpdateEfficiency( bool bInPVS )	{ SetEfficiency( ( GetSleepState() != AISS_AWAKE ) ? AIE_DORMANT : AIE_NORMAL ); SetMoveEfficiency( AIME_NORMAL ); }
	virtual void	PrescheduleThink( void );
	virtual void	GatherConditions( void );
	virtual void	TraceAttack( const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr, CDmgAccumulator *pAccumulator );
	virtual void	StartTask( const Task_t *pTask );
	virtual void	RunTask( const Task_t *pTask );
	virtual bool	HandleInteraction( int interactionType, void *data, CBaseCombatCharacter *sender );

	virtual bool	IsLightDamage( const CTakeDamageInfo &info ) { return true; }
	virtual bool	BecomeRagdollOnClient( const Vector &force );
	virtual void	UpdateOnRemove( void );

	virtual float	MaxYawSpeed( void );
	virtual bool	CanBecomeRagdoll( void );

	virtual bool	ShouldGib( const CTakeDamageInfo &info ) { return false; }

	virtual void	StartTouch( CBaseEntity *pOther );

	virtual Activity	NPC_TranslateActivity( Activity baseAct );

	virtual	bool	OverrideMoveFacing( const AILocalMoveGoal_t &move, float flInterval );
	virtual float	GetIdealSpeed( ) const;
	virtual float	GetSequenceGroundSpeed( CStudioHdr *pStudioHdr, int iSequence );

	const char *GetWeaponModel( void ) const;

	CBaseAnimating *GetWeapon( void ) const { return m_pAxe; }

	DEFINE_CUSTOM_AI;

	static int		gm_nMoveXPoseParam;
	static int		gm_nMoveYPoseParam;
private:

	inline bool CanStandAtPoint( const Vector &vecPos, Vector *pOut );
	int		SelectCombatSchedule( void );
	int		SelectUnreachableSchedule( void );

	bool	ShouldWatchEnemy( void );

	void	BuildScheduleTestBits( void );
	void	FoundEnemy( void );
	void	LostEnemy( void );
	void	UpdateBody( void );

	void	AttackTarget( CBaseCombatCharacter *victim, float dist );
	void	SelectVictim( void );
	void	ValidateChaseVictim( void );
	bool	IsPotentiallyChaseable( CBaseCombatCharacter *pVictim );
	bool	IsSwingingAxe( void );
	void	UpdateAxeSwing( void );
	void	RecomputeHomePosition( void );

	CHandle<CBaseCombatCharacter> m_hTarget;
	EHANDLE			m_hAimTarget;
	EHANDLE			m_hOldTarget;

protected:

	virtual void	PopulatePoseParameters( void );

	CBaseAnimating *m_pAxe;

	int m_iMoveX;
	int m_iMoveY;

	Vector m_vecHome;
	CountdownTimer m_recomputeHomeTimer;
	CountdownTimer m_attackTimer;
	CountdownTimer m_attackDuration;
	CountdownTimer m_emergeTimer;
	CountdownTimer m_shakeTimer;
	CountdownTimer m_evilCackleTimer;
	CountdownTimer m_terrifyTimer;
	CountdownTimer m_notifyVictimTimer;
	CountdownTimer m_rumbleTimer;
	CountdownTimer m_chaseDuration;
	CountdownTimer m_forcedTargetDuration;
	CountdownTimer m_booDelay;
	CountdownTimer m_stunDelay;
	CountdownTimer m_actionDuration;
	CountdownTimer m_dyingTimer;

	Vector m_vecTarget;
	float m_flDistance;
};

#endif