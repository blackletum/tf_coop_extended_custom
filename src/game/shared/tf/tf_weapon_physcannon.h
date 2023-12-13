//========= Copyright Valve Corporation, All rights reserved. =================
//
// Purpose: 
//
//=============================================================================

#ifndef TF_WEAPON_PHYSCANNON_H
#define TF_WEAPON_PHYSCANNON_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"

#ifdef CLIENT_DLL
	#include "beamdraw.h"
	#include "fx_interpvalue.h"
	#include "iviewrender_beams.h"
#else

#endif

#include "soundenvelope.h"
#include "physics.h"
#include "physics_saverestore.h"
#include "tf_weaponbase.h"

#define PHYSCANNON_BEAM_SPRITE "sprites/orangelight1.vmt"
#define PHYSCANNON_BEAM_SPRITE_NOZ "sprites/orangelight1_noz.vmt"
#define PHYSCANNON_GLOW_SPRITE "sprites/glow04_noz"
#define PHYSCANNON_ENDCAP_SPRITE "sprites/orangeflare1"
#define PHYSCANNON_CENTER_GLOW "sprites/orangecore1"
#define PHYSCANNON_BLAST_SPRITE "sprites/orangecore2"

#define MEGACANNON_BEAM_SPRITE "sprites/lgtning.vmt"
#define MEGACANNON_BEAM_SPRITE_NOZ "sprites/lgtning_noz.vmt"
#define MEGACANNON_GLOW_SPRITE "sprites/blueflare1_noz.vmt"
#define MEGACANNON_ENDCAP_SPRITE "sprites/blueflare1_noz.vmt"
#define MEGACANNON_CENTER_GLOW "effects/fluttercorez.vmt"
#define MEGACANNON_BLAST_SPRITE "effects/fluttercorez.vmt"

#define MEGACANNON_RAGDOLL_BOOGIE_SPRITE "sprites/lgtning_noz.vmt"

#define	MEGACANNON_MODEL "models/weapons/v_superphyscannon.mdl"
#define	MEGACANNON_SKIN	1
#define PHYSCANNON_MODEL_SCOUT "models/weapons/v_physcannon_scout.mdl"
#define PHYSCANNON_MODEL_SOLDIER "models/weapons/v_physcannon_soldier.mdl"
#define PHYSCANNON_MODEL_PYRO "models/weapons/v_physcannon_pyro.mdl"
#define PHYSCANNON_MODEL_DEMO "models/weapons/v_physcannon_demoman.mdl"
#define PHYSCANNON_MODEL_HEAVY "models/weapons/v_physcannon_heavy.mdl"
#define PHYSCANNON_MODEL_ENGINEER "models/weapons/v_physcannon_engineer.mdl"
#define PHYSCANNON_MODEL_MEDIC "models/weapons/v_physcannon_medic.mdl"
#define PHYSCANNON_MODEL_SNIPER "models/weapons/v_physcannon_sniper.mdl"
#define PHYSCANNON_MODEL_SPY "models/weapons/v_physcannon_spy.mdl"
#define PHYSCANNON_MODEL_WILDCARD "models/weapons/c_physcannon.mdl"
#define PHYSCANNON_MODEL_CIVILIAN "models/weapons/v_physcannon.mdl"
#define PHYSCANNON_MODEL_COMBINE "models/weapons/v_physcannon.mdl"
#define PHYSCANNON_MODEL_ZOMBIEFAST "models/weapons/v_physcannon.mdl"
#define MEGACANNON_MODEL_SCOUT "models/weapons/v_superphyscannon_scout.mdl"
#define MEGACANNON_MODEL_SOLDIER "models/weapons/v_superphyscannon_soldier.mdl"
#define MEGACANNON_MODEL_PYRO "models/weapons/v_superphyscannon_pyro.mdl"
#define MEGACANNON_MODEL_DEMO "models/weapons/v_superphyscannon_demoman.mdl"
#define MEGACANNON_MODEL_HEAVY "models/weapons/v_superphyscannon_heavy.mdl"
#define MEGACANNON_MODEL_ENGINEER "models/weapons/v_superphyscannon_engineer.mdl"
#define MEGACANNON_MODEL_MEDIC "models/weapons/v_superphyscannon_medic.mdl"
#define MEGACANNON_MODEL_SNIPER "models/weapons/v_superphyscannon_sniper.mdl"
#define MEGACANNON_MODEL_SPY "models/weapons/v_superphyscannon_spy.mdl"
#define MEGACANNON_MODEL_WILDCARD "models/weapons/c_superphyscannon.mdl"
#define MEGACANNON_MODEL_CIVILIAN "models/weapons/v_superphyscannon.mdl"
#define MEGACANNON_MODEL_COMBINE "models/weapons/v_superphyscannon.mdl"
#define MEGACANNON_MODEL_ZOMBIEFAST "models/weapons/v_superphyscannon.mdl"

