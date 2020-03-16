//========= Copyright © 1996-2005, Valve LLC, All rights reserved. ============
//
//=============================================================================
#ifndef TF_PLAYER_H
#define TF_PLAYER_H
#pragma once

#include "basemultiplayerplayer.h"
#include "server_class.h"
#include "tf_playeranimstate.h"
#include "tf_shareddefs.h"
#include "tf_player_shared.h"
#include "tf_playerclass.h"
#include "entity_tfstart.h"
#include "tf_inventory.h"
#include "tf_weapon_medigun.h"
#include "ihasattributes.h"
#include "hl_movedata.h"
#include "hl2_player.h"
#include "tf_revive.h"
#include "simtimer.h"
#include "soundenvelope.h"
#include "prop_portal.h"
#include "in_buttons.h"
#include "func_liquidportal.h"
#include "ai_speech.h"			// For expresser host

class CTFPlayer;
class CTFTeam;
class CTFGoal;
class CTFGoalItem;
class CTFItem;
class CTFWeaponBuilder;
class CBaseObject;
class CTFWeaponBase;
class CIntroViewpoint;
class CLogicPlayerProxy;
class CTFReviveMarker;
class CTriggerAreaCapture;
class CTFDroppedWeapon;

struct PortalPlayerStatistics_t
{
	int iNumPortalsPlaced;
	int iNumStepsTaken;
	float fNumSecondsTaken;
};

//=============================================================================
//
// Player State Information
//
class CPlayerStateInfo
{
public:

	int				m_nPlayerState;
	const char		*m_pStateName;

	// Enter/Leave state.
	void ( CTFPlayer::*pfnEnterState )();	
	void ( CTFPlayer::*pfnLeaveState )();

	// Think (called every frame).
	void ( CTFPlayer::*pfnThink )();
};

struct DamagerHistory_t
{
	DamagerHistory_t()
	{
		Reset();
	}
	void Reset()
	{
		hDamager = NULL;
		flTimeDamage = 0;
	}
	EHANDLE hDamager;
	float	flTimeDamage;
};
#define MAX_DAMAGER_HISTORY 2

//=============================================================================
//
// TF Player
//
class CTFPlayer : public CBaseMultiplayerPlayer, public IHasAttributes
{
public:
	DECLARE_CLASS( CTFPlayer, CBaseMultiplayerPlayer );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	CTFPlayer();
	~CTFPlayer();

	// Creation/Destruction.
	static CTFPlayer	*CreatePlayer( const char *className, edict_t *ed );
	static CTFPlayer	*Instance( int iEnt );

	virtual void		Spawn();
	virtual void		SharedSpawn(); // Shared between client and server.
	virtual int			ShouldTransmit( const CCheckTransmitInfo *pInfo );
	virtual void		ForceRespawn();
	virtual CBaseEntity	*EntSelectSpawnPoint( void );
	virtual void		InitialSpawn();
	virtual void		Precache();
	virtual void		CreateSounds( void );
	virtual void		StopLoopingSounds( void );
	virtual bool		IsReadyToPlay( void );
	virtual bool		IsReadyToSpawn( void );
	virtual bool		ShouldGainInstantSpawn( void );
	virtual void		ResetScores( void );

	bool UseFoundEntity( CBaseEntity *pUseEntity );
	CBaseEntity* FindUseEntity( void );
	CBaseEntity* FindUseEntityThroughPortal( void );

	virtual void		PlayerUse( void );

	void CheckTeam( void );

	void				CreateViewModel( int iViewModel = 0 );
	CBaseViewModel		*GetOffHandViewModel();
	void				SendOffHandViewModelActivity( Activity activity );

	virtual void		ImpulseCommands( void );
	virtual void		CheatImpulseCommands( int iImpulse );
	virtual void		PlayerRunCommand( CUserCmd *ucmd, IMoveHelper *moveHelper);

	virtual void		CommitSuicide( bool bExplode = false, bool bForce = false );

			bool			CanEnterVehicle( IServerVehicle *pVehicle, int nRole );
	virtual bool		GetInVehicle( IServerVehicle *pVehicle, int nRole );
	virtual void		LeaveVehicle( const Vector &vecExitPoint = vec3_origin, const QAngle &vecExitAngles = vec3_angle );

	// Combats
	virtual void		TraceAttack( const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr, CDmgAccumulator *pAccumulator );
	virtual int			TakeHealth( float flHealth, int bitsDamageType );
	virtual	void		Event_KilledOther( CBaseEntity *pVictim, const CTakeDamageInfo &info );
	virtual void		Event_Killed( const CTakeDamageInfo &info );
	virtual bool		Event_Gibbed( const CTakeDamageInfo &info );
	virtual bool		BecomeRagdoll( const CTakeDamageInfo &info, const Vector &forceVector );
	void				StopRagdollDeathAnim( void );
	virtual void		PlayerDeathThink( void );

	virtual int			OnTakeDamage( const CTakeDamageInfo &inputInfo );
	virtual int			OnTakeDamage_Alive( const CTakeDamageInfo &info );
	void				ApplyPushFromDamage( const CTakeDamageInfo &info, Vector &vecDir );
	void				SetBlastJumpState( int iJumpType, bool bPlaySound );
	void				ClearBlastJumpState( void );
	int					GetBlastJumpFlags( void ) { return m_nBlastJumpFlags; }
	void				SetAirblastState( bool bAirblastState );
	void				ClearAirblastState( void );
	void				SetForceByNature( bool bForced );
	void				AddDamagerToHistory( EHANDLE hDamager );
	void				ClearDamagerHistory();
	DamagerHistory_t	&GetDamagerHistory( int i ) { return m_DamagerHistory[i]; }
	virtual void		DamageEffect(float flDamage, int fDamageType);
	virtual	bool		ShouldCollide( int collisionGroup, int contentsMask ) const;

