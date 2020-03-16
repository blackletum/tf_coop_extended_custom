//====== Copyright © 1996-2004, Valve Corporation, All rights reserved. =======
//
// Purpose: Shared player code.
//
//=============================================================================
#ifndef TF_PLAYER_SHARED_H
#define TF_PLAYER_SHARED_H
#ifdef _WIN32
#pragma once
#endif

#include "networkvar.h"
#include "tf_shareddefs.h"
#include "tf_weaponbase.h"
#include "basegrenade_shared.h"
#include "GameEventListener.h"

#ifdef GAME_DLL
#include "SpriteTrail.h"
#endif

// Client specific.
#ifdef CLIENT_DLL
class C_TFPlayer;
// Server specific.
#else
class CTFPlayer;
#endif

//=============================================================================
//
// Tables.
//

// Client specific.
#ifdef CLIENT_DLL

	EXTERN_RECV_TABLE( DT_TFPlayerShared );

// Server specific.
#else

	EXTERN_SEND_TABLE( DT_TFPlayerShared );

#endif


//=============================================================================
// Damage storage for crit multiplier calculation
class CTFDamageEvent
{
	DECLARE_EMBEDDED_NETWORKVAR()
#ifdef CLIENT_DLL
	DECLARE_CLIENTCLASS_NOBASE();
#else
	DECLARE_SERVERCLASS_NOBASE();
#endif

public:
	float flDamage;
	float flTime;
	bool bKill;
};

//=============================================================================
//
// Shared player class.
//
class CTFPlayerShared
{
public:

// Client specific.
#ifdef CLIENT_DLL

	friend class C_TFPlayer;
	typedef C_TFPlayer OuterClass;
	DECLARE_PREDICTABLE();

// Server specific.
#else

	friend class CTFPlayer;
	typedef CTFPlayer OuterClass;

#endif

	DECLARE_EMBEDDED_NETWORKVAR()
	DECLARE_CLASS_NOBASE( CTFPlayerShared );

	// Initialization.
	CTFPlayerShared();
	void Init( OuterClass *pOuter );

	// State (TF_STATE_*).
	int		GetState() const					{ return m_nPlayerState; }
	void	SetState( int nState )				{ m_nPlayerState = nState; }
	bool	InState( int nState )				{ return ( m_nPlayerState == nState ); }

	// Condition (TF_COND_*).
	int		GetCond() const						{ return m_nPlayerCond; }
	void	SetCond( int nCond )				{ m_nPlayerCond = nCond; }
	void	AddCond( int nCond, float flDuration = PERMANENT_CONDITION );
	void	RemoveCond( int nCond );
	bool	InCond( int nCond );
	//bool	WasInCond( int nCond ) const
	void	RemoveAllCond( void );
	void	OnConditionAdded( int nCond );
	void	OnConditionRemoved( int nCond );
	void	ConditionThink( void );
	float	GetConditionDuration( int nCond );

	bool	IsCritBoosted( void );
	bool	IsMiniCritBoosted( void );
	bool	IsInvulnerable( void );
	bool	IsStealthed( void );
	bool	IsJared( void );
	bool	IsSpeedBoosted( void );
	bool	IsBuffed( void );

	void	ConditionGameRulesThink( void );

	void	InvisibilityThink( void );

	int		GetMaxBuffedHealth( void );

	// Max Health
	void	SetMaxHealth( int iMaxHealth ) { m_iMaxHealth = iMaxHealth; }

#ifdef CLIENT_DLL
	// This class only receives calls for these from C_TFPlayer, not
	// natively from the networking system
	virtual void OnPreDataChanged( void );
	virtual void OnDataChanged( void );

	// check the newly networked conditions for changes
	void	SyncConditions( int nCond, int nOldCond, int nUnused, int iOffset );

	void	ClientDemoBuffThink( void );
	void	ClientShieldChargeThink( void );
#endif

