//====== Copyright © 1996-2005, Valve Corporation, All rights reserved. =======
//
// Purpose: 
//
//=============================================================================

#ifndef TF_WEAPON_MEDIGUN_H
#define TF_WEAPON_MEDIGUN_H
#ifdef _WIN32
#pragma once
#endif

#include "tf_weaponbase_gun.h"

#if defined( CLIENT_DLL )
#define CWeaponMedigun C_WeaponMedigun
#define CTFMedigunShield C_TFMedigunShield
#endif

#define MAX_HEALING_TARGETS			1	//6

#define CLEAR_ALL_TARGETS			-1

#ifdef CLIENT_DLL
void RecvProxy_HealingTarget( const CRecvProxyData *pData, void *pStruct, void *pOut );
#endif

static const char *s_pszMedigunHealTargetThink = "MedigunHealTargetThink";


//=============================================================================
//
//
//
//=============================================================================
class CTFMedigunShield : public CBaseAnimating
{
public:
	DECLARE_CLASS( CTFMedigunShield, CBaseAnimating );
	DECLARE_NETWORKCLASS();

	CTFMedigunShield();
	~CTFMedigunShield();

	virtual void	Spawn();
	virtual void	Precache();

	virtual bool	IsCombatItem( void ) const { return true; }

	virtual	bool	ShouldCollide( int collisionGroup, int contentsMask ) const;

	virtual void	UpdateShieldPosition( void );

	unsigned int	PhysicsSolidMaskForEntity( void ) const;
#ifdef CLIENT_DLL
	virtual void	ClientThink();
#else

	virtual void	StartTouch( CBaseEntity *pOther );
	void			ShieldTouch( CBaseEntity *pOther );
	virtual void	EndTouch( CBaseEntity *pOther );

	virtual int		OnTakeDamage( const CTakeDamageInfo &inputInfo );
	virtual void	TraceAttack( const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr, CDmgAccumulator *pAccumulator );
	
	void			ShieldThink( void );

	static CTFMedigunShield *Create( CTFPlayer *pOwner );

	DECLARE_DATADESC();
#endif
private:

#ifdef CLIENT_DLL
	CSoundPatch				*m_pShieldSound;
#endif
};


//=========================================================
// Beam healing gun
//=========================================================
class CWeaponMedigun : public CTFWeaponBaseGun
{
	DECLARE_CLASS( CWeaponMedigun, CTFWeaponBaseGun );
public:
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	CWeaponMedigun( void );
	~CWeaponMedigun( void );

	virtual void	Precache();

	virtual bool	Deploy( void );
	virtual bool	Holster( CBaseCombatWeapon *pSwitchingTo );
	virtual void	UpdateOnRemove( void );
	virtual void	ItemHolsterFrame( void );
	virtual void	ItemPostFrame( void );
	virtual bool	Lower( void );
	virtual void	PrimaryAttack( void );
	virtual void	SecondaryAttack( void );
	virtual void	WeaponIdle( void );
	void			DrainCharge( void );
	void			AddCharge( float flAmount );
	void			SetCharge( float flAmount );
	void			SubtractCharge( float flAmount );
	void			SubtractChargeAndUpdateDeployState( float flAmount, bool bSomething );
	void			SetEnergyMeter( float flAmount );
	virtual void	WeaponReset( void );

	virtual float	GetTargetRange( void );
	virtual float	GetStickRange( void );
	virtual float	GetHealRate( void );
	virtual float	GetMinChargeAmount( void ) const;
	virtual bool	AppliesModifier( void ) { return true; }
	int				GetMedigunType( void ) const;
	int				GetResistType( void ) const;

	virtual int		GetWeaponID( void ) const			{ return TF_WEAPON_MEDIGUN; }

	bool			IsReleasingCharge( void ) const { return (m_bChargeRelease && !m_bHolstered); }
	medigun_charge_types GetChargeType( void ) const;

	CBaseEntity		*GetHealTarget( void ) { return m_hHealingTarget.Get(); }

	const char		*GetHealSound( void );

	void			CycleResistType( void );

#if defined( CLIENT_DLL )
	// Stop all sounds being output.
	void			StopHealSound( bool bStopHealingSound = true, bool bStopNoTargetSound = true );