	virtual int			GetMaxHealth( void ) const;
	virtual int			GetMaxHealthForBuffing( void ) const;

	void				SetHealthBuffTime( float flTime )		{ m_flHealthBuffTime = flTime; }

	CTFWeaponBase		*GetActiveTFWeapon( void ) const;
	bool				IsActiveTFWeapon(int iWeaponID);

	CEconItemView		*GetLoadoutItem( int iClass, int iSlot );
	void				HandleCommand_WeaponPreset( int iSlotNum, int iPresetNum );
	void				HandleCommand_WeaponPreset( int iClass, int iSlotNum, int iPresetNum );

	CBaseEntity			*GiveNamedItem( const char *pszName, int iSubType = 0, CEconItemView* pItem = NULL, bool bOwns = true );

	void				SaveMe( void );

	void				FireBullet( const FireBulletsInfo_t &info, bool bDoEffects, int nDamageType, int nCustomDamageType = TF_DMG_CUSTOM_NONE );
	void				ImpactWaterTrace( trace_t &trace, const Vector &vecStart );
	void				NoteWeaponFired();

	bool				HasItem( void );					// Currently can have only one item at a time.
	void				SetItem( CTFItem *pItem );
	CTFItem				*GetItem( void );

	void				Regenerate( void );
	float				GetNextRegenTime( void ){ return m_flNextRegenerateTime; }
	void				SetNextRegenTime( float flTime ){ m_flNextRegenerateTime = flTime; }

	float				GetNextChangeClassTime(void){ return m_flNextChangeClassTime; }
	void				SetNextChangeClassTime(float flTime){ m_flNextChangeClassTime = flTime; }

	float				GetNextChangeTeamTime(void){ return m_flNextChangeTeamTime; }
	void				SetNextChangeTeamTime(float flTime){ m_flNextChangeTeamTime = flTime; }

	virtual	void		RemoveAllItems( bool removeSuit );
	virtual void		RemoveAllWeapons( void );

	bool				DropCurrentWeapon( void );
	void				DropFlag( void );
	void				TFWeaponRemove( int iWeaponID );
	bool				TFWeaponDrop( CTFWeaponBase *pWeapon, bool bThrowForward );
	virtual bool		BumpWeapon( CBaseCombatWeapon *pWeapon );

	virtual void		VPhysicsShadowUpdate( IPhysicsObject *pPhysics );

	// Class.
	CTFPlayerClass		*GetPlayerClass( void ) 					{ return &m_PlayerClass; }
	int					GetDesiredPlayerClassIndex( void )			{ return m_Shared.m_iDesiredPlayerClass; }
	void				SetDesiredPlayerClassIndex( int iClass )	{ m_Shared.m_iDesiredPlayerClass = iClass; }

	// Team.
	void				ForceChangeTeam( int iTeamNum );
	virtual void		ChangeTeam( int iTeamNum ) { ChangeTeam( iTeamNum, false, false ); }
	virtual void		ChangeTeam( int iTeamNum, bool bAutoTeam, bool bSilent );

	// mp_fadetoblack
	void				HandleFadeToBlack( void );

	// Flashlight
	void				CheckFlashlight( void );
	virtual int			FlashlightIsOn( void );
	virtual void		FlashlightTurnOn( void );
	virtual void		FlashlightTurnOff( void );
	bool				IsIlluminatedByFlashlight( CBaseEntity *pEntity, float *flReturnDot );
	void				SetFlashlightPowerDrainScale( float flScale ) { m_flFlashlightPowerDrainScale = flScale; }

	// Think.
	virtual void		PreThink();
	virtual void		PostThink();
	virtual bool		HandleInteraction(int interactionType, void *data, CBaseCombatCharacter* sourceEnt);

	void				UpdatePortalPlaneSounds( void );
	void				UpdateWooshSounds( void );

	virtual void		ItemPostFrame();
	virtual void		Weapon_FrameUpdate( void );
	virtual void		Weapon_HandleAnimEvent( animevent_t *pEvent );
	virtual bool		Weapon_ShouldSetLast( CBaseCombatWeapon *pOldWeapon, CBaseCombatWeapon *pNewWeapon );

	virtual void		GetStepSoundVelocities( float *velwalk, float *velrun );
	virtual void		SetStepSoundTime( stepsoundtimes_t iStepSoundTime, bool bWalking );

	virtual void		OnEmitFootstepSound( CSoundParameters const &sound, Vector const &pos, float volume );

	// Utility.
	void				UpdateModel( void );
	void				UpdateSkin( int iTeam );

	virtual int			GiveAmmo( int iCount, int iAmmoIndex, bool bSuppressSound = false );
	virtual int			GiveAmmo( int iCount, int iAmmoIndex, bool bSuppressSound, EAmmoSource ammosource );
	int					GetMaxAmmo( int iAmmoIndex, int iClassNumber = -1 );

	bool				CanAttack( void );

	// This passes the event to the client's and server's CPlayerAnimState.
	void				DoAnimationEvent( PlayerAnimEvent_t event, int mData = 0 );

	virtual bool		ClientCommand( const CCommand &args );
	void				ClientHearVox( const char *pSentence );
	void				DisplayLocalItemStatus( CTFGoal *pGoal );

	bool				CanPickupBuilding( CBaseObject *pObject );
	bool				TryToPickupBuilding( void );

