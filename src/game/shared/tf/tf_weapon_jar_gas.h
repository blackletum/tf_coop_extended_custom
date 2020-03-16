//====== Copyright � 1996-2020, Valve Corporation, All rights reserved. =======
//
// Purpose: A remake of Throwable Jar(s) from live TF2
//
//=============================================================================
#ifndef TF_WEAPON_JAR_GAS_H
#define TF_WEAPON_JAR_GAS_H

#ifdef _WIN32
#pragma once
#endif

#include "tf_weaponbase_gun.h"
#include "tf_weapon_jar.h"

#ifdef GAME_DLL
#include "iscorer.h"
#endif

#ifdef CLIENT_DLL
#define CTFJarGas C_TFJarGas
#define CTFProjectile_JarGas C_TFProjectile_JarGas
#endif

#ifdef GAME_DLL
class CTFProjectile_JarGas : public CTFProjectile_Jar
#else
class C_TFProjectile_JarGas : public C_TFProjectile_Jar
#endif
{
	public:
	DECLARE_CLASS( CTFProjectile_JarGas, CTFProjectile_Jar );
	DECLARE_NETWORKCLASS();
#ifdef GAME_DLL
	DECLARE_DATADESC();
#endif

	virtual int	GetWeaponID( void ) const 			{ return TF_WEAPON_GRENADE_JAR_GAS; }

#ifdef GAME_DLL
	static CTFProjectile_JarGas	*Create( CBaseEntity *pWeapon, const Vector &vecOrigin, const QAngle &vecAngles, const Vector &vecVelocity, CBaseCombatCharacter *pOwner, CBaseEntity *pScorer, const AngularImpulse &angVelocity, const CTFWeaponInfo &weaponInfo );

	virtual void	Precache( void );

	virtual int		GetEffectCondition( void ) { return TF_COND_GAS; }
#endif
};

class CTFJarGas : public CTFWeaponBaseGun
{
public:
	DECLARE_CLASS( CTFJarGas, CTFWeaponBaseGun );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	// Server specific.
#ifdef GAME_DLL
	DECLARE_DATADESC();
#endif

	CTFJarGas();

	virtual int			GetWeaponID( void ) const 			{ return TF_WEAPON_JAR_GAS; }

	virtual void		Precache( void );

	virtual void		PrimaryAttack( void );

	virtual float		GetProjectileDamage( void );
	virtual float		GetProjectileSpeed( void );
	virtual float		GetProjectileGravity( void );
	virtual bool		CalcIsAttackCriticalHelper( void );

	virtual bool		HasChargeBar( void )				{ return true; }
	virtual const char* GetEffectLabelText( void )			{ return "#TF_Gas"; }
	virtual float		InternalGetEffectBarRechargeTime()	{ return 20.0; }

	virtual float		GetAfterburnRateOnHit( void ) const { return 10.0f; }
};

#endif // TF_WEAPON_JAR_GAS_H
