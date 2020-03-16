//=============================================================================//
//
// Purpose: 
//
//=============================================================================//
#ifndef TF_NPC_ARCHER_H
#define TF_NPC_ARCHER_H
#ifdef _WIN32
#pragma once
#endif

#include "ai_baseactor.h"
#include "soundent.h"

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CBotNPCArcher : public CAI_BaseActor
{
public:
	DECLARE_CLASS( CBotNPCArcher, CAI_BaseActor );
	DECLARE_DATADESC();

	CBotNPCArcher();
	~CBotNPCArcher();

	void			Precache();
	void			Spawn();
	void			Activate();

	Class_T			Classify( void ) { return CLASS_ZOMBIE; }

	void			SetupGlobalModelData();
	Activity		NPC_TranslateActivity( Activity baseAct );
	virtual	bool	OverrideMoveFacing( const AILocalMoveGoal_t &move, float flInterval );

	virtual float	GetIdealSpeed( ) const;
	virtual float	GetSequenceGroundSpeed( CStudioHdr *pStudioHdr, int iSequence );

	virtual void	DeathSound( const CTakeDamageInfo &info );
	virtual void	AlertSound( void );
	virtual void	PainSound( const CTakeDamageInfo &info );
	bool			ShouldPlayIdleSound( void );
	virtual void	IdleSound( void );

	void			StartTask( const Task_t *pTask );
	void			RunTask( const Task_t *pTask );
	void			RunAttackTask( int task );

	virtual void	PrescheduleThink( void );

	virtual void	Event_Killed( const CTakeDamageInfo &info );
	virtual int		OnTakeDamage_Alive( const CTakeDamageInfo &info );
	virtual bool	BecomeRagdollOnClient( const Vector &force );
	virtual void	UpdateOnRemove( void );

	virtual const char *GetLocalizeName();

public:
	static int		gm_nMoveXPoseParam;
	static int		gm_nMoveYPoseParam;

private:
	CBaseAnimating *m_pBow;

	CountdownTimer	m_evilCackleTimer;

	int m_iAttackLayer;

	float m_flAttRange;
	float m_flAttDamage;

	CountdownTimer m_attackTimer;
	CountdownTimer m_specialAttackTimer;

	CountdownTimer m_timeTillDeath;

	DEFINE_CUSTOM_AI;
};

#endif