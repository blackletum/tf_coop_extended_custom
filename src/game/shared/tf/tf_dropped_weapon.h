//====== Copyright © 1996-2019, Valve Corporation, All rights reserved. =======//
//
// Purpose: Gun Mettle dropped weapon
//
//=============================================================================//
#ifndef TF_DROPPED_WEAPON
#define TF_DROPPED_WEAPON
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include "tf_weaponbase.h"

#ifdef CLIENT_DLL
#include "glow_outline_effect.h"
#include "c_baseanimating.h"
#include "c_tf_player.h"
#include "soundenvelope.h"
#else
#include "baseanimating.h"
#include "tf_player.h"
#endif

#ifdef CLIENT_DLL
#define CTFDroppedWeapon C_TFDroppedWeapon
#endif

DECLARE_AUTO_LIST( IDroppedWeaponAutoList )
class CTFDroppedWeapon : public CBaseAnimating, public IDroppedWeaponAutoList
{
public:

	DECLARE_CLASS( CTFDroppedWeapon, CBaseAnimating );
	DECLARE_NETWORKCLASS();

	CTFDroppedWeapon();
	~CTFDroppedWeapon();

	virtual void	Spawn( void );

	void	SetItem( CEconItemView *pItem ){ m_Item = *pItem; }
#ifdef CLIENT_DLL

	virtual void	OnDataChanged( DataUpdateType_t type );

	void	ClientThink();
	void	UpdateGlowEffect();

	virtual bool	IsVisibleToTargetID( void ) const { return true; }
	const char		*GetWeaponName( void );
#else
	void	Precache( void );

	virtual void	Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	virtual void	EndTouch( CBaseEntity *pOther );
	virtual int		ObjectCaps() { return BaseClass::ObjectCaps() | FCAP_WCEDIT_POSITION; };

	void	RemovalThink( void );
	float	GetCreationTime( void ) { return m_flCreationTime; }
	void	SetClip( int iClip ) { m_iClip = iClip; }
	void	SetAmmo( int iAmmo ) { m_iAmmo = iAmmo; }
	void	SetMaxAmmo( int iAmmo ) { m_iMaxAmmo = iAmmo; }

	static CTFDroppedWeapon *Create( const Vector &vecOrigin, const QAngle &vecAngles, CBaseEntity *pOwner, CTFWeaponBase *pWeapon );
#endif
private:
#ifdef CLIENT_DLL
	CGlowObject *m_pGlowEffect;
	bool m_bShouldGlow;

	int m_iAmmo;
	int m_iMaxAmmo;

	// Looping sound emitted by dropped flamethrowers
	CSoundPatch *m_pPilotLightSound;
#else
	float m_flCreationTime;
	float m_flRemoveTime;

	int m_iClip;
	CNetworkVar( int, m_iAmmo );
	CNetworkVar( int, m_iMaxAmmo );
#endif
	CEconItemView m_Item;
};

#endif