	void	Disguise( int nTeam, int nClass, CBaseEntity *pTarget = nullptr, bool b1 = true );
	void	CompleteDisguise( bool bDisguiseOnKill = false );
	void	RemoveDisguise( void );
	void	FindDisguiseTarget( void );
	void	SetOffHandWatch(void);
	int		GetDisguiseTeam( void )				{ return m_nDisguiseTeam; }
	int		GetDisguiseClass( void ) 			{ return m_nDisguiseClass; }
	int		GetMaskClass( void )				{ return m_nMaskClass; }
	int		GetDesiredDisguiseClass( void )		{ return m_nDesiredDisguiseClass; }
	int		GetDesiredDisguiseTeam( void )		{ return m_nDesiredDisguiseTeam; }
	EHANDLE GetDisguiseTarget( void ) 	
	{
#ifdef CLIENT_DLL
		if ( m_iDisguiseTargetIndex == TF_DISGUISE_TARGET_INDEX_NONE )
			return NULL;
		return cl_entitylist->GetNetworkableHandle( m_iDisguiseTargetIndex );
#else
		return m_hDisguiseTarget.Get();
#endif
	}
	int		GetDisguiseHealth( void )			{ return m_iDisguiseHealth; }
	void	SetDisguiseHealth( int iDisguiseHealth );
	int		AddDisguiseHealth( int iHealthToAdd, bool bOverheal = false );
	int		GetDisguiseMaxHealth( void )		{ return m_iDisguiseMaxHealth; }
	int		GetDisguiseMaxBuffedHealth( void );

	CEconItemView *GetDisguiseItem( void )			{ return &m_DisguiseItem; }
	void	RecalcDisguiseWeapon( int iSlot = 0 );

#ifdef CLIENT_DLL
	void	OnDisguiseChanged( void );
	int		GetDisguiseWeaponModelIndex( void ) { return m_iDisguiseWeaponModelIndex; }
	CTFWeaponInfo *GetDisguiseWeaponInfo( void );

	void	UpdateCritBoostEffect( bool bForceHide = false );
#endif

#ifdef GAME_DLL
	void	Heal( CTFPlayer *pPlayer, float flAmount, bool bDispenserHeal = false );
	void	StopHealing( CTFPlayer *pPlayer );
	void	RecalculateChargeEffects( bool bInstantRemove = false );
	EHANDLE GetHealerByIndex( int index );
	int		FindHealerIndex( CTFPlayer *pPlayer );
	EHANDLE	GetFirstHealer();
	void	HealthKitPickupEffects( int iAmount );

	// Jarate Player
	EHANDLE	m_hUrineAttacker;

	// Milk Player
	EHANDLE	m_hMilkAttacker;
#endif
	int		GetNumHealers( void ) { return m_nNumHealers; }

	void	Burn( CTFPlayer *pAttacker, CTFWeaponBase *pWeapon = NULL, float flFlameDuration = -1.0f );
	void	MakeBleed( CTFPlayer *pAttacker, CTFWeaponBase *pWeapon, float flBleedDuration, int iDamage );
	void	StunPlayer( float flDuration, float flSpeed, float flResistance, int nStunFlags, CTFPlayer *pStunner );

	void	Concussion( CTFPlayer *pAttacker, float flDuration );

	bool	IsControlStunned( void );

#ifdef GAME_DLL
	void	AddPhaseEffects( void );
	CUtlVector< CSpriteTrail * > m_pPhaseTrails;
#else
	CNewParticleEffect *m_pStun;
	CNewParticleEffect *m_pWarp;
	CNewParticleEffect *m_pSpeedTrails;
	CNewParticleEffect *m_pBuffAura;
	CNewParticleEffect *m_pSapped;
	CNewParticleEffect *m_pRadiusHeal;
	CNewParticleEffect *m_pRunePlague;
	CNewParticleEffect *m_pMarkForDeath;
	CNewParticleEffect *m_pMegaHeal;
#endif

	void	ForceRespawn( void );

	void	UpdatePhaseEffects( void );
	void	UpdateSpeedBoostEffects( void );

	void	RecalculatePlayerBodygroups( void );

	// Weapons.
	CTFWeaponBase *GetActiveTFWeapon() const;