	int					BuildObservableEntityList( void );
	virtual int			GetNextObserverSearchStartPoint( bool bReverse ); // Where we should start looping the player list in a FindNextObserverTarget call
	virtual CBaseEntity *FindNextObserverTarget(bool bReverse);
	virtual bool		IsValidObserverTarget(CBaseEntity * target); // true, if player is allowed to see this target
	virtual bool		SetObserverTarget(CBaseEntity * target);
	virtual bool		ModeWantsSpectatorGUI( int iMode ) { return (iMode != OBS_MODE_FREEZECAM && iMode != OBS_MODE_DEATHCAM); }
	void				FindInitialObserverTarget( void );
	CBaseEntity		    *FindNearestObservableTarget( Vector vecOrigin, float flMaxDist );
	virtual void		ValidateCurrentObserverTarget( void );

	void CheckUncoveringSpies( CTFPlayer *pTouchedPlayer );
	void Touch( CBaseEntity *pOther );

	void TeamFortress_SetSpeed();
	EHANDLE TeamFortress_GetDisguiseTarget( int nTeam, int nClass );

	void TeamFortress_ClientDisconnected();
	void RemoveAllOwnedEntitiesFromWorld( bool bSilent = true );
	void RemoveOwnedProjectiles( void );

	CTFTeamSpawn *GetSpawnPoint( void ){ return m_pSpawnPoint; }
		
	void SetAnimation( PLAYER_ANIM playerAnim );

	bool IsPlayerClass( int iClass ) const;
	bool IsPlayerNPCClass( void ) const;

	void PlayFlinch( const CTakeDamageInfo &info );

	float PlayCritReceivedSound( void );
	void PainSound( const CTakeDamageInfo &info );
	void DeathSound( const CTakeDamageInfo &info );

	// TF doesn't want the explosion ringing sound
	virtual void			OnDamagedByExplosion( const CTakeDamageInfo &info ) { return; }

	void	OnBurnOther( CBaseEntity *pTFPlayerVictim );

	// Buildables
	void				SetWeaponBuilder( CTFWeaponBuilder *pBuilder );
	CTFWeaponBuilder*	GetWeaponBuilder( void );
	CBaseEntity				*m_pVehicle;

	int					GetBuildResources( void );
	void				RemoveBuildResources( int iAmount );
	void				AddBuildResources( int iAmount );

	bool				IsBuilding( void );
	int					CanBuild( int iObjectType, int iObjectMode );

	CBaseObject*		GetObject( int index );
	int					GetObjectCount( void );
	int					GetNumObjects( int iObjectType, int iObjectMode );
	void				RemoveAllObjects( bool bSilent );
	void				StopPlacement( void );
	int					StartedBuildingObject( int iObjectType );
	void				StoppedBuilding( int iObjectType );
	void				FinishedObject( CBaseObject *pObject );
	void				AddObject( CBaseObject *pObject );
	void				OwnedObjectDestroyed( CBaseObject *pObject );
	void				RemoveObject( CBaseObject *pObject );
	bool				PlayerOwnsObject( CBaseObject *pObject );
	void				DetonateOwnedObjectsOfType( int iType, int iMode );
	void				StartBuildingObjectOfType( int iType, int iMode );
	CBaseObject*		GetObjectOfType( int iType, int iMode );

	CTFTeam *GetTFTeam( void );
	CTFTeam *GetOpposingTFTeam( void );

	void				TeleportEffect( void );
	void				RemoveTeleportEffect( void );
	void				CallForNPCMedic( void );
	virtual bool		IsAllowedToPickUpFlag( void );
	bool				HasTheFlag( void );
	//bool 				HasTheFlag( ETFFlagType flagtype, int ?? );

	// Death & Ragdolls.
	virtual void	CreateRagdollEntity( void );
	void				CreateRagdollEntity( bool bGibbed, bool bBurning, bool bElectrocute, bool bOnGround, bool bCloak, bool bGoldStatue, bool bIceStatue, bool bDisintigrate, int iDamageCustom, bool bCreatePhysics );
	void			DestroyRagdoll( void );
	void			CreateFeignDeathRagdoll( CTakeDamageInfo const &info, bool bGibbed, bool bBurning, bool bFriendlyDisguise );
	CNetworkHandle( CBaseEntity, m_hRagdoll );	// networked entity handle 
	virtual bool	ShouldGib( const CTakeDamageInfo &info );

	// Dropping stuff
	void			DropAmmoPack( CTakeDamageInfo const &info, bool bLunchbox = false, bool bFeigning = false );
	//void			DropAmmoPackFromProjectile( CBaseEntity *pProjectile );
	void			DropWeapon( CTFWeaponBase *pWeapon, bool bKilled = false );
	void			DropHealthPack( CTakeDamageInfo const &info, bool bSomething = false );
	//void			DropCurrencyPack( CurrencyRewards_t eType, int iAmount, bool bWot, CBasePlayer *pKiller );
	void			DropDeathCallingCard( CTFPlayer *pKiller, CTFPlayer *pVictim );
	void			DropRune( void );
	void			DropReviveMarker( void );
	void			BecomeZombie( CBaseEntity *pKiller );

	bool			CanDisguise( void );
	bool			CanDisguise_OnKill( void );
	bool			CanGoInvisible( void );
	void			RemoveInvisibility( void );
	void			RemoveDisguise( void );
	void			PrintTargetWeaponInfo( void );

	bool			DoClassSpecialSkill( void );
	void			EndClassSpecialSkill( void );

	float			GetLastDamageTime( void ) { return m_flLastDamageTime; }

	void			SetClassMenuOpen( bool bIsOpen );
	bool			IsClassMenuOpen( void );

	float			GetCritMult( void ) { return m_Shared.GetCritMult(); }
	void 			RecordDamageEvent( const CTakeDamageInfo &info, bool bKill ) { m_Shared.RecordDamageEvent(info,bKill); }

	bool			GetHudClassAutoKill( void ){ return m_bHudClassAutoKill; }
	void			SetHudClassAutoKill( bool bAutoKill ){ m_bHudClassAutoKill = bAutoKill; }

