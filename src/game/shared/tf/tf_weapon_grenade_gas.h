//====== Copyright © 1996-2005, Valve Corporation, All rights reserved. ========//
//
// Purpose: TF Gas Grenade.
//
//=============================================================================//
#ifndef TF_WEAPON_GRENADE_GAS_H
#define TF_WEAPON_GRENADE_GAS_H
#ifdef _WIN32
#pragma once
#endif

#include "tf_weaponbase_grenadeproj.h"

// Client specific.
#ifdef CLIENT_DLL
#define CTFGrenadeGasProjectile C_TFGrenadeGasProjectile
#define CTFGasGrenadeEffect C_TFGasGrenadeEffect
#endif

class CTFGasGrenadeEffect : public CBaseEntity
{
public:
	DECLARE_CLASS( CTFGasGrenadeEffect, CBaseEntity );
	DECLARE_NETWORKCLASS();

#ifndef CLIENT_DLL

	virtual int UpdateTransmitState( void );

#else

	CTFGasGrenadeEffect()
	{
		m_pGasEffect = NULL;
	}

	virtual void OnDataChanged( DataUpdateType_t updateType );

	CNewParticleEffect *m_pGasEffect;

#endif

};

//=============================================================================
//
// TF Gase Grenade Projectile
//

class CTFGrenadeGasProjectile : public CTFWeaponBaseGrenadeProj
{
public:

	DECLARE_CLASS( CTFGrenadeGasProjectile, CTFWeaponBaseGrenadeProj );

	// Unique identifier.
	virtual int			GetWeaponID( void ) const			{ return TF_WEAPON_GRENADE_GAS; }

#ifdef GAME_DLL

	~CTFGrenadeGasProjectile();

	// Creation.
	static CTFGrenadeGasProjectile *Create( const Vector &position, const QAngle &angles, const Vector &velocity, 
		                                       const AngularImpulse &angVelocity, CBaseCombatCharacter *pOwner, CBaseEntity *pWeapon, float flTimer, int iFlags = 0 );

	// Overrides.
	virtual void	Spawn();
	virtual void	Precache();
	virtual void	BounceSound( void );
	virtual void	Detonate();
	virtual void	DetonateThink( void );

	void Think_Emit( void );
	void Think_Fade( void );

	DECLARE_DATADESC();

private:
	int	m_nPulses;

	CHandle<CTFGasGrenadeEffect> m_hGasEffect;
#endif
};

#endif // TF_WEAPON_GRENADE_GAS_H
