//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef HL1_BASEGRENADE_H
#define HL1_BASEGRENADE_H
#ifdef _WIN32
#pragma once
#endif


#include "basegrenade_shared.h"


class CHL1BaseGrenade : public CBaseGrenade
{
	DECLARE_CLASS( CHL1BaseGrenade, CBaseGrenade );
	DECLARE_DATADESC();
public:

	CHL1BaseGrenade( void );

	virtual void Precache();

	void Explode( trace_t *pTrace, int bitsDamageType );
	unsigned int	PhysicsSolidMaskForEntity( void ) const;

	int m_iDeflected;
	CHandle< CBaseEntity >	m_hDeflectOwner;

	virtual bool			IsDeflectable() { return true; }
	virtual void			Deflected( CBaseEntity *pDeflectedBy, Vector &vecDir );
	virtual void			IncremenentDeflected( void );
};

class CHandGrenade : public CHL1BaseGrenade
{
public:
	DECLARE_CLASS( CHandGrenade, CHL1BaseGrenade );
	DECLARE_DATADESC();

	void	Spawn( void );
	void	Precache( void );
	void	BounceSound( void );
	void	BounceTouch( CBaseEntity *pOther );

	void ShootTimed( CBaseCombatCharacter *pOwner, Vector vecVelocity, float flTime );
};

#endif // HL1_BASEGRENADE_H
