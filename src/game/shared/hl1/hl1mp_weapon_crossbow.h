//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: RPG
//
//=============================================================================//


#ifndef HL1MP_WEAPON_CROSSBOW_H
#define HL1MP_WEAPON_CROSSBOW_H
#ifdef _WIN32
#pragma once
#endif


#include "hl1mp_basecombatweapon_shared.h"

#ifdef GAME_DLL
#include "basecombatcharacter.h"
#endif

#ifdef CLIENT_DLL
#define CWeaponHL1Crossbow C_WeaponHL1Crossbow
#endif

#ifdef GAME_DLL
#define HL1_BOLT_AIR_VELOCITY	2000
#define HL1_BOLT_WATER_VELOCITY	1000

//-----------------------------------------------------------------------------
// Crossbow Bolt
//-----------------------------------------------------------------------------
class CCrossbowBoltHL1 : public CBaseCombatCharacter
{
	DECLARE_CLASS( CCrossbowBoltHL1, CBaseCombatCharacter );

public:
	CCrossbowBoltHL1()
    {
        m_bExplode = true;
    }

	Class_T Classify( void ) { return CLASS_NONE; }

public:
	void Spawn( void );
	void Precache( void );
	void BubbleThink( void );
	void BoltTouch( CBaseEntity *pOther );
    void ExplodeThink( void );
    void SetExplode( bool bVal ) { m_bExplode = bVal; }

	static CCrossbowBoltHL1 *BoltCreate( const Vector &vecOrigin, const QAngle &angAngles, CBasePlayer *pentOwner = NULL );

	DECLARE_DATADESC();

private:
    bool m_bExplode;
};
#endif

//-----------------------------------------------------------------------------
// CWeaponHL1Crossbow
//-----------------------------------------------------------------------------
class CWeaponHL1Crossbow : public CBaseHL1MPCombatWeapon
{
	DECLARE_CLASS( CWeaponHL1Crossbow, CBaseHL1MPCombatWeapon );
public:

	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	CWeaponHL1Crossbow( void );

	void	Precache( void );
	void	PrimaryAttack( void );
	void	SecondaryAttack( void );
	bool	Reload( void );
	void	WeaponIdle( void );
	bool	Deploy( void );
	bool	Holster( CBaseCombatWeapon *pSwitchingTo = NULL );

//	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

#ifndef CLIENT_DLL
//	DECLARE_ACTTABLE();
#endif

private:
	void	FireBolt( void );
	void	ToggleZoom( void );

private:
//	bool	m_fInZoom;
	CNetworkVar( bool, m_fInZoom );
};

#endif	// HL1MP_WEAPON_CROSSBOW_H
