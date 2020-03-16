//====== Copyright © 1996-2013, Valve Corporation, All rights reserved. =======//
//
// Purpose: Base class for Mannpower powerups 
//
//=============================================================================//

#ifndef ENTITY_RUNE_H
#define ENTITY_RUNE_H

#ifdef _WIN32
#pragma once
#endif

#include "tf_powerup.h"
#include "tf_shareddefs.h"

#define TF_MODEL_RUNE_STRENGTH	"models/pickups/pickup_powerup_strength.mdl"
#define TF_MODEL_RUNE_HASTE		"models/pickups/pickup_powerup_haste.mdl"
#define TF_MODEL_RUNE_REGEN		"models/pickups/pickup_powerup_regen.mdl"
#define TF_MODEL_RUNE_RESIST	"models/pickups/pickup_powerup_defense.mdl"
#define TF_MODEL_RUNE_VAMPIRE	"models/pickups/pickup_powerup_vampire.mdl"
#define TF_MODEL_RUNE_WARLOCK	"models/pickups/pickup_powerup_reflect.mdl"
#define TF_MODEL_RUNE_PRECISION	"models/pickups/pickup_powerup_precision.mdl"
#define TF_MODEL_RUNE_AGILITY	"models/pickups/pickup_powerup_agility.mdl"
#define TF_MODEL_RUNE_KNOCKOUT	"models/pickups/pickup_powerup_knockout.mdl"
#define TF_MODEL_RUNE_KING		"models/pickups/pickup_powerup_king.mdl"
#define TF_MODEL_RUNE_PLAGUE	"models/pickups/pickup_powerup_plague.mdl"
#define TF_MODEL_RUNE_SUPERNOVA	"models/pickups/pickup_powerup_supernova.mdl"

//=============================================================================

class CTFRune : public CTFPowerup
{
public:
	DECLARE_CLASS( CTFRune, CTFPowerup );
	DECLARE_DATADESC();

	CTFRune();

	void	Spawn( void );
	void	Precache( void );
	void	Materialize( void );
	bool	MyTouch( CBasePlayer *pPlayer );
	bool	NPCTouch( CAI_BaseNPC *pNPC );

	virtual const char *GetDefaultPowerupModel( void ) { return TF_MODEL_RUNE_STRENGTH; }

	static CTFRune *CreateRune( const Vector &vecOrigin, RuneTypes_t eRuneType, int iTeam, bool bSomething = false, bool bMorething = false, Vector vecSomething = vec3_origin );

	void	BlinkThink( void );
	void	RepositionRune( RuneTypes_t eRuneType, int nNumber );

protected:
	RuneTypes_t				m_eRuneType;
};

//=============================================================================

class CTFRuneTemp : public CTFRune
{
public:
	DECLARE_CLASS( CTFRuneTemp, CTFRune );
	DECLARE_DATADESC();

	CTFRuneTemp();
	virtual void	Spawn( void );
	CBaseEntity* Respawn( void );
	float	GetRespawnDelay( void );

private:
	CNetworkVar( float, m_flRespawnTime );
	CNetworkVar( float, m_flRespawnAtTime );
};

//=============================================================================

class CTFRuneTempCrit : public CTFRuneTemp
{
public:
	DECLARE_CLASS( CTFRuneTempCrit, CTFRuneTemp );
	DECLARE_DATADESC();

	CTFRuneTempCrit();

	virtual bool	MyTouch( CBasePlayer *pPlayer );
};

//=============================================================================

class CTFRuneTempUber : public CTFRuneTemp
{
public:
	DECLARE_CLASS( CTFRuneTempUber, CTFRuneTemp );
	DECLARE_DATADESC();

	CTFRuneTempUber();

	virtual bool	MyTouch( CBasePlayer *pPlayer );
};

//=============================================================================

class CTFRuneCustom : public CTFRune
{
public:
	DECLARE_CLASS( CTFRuneCustom, CTFRune );
	DECLARE_DATADESC();

	CTFRuneCustom();
	void	Precache( void );
	virtual bool	MyTouch( CBasePlayer *pPlayer );

	float	GetEffectDuration( void ) { return m_flEffectDuration; }
	void	SetEffectDuration( float flTime ) { m_flEffectDuration = flTime; }

	virtual int	GetCondition( void ) { return m_iPowerupCondition; }

	virtual const char *GetDefaultPowerupModel( void ) { return TF_MODEL_RUNE_STRENGTH; }

	string_t	m_strPickupSound;
protected:
	float		m_flEffectDuration;
private:
	int		m_iPowerupCondition;
};

DECLARE_AUTO_LIST( IInfoPowerupSpawnAutoList );

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
class CTFInfoPowerupSpawn : public CBaseEntity, public IInfoPowerupSpawnAutoList
{
public:
	DECLARE_CLASS( CTFInfoPowerupSpawn, CBaseEntity );
	DECLARE_DATADESC();

	CTFInfoPowerupSpawn();
	~CTFInfoPowerupSpawn();

	void	Spawn( void );
};
#endif // ENTITY_RUNE_H
