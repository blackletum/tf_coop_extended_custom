//====== Copyright ? 1996-2020, Valve Corporation, All rights reserved. =======
//
// Purpose: 
//
//=============================================================================

#ifndef TF_WEAPON_BOTTLE_H
#define TF_WEAPON_BOTTLE_H
#ifdef _WIN32
#pragma once
#endif

#include "tf_weaponbase_melee.h"

#ifdef CLIENT_DLL
#define CTFBreakableMelee C_TFBreakableMelee
#define CTFBottle C_TFBottle
#define CTFStickBomb C_TFStickBomb
#define CTFBreakableSign C_TFBreakableSign
#endif

//=============================================================================
//
// Breakable Melee Base class.
//
class CTFBreakableMelee : public CTFWeaponBaseMelee
{
public:

	DECLARE_CLASS( CTFBreakableMelee, CTFWeaponBaseMelee );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	virtual void		Smack( void );
	virtual void		WeaponRegenerate();
	virtual void		WeaponReset( void );
	virtual bool		DefaultDeploy( char *szViewModel, char *szWeaponModel, int iActivity, char *szAnimExt );
	virtual void		SwitchBodyGroups( void );

	virtual bool		IsBroken( void ) { return m_bBroken; }
	virtual void		SetBroken( bool bState ) { m_bBroken = bState; }

protected:
	CNetworkVar( bool,	m_bBroken );
};

//=============================================================================
//
// Bottle class.
//
class CTFBottle : public CTFBreakableMelee
{
public:

	DECLARE_CLASS( CTFBottle, CTFBreakableMelee );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	CTFBottle();
	virtual int			GetWeaponID( void ) const			{ return TF_WEAPON_BOTTLE; }
	virtual const char	*GetWorldModel( void ) const;
	virtual void		Precache( void );

#ifdef CLIENT_DLL
	virtual int			GetWorldModelIndex();
#endif

private:

	CTFBottle( const CTFBottle & ) {}
};

//=============================================================================
//
// Caber class.
//
class CTFStickBomb : public CTFBottle
{
public:
	DECLARE_CLASS( CTFStickBomb, CTFBottle );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();
	
	CTFStickBomb();
	CTFStickBomb( const CTFStickBomb& ) = delete;
	
	virtual int			GetWeaponID() const { return TF_WEAPON_STICKBOMB; }
	virtual const char	*GetWorldModel( void ) const;
	virtual void		Precache( void );
	virtual void		Smack( void );
	virtual void		SwitchBodyGroups( void );
	virtual void		WeaponRegenerate( void );
	virtual void		WeaponReset( void );

	virtual bool		IsBroken( void );
#ifdef CLIENT_DLL
	virtual int			GetWorldModelIndex();
#endif

private:
	CNetworkVar( int, m_iDetonated );
};

//=============================================================================
//
// Neon Sign class.
//
class CTFBreakableSign : public CTFBreakableMelee
{
public:

	DECLARE_CLASS( CTFBreakableSign, CTFBreakableMelee );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	CTFBreakableSign();
	virtual int			GetWeaponID( void ) const			{ return TF_WEAPON_BREAKABLE_SIGN; }
	virtual const char	*GetWorldModel( void ) const;
	virtual void		Precache( void );

#ifdef CLIENT_DLL
	virtual int			GetWorldModelIndex();
#endif

private:

	CTFBreakableSign( const CTFBreakableSign & ) {}
};

#endif // TF_WEAPON_BOTTLE_H
