//====== Copyright © 1996-2013, Valve Corporation, All rights reserved. ========//
//
// Purpose: Cow Mangler Project.
//
//=============================================================================//
#ifndef TF_PROJECTILE_ENERGY_BALL_H
#define TF_PROJECTILE_ENERGY_BALL_H
#ifdef _WIN32
#pragma once
#endif

#include "tf_weaponbase_rocket.h"
#ifdef GAME_DLL
#include "iscorer.h"
#endif

// Client specific.
#ifdef CLIENT_DLL
#define CTFProjectile_EnergyBall C_TFProjectile_EnergyBall
#endif

class CTFProjectile_EnergyBall : public CTFBaseRocket
{
public:
	DECLARE_CLASS( CTFProjectile_EnergyBall, CTFBaseRocket );
	DECLARE_DATADESC();
	DECLARE_NETWORKCLASS();

	CTFProjectile_EnergyBall();
	~CTFProjectile_EnergyBall();

	virtual void	Spawn();
	virtual void	Precache();

#ifdef GAME_DLL
	static CTFProjectile_EnergyBall *Create( CBaseEntity *pWeapon, const Vector &vecOrigin, const QAngle &vecAngles, CBaseEntity *pOwner = NULL, CBaseEntity *pScorer = NULL );
	void							 InitEnergyBall( const Vector &velocity, const QAngle &vecAngles, CBaseEntity *pOwner, CBaseEntity *pWeapon );

	// IScorer interface
	virtual CBasePlayer *GetScorer( void );
	virtual CBasePlayer *GetAssistant( void ) { return NULL; }

	virtual int		GetWeaponID( void ) const	{ return TF_WEAPON_PARTICLE_CANNON; }

	void			SetScorer( CBaseEntity *pScorer );

	virtual bool	IsDeflectable() { return true; }
	virtual void	Deflected( CBaseEntity *pDeflectedBy, Vector &vecDir );

	// Overrides.
	virtual void	Explode( trace_t *pTrace, CBaseEntity *pOther );

	virtual int		GetDamageType();
	virtual float	GetDamage( void );
	virtual int		GetDamageCustom( void );
	
	void			SetCritical( bool bCritical ) { m_bCritical = bCritical; }
#else

	virtual void	OnDataChanged( DataUpdateType_t updateType );
	virtual void	CreateTrails( void );
	virtual void	CreateLightEffects( void );
#endif

	void			SetChargedShot( bool bState );

/*CTFProjectile_EnergyBall::CanHeadshot()
CTFProjectile_EnergyBall::ImpactTeamPlayer(CTFPlayer*)*/

private:
#ifdef GAME_DLL
	CBaseHandle m_Scorer;
	CNetworkVar( bool,	m_bCritical );
	CNetworkVar( bool,	m_bChargedShot );
#else
	bool		m_bCritical;
	bool		m_bChargedShot;
	Vector		m_vColor1;
	Vector		m_vColor2;
#endif

};

#endif //TF_PROJECTILE_ENERGY_BALL_H