#ifdef CLIENT_DLL
#define CWeaponPhysCannon C_WeaponPhysCannon
#define CWeaponPhysCannon_Secondary C_WeaponPhysCannon_Secondary
#endif

//-----------------------------------------------------------------------------
// Do we have the super-phys gun?
//-----------------------------------------------------------------------------
bool PlayerHasMegaPhysCannon();

// force the physcannon to drop an object (if carried)
void PhysCannonForceDrop(CBaseCombatWeapon *pActiveWeapon, CBaseEntity *pOnlyIfHoldingThis);
void PhysCannonBeginUpgrade(CBaseAnimating *pAnim);

bool PlayerPickupControllerIsHoldingEntity(CBaseEntity *pPickupController, CBaseEntity *pHeldEntity);
void ShutdownPickupController( CBaseEntity *pPickupControllerEntity );
float PlayerPickupGetHeldObjectMass(CBaseEntity *pPickupControllerEntity, IPhysicsObject *pHeldObject);
float PhysCannonGetHeldObjectMass(CBaseCombatWeapon *pActiveWeapon, IPhysicsObject *pHeldObject);

CBaseEntity *PhysCannonGetHeldEntity(CBaseCombatWeapon *pActiveWeapon);
CBaseEntity *GetPlayerHeldEntity(CBasePlayer *pPlayer);
CBasePlayer *GetPlayerHoldingEntity( CBaseEntity *pEntity );

// derive from this so we can add save/load data to it
struct game_shadowcontrol_params_t : public hlshadowcontrol_params_t
{
	DECLARE_SIMPLE_DATADESC();
};

//-----------------------------------------------------------------------------
class CGrabController : public IMotionEvent
{
public:
	DECLARE_SIMPLE_DATADESC();

	CGrabController( void );
	~CGrabController( void );
	void AttachEntity( CBasePlayer *pPlayer, CBaseEntity *pEntity, IPhysicsObject *pPhys, bool bIsMegaPhysCannon, const Vector &vGrabPosition, bool bUseGrabPosition );
	void DetachEntity( bool bClearVelocity );
	void OnRestore();

	bool UpdateObject( CBasePlayer *pPlayer, float flError );

	void SetTargetPosition( const Vector &target, const QAngle &targetOrientation );
	void GetTargetPosition( Vector *target, QAngle *targetOrientation );
	float ComputeError();
	float GetLoadWeight( void ) const { return m_flLoadWeight; }
	void SetAngleAlignment( float alignAngleCosine ) { m_angleAlignment = alignAngleCosine; }
	void SetIgnorePitch( bool bIgnore ) { m_bIgnoreRelativePitch = bIgnore; }
	QAngle TransformAnglesToPlayerSpace( const QAngle &anglesIn, CBasePlayer *pPlayer );
	QAngle TransformAnglesFromPlayerSpace( const QAngle &anglesIn, CBasePlayer *pPlayer );

	CBaseEntity *GetAttached() { return (CBaseEntity *)m_attachedEntity; }

	IMotionEvent::simresult_e Simulate( IPhysicsMotionController *pController, IPhysicsObject *pObject, float deltaTime, Vector &linear, AngularImpulse &angular );
	float GetSavedMass( IPhysicsObject *pObject );
	float GetSavedRotDamping( IPhysicsObject* pObject );

	bool IsObjectAllowedOverhead( CBaseEntity *pEntity );

	//set when a held entity is penetrating another through a portal. Needed for special fixes
	void SetPortalPenetratingEntity( CBaseEntity *pPenetrated );

private:
	// Compute the max speed for an attached object
	void ComputeMaxSpeed( CBaseEntity *pEntity, IPhysicsObject *pPhysics );

	game_shadowcontrol_params_t	m_shadow;
	float			m_timeToArrive;
	float			m_errorTime;
	float			m_error;
	float			m_contactAmount;
	float			m_angleAlignment;
	bool			m_bCarriedEntityBlocksLOS;
	bool			m_bIgnoreRelativePitch;

