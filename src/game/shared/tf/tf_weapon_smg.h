//====== Copyright © 1996-2005, Valve Corporation, All rights reserved. =======
//
//
//=============================================================================
#ifndef TF_WEAPON_SMG_H
#define TF_WEAPON_SMG_H
#ifdef _WIN32
#pragma once
#endif

#include "tf_weaponbase_gun.h"

// Client specific.
#ifdef CLIENT_DLL
#define CTFSMG C_TFSMG
#define CTFChargedSMG C_TFChargedSMG
#endif

//=============================================================================
//
// TF Weapon Sub-machine gun.
//
class CTFSMG : public CTFWeaponBaseGun
{
public:

	DECLARE_CLASS( CTFSMG, CTFWeaponBaseGun );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	CTFSMG() {}
	~CTFSMG() {}

	virtual int		GetWeaponID( void ) const { return TF_WEAPON_SMG; }

	virtual int		GetDamageType() const;
private:

	CTFSMG( const CTFSMG & ) {}
};

//=============================================================================
//
// TF Weapon Charged Sub-machine gun.
//
class CTFChargedSMG : public CTFSMG
{
public:

	DECLARE_CLASS( CTFChargedSMG, CTFSMG );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	CTFChargedSMG() {}
	~CTFChargedSMG() {}

	virtual int		GetWeaponID( void ) const { return TF_WEAPON_CHARGED_SMG; }

	//virtual void	SecondaryAttack( void );
	//virtual void	WeaponReset( void );

	//virtual void	ApplyOnHitAttributes( CBaseEntity*, CTFPlayer*, CTakeDamageInfo const& );
	//virtual bool	CanPerformSecondaryAttack( void ) const;
private:

	CNetworkVar( float, m_flMinicritCharge );

	CTFChargedSMG( const CTFChargedSMG & ) {}
};

#endif // TF_WEAPON_SMG_H