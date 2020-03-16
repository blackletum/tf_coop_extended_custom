//====== Copyright © 1996-2013, Valve Corporation, All rights reserved. ========//
//
// Purpose: Flare used by the flaregun.
//
//=============================================================================//
#ifndef TF_TFC_FLAME_H
#define TF_TFC_FLAME_H
#ifdef _WIN32
#pragma once
#endif

#include "tf_weaponbase_rocket.h"
#include "tf_weapon_dragons_fury.h"
#ifdef GAME_DLL
#include "iscorer.h"
#endif

// Client specific.
#ifdef CLIENT_DLL
#define CTFCFlame C_TFCFlame
#endif

class CTFCFlame : public CTFBaseRocket
{
public:
	DECLARE_CLASS( CTFCFlame, CTFBaseRocket );
	DECLARE_DATADESC();
	DECLARE_NETWORKCLASS();

	CTFCFlame();
	~CTFCFlame();

#ifdef GAME_DLL

	static CTFCFlame *Create( CBaseEntity *pWeapon, const Vector &vecOrigin, const QAngle &vecAngles, CBaseEntity *pOwner = NULL, CBaseEntity *pScorer = NULL );
	virtual void	Spawn();
	virtual void	Precache();

	void ExpireDelayThink( void );

	// IScorer interface
	virtual CBasePlayer *GetScorer( void );
	virtual CBasePlayer *GetAssistant( void ) { return NULL; }

	virtual int		GetWeaponID( void ) const	{ return TF_WEAPON_TFC_FLAMETHROWER; }
	float			GetDamageRadius( void ) const;
	float			GetFireballScale( void ) const;

	void	SetScorer( CBaseEntity *pScorer );

	void	SetCritical( bool bCritical ) { m_bCritical = bCritical; }
	virtual int		GetDamageType();

	virtual bool IsDeflectable() { return true; }
	virtual void Deflected( CBaseEntity *pDeflectedBy, Vector &vecDir );

	// Overrides.
	virtual void	Explode( trace_t *pTrace, CBaseEntity *pOther );

	virtual void	RocketTouch( CBaseEntity *pOther );
#else

	virtual void	OnDataChanged( DataUpdateType_t updateType );
	virtual void	CreateTrails( void );
	virtual void	CreateLightEffects( void );

#endif

private:
	Vector						m_vecInitialPos;		// position the flame was fired from
	Vector						m_vecPrevPos;			// position from previous frame
	CHandle<CTFWeaponBase>		m_hLauncher;			// weapon that fired this flame
#ifdef GAME_DLL
	CUtlVector<EHANDLE>			m_hEntitiesBurnt;		// list of entities this flame has burnt

	CBaseHandle m_Scorer;
	CNetworkVar( bool,	m_bCritical );

	void Burn( CBaseEntity *pOther );
	//void SetHitTarget( void );
#else
	bool		m_bCritical;
#endif

};

#endif //TF_TFC_FLAME_H