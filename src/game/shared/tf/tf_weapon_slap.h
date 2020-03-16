//====== Copyright ? 1996-2020, Valve Corporation, All rights reserved. =======
//
// Purpose: 
//
//=============================================================================

#ifndef TF_WEAPON_SLAP_H
#define TF_WEAPON_SLAP_H
#ifdef _WIN32
#pragma once
#endif

#include "tf_weaponbase_melee.h"

#ifdef CLIENT_DLL
#define CTFSlap C_TFSlap
#endif

//=============================================================================
//
// SLAP class.
//
class CTFSlap : public CTFWeaponBaseMelee
{
public:

	DECLARE_CLASS( CTFSlap, CTFWeaponBaseMelee );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	CTFSlap();
	~CTFSlap();

	//virtual void		Precache();
	virtual int			GetWeaponID( void ) const			{ return TF_WEAPON_SLAP; }
	virtual void		PrimaryAttack( void );
	virtual void		Slap( void );
	//virtual void		SecondaryAttack( void );

	//virtual bool Deploy( void );
	//virtual bool Holster( CBaseCombatWeapon *pSwitchingTo = NULL );

	//virtual const char *GetShootSound( int iIndex ) const;
	//virtual bool	HideWhileStunned( void );

	//void	PlaySwingSound( void );

	//virtual void	SendPlayerAnimEvent( CTFPlayer *pPlayer );

	//virtual void	Smack( void );

	virtual void	OnEntityHit( CBaseEntity *pEntity, CTakeDamageInfo *pInfo );

#ifdef GAME_DLL
	virtual void	OnPlayerKill( CBaseEntity *pVictim, const CTakeDamageInfo &info );
#endif
private:

	CNetworkVar( bool, m_bFirstHit );
	CNetworkVar( int, m_nNumKills );

	CTFSlap( const CTFSlap & ) {}
};

#endif // TF_WEAPON_SLAP_H