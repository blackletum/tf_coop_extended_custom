//=========== Copyright © 2018, LFE-Team, Not All rights reserved. ============
//
// Purpose:
//
//=============================================================================
#ifndef TF_FLAME_H
#define TF_FLAME_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include "tf_weapon_flamethrower.h"
// Client specific.
#ifdef CLIENT_DLL
	#include "particlemgr.h"
	#include "particle_util.h"
	#include "particles_simple.h"
	#include "c_tf_projectile_rocket.h"

#else
	#include "tf_projectile_rocket.h"
	#include "baseentity.h"
#endif

// Client specific.
#ifdef CLIENT_DLL
#define CTFFlameEntity C_TFFlameEntity
#endif

class CTFFlameThrower;

#ifdef GAME_DLL
DECLARE_AUTO_LIST( ITFFlameEntityAutoList );
#endif

class CTFFlameEntity : public CBaseEntity
#ifdef GAME_DLL
	, public ITFFlameEntityAutoList
#endif
{
public:
	DECLARE_CLASS( CTFFlameEntity, CBaseEntity );
	DECLARE_NETWORKCLASS();
#ifdef GAME_DLL
	DECLARE_DATADESC();
#endif

	CTFFlameEntity();
	~CTFFlameEntity();

	virtual void Spawn( void );
	void	Precache( void );

#ifdef CLIENT_DLL
	virtual void	OnDataChanged( DataUpdateType_t updateType );
	virtual void	ClientThink( void );

	CNewParticleEffect *m_pFlameEffect;
#else
public:
	static CTFFlameEntity *Create( const Vector &vecOrigin, const QAngle &vecAngles, CBaseEntity *pOwner, int iDmgType, float flDmgAmount, bool bCritFromBehind );

	void FlameThink( void );
	void CheckCollision( CBaseEntity *pOther, bool *pbHitWorld );
	CBaseEntity *GetAttacker( void ) { return m_hAttacker.Get(); }
	bool IsBehindTarget( CBaseEntity *pVictim );

private:
	void OnCollide( CBaseEntity *pOther );
	void SetHitTarget( void );

	Vector						m_vecInitialPos;		// position the flame was fired from
	Vector						m_vecPrevPos;			// position from previous frame
	Vector						m_vecBaseVelocity;		// base velocity vector of the flame (ignoring rise effect)
	Vector						m_vecAttackerVelocity;	// velocity of attacking player at time flame was fired
	float						m_flTimeRemove;			// time at which the flame should be removed
	int							m_iDmgType;				// damage type
	float						m_flDmgAmount;			// amount of base damage
	CUtlVector<EHANDLE>			m_hEntitiesBurnt;		// list of entities this flame has burnt
	int							m_iAttackerTeam;		// team of attacking player
	bool						m_bCritFromBehind;
#endif
	CNetworkHandle( CBaseEntity, m_hAttacker );			// attacking player
	CHandle<CTFFlameThrower>	m_hLauncher;			// weapon that fired this flame
};

#endif // TF_FLAME_H