	bool			GetMedigunAutoHeal( void ){ return m_bMedigunAutoHeal; }
	void			SetMedigunAutoHeal( bool bMedigunAutoHeal ){ m_bMedigunAutoHeal = bMedigunAutoHeal; }

	bool			ShouldAutoRezoom( void ) { return m_bAutoRezoom; }
	void			SetAutoRezoom( bool bAutoRezoom ) { m_bAutoRezoom = bAutoRezoom; }

	bool			ShouldAutoReload( void ) { return m_bAutoReload; }
	void			SetAutoReload( bool bAutoReload ) { m_bAutoReload = bAutoReload; }

	bool			ShouldFlipViewModel( void ) { return m_bFlipViewModel; }
	void			SetFlipViewModel( bool bFlip ) { m_bFlipViewModel = bFlip; }

	virtual void	ModifyOrAppendCriteria( AI_CriteriaSet& criteriaSet );

	virtual bool	CanHearAndReadChatFrom( CBasePlayer *pPlayer );

	Vector 	GetClassEyeHeight( void );

	void	UpdateExpression( void );
	void	ClearExpression( void );

	void	AddPhaseEffects( void );

	virtual IResponseSystem *GetResponseSystem();
	virtual bool			SpeakConceptIfAllowed( int iConcept, const char *modifiers = NULL, char *pszOutResponseChosen = NULL, size_t bufsize = 0, IRecipientFilter *filter = NULL );

	virtual bool CanSpeakVoiceCommand( void );
	virtual bool ShouldShowVoiceSubtitleToEnemy( void );
	virtual void NoteSpokeVoiceCommand( const char *pszScenePlayed );
	void	SpeakWeaponFire( int iCustomConcept = MP_CONCEPT_NONE );
	void	ClearWeaponFireScene( void );

	virtual int DrawDebugTextOverlays( void );

	float		m_flNextVoiceCommandTime;
	float		m_flNextSpeakWeaponFire;
	float		m_flNextGrenadesFire;

	virtual int	CalculateTeamBalanceScore( void );

	bool ShouldAnnouceAchievement( void );

	virtual void SetupVisibility( CBaseEntity *pViewEntity, unsigned char *pvs, int pvssize );
	virtual void UpdatePortalViewAreaBits( unsigned char *pvs, int pvssize );
	
	virtual void		PlayStepSound( Vector &vecOrigin, surfacedata_t *psurface, float fvol, bool force );
	virtual const char	*GetOverrideStepSound( const char *pszBaseStepSoundName );
	virtual bool		IsDeflectable( void ) { return true; }

	virtual CAttributeManager *GetAttributeManager( void ) { return &m_AttributeManager; }
	virtual CAttributeContainer *GetAttributeContainer( void ) { return NULL; }
	virtual CBaseEntity *GetAttributeOwner( void ) { return NULL; }
	virtual CAttributeList *GetAttributeList() { return &m_AttributeList; }
	virtual void ReapplyProvision( void ) { /*Do nothing*/ };

	// Entity inputs
	void	InputIgnitePlayer( inputdata_t &inputdata );
	void	InputExtinguishPlayer( inputdata_t &inputdata );
	void	InputBleedPlayer( inputdata_t &inputdata );
	void	InputSpeakResponseConcept( inputdata_t &inputdata );
	void	InputSetForcedTauntCam( inputdata_t &inputdata );

	// HL2 ladder related methods
	LadderMove_t		*GetLadderMove() { return &m_LadderMove; }
	virtual void		ExitLadder();
	virtual surfacedata_t *GetLadderSurface( const Vector &origin );

	// physics interactions
	virtual void		PickupObject( CBaseEntity *pObject, bool bLimitMassAndSize );
	virtual	bool		IsHoldingEntity( CBaseEntity *pEnt );
	virtual void		ForceDropOfCarriedPhysObjects( CBaseEntity *pOnlyIfHoldindThis );
	virtual float		GetHeldObjectMass( IPhysicsObject *pHeldObject );
	virtual CBaseEntity	*GetHeldObject( void );

	virtual bool		IsFollowingPhysics( void ) { return (m_afPhysicsFlags & PFLAG_ONBARNACLE) > 0; }
	void				InputForceDropPhysObjects( inputdata_t &data );

	void ToggleHeldObjectOnOppositeSideOfPortal( void ) { m_bHeldObjectOnOppositeSideOfPortal = !m_bHeldObjectOnOppositeSideOfPortal; }
	void SetHeldObjectOnOppositeSideOfPortal( bool p_bHeldObjectOnOppositeSideOfPortal ) { m_bHeldObjectOnOppositeSideOfPortal = p_bHeldObjectOnOppositeSideOfPortal; }
	bool IsHeldObjectOnOppositeSideOfPortal( void ) { return m_bHeldObjectOnOppositeSideOfPortal; }
	CProp_Portal *GetHeldObjectPortal( void ) { return m_pHeldObjectPortal; }
	void SetHeldObjectPortal( CProp_Portal *pPortal ) { m_pHeldObjectPortal = pPortal; }

	void SetStuckOnPortalCollisionObject( void ) { m_bStuckOnPortalCollisionObject = true; }

	void IncrementPortalsPlaced( void );
	void IncrementStepsTaken( void );
	void UpdateSecondsTaken( void );
	void ResetThisLevelStats( void );
	int NumPortalsPlaced( void ) const { return m_StatsThisLevel.iNumPortalsPlaced; }
	int NumStepsTaken( void ) const { return m_StatsThisLevel.iNumStepsTaken; }
	float NumSecondsTaken( void ) const { return m_StatsThisLevel.fNumSecondsTaken; }

