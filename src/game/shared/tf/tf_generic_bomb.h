//====== Copyright © 1996-2006, Valve Corporation, All rights reserved. =======//
//
// Purpose: 
//
//=============================================================================//
#ifndef TF_GENERIC_BOMB_H
#define TF_GENERIC_BOMB_H
#ifdef _WIN32
#pragma once
#endif


#include "tf_shareddefs.h"

// Client specific.
#ifdef CLIENT_DLL
#define CTFGenericBomb C_TFGenericBomb
#define CTFPumpkinBomb C_TFPumpkinBomb
#endif

//-----------------------------------------------------------------------------
// Purpose: This class is to get around the fact that DEFINE_FUNCTION doesn't like multiple inheritance
//-----------------------------------------------------------------------------
class CTFGenericBombShim : public CBaseAnimating
{
	virtual void GenericTouch( CBaseEntity *pOther ) = 0;

public:
	void Touch( CBaseEntity *pOther ) { return GenericTouch( pOther ); }
};

//=============================================================================
//
// TF Generic Bomb Class
//

DECLARE_AUTO_LIST( ITFGenericBomb );
class CTFGenericBomb : public CTFGenericBombShim, public ITFGenericBomb
{
public:
	DECLARE_CLASS( CTFGenericBomb, CBaseAnimating );
	DECLARE_NETWORKCLASS();
#ifdef GAME_DLL
	DECLARE_DATADESC();
#endif

	CTFGenericBomb();

	virtual void		Precache( void );
	virtual void		Spawn( void );
	
#ifdef GAME_DLL
	virtual void		Event_Killed( const CTakeDamageInfo &info );
	virtual void		InputDetonate( inputdata_t &inputdata );

	virtual int			GetCustomDamageType( void ) { return TF_DMG_CUSTOM_PUMPKIN_BOMB; }
#endif

	virtual void		GenericTouch( CBaseEntity *pOther );

protected:
#ifdef GAME_DLL
	float				m_flDamage;
	float				m_flRadius;
	bool				m_bFriendlyFire;

	string_t			m_iszParticleName;
	string_t			m_iszExplodeSound;

	COutputEvent		m_OnDetonate;
#endif
};

//-----------------------------------------------------------------------------
// Purpose: This class is to get around the fact that DEFINE_FUNCTION doesn't like multiple inheritance
//-----------------------------------------------------------------------------
class CTFPumpkinBombShim : public CBaseAnimating
{
	virtual void PumpkinTouch( CBaseEntity *pOther ) = 0;

public:
	void Touch( CBaseEntity *pOther ) { return PumpkinTouch( pOther ); }
};

DECLARE_AUTO_LIST( ITFPumpkinBomb );
class CTFPumpkinBomb : public CTFPumpkinBombShim, public ITFPumpkinBomb
{
public:
	DECLARE_CLASS( CTFPumpkinBomb, CBaseAnimating );
	DECLARE_NETWORKCLASS();

	CTFPumpkinBomb();

	virtual void	Precache( void );
	virtual void	Spawn( void );

#ifdef GAME_DLL
	virtual int		OnTakeDamage( CTakeDamageInfo const &info );
	virtual void	Event_Killed( CTakeDamageInfo const &info );

	void			RemovePumpkin( void );
	void			Break();

	virtual int		GetCustomDamageType( void ) { return TF_DMG_CUSTOM_PUMPKIN_BOMB; }

	bool	m_bSpell;
#endif

	virtual void	PumpkinTouch( CBaseEntity *pOther );

private:
#ifdef GAME_DLL
	bool	m_bKilled;
	bool	m_bPrecached;
	int		m_iTeam;
	float	m_flDamage;
	float	m_flScale;
	float	m_flRadius;
	float	m_flLifeTime;
#endif
};

#endif // TF_GENERIC_BOMB_H