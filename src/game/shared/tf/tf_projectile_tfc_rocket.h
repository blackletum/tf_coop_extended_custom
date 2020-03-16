//====== Copyright ฉ 1996-2005, Valve Corporation, All rights reserved. =======//
//
// Purpose: TFC RPG
//
//=============================================================================//
#ifndef TF_PROJECTILE_TFC_ROCKET_H
#define TF_PROJECTILE_TFC_ROCKET_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include "tf_weaponbase_rocket.h"

#ifdef CLIENT_DLL
#include "iviewrender_beams.h"
#include "c_smoke_trail.h"
#endif

#ifdef GAME_DLL
#include "smoke_trail.h"
#include "Sprite.h"
#include "npcevent.h"
#include "beam_shared.h"
#include "iscorer.h"
#include "tf_player.h"
#endif

#ifdef CLIENT_DLL
#define CTFRpgRocket C_TFRpgRocket
#define CTFRpgIC C_TFRpgIC
#endif

class CTFRpgRocket : public CTFBaseRocket
{
	DECLARE_CLASS( CTFRpgRocket, CTFBaseRocket );
	DECLARE_NETWORKCLASS();
#ifdef GAME_DLL
	DECLARE_DATADESC();
#endif
public:
	CTFRpgRocket();

	virtual void		Precache( void );
	virtual void		Spawn( void );

	virtual int		GetWeaponID( void ) const			{ return TF_WEAPON_TFC_ROCKETLAUNCHER; }

#ifdef GAME_DLL
	virtual void		UpdateOnRemove( void );

	virtual void		RocketTouch( CBaseEntity *pOther );
	void				IgniteThink( void );
	void				SeekThink( void );
	virtual void		Explode( trace_t *pTrace, CBaseEntity *pOther );
	virtual float		GetRadius( void );

	static CTFRpgRocket *Create( CBaseEntity *pWeapon, const Vector &vecOrigin, const QAngle &vecAngles, CBaseEntity *pOwner = NULL );
#endif

protected:
	float					m_flIgniteTime;
	int						m_iTrail;
};

class CTFRpgIC : public CTFRpgRocket
{
	DECLARE_CLASS( CTFRpgIC, CTFRpgRocket );
	DECLARE_NETWORKCLASS();
public:
	CTFRpgIC();

	virtual void		Precache( void );
	virtual void		Spawn( void );

	virtual int		GetWeaponID( void ) const			{ return TF_WEAPON_TFC_INCENDIARYCANNON; }

#ifdef GAME_DLL

	virtual void		Explode( trace_t *pTrace, CBaseEntity *pOther );

	static CTFRpgIC *Create( CBaseEntity *pWeapon, const Vector &vecOrigin, const QAngle &vecAngles, CBaseEntity *pOwner = NULL );
#endif
};

#endif	// TF_PROJECTILE_TFC_ROCKET_H
