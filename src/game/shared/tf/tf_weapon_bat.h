//====== Copyright © 1996-2005, Valve Corporation, All rights reserved. =======
//
// Purpose: 
//
//=============================================================================

#ifndef TF_WEAPON_BAT_H
#define TF_WEAPON_BAT_H
#ifdef _WIN32
#pragma once
#endif

#include "tf_weaponbase_melee.h"
#include "tf_weaponbase_grenadeproj.h"
#ifdef GAME_DLL
#include "iscorer.h"
#endif

#ifdef CLIENT_DLL
#define CTFBat C_TFBat
#define CTFBat_Wood C_TFBat_Wood
#define CTFBat_Giftwrap C_TFBat_Giftwrap
#define CTFBat_Fish C_TFBat_Fish
#define CTFStunBall C_TFStunBall
#endif

#define TF_STUNBALL_VIEWMODEL "models/weapons/v_models/v_baseball.mdl"

//=============================================================================
//
// Bat class.
//
class CTFBat : public CTFWeaponBaseMelee
{
public:

	DECLARE_CLASS( CTFBat, CTFWeaponBaseMelee );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	CTFBat();
	virtual int			GetWeaponID( void ) const			{ return TF_WEAPON_BAT; }

private:

	CTFBat( const CTFBat & ) {}
};

//=============================================================================
//
// Wood Bat class.
//
class CTFBat_Wood : public CTFWeaponBaseMelee
{
public:

	DECLARE_CLASS( CTFBat_Wood, CTFWeaponBaseMelee );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	CTFBat_Wood();

	virtual void		Precache( void );
	virtual int			GetWeaponID( void ) const					{ return TF_WEAPON_BAT_WOOD; }

	virtual bool		Deploy( void );
	virtual void		PrimaryAttack( void );
	virtual void		SecondaryAttack( void );
	virtual void		ItemPostFrame( void );

	virtual bool		HasChargeBar( void )						{ return true; }
	virtual const char* GetEffectLabelText( void )					{ return "#TF_Ball"; }
	virtual float		InternalGetEffectBarRechargeTime( void );

	virtual bool       	SendWeaponAnim( int iActivity );

	virtual bool		CanCreateBall( CTFPlayer *pPlayer );
	virtual bool	    PickedUpBall( CTFPlayer *pPlayer );

	CBaseEntity			*LaunchBall( CTFPlayer *pPlayer );
	virtual void		LaunchBallThink( void );

#ifdef CLIENT_DLL
	virtual void		CreateMove( float flInputSampleTime, CUserCmd *pCmd, const QAngle &vecOldViewAngles );

	virtual const char	*GetStunballViewmodel( void )				{ return ( m_bHasBall ? TF_STUNBALL_VIEWMODEL : NULL_STRING ); }

private:
	bool m_bHasBall;
#endif

private:
	CTFBat_Wood( const CTFBat_Wood & ) {}

	// prediction
	CNetworkVar( float, m_flNextFireTime );
	CNetworkVar( bool, m_bFiring );
};

#ifdef GAME_DLL
class CTFStunBall : public CTFWeaponBaseGrenadeProj, public IScorer
#else
class C_TFStunBall : public C_TFWeaponBaseGrenadeProj
#endif
{
public:
	DECLARE_CLASS( CTFStunBall, CTFWeaponBaseGrenadeProj );
	DECLARE_NETWORKCLASS();
#ifdef GAME_DLL
	DECLARE_DATADESC();
#endif

	CTFStunBall();
	~CTFStunBall();

	virtual int			GetWeaponID( void ) const 	{ return TF_WEAPON_BAT_WOOD; }

#ifdef GAME_DLL
	static CTFStunBall 	*Create( CBaseEntity *pWeapon, const Vector &vecOrigin, const QAngle &vecAngles, const Vector &vecVelocity, CBaseCombatCharacter *pOwner, CBaseEntity *pScorer, const AngularImpulse &angVelocity, const CTFWeaponInfo &weaponInfo );

	// IScorer interface
	virtual CBasePlayer *GetScorer( void ) 			{ return NULL; }
	virtual CBasePlayer *GetAssistant( void );

	virtual void	Precache( void );
	virtual void	Spawn( void );

	virtual int		GetDamageType();

	virtual void	Detonate( void );
	virtual void	VPhysicsCollision( int index, gamevcollisionevent_t *pEvent );

	void			StunBallTouch( CBaseEntity *pOther );
	const char		*GetTrailParticleName( void );
	void			CreateTrail( void );

	void			SetScorer( CBaseEntity *pScorer );

	void			SetCritical( bool bCritical )	{ m_bCritical = bCritical; }

	virtual bool	IsDeflectable() 				{ return true; }
	virtual void	Deflected( CBaseEntity *pDeflectedBy, Vector &vecDir );

	virtual bool	IsDestroyable( void ) 			{ return false; }

	virtual void	Explode( trace_t *pTrace, int bitsDamageType );

	bool			CanStun( CBaseEntity *pOther );

	virtual void	OnPhysGunDrop( CBasePlayer *pPhysGunUser, PhysGunDrop_t reason );
#else
	virtual void	OnDataChanged( DataUpdateType_t updateType );
	virtual void	CreateTrails( void );
	virtual int		DrawModel( int flags );
#endif

private:
#ifdef GAME_DLL
	CNetworkVar( bool, m_bCritical );

	CHandle<CBaseEntity>	m_hEnemy;
	EHANDLE					m_Scorer;
	EHANDLE					m_hSpriteTrail;
#else
	bool					m_bCritical;
#endif

	float					m_flCreationTime;
};

//=============================================================================
//
// Wood Giftwarp class.
//
class CTFBat_Giftwrap : public CTFBat_Wood
{
public:

	DECLARE_CLASS( CTFBat_Giftwrap, CTFBat_Wood );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	virtual int			GetWeaponID( void ) const			{ return TF_WEAPON_BAT_GIFTWARP; }
	//CBaseEntity			*LaunchBall( CTFPlayer *pPlayer );
};

//=============================================================================
//
// Fish Bat class.
//
class CTFBat_Fish : public CTFBat
{
public:

	DECLARE_CLASS( CTFBat_Fish, CTFBat );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	virtual int			GetWeaponID( void ) const			{ return TF_WEAPON_BAT_FISH; }
};

#endif // TF_WEAPON_BAT_H
