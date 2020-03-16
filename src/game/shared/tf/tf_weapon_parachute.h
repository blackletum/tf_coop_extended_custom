//====== Copyright © 1996-2020, Valve Corporation, All rights reserved. =======
//
// Purpose: 
//
//=============================================================================
#ifndef TF_WEAPON_PARACHUTE_H
#define TF_WEAPON_PARACHUTE_H
#ifdef _WIN32
#pragma once
#endif

#include "tf_weapon_buff_item.h"

#ifdef CLIENT_DLL
#define CTFParachute C_TFParachute
#define CTFParachute_Primary C_TFParachute_Primary
#define CTFParachute_Secondary C_TFParachute_Secondary
#endif

//=============================================================================
//
// Bottle class.
//
class CTFParachute : public CTFBuffItem
{
public:

	DECLARE_CLASS( CTFParachute, CTFBuffItem );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	CTFParachute();
	~CTFParachute();

	virtual int			GetWeaponID( void ) const			{ return TF_WEAPON_PARACHUTE; }

	virtual bool		CanBeSelected( void )				{ return false; }
	virtual bool		CanDeploy( void )					{ return false; }
	virtual bool		VisibleInWeaponSelection( void )	{ return false; }

	void				CreateBanner( int iBuffType );

#ifdef CLIENT_DLL
	void				ClientThink( void );
	void				ParachuteAnimThink( void );
#endif

private:

	CTFParachute( const CTFParachute & ) {}
};

class CTFParachute_Primary : public CTFParachute
{
public:
	DECLARE_CLASS( CTFParachute_Primary, CTFParachute );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();
};

class CTFParachute_Secondary : public CTFParachute
{
public:
	DECLARE_CLASS( CTFParachute_Secondary, CTFParachute );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();
};

#endif // TF_WEAPON_PARACHUTE_H