	virtual void	OnDataChanged( DataUpdateType_t updateType );
	virtual void	ClientThink();
	void			UpdateEffects( void );
	void			ForceHealingTargetUpdate( void ) { m_bUpdateHealingTargets = true; }

	void			ManageChargeEffect( void );
#else

	void			HealTargetThink( void );

	void			CreateMedigunShield( void );

	bool			IsAttachedToBuilding( void ) const;
#endif

	float			GetChargeLevel( void ) const { return m_flChargeLevel; }
	float			GetEnergyLevel( void ) const { return m_flEnergyLevel; }

	virtual float	GetEffectBarProgress( void );
	virtual const char *GetEffectLabelText( void ) { return "#TF_Rescue"; }
private:
	bool					FindAndHealTargets( void );
	virtual bool			HealingTarget( CBaseEntity *pTarget );
	bool					CouldHealTarget( CBaseEntity *pTarget );
	bool					AllowedToHealTarget( CBaseEntity *pTarget );

protected:
	virtual void			RemoveHealingTarget( bool bStopHealingSelf = false );
	virtual void			MaintainTargetInSlot();
	virtual void			FindNewTargetForSlot();


public:

#ifdef GAME_DLL
	CNetworkHandle( CBaseEntity, m_hHealingTarget );
#else
	CNetworkHandle( C_BaseEntity, m_hHealingTarget );
#endif

protected:
	// Networked data.
	CNetworkVar( bool,		m_bHealing );
	CNetworkVar( bool,		m_bAttacking );
	CNetworkVar( bool,		m_bShieldActive );

	double					m_flNextBuzzTime;
	float					m_flHealEffectLifetime;	// Count down until the healing effect goes off.
	float					m_flReleaseStartedAt;

	CNetworkVar( bool,		m_bHolstered );
	CNetworkVar( bool,		m_bChargeRelease );
	CNetworkVar( float,		m_flChargeLevel );
	CNetworkVar( float,		m_flEnergyLevel );

	float					m_flNextTargetCheckTime;
	bool					m_bCanChangeTarget; // used to track the PrimaryAttack key being released for AutoHeal mode
	
#ifdef GAME_DLL
	CDamageModifier			m_DamageModifier;		// This attaches to whoever we're healing.
	bool					m_bHealingSelf;

	CHandle<CTFMedigunShield> m_hShield;
#endif

#ifdef CLIENT_DLL
	bool					m_bPlayingSound;
	bool					m_bUpdateHealingTargets;
	struct healingtargeteffects_t
	{
		EHANDLE				hOwner;
		C_BaseEntity		*pTarget;
		CNewParticleEffect	*pEffect;
		CNewParticleEffect  *pCustomEffect; // custom_particlesystem
	};
	healingtargeteffects_t m_hHealingTargetEffect;

	float					m_flFlashCharge;
	bool					m_bOldChargeRelease;

	CNewParticleEffect		*m_pChargeEffect;
	EHANDLE					m_hChargeEffectHost;
	CSoundPatch				*m_pChargedSound;
#endif

	CNetworkVar( int,		m_nChargeResistType );
private:														
	CWeaponMedigun( const CWeaponMedigun & );
};

// Now make sure there isn't something other than team players in the way.
class CMedigunFilter : public CTraceFilterSimple
{
public:
	CMedigunFilter(CBaseEntity *pShooter) : CTraceFilterSimple(pShooter, COLLISION_GROUP_WEAPON)
	{
		m_pShooter = pShooter;
	}

	virtual bool ShouldHitEntity(IHandleEntity *pHandleEntity, int contentsMask)
	{
		// If it hit an edict the isn't the target and is on our team, then the ray is blocked.
		CBaseEntity *pEnt = static_cast<CBaseEntity*>(pHandleEntity);

		// Ignore collisions with the shooter
		if (pEnt == m_pShooter)
			return false;

		if (pEnt->GetTeam() == m_pShooter->GetTeam())
			return false;

		return CTraceFilterSimple::ShouldHitEntity(pHandleEntity, contentsMask);
	}

	CBaseEntity	*m_pShooter;
};

#endif // TF_WEAPON_MEDIGUN_H
