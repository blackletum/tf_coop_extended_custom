//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef C_AI_BASENPC_H
#define C_AI_BASENPC_H
#ifdef _WIN32
#pragma once
#endif

#include "c_basecombatcharacter.h"

#ifdef TF_CLASSIC_CLIENT
#include "ai_basenpc_shared.h"
#include "tf_shareddefs.h"
#include "c_tf_player.h"
#include "tf_item.h"
#include "ihasattributes.h"
#include "game_item_schema.h"
#include "econ_item_view.h"
#endif

// NOTE: Moved all controller code into c_basestudiomodel
class C_AI_BaseNPC : public C_BaseCombatCharacter, public IHasAttributes
{
	DECLARE_CLASS( C_AI_BaseNPC, C_BaseCombatCharacter );

public:
	DECLARE_CLIENTCLASS();
	#ifdef TF_CLASSIC_CLIENT
	DECLARE_PREDICTABLE();
	#endif
	C_AI_BaseNPC();
	virtual unsigned int	PhysicsSolidMaskForEntity( void ) const;
	virtual bool			IsNPC( void ) { return true; }
	bool					IsMoving( void ){ return m_bIsMoving; }
	bool					ShouldAvoidObstacle( void ){ return m_bPerformAvoidance; }
	virtual bool			AddRagdollToFadeQueue( void ) { return m_bFadeCorpse; }

	virtual bool			GetRagdollInitBoneArrays( matrix3x4_t *pDeltaBones0, matrix3x4_t *pDeltaBones1, matrix3x4_t *pCurrentBones, float boneDt );

	int						GetDeathPose( void ) { return m_iDeathPose; }

	bool					ShouldModifyPlayerSpeed( void ) { return m_bSpeedModActive;	}
	int						GetSpeedModifyRadius( void ) { return m_iSpeedModRadius; }
	int						GetSpeedModifySpeed( void ) { return m_iSpeedModSpeed;	}

	void					ClientThink( void );
	virtual void			OnPreDataChanged( DataUpdateType_t updateType );
	virtual void			OnDataChanged( DataUpdateType_t type );
	bool					ImportantRagdoll( void ) { return m_bImportanRagdoll;	}
	virtual void			UpdateOnRemove( void );

	virtual int				GetHealth() const { return m_iHealth; }
	void					SetHealth( int health ) { m_iHealth = health; }
	virtual int				GetMaxHealth() const { return m_iMaxHealth; }

	const char				*GetClassname( void ) { return m_szClassname; }
	virtual const char		*GetLocalizeName( void );

#ifdef TF_CLASSIC_CLIENT
	virtual int				InternalDrawModel( int flags );
	virtual void			AddDecal( const Vector& rayStart, const Vector& rayEnd,
		const Vector& decalCenter, int hitbox, int decalIndex, bool doTrace, trace_t& tr, int maxLODToDecal = ADDDECAL_TO_ALL_LODS );

	virtual void			BuildTransformations( CStudioHdr *pStudioHdr, Vector *pos, Quaternion q[], const matrix3x4_t &cameraTransform, int boneMask, CBoneBitList &boneComputed );

	virtual C_BaseAnimating	*BecomeRagdollOnClient();
	virtual IRagdoll		*GetRepresentativeRagdoll() const;
	virtual Vector			GetObserverCamOrigin( void );

	virtual void			GetTargetIDString( wchar_t *sIDString, int iMaxLenInBytes );
	virtual void			GetTargetIDDataString( wchar_t *sDataString, int iMaxLenInBytes );

	int						GetNumHealers( void ) { return m_nNumHealers; }
	int						GetMaxBuffedHealth( void );

	// TF2 conditions
	int		GetCond() const						{ return m_nPlayerCond; }
	void	SetCond( int nCond )				{ m_nPlayerCond = nCond; }
	void	AddCond( int nCond, float flDuration = PERMANENT_CONDITION );
	void	RemoveCond( int nCond );
	bool	InCond( int nCond );
	void	RemoveAllCond( void );
	void	OnConditionAdded( int nCond );
	void	OnConditionRemoved( int nCond );
	//void	ConditionThink( void );
	float	GetConditionDuration( int nCond );

	// check the newly networked conditions for changes
	void	SyncConditions( int nCond, int nOldCond, int nUnused, int iOffset );

	bool	IsCritBoosted( void );
	bool	IsMiniCritBoosted( void );
	bool	IsInvulnerable( void );
	bool	IsStealthed( void );
	bool	IsJared( void );
	bool	IsSpeedBoosted( void );
	bool	IsBuffed( void );

	virtual bool IsOnFire() { return InCond( TF_COND_BURNING ); }

