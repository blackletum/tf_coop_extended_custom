//=============================================================================//
//
// Purpose: 
//
//=============================================================================//

#ifndef TF_ZOMBIE_H
#define TF_ZOMBIE_H
#ifdef _WIN32
#pragma once
#endif

#include "ai_baseactor.h"
#include "soundent.h"

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CTFZombie : public CAI_BaseActor
{
public:
	DECLARE_CLASS( CTFZombie, CAI_BaseActor );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	CTFZombie();
	~CTFZombie();

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

	void			StartTask( const Task_t *pTask );
	void			RunTask( const Task_t *pTask );
	void			RunAttackTask( int task );

	virtual void	PrescheduleThink( void );

	virtual void	Event_Killed( const CTakeDamageInfo &info );
	virtual int		OnTakeDamage_Alive( const CTakeDamageInfo &info );
	virtual bool	BecomeRagdollOnClient( const Vector &force );
	virtual void	UpdateOnRemove( void );

	enum SkeletonType_t
	{
		SKELETON_NORMAL = 0,
		SKELETON_SMALL,
		SKELETON_KING,
		SKELETON_ZOMBIE,
		SKELETON_ZOMBIE_FAST,
		SKELETON_ZOMBIE_POISON,

		SKELETON_LAST
	};

	void			SetZombieOwner( CBaseEntity *pOwner, CBaseEntity *pKiller );
	void			SetSkeletonType( int nType );

	static CTFZombie *CreateZombie( const Vector &vecOrigin, CBaseEntity *pOwner, CBaseEntity *pKiller, SkeletonType_t eType = SKELETON_NORMAL, float flLifeTime = -1 );

	void			AddHat( const char *pModel );

	CHandle<CBasePlayer> m_hZombieOwner;

	virtual const char *GetLocalizeName();

public:
	static int		gm_nMoveXPoseParam;
	static int		gm_nMoveYPoseParam;

private:
	CBaseAnimating	*m_pHat;

	CNetworkVar( int, m_nSkeletonType );
	CNetworkVar( int, m_nZombieClass );

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