	// Utility.
	bool	IsAlly( CBaseEntity *pEntity );

	bool	IsLoser( void );

	// Separation force
	bool	IsSeparationEnabled( void ) const	{ return m_bEnableSeparation; }
	void	SetSeparation( bool bEnable )		{ m_bEnableSeparation = bEnable; }
	const Vector &GetSeparationVelocity( void ) const { return m_vSeparationVelocity; }
	void	SetSeparationVelocity( const Vector &vSeparationVelocity ) { m_vSeparationVelocity = vSeparationVelocity; }

	void	FadeInvis( float flInvisFadeTime );
	float	GetPercentInvisible( void );
	void	NoteLastDamageTime( int nDamage );
	void	OnSpyTouchedByEnemy( void );
	float	GetLastStealthExposedTime( void ) { return m_flLastStealthExposeTime; }

	int		GetDesiredPlayerClassIndex( void );

	int		GetDesiredWeaponIndex( void ) { return m_iDesiredWeaponID; }
	void	SetDesiredWeaponIndex( int iWeaponID ) { m_iDesiredWeaponID = iWeaponID; }
	int		GetRespawnParticleID( void ) { return m_iRespawnParticleID; }
	void	SetRespawnParticleID(int iParticleID) { m_iRespawnParticleID = iParticleID; }

	bool	AddToSpyCloakMeter( float amt, bool bForce = false, bool bIgnoreAttribs = false );
	float	GetSpyCloakMeter() const { return m_flCloakMeter; }
	void	SetSpyCloakMeter( float val ) { m_flCloakMeter = val; }

	bool	IsJumping( void ) { return m_bJumping; }
	void	SetJumping( bool bJumping );
	void    SetAirDash( int iAirDash );
	int   	GetAirDash( void )	{ return m_iAirDash; }
	int		GetAirDucks( void ) { return m_nAirDucked; }
	void	IncrementAirDucks( void );
	void	ResetAirDucks( void );

	void	DebugPrintConditions( void );

	float	GetStealthNoAttackExpireTime( void );

	void	SetPlayerDominated( CTFPlayer *pPlayer, bool bDominated );
	bool	IsPlayerDominated( int iPlayerIndex );
	bool	IsPlayerDominatingMe( int iPlayerIndex );
	void	SetPlayerDominatingMe( CTFPlayer *pPlayer, bool bDominated );

	void	SetLivesCount( int iCount ) { m_iLives = iCount; }
	int		GetLivesCount( void ) { return m_iLives; }

	void InCutScene( bool bCutscene );
	bool IsInCutScene( void );

	bool	IsCarryingObject( void ) { return m_bCarryingObject; }

	virtual void FireGameEvent( IGameEvent *event );

#ifdef GAME_DLL
	void				SetCarriedObject( CBaseObject *pObj );
	CBaseObject*		GetCarriedObject( void );
#endif

	int		GetKillstreak( void ) { return m_nStreaks.Get( 0 ); }
	void	SetKillstreak( int iKillstreak ) { m_nStreaks.Set( 0, iKillstreak ); }
	void	IncKillstreak() { m_nStreaks.Set( 0, m_nStreaks.Get( 0 ) + 1 ); }

	int		GetStunPhase( void ) { return m_iStunPhase; }
	void	SetStunPhase( int iPhase ) { m_iStunPhase = iPhase; }
	float	GetStunExpireTime( void ) { return m_flStunExpireTime; }
	void	SetStunExpireTime( float flTime ) { m_flStunExpireTime = flTime; }

	int		GetStunFlags( void )				{ return m_nStunFlags; }

	int		GetTeleporterEffectColor( void ) { return m_nTeamTeleporterUsed; }
	void	SetTeleporterEffectColor( int iTeam ) { m_nTeamTeleporterUsed = iTeam; }
#ifdef CLIENT_DLL
	bool	ShouldShowRecentlyTeleported( void );
#endif

	int		GetSequenceForDeath( CBaseAnimating *pAnim, int iDamageCustom );