	void SetNeuroToxinDamageTime( float fCountdownSeconds ) { m_fNeuroToxinDamageTime = gpGlobals->curtime + fCountdownSeconds; }

	void IncNumCamerasDetatched( void ) { ++m_iNumCamerasDetatched; }
	int GetNumCamerasDetatched( void ) const { return m_iNumCamerasDetatched; }

	bool m_bSilentDropAndPickup;

	// Required for func_tank and some other things.
	virtual Vector	EyeDirection2D( void );
	virtual Vector	EyeDirection3D( void );

	virtual Vector	HeadTarget( const Vector &posSrc );

	// Commander Mode for controller NPCs
	enum CommanderCommand_t
	{
		CC_NONE,
		CC_TOGGLE,
		CC_FOLLOW,
		CC_SEND,
	};

	virtual CAI_Squad	*GetPlayerSquad() { return m_pPlayerAISquad; }
	virtual CAI_Squad	*GetPlayerSquad() const { return m_pPlayerAISquad; }
	virtual void	CommanderMode();
	void			CommanderUpdate();
	void			CommanderExecute( CommanderCommand_t command = CC_TOGGLE );
	bool			CommanderFindGoal( commandgoal_t *pGoal );
	void			NotifyFriendsOfDamage( CBaseEntity *pAttackerEntity );
	CAI_BaseNPC		*GetSquadCommandRepresentative();
	int				GetNumSquadCommandables();
	int				GetNumSquadCommandableMedics();

	// Locator
	void			UpdateLocatorPosition( const Vector &vecPosition );

	// Transition
	void			SaveForTransition( void );
	void			LoadSavedTransition( void );
	void			DeleteForTransition( void );

	virtual void	ResetPerRoundStats( void );
	virtual void	UpdatePlayerSound( void );

	bool			IsOnStoryTeam( void ) { return ( GetTeamNumber() == TF_STORY_TEAM ); }
	bool			IsOnCombineTeam( void ) { return ( GetTeamNumber() == TF_COMBINE_TEAM ); }

	char			GetPrevTextureType( void ) { return m_chPreviousTextureType; }

	Class_T			Classify ( void );

	void			CombineBallSocketed( CPropCombineBall *pCombineBall );

	void			SetLocatorTargetEntity( CBaseEntity *pEntity ) { m_hLocatorTargetEntity.Set( pEntity ); }

	void			FirePlayerProxyOutput( const char *pszOutputName, variant_t variant, CBaseEntity *pActivator, CBaseEntity *pCaller );

	CLogicPlayerProxy	*GetPlayerProxy( void );

	void			StartWaterDeathSounds( void );
	void			StopWaterDeathSounds( void );

	void			MissedAR2AltFire();

	inline void		EnableCappedPhysicsDamage();
	inline void		DisableCappedPhysicsDamage();

	const impactdamagetable_t &GetPhysicsImpactDamageTable();

	void			ApplyAbsVelocityImpulse( const Vector &vecImpulse );
	virtual void	ApplyGenericPushbackImpulse( const Vector &vecDir );

	void			FeignDeath( const CTakeDamageInfo &info );

	virtual void	OnAchievementEarned( int iAchievement );

	bool			IsSearchingSpawn( void ) { return m_bSearchingSpawn; }

	void			ThrowGrenade( int nType );

	void			UseActionSlotItemPressed( void );
	void			UseActionSlotItemReleased( void );

	void			InspectButtonPressed( void );
	void			InspectButtonReleased( void );

	void			MerasmusPlayerBombExplode( bool bKilled = false );

	bool			CanAirDash( void ) const;
	bool			CanBeForcedToLaugh( void );
	virtual bool	CanBreatheUnderwater() const;
	bool			CanDuck( void ) const;
	bool			CanGetWet( void ) const;
	bool			CanJump( void ) const;
	bool			CanMoveDuringTaunt( void );
	bool			CanPickupDroppedWeapon( const CTFDroppedWeapon *pWeapon );
	bool			CanPlayerMove( void );

	bool			IsInspecting( void ) const;

	CBaseEntity		*GetGrapplingHookTarget( void ) { return m_hGrapplingHookTarget; }

	CSoundPatch		*m_sndLeeches;
	CSoundPatch		*m_sndWaterSplashes;

	CSoundPatch		*m_sndTauntLoop;

public:

	CTFPlayerShared m_Shared;

	int	    item_list;			// Used to keep track of which goalitems are 
								// affecting the player at any time.
								// GoalItems use it to keep track of their own 
								// mask to apply to a player's item_list

	float invincible_finished;
	float invisible_finished;
	float super_damage_finished;
	float radsuit_finished;

	int m_flNextTimeCheck;		// Next time the player can execute a "timeleft" command

	// TEAMFORTRESS VARIABLES
	int		no_sentry_message;
	int		no_dispenser_message;
	
	CNetworkVar( bool, m_bSaveMeParity );
	CNetworkVar( bool, m_bAllowMoveDuringTaunt );
	CNetworkVar( bool, m_bIsReadyToHighFive );
	CNetworkHandle( CTFPlayer, m_hHighFivePartner );
	CNetworkVar( float, m_flCurrentTauntMoveSpeed );
	CNetworkVar( float, m_flTauntYaw );
	CNetworkVar( int, m_iTauntItemDefIndex );
	CNetworkVar( int, m_nActiveTauntSlot );
	CNetworkVar( int, m_nForceTauntCam );

	CNetworkVar( bool, m_bHasLongJump );

	// teleporter variables
	int		no_entry_teleporter_message;
	int		no_exit_teleporter_message;

	float	m_flNextNameChangeTime;

	bool	m_bBlastLaunched;
	bool	m_bForceByNature;

	int					StateGet( void ) const;