	float			m_flLoadWeight;
	float			m_savedRotDamping[VPHYSICS_MAX_OBJECT_LIST_COUNT];
	float			m_savedMass[VPHYSICS_MAX_OBJECT_LIST_COUNT];
	EHANDLE			m_attachedEntity;
	QAngle			m_vecPreferredCarryAngles;
	bool			m_bHasPreferredCarryAngles;
	float			m_flDistanceOffset;

	QAngle			m_attachedAnglesPlayerSpace;
	Vector			m_attachedPositionObjectSpace;

	IPhysicsMotionController *m_controller;

	bool			m_bAllowObjectOverhead; // Can the player hold this object directly overhead? (Default is NO)

	//set when a held entity is penetrating another through a portal. Needed for special fixes
	EHANDLE			m_PenetratedEntity;
	int				m_frameCount;
	friend class CWeaponPhysCannon;
	friend void GetSavedParamsForCarriedPhysObject( CGrabController *pGrabController, IPhysicsObject *pObject, float *pSavedMassOut, float *pSavedRotationalDampingOut );
};

CGrabController *GetGrabControllerForPlayer( CBasePlayer *pPlayer );
CGrabController *GetGrabControllerForPhysCannon( CBaseCombatWeapon *pActiveWeapon );
void GetSavedParamsForCarriedPhysObject( CGrabController *pGrabController, IPhysicsObject *pObject, float *pSavedMassOut, float *pSavedRotationalDampingOut );
void UpdateGrabControllerTargetPosition( CBasePlayer *pPlayer, Vector *vPosition, QAngle *qAngles );
bool PhysCannonAccountableForObject( CBaseCombatWeapon *pPhysCannon, CBaseEntity *pObject );

void GrabController_SetPortalPenetratingEntity( CGrabController *pController, CBaseEntity *pPenetrated );

#ifdef CLIENT_DLL

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//  CPhysCannonEffect class
//----------------------------------------------------------------------------------------------------------------------------------------------------------

class CPhysCannonEffect
{
public:
	CPhysCannonEffect( void ) : m_vecColor( 255, 255, 255 ), m_bVisible( true ), m_nAttachment( -1 ), m_flForceVisibleUntil( 0.0f ) {};

	void SetAttachment( int attachment ) { m_nAttachment = attachment; }
	int	GetAttachment( void ) const { return m_nAttachment; }

	void SetVisible( bool visible = true ) { m_bVisible = visible; }
	int IsVisible( void ) const { return m_bVisible || gpGlobals->curtime < m_flForceVisibleUntil; }
	void ForceVisibleUntil( float flTime ) { m_flForceVisibleUntil = flTime; }

	void SetColor( const Vector &color ) { m_vecColor = color; }
	const Vector &GetColor( void ) const { return m_vecColor; }

	bool SetMaterial(  const char *materialName, int i )
	{
		m_hMaterial[i].Init( materialName, TEXTURE_GROUP_CLIENT_EFFECTS );
		return ( m_hMaterial[i] != NULL );
	}

	CMaterialReference &GetMaterial( int i ) { return m_hMaterial[i]; }

	CInterpolatedValue &GetAlpha( void ) { return m_Alpha; }
	CInterpolatedValue &GetScale( void ) { return m_Scale; }

private:
	CInterpolatedValue	m_Alpha;
	CInterpolatedValue	m_Scale;

	Vector				m_vecColor;
	bool				m_bVisible;
	float				m_flForceVisibleUntil;
	int					m_nAttachment;
	CMaterialReference	m_hMaterial[2];
};

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//  CPhysCannonEffectBeam class
//----------------------------------------------------------------------------------------------------------------------------------------------------------

class CPhysCannonEffectBeam
{
public:
	CPhysCannonEffectBeam( void ) : m_pBeam( NULL ) {};

	~CPhysCannonEffectBeam( void )
	{
		Release();
	}

	void Release( void )
	{
		if ( m_pBeam != NULL )
		{
			m_pBeam->flags = 0;
			m_pBeam->die = gpGlobals->curtime - 1;
			
			m_pBeam = NULL;
		}
	}

