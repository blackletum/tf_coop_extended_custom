//====== Copyright © 1996-2003, Valve Corporation, All rights reserved. =======
//
// Purpose: 
//
//=============================================================================

#ifndef C_TF_PLAYER_H
#define C_TF_PLAYER_H
#ifdef _WIN32
#pragma once
#endif

#include "tf_playeranimstate.h"
#include "c_baseplayer.h"
#include "tf_shareddefs.h"
#include "baseparticleentity.h"
#include "tf_player_shared.h"
#include "c_tf_playerclass.h"
#include "tf_item.h"
#include "props_shared.h"
#include "hintsystem.h"
#include "c_playerattachedmodel.h"
#include "iinput.h"
#include "tf_weapon_medigun.h"
#include "ihasattributes.h"
#include "hl_movedata.h"
#include "c_prop_portal.h"
#include "c_func_liquidportal.h"
#include "beamdraw.h"
#include "beam_shared.h"

class C_MuzzleFlashModel;
class C_BaseObject;
class C_TFDroppedWeapon;

extern ConVar tf_medigun_autoheal;
extern ConVar cl_autorezoom;
extern ConVar cl_autoreload;
extern ConVar cl_flipviewmodels;

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class C_TFPlayer : public C_BasePlayer, public IHasAttributes
{
public:

	DECLARE_CLASS( C_TFPlayer, C_BasePlayer );
	DECLARE_CLIENTCLASS();
	DECLARE_PREDICTABLE();
	DECLARE_INTERPOLATION();

	C_TFPlayer();
	~C_TFPlayer();

	static C_TFPlayer* GetLocalTFPlayer();

	virtual void SharedSpawn(); // Shared between client and server.

	virtual void UpdateOnRemove( void );

	virtual const QAngle& GetRenderAngles();
	virtual void UpdateClientSideAnimation();
	virtual void SetDormant( bool bDormant );
	virtual void OnPreDataChanged( DataUpdateType_t updateType );
	virtual void OnDataChanged( DataUpdateType_t updateType );
	bool		 DetectAndHandlePortalTeleportation( void ); //detects if the player has portalled and fixes views
	virtual void ProcessMuzzleFlashEvent();
	virtual void ValidateModelIndex( void );
	virtual void NotifyShouldTransmit( ShouldTransmitState_t state );

	virtual void PostDataUpdate( DataUpdateType_t updateType );

	virtual void BuildTransformations( CStudioHdr *pStudioHdr, Vector *pos, Quaternion q[], const matrix3x4_t &cameraTransform, int boneMask, CBoneBitList &boneComputed );

	virtual Vector GetObserverCamOrigin( void );
	virtual int DrawModel( int flags );
	virtual bool CreateMove( float flInputSampleTime, CUserCmd *pCmd );

	QAngle GetAnimEyeAngles( void ) { return m_angEyeAngles; }

	// Called when not in tactical mode. Allows view to be overriden for things like driving a tank.
	virtual void				OverrideView( CViewSetup *pSetup );

	virtual bool IsAllowedToSwitchWeapons( void );

	virtual void ClientThink();
	void FixTeleportationRoll( void );

	// Deal with recording
	virtual void GetToolRecordingState( KeyValues *msg );

	CTFWeaponBase *GetActiveTFWeapon( void ) const;
	bool		 IsActiveTFWeapon(int iWeaponID);

	virtual void Simulate( void );
	virtual void FireEvent( const Vector& origin, const QAngle& angles, int event, const char *options );
	virtual void PlayStepSound( Vector &vecOrigin, surfacedata_t *psurface, float fvol, bool force );
	virtual const char *GetOverrideStepSound( const char *pszBaseStepSoundName );
	virtual void		OnEmitFootstepSound(const CSoundParameters& params, const Vector& vecOrigin, float fVolume);

	void LoadInventory(void);
	void EditInventory(int iSlot, int iWeapon);

	void FireBullet( const FireBulletsInfo_t &info, bool bDoEffects, int nDamageType, int nCustomDamageType = TF_DMG_CUSTOM_NONE );

	void ImpactWaterTrace( trace_t &trace, const Vector &vecStart );

	bool CanAttack( void );

	C_TFPlayerClass *GetPlayerClass( void )		{ return &m_PlayerClass; }
	bool IsPlayerClass( int iClass );
	bool IsPlayerNPCClass( void );
	virtual int GetMaxHealth( void ) const;
	virtual int	GetMaxHealthForBuffing( void ) const;

	virtual int GetRenderTeamNumber( void );

	bool IsWeaponLowered( void );

	void	AvoidPlayers( CUserCmd *pCmd );

	// Get the ID target entity index. The ID target is the player that is behind our crosshairs, used to
	// display the player's name.
	void UpdateIDTarget();
	int GetIDTarget() const;
	void SetForcedIDTarget( int iTarget );

	void SetAnimation( PLAYER_ANIM playerAnim );

	virtual float GetMinFOV() const;

	virtual const QAngle& EyeAngles();

	int GetBuildResources( void );

	// MATTTODO: object selection if necessary
	void SetSelectedObject( C_BaseObject *pObject ) {}

	void GetTeamColor( Color &color );

	virtual void ComputeFxBlend( void );

	virtual Vector			EyePosition();
	Vector					EyeFootPosition( const QAngle &qEyeAngles );//interpolates between eyes and feet based on view angle roll
	inline Vector			EyeFootPosition( void ) { return EyeFootPosition( EyeAngles() ); }; 
	void					PlayerPortalled( C_Prop_Portal *pEnteredPortal );

	// Taunts/VCDs
	virtual bool	StartSceneEvent( CSceneEventInfo *info, CChoreoScene *scene, CChoreoEvent *event, CChoreoActor *actor, C_BaseEntity *pTarget );
	virtual void	CalcView( Vector &eyeOrigin, QAngle &eyeAngles, float &zNear, float &zFar, float &fov );
	void			CalcPortalView( Vector &eyeOrigin, QAngle &eyeAngles );
	bool			StartGestureSceneEvent( CSceneEventInfo *info, CChoreoScene *scene, CChoreoEvent *event, CChoreoActor *actor, CBaseEntity *pTarget );
	void			TurnOnTauntCam( void );
	void			TurnOffTauntCam( void );
	void			TauntCamInterpolation( void );
	bool			InTauntCam( void ) { return m_bWasTaunting; }
	bool			IsAllowedToTaunt( void );
	virtual void	ThirdPersonSwitch( bool bThirdperson );
	//void			FireClientTauntParticleEffects( void );
	//void			GetClientTauntParticleDesiredState( void ) const;

	//void			SetTauntYaw( float )

	// Minimal Viewmodels
	void			CalcMinViewmodelOffset( void );

	virtual void	InitPhonemeMappings();

	virtual void	GetGlowEffectColor( byte *r, byte *g, byte *b, byte *a );

	CBaseEntity*	FindUseEntity( void );
	CBaseEntity*	FindUseEntityThroughPortal( void );

	inline bool		IsCloseToPortal( void ) //it's usually a good idea to turn on draw hacks when this is true
	{
		return ((PortalEyeInterpolation.m_bEyePositionIsInterpolating) || (m_hPortalEnvironment.Get() != NULL));	
	} 

	// Gibs.
	void			InitPlayerGibs( void );
	void			CreatePlayerGibs( const Vector &vecOrigin, const Vector &vecVelocity, float flImpactScale, bool bBurning = false, bool bHeadGib = false );
	void			DropPartyHat( breakablepropparams_t &breakParams, Vector &vecBreakVelocity );

	int				GetObjectCount( void );
	C_BaseObject	*GetObject( int index );
	C_BaseObject	*GetObjectOfType( int iObjectType, int iObjectMode );
	int 			GetNumObjects( int iObjectType, int iObjectMode );

	virtual bool	ShouldCollide( int collisionGroup, int contentsMask ) const;

	float			GetPercentInvisible( void );
	float			GetEffectiveInvisibilityLevel( void );	// takes viewer into account

	virtual void	AddDecal( const Vector& rayStart, const Vector& rayEnd,
		const Vector& decalCenter, int hitbox, int decalIndex, bool doTrace, trace_t& tr, int maxLODToDecal = ADDDECAL_TO_ALL_LODS );

	virtual void	CalcDeathCamView(Vector& eyeOrigin, QAngle& eyeAngles, float& fov);
	virtual void	CalcFreezeCamView( Vector& eyeOrigin, QAngle& eyeAngles, float& fov);
	virtual Vector	GetChaseCamViewOffset( CBaseEntity *target );

	void			ClientPlayerRespawn( void );

	void			CreateSaveMeEffect( MedicCallerType eType );

	void			CreateTauntWithMeEffect( void );
	void			StopTauntWithMeEffect( void );

	virtual bool	IsOverridingViewmodel( void );
	virtual int		DrawOverriddenViewmodel( C_BaseViewModel *pViewmodel, int flags );

	void			SetHealer( C_TFPlayer *pHealer, float flChargeLevel );
	void			GetHealer( C_TFPlayer **pHealer, float *flChargeLevel ) { *pHealer = m_hHealer; *flChargeLevel = m_flHealerChargeLevel; }
	float			MedicGetChargeLevel( void );
	CBaseEntity		*MedicGetHealTarget( void );

	void			StartBurningSound( void );
	void			StopBurningSound( void );
	void			UpdateRecentlyTeleportedEffect( void );

	void			StopViewModelParticles( C_BaseEntity *pEntity );

	bool			CanShowClassMenu( void );

	void			InitializePoseParams( void );
	void			UpdateLookAt( void );

	bool			IsEnemyPlayer( void );
	void			ShowNemesisIcon( bool bShow );

	CUtlVector<EHANDLE>		*GetSpawnedGibs( void ) { return &m_hSpawnedGibs; }

	Vector 			GetClassEyeHeight( void );

	void			ForceUpdateObjectHudState( void );

	bool			GetMedigunAutoHeal( void ){ return tf_medigun_autoheal.GetBool(); }
	bool			ShouldAutoRezoom( void ){ return cl_autorezoom.GetBool(); }
	bool			ShouldAutoReload( void ){ return cl_autoreload.GetBool(); }
	bool			ShouldFlipViewModel( void ) { return cl_flipviewmodels.GetBool(); }

	bool			IsSearchingSpawn( void ) { return m_bSearchingSpawn; }

	// HL2 ladder related methods
	LadderMove_t		*GetLadderMove() { return &m_LadderMove; }
	virtual void		ExitLadder();

	// Flashlight
	void	Flashlight( void );
	void	UpdateFlashlight( void );

	virtual bool ShouldReceiveProjectedTextures( int flags );

	void ToggleHeldObjectOnOppositeSideOfPortal( void ) { m_bHeldObjectOnOppositeSideOfPortal = !m_bHeldObjectOnOppositeSideOfPortal; }
	void SetHeldObjectOnOppositeSideOfPortal( bool p_bHeldObjectOnOppositeSideOfPortal ) { m_bHeldObjectOnOppositeSideOfPortal = p_bHeldObjectOnOppositeSideOfPortal; }
	bool IsHeldObjectOnOppositeSideOfPortal( void ) { return m_bHeldObjectOnOppositeSideOfPortal; }
	CProp_Portal *GetHeldObjectPortal( void ) { return m_pHeldObjectPortal; }

	virtual int		GetVisionFilterFlags( bool bWeaponsCheck = false );
	bool			HasVisionFilterFlags( int nFlags, bool bWeaponsCheck = false );
	virtual void	CalculateVisionUsingCurrentFlags( void );

	bool			IsDeveloper( void ) { return m_Shared.m_bIsPlayerADev; }
	bool			IsNicknine( void ) { return m_Shared.m_bIsPlayerNicknine; }

	virtual void	OnAchievementAchieved( int iAchievement );

public:
	// Shared functions
	void			TeamFortress_SetSpeed();
	bool			HasItem( void );					// Currently can have only one item at a time.
	void			SetItem( C_TFItem *pItem );
	C_TFItem		*GetItem( void );
	bool			IsAllowedToPickUpFlag( void );
	bool			HasTheFlag( void );
	float			GetCritMult( void ) { return m_Shared.GetCritMult(); }

	virtual void	ItemPostFrame( void );

	void			SetOffHandWeapon( CTFWeaponBase *pWeapon );
	void			HolsterOffHandWeapon( void );

	virtual int		GetSkin();

	virtual bool	Weapon_ShouldSetLast( CBaseCombatWeapon *pOldWeapon, CBaseCombatWeapon *pNewWeapon );
	virtual	bool	Weapon_Switch( C_BaseCombatWeapon *pWeapon, int viewmodelindex = 0 );

	CWeaponMedigun	*GetMedigun( void );
	CTFWeaponBase	*Weapon_OwnsThisID( int iWeaponID );
	CTFWeaponBase	*Weapon_GetWeaponByType( int iType );
	virtual bool	Weapon_SlotOccupied( CBaseCombatWeapon *pWeapon );
	virtual CBaseCombatWeapon *Weapon_GetSlot( int slot ) const;
	C_EconEntity	*GetEntityForLoadoutSlot( int iSlot );
	C_EconWearable	*GetWearableForLoadoutSlot( int iSlot );

	virtual void	GetStepSoundVelocities( float *velwalk, float *velrun );
	virtual void	SetStepSoundTime( stepsoundtimes_t iStepSoundTime, bool bWalking );

	bool			DoClassSpecialSkill( void );
	bool			CanGoInvisible( void );

	int				GetMaxAmmo( int iAmmoIndex, int iClassNumber = -1 );

	bool			CanPickupBuilding( C_BaseObject *pObject );
	
	virtual CAttributeManager *GetAttributeManager( void ) { return &m_AttributeManager; }
	virtual CAttributeContainer *GetAttributeContainer( void ) { return NULL; }
	virtual CBaseEntity *GetAttributeOwner( void ) { return NULL; }
	virtual CAttributeList *GetAttributeList() { return &m_AttributeList; }
	virtual void	ReapplyProvision( void ) { /*Do nothing*/ };

	// Vintage Gunslinger
	bool			HasGunslinger( void ) { return m_Shared.m_bGunslinger; }

	bool			CanAirDash( void ) const;
	bool			CanDuck( void ) const;
	bool			CanJump( void ) const;
	bool			CanMoveDuringTaunt( void );
	bool			CanPickupDroppedWeapon( const C_TFDroppedWeapon *pWeapon );
	bool			CanPlayerMove( void );

	bool			IsInspecting( void ) const;

	C_BaseEntity	*GetGrapplingHookTarget( void ) { return m_hGrapplingHookTarget; }
	void			SetGrapplingHookTarget( CBaseEntity *pTarget, bool bSomething = false );
public:
	// Ragdolls.
	virtual C_BaseAnimating *BecomeRagdollOnClient();
	virtual IRagdoll		*GetRepresentativeRagdoll() const;
	EHANDLE	m_hRagdoll;
	Vector m_vecRagdollVelocity;

	// Objects
	int CanBuild( int iObjectType, int iObjectMode );
	CUtlVector< CHandle<C_BaseObject> > m_aObjects;

	virtual CStudioHdr *OnNewModel( void );

	void				DisplaysHintsForTarget( C_BaseEntity *pTarget );

	// Shadows
	virtual ShadowType_t ShadowCastType( void ) ;
	virtual void GetShadowRenderBounds( Vector &mins, Vector &maxs, ShadowType_t shadowType );
	virtual void GetRenderBounds( Vector& theMins, Vector& theMaxs );
	virtual bool GetShadowCastDirection( Vector *pDirection, ShadowType_t shadowType ) const;

	CMaterialReference *GetInvulnMaterialRef( void ) { return &m_InvulnerableMaterial; }
	bool IsNemesisOfLocalPlayer();
	bool ShouldShowNemesisIcon();

	virtual	IMaterial *GetHeadLabelMaterial( void );

	virtual void FireGameEvent( IGameEvent *event );

	void		UpdateTypingBubble( void );
	void		UpdateOverhealEffect( void );
	void		UpdateDemomanEyeEffect( int iDecapCount );
	void		UpdateRuneIcon( bool bHasRune );

	void		ParseSharedTauntDataFromEconItemView( const CEconItemView* pItem );
protected:

	void ResetFlexWeights( CStudioHdr *pStudioHdr );

private:

	void HandleTaunting( void );

	void OnPlayerClassChange( void );
	void UpdatePartyHat( void );

	bool CanLightCigarette( void );

	void InitInvulnerableMaterial( void );

	bool				m_bWasTaunting;
	float				m_flTauntOffTime;
	CameraThirdData_t	m_TauntCameraData;

	QAngle				m_angTauntPredViewAngles;
	QAngle				m_angTauntEngViewAngles;

public:
	QAngle		m_vecUseAngles;

	// Custom Models
	char m_iszCustomModel[MAX_PATH];
private:

	C_TFPlayerClass		m_PlayerClass;

	// ID Target
	int					m_iIDEntIndex;
	int					m_iForcedIDTarget;

	CountdownTimer m_blinkTimer;

	CNewParticleEffect	*m_pTeleporterEffect;
	bool				m_bToolRecordingVisibility;

	int					m_iOldState;
	int					m_iOldSpawnCounter;

	// Healer
	CHandle<C_TFPlayer>	m_hHealer;
	float				m_flHealerChargeLevel;
	int					m_iOldHealth;

	CNetworkVar( int, m_iPlayerModelIndex );

	// Look At
	/*
	int m_headYawPoseParam;
	int m_headPitchPoseParam;
	float m_headYawMin;
	float m_headYawMax;
	float m_headPitchMin;
	float m_headPitchMax;
	float m_flLastBodyYaw;
	float m_flCurrentHeadYaw;
	float m_flCurrentHeadPitch;
	*/

	// Spy cigarette smoke
	bool m_bCigaretteSmokeActive;

	// Callouts particle effect
	CNewParticleEffect	*m_pSaveMeEffect;
	CNewParticleEffect	*m_pTauntWithMeEffect;

	bool m_bUpdateObjectHudState;

public:

	CTFPlayerShared m_Shared;

// Called by shared code.
public:

	void DoAnimationEvent( PlayerAnimEvent_t event, int nData = 0 );

	CTFPlayerAnimState *m_PlayerAnimState;

	QAngle	m_angEyeAngles;
	CInterpolatedVar< QAngle >	m_iv_angEyeAngles;

	CNetworkHandle( C_TFItem, m_hItem );

	CNetworkHandle( C_TFWeaponBase, m_hOffHandWeapon );
	CNetworkHandle( C_BaseEntity, m_hGrapplingHookTarget );

	int				m_iOldPlayerClass;	// Used to detect player class changes
	bool			m_bIsDisplayingNemesisIcon;

	int				m_iSpawnCounter;

	bool			m_bSaveMeParity;
	bool			m_bOldSaveMeParity;

	int				m_nOldWaterLevel;
	float			m_flWaterEntryTime;
	bool			m_bWaterExitEffectActive;

	CMaterialReference	m_InvulnerableMaterial;


	// Burning
	CSoundPatch			*m_pBurningSound;
	CNewParticleEffect	*m_pBurningEffect;
	float				m_flBurnEffectStartTime;
	float				m_flBurnEffectEndTime;

	CNewParticleEffect	*m_pDisguisingEffect;
	float m_flDisguiseEffectStartTime;
	float m_flDisguiseEndEffectStartTime;

	EHANDLE					m_hFirstGib;
	CUtlVector<EHANDLE>		m_hSpawnedGibs;

	int				m_iOldTeam;
	int				m_iOldClass;
	int				m_iOldDisguiseTeam;
	int				m_iOldDisguiseClass;

	bool			m_bDisguised;
	int				m_iPreviousMetal;

	EHANDLE			m_hOldActiveWeapon;

	CNewParticleEffect *m_pDemoEyeEffect;

	CNewParticleEffect *m_pRuneEffect;

	int GetNumActivePipebombs( void );

	int				m_iSpyMaskBodygroup;

	bool			m_bUpdatePartyHat;
	CHandle<C_PlayerAttachedModel>	m_hPartyHat;

	float			m_flLastDamageTime;
	float			m_flNextNoiseMakerTime;

	bool			m_bSearchingSpawn;

	bool			m_bTyping;
	bool			m_bHasLongJump;
	CNewParticleEffect	*m_pTypingEffect;

	CNewParticleEffect *m_pOverhealEffect;

	CAttributeManager m_AttributeManager;

	int				m_iSquadMemberCount;
	int				m_iSquadMedicCount;
	bool			m_fSquadInFollowMode;

	// HL2 Ladder related data
	EHANDLE			m_hLadder;
	LadderMove_t	m_LadderMove;

	Vector			m_vecLocatorOrigin;

	bool			m_bAllowMoveDuringTaunt;
	bool			m_bIsReadyToHighFive;
	CNetworkHandle( C_TFPlayer, m_hHighFivePartner );
	float			m_flCurrentTauntMoveSpeed;
	float			m_flTauntYaw;
	int				m_nForceTauntCam;
	int				m_nActiveTauntSlot;
	int				m_iTauntItemDefIndex;
private:
	/*int	  m_iSpawnInterpCounter;
	int	  m_iSpawnInterpCounterCache;

	int	  m_iPlayerSoundType;*/

	bool			m_bHeldObjectOnOppositeSideOfPortal;
	CProp_Portal	*m_pHeldObjectPortal;

	int				m_iForceNoDrawInPortalSurface; //only valid for one frame, used to temp disable drawing of the player model in a surface because of freaky artifacts

	struct PortalEyeInterpolation_t
	{
		bool	m_bEyePositionIsInterpolating; //flagged when the eye position would have popped between two distinct positions and we're smoothing it over
		Vector	m_vEyePosition_Interpolated; //we'll be giving the interpolation a certain amount of instant movement per frame based on how much an uninterpolated eye would have moved
		Vector	m_vEyePosition_Uninterpolated; //can't have smooth movement without tracking where we just were
		//bool	m_bNeedToUpdateEyePosition;
		//int		m_iFrameLastUpdated;

		int		m_iTickLastUpdated;
		float	m_fTickInterpolationAmountLastUpdated;
		bool	m_bDisableFreeMovement; //used for one frame usually when error in free movement is likely to be high
		bool	m_bUpdatePosition_FreeMove;

		PortalEyeInterpolation_t( void ) : m_iTickLastUpdated(0), m_fTickInterpolationAmountLastUpdated(0.0f), m_bDisableFreeMovement(false), m_bUpdatePosition_FreeMove(false) { };
	} PortalEyeInterpolation;

	struct PreDataChanged_Backup_t
	{
		CHandle<C_Prop_Portal>	m_hPortalEnvironment;
		CHandle<C_Func_LiquidPortal>	m_hSurroundingLiquidPortal;
		//Vector					m_ptPlayerPosition;
		QAngle					m_qEyeAngles;
	} PreDataChanged_Backup;

	Vector			m_ptEyePosition_LastCalcView;
	QAngle			m_qEyeAngles_LastCalcView; //we've got some VERY persistent single frame errors while teleporting, this will be updated every frame in CalcView() and will serve as a central source for fixed angles
	C_Prop_Portal	*m_pPortalEnvironment_LastCalcView;

	bool			m_bPortalledMessagePending; //Player portalled. It's easier to wait until we get a OnDataChanged() event or a CalcView() before we do anything about it. Otherwise bits and pieces can get undone
	VMatrix			m_PendingPortalMatrix;

	float			m_flHeadScale;
	float			m_flTorsoScale;
	float			m_flHandScale;
public:

	bool	m_bPitchReorientation;
	float	m_fReorientationRate;
	bool	m_bEyePositionIsTransformedByPortal; //when the eye and body positions are not on the same side of a portal

	CHandle<C_Prop_Portal>	m_hPortalEnvironment; //a portal whose environment the player is currently in, should be invalid most of the time
	CHandle<C_Func_LiquidPortal>	m_hSurroundingLiquidPortal; //a liquid portal whose volume the player is standing in
private:

	void UpdatePortalEyeInterpolation( void );
	
	float m_flWaterImpactTime;

	Beam_t	*m_pFlashlightBeam;

	// Gibs.
	CUtlVector<breakmodel_t>	m_aGibs;

	C_TFPlayer( const C_TFPlayer & );
};

inline C_TFPlayer* ToTFPlayer( C_BaseEntity *pEntity )
{
	if ( !pEntity || !pEntity->IsPlayer() )
		return NULL;

	Assert( dynamic_cast<C_TFPlayer*>( pEntity ) != 0 );
	return static_cast< C_TFPlayer* >( pEntity );
}

#endif // C_TF_PLAYER_H