	// Vintage Banners
	void	UpdateRageBuffsAndRage( void );
	void	SetRageMeter( float flRagePercent, int iBuffType );
	void	ActivateRageBuff( CBaseEntity *pEntity, int iBuffType );
	void	PulseRageBuff( /*CTFPlayerShared::ERageBuffSlot*/ );
	void	SetRageActive( bool bSet )		{ m_bRageActive = bSet; }
	bool	IsRageActive( void ) { return m_bRageActive; }
	float	GetRageProgress( void ) { return m_flEffectBarProgress; }
	void	ResetRageSystem( void );

	float	GetScoutHypeMeter( void ) { return m_flHypeMeter; }
	void	SetScoutHypeMeter( float flPercent );

	void	UpdateFlashlightBattery( void );
	float	GetFlashlightBattery( void ) { return m_flFlashBattery; }
	void	SetFlashlightBattery( float flPercent ) { m_flFlashBattery = flPercent; }

	void	SetDefaultItemChargeMeters( void );
	void	SetItemChargeMeter( loadout_positions_t eSlot, float flPercent );

	void	SetRevengeCrits( int nKills );

	// Knights
	int		GetDecapitationCount( void ) const       { return m_iDecapitations; }
	void	SetDecapitationCount( int count )        { m_iDecapitations = count; }
	bool	HasDemoShieldEquipped( void ) const;
	void	SetDemoShieldEquipped( bool bEquipped )  { m_bShieldEquipped = bEquipped; }
	int		GetNextMeleeCrit( void ) const           { return m_iNextMeleeCrit; }
	void	SetNextMeleeCrit( int iType )            { m_iNextMeleeCrit = iType; }

	float	GetShieldChargeMeter( void ) const       { return m_flChargeMeter; }
	void	SetShieldChargeMeter( float flVal )      { m_flChargeMeter = flVal; }
	void	SetShieldChargeDrainRate( float flRate ) { m_flChargeDrainRate = flRate; }
	void	SetShieldChargeRegenRate( float flRate ) { m_flChargeRegenRate = flRate; }
	void	EndCharge( void );

	bool	HasParachuteEquipped( void ) const { return m_bParachuteEquipped; }
	void	SetParachuteEquipped( bool bEquipped )  { m_bParachuteEquipped = bEquipped; }

#ifdef GAME_DLL
	void	UpdateCloakMeter( void );
	void	UpdateChargeMeter( void );
	void	UpdateEnergyDrinkMeter( void );
	void	CalcChargeCrit( bool bForceFull );
#endif

	void	PulseKingRuneBuff( void );
	void	PulseMedicRadiusHeal( void );

#ifdef GAME_DLL
	void	SetCarryingRuneType( RuneTypes_t RuneType );
#endif
	RuneTypes_t			GetCarryingRuneType( void ) const;

	bool	CanRuneCharge( void ) const;
	float	GetRuneProgress( void ) { return m_flRuneCharge; }

	bool	CanFallStomp( void );

#ifdef GAME_DLL
	void	ApplyAttributeToPlayer( char const *pAttribute, float flValue );
	void	RemoveAttributeFromPlayer( char const *pAttribute );
#endif

	int		GetCurrency( void ) const			{ return m_nCurrency; }
	void	SetCurrency( int iAmount )			{ m_nCurrency = iAmount; }
	void	AddCurrency( int iAmount )			{ m_nCurrency += iAmount; }
	void	RemoveCurrency( int iAmount )		{ m_nCurrency -= iAmount; }

	void	SetInUpgradeZone( bool bIn ) { m_bInUpgradeZone = bIn; }
	bool	InUpgradeZone( void ) { return m_bInUpgradeZone; }

	CBaseEntity	*GetPasstimePassTarget( void ) const { return m_hPasstimePassTarget; }
	void	SetPasstimePassTarget( CBaseEntity *pTarget );
private:

	void ImpactWaterTrace( trace_t &trace, const Vector &vecStart );