	void OnAddInvulnerable( void );
	void OnAddBurning( void );
	//void OnAddSlowed( void );
	void OnAddCritboosted( void );
	void OnAddStunned( void );
	//void OnAddHalloweenGiant( void );
	//void OnAddHalloweenTiny( void );
	void OnAddPhase( void );
	void OnAddShieldCharge( void );
	void OnAddSpeedBoost( bool bParticle );
	void OnAddUrine( void );
	void OnAddMadMilk( void );
	void OnAddCondGas( void );
	void OnAddBleeding( void );
	void OnAddBuff( void );
	void OnAddSapped( void );
	void OnAddRune( void );
	//void OnAddPlague( void );
	void OnAddRadiusHeal( void );
	void OnAddRunePlague( void );
	void OnAddMegaHeal( void );

	void OnRemoveBurning( void );
	void OnRemoveInvulnerable( void );
	//void OnRemoveSlowed( void );
	void OnRemoveCritboosted( void );
	void OnRemoveStunned( void );
	//void OnRemoveHalloweenGiant( void );
	//void OnRemoveHalloweenTiny( void );
	void OnRemovePhase( void );
	void OnRemoveShieldCharge( void );
	void OnRemoveSpeedBoost( void );
	void OnRemoveUrine( void );
	void OnRemoveMadMilk( void );
	void OnRemoveCondGas( void );
	void OnRemoveBleeding( void );
	void OnRemoveBuff( void );
	void OnRemoveSapped( void );
	void OnRemoveRune( void );
	void OnRemoveRadiusHeal( void );
	void OnRemoveRunePlague( void );
	void OnRemoveMegaHeal( void );

	void	StartBurningSound( void );
	void	StopBurningSound( void );

	CUtlVector<EHANDLE>		*GetSpawnedGibs( void ) { return &m_hSpawnedGibs; }

	void	Burn( CTFPlayer *pAttacker, CTFWeaponBase *pWeapon = NULL, float flFlameDuration = -1.0f );
	void	MakeBleed( CTFPlayer *pAttacker, CTFWeaponBase *pWeapon, float flBleedDuration, int iDamage );
	void	StunNPC( float flDuration, float flSpeed, float flResistance, int nStunFlags, CTFPlayer *pStunner );
	void	Concussion( CTFPlayer *pAttacker, float flDuration );
	void	AttachSapper( CTFPlayer *pAttacker );

	CNewParticleEffect *m_pStun;
	CNewParticleEffect *m_pWarp;
	CNewParticleEffect *m_pSpeedTrails;
	CNewParticleEffect *m_pBuffAura;
	CNewParticleEffect *m_pSapped;
	CNewParticleEffect *m_pRadiusHeal;
	CNewParticleEffect *m_pRunePlague;
	CNewParticleEffect *m_pMarkForDeath;
	CNewParticleEffect *m_pMegaHeal;
	CNewParticleEffect *m_pRuneEffect;

	void	UpdatePhaseEffects( void );
	void	UpdateSpeedBoostEffects( void );

	float	GetStunExpireTime( void )			{ return m_flStunExpireTime; }
	void	SetStunExpireTime( float flTime )	{ m_flStunExpireTime = flTime; }

	int		GetStunFlags( void )				{ return m_nStunFlags; }

	CMaterialReference *GetInvulnMaterialRef( void ) { return &m_InvulnerableMaterial; }
	void	InitInvulnerableMaterial( void );

	bool	AllowBackstab( void ) { return ( m_nTFFlags & TFFL_NOBACKSTAB ) == 0; }
	bool	IsMech( void ) { return ( m_nTFFlags & TFFL_MECH ) != 0; }
	bool	CanBeHealed( void ) { return ( m_nTFFlags & TFFL_NOHEALING ) == 0; }
	bool	AllowJar( void ) { return ( m_nTFFlags & TFFL_NOJAR ) == 0; }
	bool	AllowDeathNotice( void ) { return ( m_nTFFlags & TFFL_NODEATHNOTICE ) == 0; }
	bool	NoReward( void ) { return ( m_nTFFlags & TFFL_NOREWARD ) == 0; }
	bool	CanBleed( void ) { return ( m_nTFFlags & TFFL_CANBLEED ) == 0; }
	bool	AllowCapturePoint( void ) { return ( m_nTFFlags & TFFL_NOTOUCH_CP ) == 0; }
	bool	CanSap( void ) { return ( m_nTFFlags & TFFL_CANSAP ) != 0; }
	bool	CanUpgrade( void ) { return ( m_nTFFlags & TFFL_CANUPGRADE ) != 0; }
	bool	AllowStun( void ) { return ( m_nTFFlags & TFFL_NOSTUN ) != 0; }
	bool	AllowPlayerForces( void ) { return ( m_nTFFlags & TFFL_NOFORCE ) == 0; }