	void Init( int startAttachment, int endAttachment, CBaseEntity *pEntity, bool firstPerson )
	{
		if ( m_pBeam != NULL )
			return;

		BeamInfo_t beamInfo;

		beamInfo.m_pStartEnt = pEntity;
		beamInfo.m_nStartAttachment = startAttachment;
		beamInfo.m_pEndEnt = pEntity;
		beamInfo.m_nEndAttachment = endAttachment;
		beamInfo.m_nType = TE_BEAMPOINTS;
		beamInfo.m_vecStart = vec3_origin;
		beamInfo.m_vecEnd = vec3_origin;
		
		if (PlayerHasMegaPhysCannon())
			beamInfo.m_pszModelName = (firstPerson) ? MEGACANNON_BEAM_SPRITE_NOZ : MEGACANNON_BEAM_SPRITE;
		else
			beamInfo.m_pszModelName = (firstPerson) ? PHYSCANNON_BEAM_SPRITE_NOZ : PHYSCANNON_BEAM_SPRITE;
		
		beamInfo.m_flHaloScale = 0.0f;
		beamInfo.m_flLife = 0.0f;
		
		if ( firstPerson )
		{
			beamInfo.m_flWidth = 0.0f;
			beamInfo.m_flEndWidth = 4.0f;
		}
		else
		{
			beamInfo.m_flWidth = 0.5f;
			beamInfo.m_flEndWidth = 2.0f;
		}

		beamInfo.m_flFadeLength = 0.0f;
		beamInfo.m_flAmplitude = 16;
		beamInfo.m_flBrightness = 255.0;
		beamInfo.m_flSpeed = 150.0f;
		beamInfo.m_nStartFrame = 0.0;
		beamInfo.m_flFrameRate = 30.0;
		beamInfo.m_flRed = 255.0;
		beamInfo.m_flGreen = 255.0;
		beamInfo.m_flBlue = 255.0;
		beamInfo.m_nSegments = 8;
		beamInfo.m_bRenderable = true;
		beamInfo.m_nFlags = FBEAM_FOREVER;
	
		m_pBeam = beams->CreateBeamEntPoint( beamInfo );
	}

	void SetVisible( bool state = true )
	{
		if ( m_pBeam == NULL )
			return;

		m_pBeam->brightness = ( state ) ? 255.0f : 0.0f;
	}

private:
	Beam_t	*m_pBeam;
};
	// Reasons behind a pickup
	enum PhysGunPickup_t
	{
		PICKED_UP_BY_CANNON,
		PUNTED_BY_CANNON,
		PICKED_UP_BY_PLAYER, // Picked up by +USE, not physgun.
	};

	// Reasons behind a drop
	enum PhysGunDrop_t
	{
		DROPPED_BY_PLAYER,
		THROWN_BY_PLAYER,
		DROPPED_BY_CANNON,
		LAUNCHED_BY_CANNON,
	};

	enum PhysGunForce_t
	{
		PHYSGUN_FORCE_DROPPED,	// Dropped by +USE
		PHYSGUN_FORCE_THROWN,	// Thrown from +USE
		PHYSGUN_FORCE_PUNTED,	// Punted by cannon
		PHYSGUN_FORCE_LAUNCHED,	// Launched by cannon
	};
#else
struct thrown_objects_t
{
	float				fTimeThrown;
	EHANDLE				hEntity;

	DECLARE_SIMPLE_DATADESC();
};
#endif

class CWeaponPhysCannon : public CTFWeaponBase
{
public:
	DECLARE_CLASS( CWeaponPhysCannon, CTFWeaponBase );

	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

#ifdef GAME_DLL
	DECLARE_DATADESC();
#endif

	CWeaponPhysCannon( void );

	virtual int		GetWeaponID( void ) const { return TF_WEAPON_PHYSCANNON; }
	virtual void	WeaponReset( void );
	virtual int		GetSlot( void ) const;
	virtual void	FallInit( void );
	void	Drop( const Vector &vecVelocity );
	void	Precache();

	virtual void	Spawn();

	virtual void	OnRestore();
	virtual void	StopLoopingSounds();
	virtual void	UpdateOnRemove(void);
	virtual void	PrimaryAttack();
	virtual void	SecondaryAttack();
	void	TertiaryAttack();
	virtual void	WeaponIdle();
	virtual void	ItemPreFrame();
	virtual void	ItemPostFrame();
	virtual void	ItemHolsterFrame();

	void	ForceDrop( void );
	bool	DropIfEntityHeld( CBaseEntity *pTarget );	// Drops its held entity if it matches the entity passed in
	CGrabController &GetGrabController() { return m_grabController; }

	bool	CanHolster( void ) const;
	bool	Holster( CBaseCombatWeapon *pSwitchingTo = NULL );
	bool	Deploy( void );