	void OnAddStealthed( void );
	void OnAddInvulnerable( void );
	void OnAddTeleported( void );
	void OnAddBurning( void );
	void OnAddDisguising( void );
	void OnAddDisguised( void );
	void OnAddTaunting( void );
	void OnAddCritboosted( void );
	void OnAddStunned( void );
	void OnAddShieldCharge( void );
	void OnAddHalloweenGiant( void );
	void OnAddHalloweenTiny( void );
	void OnAddPhase( void );
	void OnAddSpeedBoost( bool bParticle );
	void OnAddUrine( void );
	void OnAddMadMilk( void );
	void OnAddCondGas( void );
	void OnAddCondParachute( void );
	void OnAddTeamGlows( void );
	void OnAddBleeding( void );
	void OnAddRune( void );
	void OnAddBuff( void );
	void OnAddFeignDeath( void );
	void OnAddSapped( void );
	void OnAddFlashlight( void );
	void OnAddNoclip( void );
	void OnAddPlague( void );
	void OnAddRadiusHeal( void );
	void OnAddRunePlague( void );
	void OnAddMegaHeal( void );
	void OnAddInPurgatory( void );
	void OnAddCutscene( void );
	void OnAddPowerPlay( void );
	void OnAddZombieSpawn( void );
	void OnAddZombieLeap( void );

	void OnRemoveZoomed( void );
	void OnRemoveBurning( void );
	void OnRemoveStealthed( void );
	void OnRemoveDisguised( void );
	void OnRemoveDisguising( void );
	void OnRemoveInvulnerable( void );
	void OnRemoveTeleported( void );
	void OnRemoveTaunting( void );
	void OnRemoveCritboosted( void );
	void OnRemoveStunned( void );
	void OnRemoveShieldCharge( void );
	void OnRemoveHalloweenGiant( void );
	void OnRemoveHalloweenTiny( void );
	void OnRemovePhase( void );
	void OnRemoveSpeedBoost( void );
	void OnRemoveUrine( void );
	void OnRemoveMadMilk( void );
	void OnRemoveCondGas( void );
	void OnRemoveCondParachute( void );
	void OnRemoveTeamGlows( void );
	void OnRemoveBleeding( void );
	void OnRemoveRune( void );
	void OnRemoveBuff( void );
	void OnRemoveFeignDeath( void );
	void OnRemoveSapped( void );
	void OnRemoveFlashlight( void );
	void OnRemoveNoclip( void );
	void OnRemoveRadiusHeal( void );
	void OnRemoveRunePlague( void );
	void OnRemoveMegaHeal( void );
	void OnRemoveInPurgatory( void );
	void OnRemoveCutscene( void );
	void OnRemovePowerPlay( void );
	void OnRemoveZombieSpawn( void );
	void OnRemoveZombieLeap( void );

	float	GetCritMult( void );

#ifdef GAME_DLL
	void	UpdateCritMult( void );
	void	RecordDamageEvent( const CTakeDamageInfo &info, bool bKill );
	void	ClearDamageEvents( void ) { m_DamageEvents.Purge(); }
	int		GetNumKillsInTime( float flTime );

	// Invulnerable.
	medigun_charge_types  GetChargeEffectBeingProvided( CTFPlayer *pPlayer );
	void	SetChargeEffect( medigun_charge_types chargeType, bool bShouldCharge, bool bInstantRemove, const MedigunEffects_t &chargeEffect, float flRemoveTime, CTFPlayer *pProvider );
	void	TestAndExpireChargeEffect( medigun_charge_types chargeType );
#endif
public:

	void	AddTempCritBonus( float flDuration = PERMANENT_CONDITION );

	void	CreatePingEffect( int nType );

	CNetworkVar( bool, m_bFeignDeathReady );

	CNetworkVar( bool, m_bIsPlayerADev );
	CNetworkVar( bool, m_bIsPlayerNicknine );

private:

	// Vars that are networked.
	CNetworkVar( int, m_nPlayerState );			// Player state.
	CNetworkVar( int, m_nPlayerCond );			// Player condition flags.
	/*CNetworkVar( float, m_flMovementStunTime );
	CNetworkVar( int, m_iMovementStunAmount );
	CNetworkVar( int, m_iMovementStunParity );*/
	CNetworkHandle( CTFPlayer, m_hStunner );
	CNetworkVar( float, m_flStunExpireTime );
	int m_iStunPhase;
	CNetworkVar( int, m_nStunFlags );
	CNetworkVar( float, m_flStunMovementSpeed );
	CNetworkVar( float, m_flStunResistance );
	// Ugh...
	CNetworkVar( int, m_nPlayerCondEx ); // 33-64
	CNetworkVar( int, m_nPlayerCondEx2 ); // 65-96
	CNetworkVar( int, m_nPlayerCondEx3 ); // 97-128
	CNetworkVar( int, m_nPlayerCondEx4 ); // 129-160
	CNetworkArray( float, m_flCondExpireTimeLeft, TF_COND_LAST ); // Time until each condition expires

//TFTODO: What if the player we're disguised as leaves the server?
//...maybe store the name instead of the index?
	CNetworkVar( int, m_nDisguiseTeam );		// Team spy is disguised as.
	CNetworkVar( int, m_nDisguiseClass );		// Class spy is disguised as.
	CNetworkVar( int, m_nMaskClass );			// Fake disguise class.
	EHANDLE m_hDisguiseTarget;					// Playing the spy is using for name disguise.
	CNetworkVar( int, m_iDisguiseTargetIndex );
	CNetworkVar( int, m_iDisguiseHealth );		// Health to show our enemies in player id
	CNetworkVar( int, m_iDisguiseMaxHealth );
	CNetworkVar( float, m_flDisguiseChargeLevel );
	CNetworkVar( int, m_nDesiredDisguiseClass );
	CNetworkVar( int, m_nDesiredDisguiseTeam );
	CEconItemView m_DisguiseItem;
	EHANDLE m_hForcedDisguise;

	CNetworkVector( m_vecPingOrigin );
	CNetworkHandle( CBaseEntity, m_hPingTarget );

	CNetworkHandle( CBaseEntity, m_hPasstimePassTarget );

	CNetworkVar( int, m_iMaxHealth );

	bool m_bEnableSeparation;		// Keeps separation forces on when player stops moving, but still penetrating
	Vector m_vSeparationVelocity;	// Velocity used to keep player seperate from teammates

	float m_flInvisibility;
	CNetworkVar( float, m_flInvisChangeCompleteTime );		// when uncloaking, must be done by this time
	float m_flLastStealthExposeTime;

	CNetworkVar( int, m_nNumHealers );

	// Vars that are not networked.
	OuterClass			*m_pOuter;					// C_TFPlayer or CTFPlayer (client/server).

	bool m_bRageActive;

#ifdef GAME_DLL
	// Healer handling
	struct healers_t
	{
		EHANDLE	pPlayer;
		float	flAmount;
		bool	bDispenserHeal;
		float	iRecentAmount;
		float	flNextNofityTime;
	};
	CUtlVector< healers_t >	m_aHealers;	
	float					m_flHealFraction;	// Store fractional health amounts
	float					m_flDisguiseHealFraction;	// Same for disguised healing

	float		m_flChargeOffTime[TF_CHARGE_COUNT];
	bool		m_bChargeSounds[TF_CHARGE_COUNT];
#endif

	// Burn handling
	CHandle<CTFPlayer>		m_hBurnAttacker;
	CHandle<CTFWeaponBase>	m_hBurnWeapon;
	CNetworkVar( int,		m_nNumFlames );
	float					m_flFlameBurnTime;
	float					m_flFlameRemoveTime;
	float					m_flTauntRemoveTime;
	float					m_flStunTime;
	float					m_flPhaseTime;

#ifdef GAME_DLL
	struct bleed_struct_t
	{
		CHandle<CTFPlayer> m_hAttacker;
		CHandle<CTFWeaponBase> m_hWeapon;
		float m_flBleedTime;
		float m_flEndTime;
		int m_iDamage;
	};
	CUtlVector< bleed_struct_t > m_aBleeds;

	/*struct pulledcurrencypacks_t
	{
	};*/
#endif

	CNetworkVar( float, m_flRuneCharge );