	void	UpdateCritBoostEffect( bool bForceHide = false );

	CNewParticleEffect *m_pCritEffect;
	EHANDLE m_hCritEffectHost;
	CSoundPatch *m_pCritSound;

	// Gibs.
	void			InitNPCGibs( void );
	void			CreateNPCGibs( const Vector &vecOrigin, const Vector &vecVelocity, float flImpactScale, bool bBurning = false, bool bHeadGib = false );
	void			DropPartyHat( breakablepropparams_t &breakParams, Vector &vecBreakVelocity );

	virtual bool	ShouldCollide( int collisionGroup, int contentsMask ) const;
	
	void			FireBullet( const FireBulletsInfo_t &info, bool bDoEffects, int nDamageType, int nCustomDamageType = TF_DMG_CUSTOM_NONE );
	void			ImpactWaterTrace( trace_t &trace, const Vector &vecStart );

	void			AddTempCritBonus( float flDuration = PERMANENT_CONDITION );

	bool			HasItem( void );					// Currently can have only one item at a time.
	void			SetItem( C_TFItem *pItem );
	C_TFItem		*GetItem( void );
	bool			IsAllowedToPickUpFlag( void );
	bool			HasTheFlag( void );

	bool			HasSapper( void ) { return InCond( TF_COND_SAPPED ); }

	CNetworkHandle( C_TFItem, m_hItem );

	virtual CAttributeManager *GetAttributeManager( void ) { return &m_AttributeManager; }
	virtual CAttributeContainer *GetAttributeContainer( void ) { return NULL; }
	virtual CBaseEntity *GetAttributeOwner( void ) { return NULL; }
	virtual CAttributeList *GetAttributeList() { return &m_AttributeList; }
	virtual void ReapplyProvision( void ) { /*Do nothing*/ };

	CAttributeManager m_AttributeManager;

	void UpdateOverhealEffect( void );
	void UpdateRuneIcon( bool bHasRune );

	RuneTypes_t			GetCarryingRuneType( void );

	void	ConditionThink( void );
private:

	float m_flWaterImpactTime;
#endif

private:
	C_AI_BaseNPC( const C_AI_BaseNPC & ); // not defined, not accessible
	float m_flTimePingEffect;
	int  m_iDeathPose;
	int	 m_iDeathFrame;

	int	 m_iHealth;
	int	 m_iMaxHealth;

	int m_iSpeedModRadius;
	int m_iSpeedModSpeed;

	bool m_bPerformAvoidance;
	bool m_bIsMoving;
	bool m_bFadeCorpse;
	bool m_bSpeedModActive;
	bool m_bImportanRagdoll;

	char m_szClassname[128];

#ifdef TF_CLASSIC_CLIENT
	int m_iOldTeam;
public:

	CMaterialReference	m_InvulnerableMaterial;

	// Burning
	CSoundPatch			*m_pBurningSound;
	CNewParticleEffect	*m_pBurningEffect;
	float				m_flBurnEffectStartTime;
	float				m_flBurnEffectEndTime;

	bool				m_bBurningDeath;
	bool				m_bIceDeath;
	bool				m_bAshDeath;
	int					m_iDamageCustomDeath;

	bool				m_bKingRuneBuffActive;

	EHANDLE					m_hFirstGib;
	CUtlVector<EHANDLE>		m_hSpawnedGibs;

protected:
	// Conditions
	CNetworkVar( int, m_nPlayerCond );

	float	m_flStunExpireTime;

	int		m_nStunFlags;
	float	m_flStunMovementSpeed;
	float	m_flStunResistance;

	CNetworkVar( int, m_nPlayerCondEx ); // 33-64
	CNetworkVar( int, m_nPlayerCondEx2 ); // 65-96
	CNetworkVar( int, m_nPlayerCondEx3 ); // 97-128
	CNetworkVar( int, m_nPlayerCondEx4 ); // 129-160
	CNetworkArray( float, m_flCondExpireTimeLeft, TF_COND_LAST );
	int m_nNumHealers;

	int	m_nOldConditions;
	int m_nOldConditionsEx;
	int m_nOldConditionsEx2;
	int m_nOldConditionsEx3;
	int m_nOldConditionsEx4;

	bool m_bWasCritBoosted;
	bool m_bWasMiniCritBoosted;

	char m_pszLocalizeName[MAX_PLAYER_NAME_LENGTH];

	CNewParticleEffect *m_pOverhealEffect;

private:
	float			m_flHeadScale;
	float			m_flTorsoScale;
	float			m_flHandScale;

	// Ragdoll
	EHANDLE				m_hRagdoll;

	// Gibs.
	CUtlVector<breakmodel_t>	m_aGibs;

	// Flags
	int m_nTFFlags;
#endif
};


#endif // C_AI_BASENPC_H