	bool	HasAnyAmmo( void ) { return true; }

	void	InputBecomeMegaCannon(inputdata_t &inputdata);

	void	BeginUpgrade();

	virtual void SetViewModel( void );
	virtual const char *GetShootSound( int iIndex ) const;

	virtual void 	DefaultTouch( CBaseEntity *pOther );
	virtual void	OnPickedUp( CBaseCombatCharacter *pNewOwner );
#ifndef CLIENT_DLL
	CNetworkQAngle	( m_attachedAnglesPlayerSpace );
#else
	QAngle m_attachedAnglesPlayerSpace;
#endif

	CNetworkVector	( m_attachedPositionObjectSpace );

	CNetworkHandle( CBaseEntity, m_hAttachedObject );

	EHANDLE m_hOldAttachedObject;

	CNetworkVar( float, m_flNextTertiaryAttack );					// soonest time ItemPostFrame will call TertiaryAttack

	virtual bool	HasChargeBar( void );
	virtual float	InternalGetEffectBarRechargeTime( void );
	virtual const char	*GetEffectLabelText( void ) { return "#TF_Charge"; }

	// Do we have the super-phys gun?
	bool	IsMegaPhysCannon( void );

#ifndef CLIENT_DLL
	void	RecordThrownObject(CBaseEntity* pObject);
	void	PurgeThrownObjects();
	bool	IsAccountableForObject(CBaseEntity* pObject);
#endif

	virtual int		GetActivityWeaponRole( void );
protected:
	enum FindObjectResult_t
	{
		OBJECT_FOUND = 0,
		OBJECT_NOT_FOUND,
		OBJECT_BEING_DETACHED,
	};

	void	DoMegaEffect(int effectType, Vector *pos = NULL);

	void	DoEffect(int effectType, Vector *pos = NULL);

	void	OpenElements( void );
	void	CloseElements( void );

	// Pickup and throw objects.
	bool	CanPickupObject( CBaseEntity *pTarget );
	void	CheckForTarget( void );
	
#ifndef CLIENT_DLL
	bool	AttachObject( CBaseEntity *pObject, const Vector &vPosition );
	FindObjectResult_t		FindObject( void );

	CBaseEntity *MegaPhysCannonFindObjectInCone(const Vector &vecOrigin, const Vector &vecDir, float flCone, float flCombineBallCone, bool bOnlyCombineBalls);

	CBaseEntity *FindObjectInCone( const Vector &vecOrigin, const Vector &vecDir, float flCone );
#endif	// !CLIENT_DLL

	void	UpdateObject( void );
	void	DetachObject( bool playSound = true, bool wasLaunched = false );
	void	LaunchObject( const Vector &vecDir, float flForce );
	void	StartEffects( void );	// Initialize all sprites and beams
	void	StopEffects( bool stopSound = true );	// Hide all effects temporarily
	void	DestroyEffects( void );	// Destroy all sprites and beams

	// Punt objects - this is pointing at an object in the world and applying a force to it.
	void	PuntNonVPhysics( CBaseEntity *pEntity, const Vector &forward, trace_t &tr );
	void	PuntVPhysics( CBaseEntity *pEntity, const Vector &forward, trace_t &tr );

#ifdef GAME_DLL
	void	PuntRagdoll( CBaseEntity *pEntity, const Vector &forward, trace_t &tr );
#endif

	// Velocity-based throw common to punt and launch code.
	void	ApplyVelocityBasedForce( CBaseEntity* pEntity, const Vector& forward, const Vector& vecHitPos, PhysGunForce_t reason );

	// Physgun effects
	void	DoEffectClosed( void );
	void	DoMegaEffectClosed( void );
	void	DoEffectReady( void );
	void	DoMegaEffectReady( void );
	void	DoEffectHolding( void );
	void	DoMegaEffectHolding( void );
	void	DoEffectLaunch( Vector *pos );
	void	DoMegaEffectLaunch( Vector *pos );

	void	DoEffectNone( void );
	void	DoEffectIdle( void );

	// Trace length
	float	TraceLength( void );

	// Sprite scale factor 
	float	SpriteScaleFactor( void );

	float			GetLoadPercentage( void );
	CSoundPatch		*GetMotorSound( void );