	// Vintage Banner
	CNetworkVar( float, m_flEffectBarProgress );
	float					m_flNextRageCheckTime;
	float					m_flRageTimeRemaining;
	int						m_iActiveBuffType;

	CNetworkVar( float, m_flFlashBattery );

	float m_flDisguiseCompleteTime;

	CNetworkVar( int, m_iDesiredPlayerClass );
	CNetworkVar( int, m_iDesiredWeaponID );
	CNetworkVar( int, m_iRespawnParticleID );

	float m_flNextBurningSound;

	CNetworkVar( float, m_flCloakMeter );	// [0,100]

	CNetworkVar( bool, m_bJumping );
	CNetworkVar( int, m_nAirDucked );

	CNetworkVar( float, m_flStealthNoAttackExpire );
	CNetworkVar( float, m_flStealthNextChangeTime );

	CNetworkVar( int, m_iCritMult );

	CNetworkArray( int, m_nStreaks, 3 );

	CNetworkArray( bool, m_bPlayerDominated, MAX_PLAYERS+1 );		// array of state per other player whether player is dominating other players
	CNetworkArray( bool, m_bPlayerDominatingMe, MAX_PLAYERS+1 );	// array of state per other player whether other players are dominating this player

	CNetworkVar( int, m_iDecapitations );

	CNetworkVar( float, m_flEnergyDrinkMeter );
	CNetworkVar( float, m_flHypeMeter );
	CNetworkVar( float, m_flChargeMeter );

	CNetworkVar( bool, m_bShieldEquipped );
	CNetworkVar( bool, m_bParachuteEquipped );

	CNetworkVar( int, m_iNextMeleeCrit );

	CNetworkVar( int, m_iAirDash );

	CNetworkVar( int, m_nCurrency );
	CNetworkVar( bool, m_bInUpgradeZone );
#ifdef CLIENT_DLL
public:
	int m_iDecapitationsParity;
	float m_flShieldChargeEndTime;
	bool m_bShieldChargeStopped;
private:
#endif

	float m_flEnergyDrinkDrainRate;
	float m_flEnergyDrinkRegenRate;
	float m_flChargeDrainRate;
	float m_flChargeRegenRate;

	CNetworkHandle( CBaseObject, m_hCarriedObject );
	CNetworkVar( bool, m_bCarryingObject );

	CNetworkVar( int, m_nTeamTeleporterUsed );

	// Co-Op Stuff
	CNetworkVar( int, m_iLives );

	// Vintage Gunslinger
	CNetworkVar( bool, m_bGunslinger );

	CNetworkVar( bool, m_bKingRuneBuffActive );

#ifdef GAME_DLL
	float	m_flNextCritUpdate;
	CUtlVector<CTFDamageEvent> m_DamageEvents;
#else
	int m_iDisguiseWeaponModelIndex;
	int m_iOldDisguiseWeaponModelIndex;
	CTFWeaponInfo *m_pDisguiseWeaponInfo;

	WEAPON_FILE_INFO_HANDLE	m_hDisguiseWeaponInfo;

	CNewParticleEffect *m_pCritEffect;
	EHANDLE m_hCritEffectHost;
	CSoundPatch *m_pCritSound;
	CSoundPatch *m_pInvulnerableSound;

	int	m_nOldDisguiseClass;
	int m_nOldDisguiseTeam;

	int m_iOldDisguiseWeaponID;

	int	m_nOldConditions;
	int m_nOldConditionsEx;
	int m_nOldConditionsEx2;
	int m_nOldConditionsEx3;
	int m_nOldConditionsEx4;

	bool m_bWasCritBoosted;
	bool m_bWasMiniCritBoosted;
#endif
};

#define TF_DEATH_DOMINATION				0x0001	// killer is dominating victim
#define TF_DEATH_ASSISTER_DOMINATION	0x0002	// assister is dominating victim
#define TF_DEATH_REVENGE				0x0004	// killer got revenge on victim
#define TF_DEATH_ASSISTER_REVENGE		0x0008	// assister got revenge on victim

#endif // TF_PLAYER_SHARED_H