	void				SetOffHandWeapon( CTFWeaponBase *pWeapon );
	void				HolsterOffHandWeapon( void );

	float				GetSpawnTime() { return m_flSpawnTime; }

	virtual bool Weapon_Switch( CBaseCombatWeapon *pWeapon, int viewmodelindex = 0 );
	virtual void Weapon_Drop( CBaseCombatWeapon *pWeapon, const Vector *pvecTarget , const Vector *pVelocity );

	bool				ItemsMatch( CEconItemView *pItem1, CEconItemView *pItem2, CTFWeaponBase *pWeapon = NULL );
	void				ValidateWeapons( bool bRegenerate );
	void				ValidateWearables( void );
	void				ManageRegularWeapons( TFPlayerClassData_t *pData );
	void				ManageRegularWeaponsLegacy( TFPlayerClassData_t *pData );
	void				ManageRandomWeapons( TFPlayerClassData_t *pData );
	void				ManageBuilderWeapons( TFPlayerClassData_t *pData );
	void				ManageTeamWeapons( TFPlayerClassData_t *pData );
	void				ManageGrenades( TFPlayerClassData_t *pData );

	void				PostInventoryApplication( void );

	CTriggerAreaCapture*GetControlPointStandingOn( void );
	bool				IsCapturingPoint( void );

	// Taunts.
	void				Taunt( taunts_t eTaunt = TAUNTATK_NONE, int iConcept = MP_CONCEPT_PLAYER_TAUNT );
	bool				IsTaunting( void ) { return m_Shared.InCond( TF_COND_TAUNTING ); }
	void				DoTauntAttack( void );
	void				ClearTauntAttack( void );
	void				EndLongTaunt( void );
	void				StopTaunt( bool bForce = false );
	void				StopTauntSoundLoop( void );
	bool				IsAllowedToTaunt( void );
	//bool				IsAllowedToInitiateTauntWithPartner( const CEconItemView* pItem, char*, int)
	void				ParseSharedTauntDataFromEconItemView( const CEconItemView* pItem );
	void				PlayTauntSceneFromItem( const CEconItemView* pItem );
	void				PlayTauntSceneFromItemID( int iItemID );
	void				PlayTauntOutroScene( void );
	void				PlayTauntRemapInputScene( void );
	//void				OnTauntSucceeded( char const*, int, int )
	void				SetCurrentTauntMoveSpeed( float flSpeed ) { m_flCurrentTauntMoveSpeed = flSpeed;}
	//void				SetTauntYaw( float )
	//bool				ShouldStopTaunting( void );
	void				AcceptTauntWithPartner( CTFPlayer *pPartner );
	void				HandleTauntCommand( int iSlot );
	//void				HandleWeaponSlotAfterTaunt( void );
	//void				FindOpenTauntPartnerPosition( const CEconItemView* pItem, Vector&, float*);
	//void				FindPartnerTauntInitiator( void );

	QAngle				m_angTauntCamera;
	Vector				m_vecTauntCamera;

	virtual float		PlayScene( const char *pszScene, float flDelay = 0.0f, AI_Response *response = NULL, IRecipientFilter *filter = NULL );
	void				ResetTauntHandle( void )				{ m_hTauntScene = NULL; }
	void				SetDeathFlags( int iDeathFlags ) { m_iDeathFlags = iDeathFlags; }
	int					GetDeathFlags() { return m_iDeathFlags; }
	void				SetMaxSentryKills( int iMaxSentryKills ) { m_iMaxSentryKills = iMaxSentryKills; }
	int					GetMaxSentryKills() { return m_iMaxSentryKills; }

	void				DoNoiseMaker( void );

	CNetworkVar( int, m_iSpawnCounter );
	
	void				CheckForIdle( void );
	void				PickWelcomeObserverPoint();

	void				StopRandomExpressions( void ) { m_flNextRandomExpressionTime = -1; }
	void				StartRandomExpressions( void ) { m_flNextRandomExpressionTime = gpGlobals->curtime; }

	virtual bool			WantsLagCompensationOnEntity( const CBaseEntity	*pEntity, const CUserCmd *pCmd, const CBitVec<MAX_EDICTS> *pEntityTransmitBits ) const;

	float				MedicGetChargeLevel( void );
	CBaseEntity			*MedicGetHealTarget( void );

	CWeaponMedigun		*GetMedigun( void );
	CTFWeaponBase		*Weapon_OwnsThisID( int iWeaponID );
	CTFWeaponBase		*Weapon_GetWeaponByType( int iType );
	CEconEntity			*GetEntityForLoadoutSlot( int iSlot );
	CEconWearable		*GetWearableForLoadoutSlot( int iSlot );

	bool CalculateAmmoPackPositionAndAngles( CTFWeaponBase *pWeapon, Vector &vecOrigin, QAngle &vecAngles );

	// Stun
	void				StunSound( CTFPlayer *pStunner, int nStunFlags/*, int nCurrentStunFlags*/ );

	bool				SelectFurthestSpawnSpot( const char *pEntClassName, CBaseEntity* &pSpot, bool bTelefrag = true );

	// Vintage Gunslinger
	bool				HasGunslinger( void ) { return m_Shared.m_bGunslinger; }

	float				GetDesiredHeadScale( void );
	float				GetDesiredTorsoScale( void );
	float				GetDesiredHandScale( void );
	float				GetHeadScaleSpeed( void );
	float				GetTorsoScaleSpeed( void );
	float				GetHandScaleSpeed( void );

	// Custom Models
	CNetworkString( m_iszCustomModel, MAX_PATH );
	bool				m_bCustomModelRotates;
	bool				m_bCustomModelRotationSet;
	bool				m_bCustomModelVisibleToSelf;
	Vector				m_vecCustomModelOffset;
	QAngle				m_angCustomModelRotation;

