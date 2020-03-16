//=============================================================================//
//
// Purpose: Arrow used by Huntsman.
//
//=============================================================================//

#ifndef TF_PROJECTILE_ARROW_H
#define TF_PROJECTILE_ARROW_H

#ifdef _WIN32
#pragma once
#endif

#include "tf_weaponbase_rocket.h"

#ifdef GAME_DLL
#include "iscorer.h"
#include "tf_player.h"
#endif

#ifdef CLIENT_DLL
#define CTFProjectile_Arrow C_TFProjectile_Arrow
#define CTFProjectile_HealingBolt C_TFProjectile_HealingBolt
#define CTFProjectile_GrapplingHook C_TFProjectile_GrapplingHook
#endif

extern const char *g_pszArrowModels[];

class CTFProjectile_Arrow : public CTFBaseRocket
{
public:
	DECLARE_CLASS( CTFProjectile_Arrow, CTFBaseRocket );
	DECLARE_NETWORKCLASS();
#ifdef GAME_DLL
	DECLARE_DATADESC();
#endif

	CTFProjectile_Arrow();
	~CTFProjectile_Arrow();

	void				SetType( int iType ) 				{ m_iProjectileType = iType; }
	void				SetFlameArrow( bool bFlame ) 		{ m_bArrowAlight = bFlame; }

	virtual int 		GetWeaponID( void ) const 			{ return TF_WEAPON_COMPOUND_BOW; }



#ifdef GAME_DLL
	static CTFProjectile_Arrow *Create( const Vector &vecOrigin, const QAngle &vecAngles, float flSpeed, float flGravity, ProjectileType_t eType, CBaseEntity *pOwner, CBaseEntity *pWeapon );
	virtual void				InitArrow( const QAngle &vecAngles, float flSpeed, float flGravity, ProjectileType_t eType, CBaseEntity *pOwner, CBaseEntity *pWeapon );

	// IScorer interface
	virtual CBasePlayer *GetScorer( void );
	virtual CBasePlayer *GetAssistant( void ) { return NULL; }

	virtual void		Precache( void );
	virtual void		Spawn( void );
	virtual void		RemoveThink( void ) 			{ UTIL_Remove( this ); }

	void				SetScorer( CBaseEntity *pScorer );

	void				SetCritical( bool bCritical ) 	{ m_bCritical = bCritical; }
	int					GetDamageType();

	virtual bool		IsDeflectable();
	virtual bool		IsDestroyable( void ) 				{ return false; }
	virtual void		Deflected( CBaseEntity *pDeflectedBy, Vector &vecDir );
	virtual void		IncremenentDeflected( void );

	virtual void		UpdateOnRemove( void );

	virtual bool		CanHeadshot( void );

	virtual int			GetProjectileType( void ) const { return m_iProjectileType; }
	virtual void		ArrowTouch( CBaseEntity *pOther );
	virtual bool		StrikeTarget( mstudiobbox_t *pBox, CBaseEntity *pTarget );
	virtual bool		CheckSkyboxImpact( CBaseEntity *pOther );
	virtual void		ImpactTeamPlayerOrNPC( CBaseCombatCharacter *pTarget ) {}
	void				BuildingHealingArrow( CBaseEntity *pTarget );

	virtual float		GetCollideWithTeammatesDelay( void ) const { return 0.10; }
	void				FlyThink( void );

	const char			*GetTrailParticleName( void );
	void				CreateTrail( void );
	void				RemoveTrail( void );

	void				FadeOut( int iTime );
	void				BreakArrow( void );

	virtual void		AdjustDamageDirection( CTakeDamageInfo const &info, Vector &vecDirection, CBaseEntity *pEntity );

	// Arrow attachment functions
	bool				PositionArrowOnBone( mstudiobbox_t *pbox, CBaseAnimating *pAnim );
	void				GetBoneAttachmentInfo( mstudiobbox_t *pbox, CBaseAnimating *pAnim, Vector &vecOrigin, QAngle &vecAngles, int &bone, int &iPhysicsBone );
	bool				CheckRagdollPinned( Vector const& vecOrigin, Vector const& vecDirection, int iBone, int iPhysBone, CBaseEntity *pEntity, int iHitGroup, int iVictim );

	virtual void		ImpactSound( const char *pszImpactSound, bool bIsPlayerImpact = false );

	int 				GetArrowSkin( void ) const;
#else
	virtual void		ClientThink( void );
	virtual void		OnDataChanged( DataUpdateType_t updateType );
	void				CreateCritTrail( void );
	void				CheckNearMiss( void );

	virtual void   		NotifyBoneAttached( C_BaseAnimating* attachTarget );

	// Tell the object when to die
	void				SetDieTime( float flDieTime ) 	{ m_flDieTime = flDieTime; }

#endif

protected:
#ifdef GAME_DLL
	EHANDLE m_Scorer;

	CNetworkVar( bool, m_bCritical );
	CNetworkVar( bool, m_bArrowAlight );
	CNetworkVar( int, m_iProjectileType );

	bool m_bImpacted;

	float m_flTrailReflectLifetime;
	EHANDLE m_hSpriteTrail;
#else
	bool		m_bCritical;
	bool		m_bArrowAlight;
	int			m_iProjectileType;

	float		m_flDieTime;
	bool		m_bAttachment;
	bool		m_bWhizzed;
	float       m_flCheckNearMiss;
	CNewParticleEffect *m_pCritEffect;
	int         m_iDeflectedParity;
#endif
};

class CTFProjectile_HealingBolt : public CTFProjectile_Arrow
{
	DECLARE_CLASS( CTFProjectile_HealingBolt, CTFProjectile_Arrow );
	DECLARE_NETWORKCLASS();
public:

	CTFProjectile_HealingBolt();
	~CTFProjectile_HealingBolt();

#ifdef GAME_DLL
	virtual float		GetCollideWithTeammatesDelay( void ) const { return 0.0; }

	virtual void		InitArrow( const QAngle &vecAngles, float flSpeed, float flGravity, ProjectileType_t eType, CBaseEntity *pOwner, CBaseEntity *pWeapon );

	virtual bool		CanHeadshot( void );
	virtual void		ImpactTeamPlayerOrNPC( CBaseCombatCharacter *pTarget );
#endif
};

class CTFProjectile_GrapplingHook : public CTFProjectile_Arrow
{
	DECLARE_CLASS( CTFProjectile_GrapplingHook, CTFProjectile_Arrow );
	DECLARE_NETWORKCLASS();
public:

	CTFProjectile_GrapplingHook();
	~CTFProjectile_GrapplingHook();
};

#endif // TF_PROJECTILE_ARROW_H
