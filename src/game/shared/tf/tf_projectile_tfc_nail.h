//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//
#ifndef TF_PROJECTILE_TFC_NAIL_H
#define TF_PROJECTILE_TFC_NAIL_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include "tf_projectile_base.h"

class CTFNailgunNail : public CTFBaseProjectile
{
public:
	DECLARE_CLASS( CTFNailgunNail, CTFBaseProjectile );

#ifdef GAME_DLL
	DECLARE_DATADESC();
#endif

	CTFNailgunNail();
	~CTFNailgunNail();

	void	Spawn();
	void	Precache();
	
	// Functions to create all the various types of nails.
	static	CTFNailgunNail *CreateNail( const Vector &vecOrigin, const QAngle &vecAngles, CBaseEntity *pOwner = NULL, CBaseEntity *pWeapon = NULL, bool bCritical = false );
	static	CTFNailgunNail *CreateSuperNail( const Vector &vecOrigin, const QAngle &vecAngles, CBaseEntity *pOwner = NULL, CBaseEntity *pWeapon = NULL, bool bCritical = false );
	static	CTFNailgunNail *CreateTranqNail( const Vector &vecOrigin, const QAngle &vecAngles, CBaseEntity *pOwner = NULL, CBaseEntity *pWeapon = NULL, bool bCritical = false );
	static	CTFNailgunNail *CreateRailgunNail( const Vector &vecOrigin, const QAngle &vecAngles, CBaseEntity *pOwner = NULL, CBaseEntity *pWeapon = NULL, bool bCritical = false );
	static	CTFNailgunNail *CreateNailGrenNail( Vector vecOrigin, QAngle vecAngles, CBaseEntity *pOwner, CBaseEntity *pNailGren );

	virtual const char *GetProjectileModelName( void );
	virtual float GetGravity( void ) { return 0.5f; }

	static float	GetInitialVelocity( void ) { return 1000.0; }
private:

	void	RailgunNail_Think();
#ifdef GAME_DLL
	void	NailTouch( CBaseEntity *pOther );
	void	TranqTouch( CBaseEntity *pOther );
	void	RailgunNailTouch( CBaseEntity *pOther );
#endif

private:
	Vector m_vecPreviousVelocity;
};

#endif // TF_PROJECTILE_TFC_NAIL_H