	void	InputSetCustomModel( inputdata_t &inputdata );
	//void	InputSetCustomModelOffset( inputdata_t &inputdata );
	//void	InputSetCustomModelRotates( inputdata_t &inputdata );
	//void	InputSetCustomModelRotation( inputdata_t &inputdata );
	//void	InputSetCustomModelVisibleToSelf( inputdata_t &inputdata );
	//void	InputClearCustomModelRotation( inputdata_t &inputdata );

	void				SetGrapplingHookTarget( CBaseEntity *pTarget, bool bSomething = false );

	CountdownTimer m_purgatoryDuration;

private:

	int					GetAutoTeam( void );

	// Creation/Destruction.
	virtual void		InitClass( void );
	void				GiveDefaultItems();
	bool				SelectSpawnSpot( const char *pEntClassName, CBaseEntity* &pSpot );
	void				SearchCoopSpawnSpot( void );
	bool				ShouldUseCoopSpawning( void );
	void				PrecachePlayerModels( void );
	void				RemoveNemesisRelationships();

	// Think.
	void				TFPlayerThink();
	void				RegenThink();
	void				RuneRegenThink();
	void				UpdateTimers( void );

public:
	void				RemoveMeleeCrit( void );

	void				AddCustomAttribute( char const *pAttribute, float flValue, float flDuration = PERMANENT_ATTRIBUTE );
	void				UpdateCustomAttributes( void );
	void				RemoveCustomAttribute( char const *pAttribute );

	//void				RemoveAllCustomAttributes( void );
	//void				RemovePlayerAttributes( bool bRemove? );

	CNetworkVar( int,	m_iSquadMemberCount );
	CNetworkVar( int,	m_iSquadMedicCount );
	CNetworkVar( bool,	m_fSquadInFollowMode );
	CNetworkVar( Vector, m_vecLocatorOrigin );

private:
	// Taunt.
	EHANDLE				m_hTauntScene;
	bool				m_bInitTaunt;

	// Client commands.
	void				HandleCommand_JoinTeam( const char *pTeamName );
	void				HandleCommand_JoinClass( const char *pClassName );
	void				HandleCommand_JoinTeam_NoMenus( const char *pTeamName );
	void				HandleCommand_JoinTeam_NoKill( const char *pTeamName );

	// Bots.
	friend void			Bot_Think( CTFPlayer *pBot );

	// Physics.
	void				PhysObjectSleep();
	void				PhysObjectWake();

	// Ammo pack.
	void AmmoPackCleanUp( void );
	void DroppedWeaponCleanUp( void );

	// State.
	CPlayerStateInfo	*StateLookupInfo( int nState );
	void				StateEnter( int nState );
	void				StateLeave( void );
	void				StateTransition( int nState );
	void				StateEnterWELCOME( void );
	void				StateThinkWELCOME( void );
	void				StateEnterPICKINGTEAM( void );
	void				StateEnterACTIVE( void );
	void				StateEnterOBSERVER( void );
	void				StateThinkOBSERVER( void );
	void				StateEnterDYING( void );
	void				StateThinkDYING( void );

	virtual bool		SetObserverMode(int mode);
	virtual void		AttemptToExitFreezeCam( void );

	bool				PlayGesture( const char *pGestureName );
	bool				PlaySpecificSequence( const char *pSequenceName );
	bool				PlayDeathAnimation( const CTakeDamageInfo &info, CTakeDamageInfo &info_modified );

	bool				GetResponseSceneFromConcept( int iConcept, char *chSceneBuffer, int numSceneBufferBytes );

	bool				CommanderExecuteOne( CAI_BaseNPC *pNpc, const commandgoal_t &goal, CAI_BaseNPC **Allies, int numAllies );

private:
	// Map introductions
	int					m_iIntroStep;
	CHandle<CIntroViewpoint> m_hIntroView;
	float				m_flIntroShowHintAt;
	float				m_flIntroShowEventAt;
	bool				m_bHintShown;
	bool				m_bAbortFreezeCam;
	bool				m_bSeenRoundInfo;
	bool				m_bRegenerating;

	// Items.
	CNetworkHandle( CTFItem, m_hItem );

	// Combat.
	CNetworkHandle( CTFWeaponBase, m_hOffHandWeapon );
	CNetworkHandle( CBaseEntity, m_hGrapplingHookTarget );

	float					m_flHealthBuffTime;

	float					m_flNextRegenerateTime;
	float					m_flNextChangeClassTime;
	float					m_flNextChangeTeamTime;
	float					m_flNextHealthRegen;

	// Ragdolls.
	Vector					m_vecTotalBulletForce;

	// State.
	CPlayerStateInfo		*m_pStateInfo;

	// Spawn Point
	CTFTeamSpawn			*m_pSpawnPoint;

	CSoundPatch		*m_pWooshSound;

	// Networked.
	CNetworkQAngle( m_angEyeAngles );					// Copied from EyeAngles() so we can send it to the client.

public:
	QAngle				m_angPrevEyeAngles;

protected:
	CTFPlayerClass		m_PlayerClass;
	int					m_WeaponPreset[TF_CLASS_COUNT_ALL][LOADOUT_POSITION_BUFFER];

private:
	CTFPlayerAnimState	*m_PlayerAnimState;
	int					m_iLastWeaponFireUsercmd;				// Firing a weapon.  Last usercmd we shot a bullet on.
	int					m_iLastSkin;

	CNetworkVar( float, m_flLastDamageTime );
	CNetworkVar( float, m_flNextNoiseMakerTime );

	float				m_flNextPainSoundTime;
	int					m_LastDamageType;
	int					m_iDeathFlags;				// TF_DEATH_* flags with additional death info
	int					m_iMaxSentryKills;			// most kills by a single sentry