	void	DryFire( void );
	void	PrimaryFireEffect( void );

#ifndef CLIENT_DLL 
	bool	EntityAllowsPunts( CBaseEntity *pEntity );
#endif
	// Wait until we're done upgrading
	void	WaitForUpgradeThink( void );

#ifndef CLIENT_DLL
	// What happens when the physgun picks up something 
	void	Physgun_OnPhysGunPickup( CBaseEntity *pEntity, CBasePlayer *pOwner, PhysGunPickup_t reason );
#endif	// !CLIENT_DLL

#ifdef CLIENT_DLL

	enum EffectType_t
	{
		PHYSCANNON_CORE = 0,
		
		PHYSCANNON_BLAST,

		PHYSCANNON_GLOW1,	// Must be in order!
		PHYSCANNON_GLOW2,
		PHYSCANNON_GLOW3,
		PHYSCANNON_GLOW4,
		PHYSCANNON_GLOW5,
		PHYSCANNON_GLOW6,

		PHYSCANNON_ENDCAP1,	// Must be in order!
		PHYSCANNON_ENDCAP2,
		PHYSCANNON_ENDCAP3,	// Only used in third-person!

		NUM_PHYSCANNON_PARAMETERS	// Must be last!
	};

#define	NUM_GLOW_SPRITES ((CWeaponPhysCannon::PHYSCANNON_GLOW6-CWeaponPhysCannon::PHYSCANNON_GLOW1)+1)
#define NUM_ENDCAP_SPRITES ((CWeaponPhysCannon::PHYSCANNON_ENDCAP3-CWeaponPhysCannon::PHYSCANNON_ENDCAP1)+1)

#define	NUM_PHYSCANNON_BEAMS	3

	virtual int		DrawModel( int flags );
	virtual void	ViewModelDrawn( C_BaseViewModel *pBaseViewModel );
	virtual bool	IsTransparent( void );
	virtual void	OnDataChanged( DataUpdateType_t type );
	virtual void	ClientThink( void );
	
	void			ManagePredictedObject( void );
	void			DrawEffects( void );
	void			GetEffectParameters( EffectType_t effectID, color32 &color, float &scale, IMaterial **pMaterial, Vector &vecAttachment );
	void			DrawEffectSprite( EffectType_t effectID );
	inline bool		IsEffectVisible( EffectType_t effectID );
	void			UpdateElementPosition( void );

	// We need to render opaque and translucent pieces
	RenderGroup_t	GetRenderGroup( void ) {	return RENDER_GROUP_TWOPASS;	}

	CInterpolatedValue		m_ElementParameter;							// Used to interpolate the position of the articulated elements
	CPhysCannonEffect		m_Parameters[NUM_PHYSCANNON_PARAMETERS];	// Interpolated parameters for the effects
	CPhysCannonEffectBeam	m_Beams[NUM_PHYSCANNON_BEAMS];				// Beams

	int				m_nOldEffectState;	// Used for parity checks
	bool			m_bOldOpen;			// Used for parity checks

	void			NotifyShouldTransmit( ShouldTransmitState_t state );

#endif	// CLIENT_DLL

	int		m_nChangeState;				// For delayed state change of elements
	float	m_flCheckSuppressTime;		// Amount of time to suppress the checking for targets
	bool	m_flLastDenySoundPlayed;	// Debounce for deny sound
	int		m_nAttack2Debounce;

	CNetworkVar( bool,	m_bActive );
	CNetworkVar( int,	m_EffectState );		// Current state of the effects on the gun
	CNetworkVar( bool,	m_bOpen );

	CNetworkVar( bool, m_bIsCurrentlyUpgrading );
	CNetworkVar( float, m_flTimeForceView );
	bool				m_bPhyscannonState;

	bool	m_bResetOwnerEntity;
	
	float	m_flElementDebounce;

	CSoundPatch			*m_sndMotor;		// Whirring sound for the gun
	
	CGrabController		m_grabController;

	float	m_flRepuntObjectTime;
	EHANDLE m_hLastPuntedObject;

#ifndef CLIENT_DLL
	// A list of the objects thrown or punted recently, and the time done so.
	CUtlVector< thrown_objects_t >	m_ThrownEntities;

	float				m_flTimeNextObjectPurge;
#endif

private:
	CWeaponPhysCannon( const CWeaponPhysCannon & );
};

class CWeaponPhysCannon_Secondary : public CWeaponPhysCannon
{
public:
	DECLARE_CLASS( CWeaponPhysCannon_Secondary, CWeaponPhysCannon );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	void	Precache();
};

#endif // TF_WEAPON_PHYSCANNON_H