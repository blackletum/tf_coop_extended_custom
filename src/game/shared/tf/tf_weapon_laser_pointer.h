//=============================================================================
//
// Purpose: A remake of the Wrangler from live TF2
//
//=============================================================================
#ifndef TF_WEAPON_LASER_POINTER_H
#define TF_WEAPON_LASER_POINTER_H
#ifdef _WIN32
#pragma once
#endif

#include "tf_weaponbase_gun.h"
#include "tf_weapon_shotgun.h"

#ifdef GAME_DLL
#include "tf_obj_sentrygun.h"
#endif

// Client specific.
#ifdef CLIENT_DLL
#define CTFLaserPointer C_TFLaserPointer
#endif

//=============================================================================
//
// TF Weapon Laser Pointer.
//
class CTFLaserPointer : public CTFWeaponBaseGun
{
public:

	DECLARE_CLASS( CTFLaserPointer, CTFWeaponBaseGun );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	CTFLaserPointer();
	~CTFLaserPointer() {}

	virtual void	PrimaryAttack( void );
	virtual void	SecondaryAttack( void );

	virtual void	ItemPostFrame( void );
	virtual bool	Deploy( void );
	virtual bool	Holster( CBaseCombatWeapon *pSwitchingTo );

	virtual int		GetWeaponID( void ) const	{ return TF_WEAPON_LASER_POINTER; }

#ifdef GAME_DLL
	void			UpdateLaserDot( void );
	void			RemoveGun( void )			{ m_hGun = NULL; }

private:
	CHandle<CObjectSentrygun> m_hGun;
#endif
};

#endif // TF_WEAPON_LASER_POINTER_H