	bool				m_bPlayedFreezeCamSound;

	CHandle< CTFWeaponBuilder > m_hWeaponBuilder;

	CUtlVector<EHANDLE>	m_aObjects;			// List of player objects

	bool m_bIsClassMenuOpen;

	Vector m_vecLastDeathPosition;

	float				m_flSpawnTime;

	float				m_flLastAction;
	bool				m_bIsIdle;

	CUtlVector<EHANDLE>	m_hObservableEntities;
	DamagerHistory_t m_DamagerHistory[MAX_DAMAGER_HISTORY];	// history of who has damaged this player
	CUtlVector<float>	m_aBurnOtherTimes;					// vector of times this player has burned others

	// Background expressions
	string_t			m_iszExpressionScene;
	EHANDLE				m_hExpressionSceneEnt;
	float				m_flNextRandomExpressionTime;
	EHANDLE				m_hWeaponFireSceneEnt;

	bool				m_bSpeakingConceptAsDisguisedSpy;

	bool				m_bHudClassAutoKill;
	bool 				m_bMedigunAutoHeal;
	bool				m_bAutoRezoom;	// does the player want to re-zoom after each shot for sniper rifles
	bool				m_bAutoReload;
	bool				m_bFlipViewModel;

	float				m_flTauntAttackTime;
	int					m_iTauntAttack;
	//int					m_iTauntConcept;
	//int					m_iTauntIndex;

	// Gunslinger taunt
	short				m_nTauntDamageCount;

	float				m_flNextCarryTalkTime;

	EHANDLE				m_hTempSpawnSpot;
	CHandle<CTFReviveMarker> m_hReviveSpawnSpot;
	CNetworkVar( bool, m_bSearchingSpawn )

	int					m_nBlastJumpFlags;
	bool				m_bJumpEffect;

	CNetworkVar( bool, m_bTyping );

	friend class CAttributeContainerPlayer;
	CAttributeContainerPlayer m_AttributeManager;

	COutputEvent		m_OnDeath;

	// HL2 squad stuff
	CAI_Squad *			m_pPlayerAISquad;
	CSimpleSimTimer		m_CommanderUpdateTimer;
	float				m_RealTimeLastSquadCommand;
	CommanderCommand_t	m_QueuedCommand;

	// Suit power fields
	float				m_flSuitPowerLoad;	// net suit power drain (total of all device's drainrates)
	float				m_flAdmireGlovesAnimTime;

	float				m_flNextFlashlightCheckTime;
	float				m_flFlashlightPowerDrainScale;

	EHANDLE				m_hPlayerProxy;

	bool				m_bFlashlightDisabled;
	bool				m_bUseCappedPhysicsDamageTable;

	//CNetworkVar( int, m_iSpawnInterpCounter );
	//CNetworkVar( int, m_iPlayerSoundType );

	CNetworkVar( bool, m_bHeldObjectOnOppositeSideOfPortal );
	CNetworkHandle( CProp_Portal, m_pHeldObjectPortal );	// networked entity handle

	bool				m_bIntersectingPortalPlane;
	bool				m_bStuckOnPortalCollisionObject;

	float				m_fNeuroToxinDamageTime;

	PortalPlayerStatistics_t m_StatsThisLevel;
	float				m_fTimeLastNumSecondsUpdate;

	int					m_iNumCamerasDetatched;

	QAngle				m_qPrePortalledViewAngles;
	bool				m_bFixEyeAnglesFromPortalling;
	VMatrix				m_matLastPortalled;

	CNetworkVar( float, m_flHeadScale );
	CNetworkVar( float, m_flTorsoScale );
	CNetworkVar( float, m_flHandScale );

	bool				m_bTransitioned;

public:
	bool				SetPowerplayEnabled( bool bOn );
	bool				PlayerHasPowerplay( void );
	bool				IsDeveloper( void ) { return m_Shared.m_bIsPlayerADev; }
	bool				IsNicknine( void );
	void				PowerplayThink( void );
	float				m_flPowerPlayTime;

	// HL2 Ladder related data
	CNetworkVar( EHANDLE, m_hLadder );
	LadderMove_t		m_LadderMove;

	bool				m_bInTransition;

	CNetworkVar( bool, m_bPitchReorientation );
	CNetworkHandle( CProp_Portal, m_hPortalEnvironment ); //if the player is in a portal environment, this is the associated portal
	CNetworkHandle( CFunc_LiquidPortal, m_hSurroundingLiquidPortal ); 

	EHANDLE				m_hLocatorTargetEntity; // The entity that's being tracked by the suit locator.

	friend class CProp_Portal;
};

//-----------------------------------------------------------------------------
// FIXME: find a better way to do this
// Switches us to a physics damage table that caps the max damage.
//-----------------------------------------------------------------------------
void CTFPlayer::EnableCappedPhysicsDamage()
{
	m_bUseCappedPhysicsDamageTable = true;
}


void CTFPlayer::DisableCappedPhysicsDamage()
{
	m_bUseCappedPhysicsDamageTable = false;
}

//-----------------------------------------------------------------------------
// Purpose: Utility function to convert an entity into a tf player.
//   Input: pEntity - the entity to convert into a player
//-----------------------------------------------------------------------------
inline CTFPlayer *ToTFPlayer( CBaseEntity *pEntity )
{
	if ( !pEntity || !pEntity->IsPlayer() )
		return NULL;

	Assert( dynamic_cast<CTFPlayer*>( pEntity ) != 0 );
	return static_cast< CTFPlayer* >( pEntity );
}

inline int CTFPlayer::StateGet( void ) const
{
	return m_Shared.m_nPlayerState;
}



#endif	// TF_PLAYER_H
