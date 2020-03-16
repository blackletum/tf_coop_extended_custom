//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================
#ifndef TF_EYEBALL_BOSS_H
#define TF_EYEBALL_BOSS_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include "player_pickup.h"
#include "tf_player.h"
#include "smoke_trail.h"
#include "ai_basenpc_flyer.h"

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CEyeBallBoss : public CAI_BaseFlyingBot
{
	DECLARE_CLASS( CEyeBallBoss, CAI_BaseFlyingBot );

public:
	DECLARE_SERVERCLASS();

	CEyeBallBoss();

	enum
	{
		ATTITUDE_CALM,
		ATTITUDE_GRUMPY,
		ATTITUDE_ANGRY,
		ATTITUDE_HATEBLUE,
		ATTITUDE_HATERED
	};

	void			Precache();
	void			Spawn( void );

	virtual void	UpdateEfficiency( bool bInPVS );
	virtual void	PrescheduleThink( void );

	Class_T			Classify( void ) { return(CLASS_ZOMBIE); }

	virtual void	Event_Killed( const CTakeDamageInfo &info );
	virtual	void	Event_KilledOther( CBaseEntity *pVictim, const CTakeDamageInfo &info );
	int				OnTakeDamage_Alive( const CTakeDamageInfo &info );

	void			OnStateChange( NPC_STATE eOldState, NPC_STATE eNewState );
	void			ClampMotorForces( Vector &linear, AngularImpulse &angular );

	int				DrawDebugTextOverlays(void);

	virtual float	GetHeadTurnRate( void );

	bool			ShouldPlayIdleSound( void );
	void			IdleSound( void );
	void			DeathSound( const CTakeDamageInfo &info );
	void			PainSound( const CTakeDamageInfo &info );

	int				SelectSchedule(void);
	void			StartTask( const Task_t *pTask );
	void			OnScheduleChange( void );
	void			UpdateOnRemove( void );
	virtual Vector	EyePosition( void );
	virtual void	UpdateLastKnownArea( void );

	void			PostRunStopMoving()	{} // scanner can use "movement" activities but not be moving

	virtual bool	CanBecomeRagdoll( void ) { return false; }

	// Inputs
	void			InputSetDistanceOverride( inputdata_t &inputdata );

	void			BecomeEnraged( float duration );
	const Vector&	PickNewSpawnSpot( void ) const;

	virtual int		GetLevel( void ) const { return m_level; }

	void			GetVelocity( Vector *vVelocity, AngularImpulse *vAngVelocity );
	const Vector	&GetCurrentVelocity() const		{ return m_localVelocity; }

	virtual void	TurnHeadToTarget( float flInterval, const Vector &MoveTarget );

	virtual float	GetDesiredAltitude( void ) const;
	virtual void	SetDesiredAltitude( float fHeight );

protected:

	void			MaintainAltitude( void );
	float			m_desiredAltitude;

	void			UpdateHead( float flInterval );

	// Movement
	virtual bool	OverrideMove( float flInterval );
	Vector			IdealGoalForMovement( const Vector &goalPos, const Vector &startPos, float idealRange, float idealHeight );
	virtual void	MoveToTarget( float flInterval, const Vector &vecMoveTarget );
	virtual float	MinGroundDist(void) { return 64; }
	Vector			VelocityToEvade(CBaseCombatCharacter *pEnemy);
	virtual float	GetGoalDistance( void );

private:
	bool	GetGoalDirection( Vector *vOut );

	float m_desiredSpeed;
	Vector m_vecMotion;
	float m_verticalSpeed;
	Vector m_localVelocity;
	Vector m_wishVelocity;

private:

	CNetworkVector( m_lookAtSpot );

	CNetworkVar( int, m_attitude );
	CountdownTimer m_attitudeTimer;

	int					m_iOldHealth;

	int					m_iAngerPose;

	float				m_flAttackRange;

	CountdownTimer m_idleTimer;
	CountdownTimer m_idleNoiseTimer;
	bool m_bTaunt;

	CountdownTimer m_lifeTimeDuration;
	float m_flTimeLeftAlive;

	CountdownTimer m_teleportTimer;

	CountdownTimer m_stunDelay;
	CountdownTimer m_stunDuration;

	static int m_level;

	enum
	{
		TELEPORT_VANISH,
		TELEPORT_APPEAR,
		TELEPORT_FINISH
	};
	int m_iTeleportState;

	CountdownTimer m_lookAroundTimer;
	CountdownTimer m_chaseDelay;

	CountdownTimer m_rocketLaunchDelay;
	CountdownTimer m_refireDelay;
	int m_iNumRockets;
	Vector m_vecShootAt;

	CountdownTimer m_dyingDuration;

	CountdownTimer m_emergeTimer;
	CountdownTimer m_shakeTimer;
	Vector m_vecTarget;
	float m_flDistance;

	CUtlVector<EHANDLE> m_hSpawnEnts;

protected:
	DEFINE_CUSTOM_AI;

	// Custom schedules
	enum
	{
		SCHED_EYEBALL_IDLE = BaseClass::NEXT_SCHEDULE,
		SCHED_EYEBALL_EMERGE,
		SCHED_EYEBALL_NOTICE,
		SCHED_EYEBALL_LAUNCH_ROCKET,
		SCHED_EYEBALL_TELEPORT,
		SCHED_EYEBALL_EMOTE,
		SCHED_EYEBALL_STUN,
		SCHED_EYEBALL_ESCAPE,

		NEXT_SCHEDULE,
	};

	// Custom tasks
	enum
	{
		TASK_EYEBALL_IDLE = BaseClass::NEXT_TASK,
		TASK_EYEBALL_EMERGE,
		TASK_EYEBALL_LAUNCH_ROCKET,
		TASK_EYEBALL_NOTICE,
		TASK_EYEBALL_ESCAPE,
		TASK_EYEBALL_TELEPORT,
		TASK_EYEBALL_EMOTE,
		TASK_EYEBALL_STUN,

		NEXT_TASK,
	};

	DECLARE_DATADESC();
};

#endif
