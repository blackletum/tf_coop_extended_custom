//=============================================================================//
//
// Purpose: arrow from tf2vintage
//
//=============================================================================//

#include "cbase.h"
#include "tf_projectile_arrow.h"
#include "effect_dispatch_data.h"
#include "tf_weapon_compound_bow.h"
#include "tf_gamerules.h"
#include "particle_parse.h"
#include "beam_shared.h"

#ifdef GAME_DLL
	#include "SpriteTrail.h"
	#include "props_shared.h"
	#include "debugoverlay_shared.h"
	#include "collisionutils.h"
	#include "te_effect_dispatch.h"
	#include "decals.h"
	#include "bone_setup.h"
	#include "tf_fx.h"
	#include "tf_gamestats.h"
	#include "tf_generic_bomb.h"
	#include "tf_obj.h"
	#include "soundent.h"
	#include "ai_basenpc.h"
#endif

#ifdef GAME_DLL
ConVar tf_debug_arrows( "tf_debug_arrows", "0", FCVAR_CHEAT );
#endif

const char *g_pszArrowModels[] =
{
	"models/weapons/w_models/w_arrow.mdl",
	"models/weapons/w_models/w_syringe_proj.mdl",
	"models/weapons/w_models/w_repair_claw.mdl",
	"models/weapons/w_models/w_arrow_xmas.mdl",
	"models/workshop/weapons/c_models/c_crusaders_crossbow/c_crusaders_crossbow_xmas_proj.mdl",
	"models/weapons/c_models/c_grapple_proj/c_grapple_proj.mdl",
};

#define ARROW_FADE_TIME		3.f
#define MASK_TFARROWS		CONTENTS_SOLID|CONTENTS_HITBOX|CONTENTS_MONSTER


class CTraceFilterCollisionArrows : public CTraceFilterEntitiesOnly
{
public:
	CTraceFilterCollisionArrows( CBaseEntity *pPass1, CBaseEntity *pPass2 )
		: m_pArrow( pPass1 ), m_pOwner( pPass2 ) {}

	virtual bool ShouldHitEntity( IHandleEntity *pEntity, int contentsMask )
	{
		if ( !PassServerEntityFilter( pEntity, m_pArrow ) )
			return false;

		const CBaseEntity *pEntTouch = EntityFromEntityHandle( pEntity );
		if ( pEntTouch == nullptr )
			return true;

		if ( pEntTouch == m_pOwner )
			return false;

		int iCollisionGroup = pEntTouch->GetCollisionGroup();
		if ( iCollisionGroup == COLLISION_GROUP_DEBRIS )
			return false;

		if ( iCollisionGroup == TF_COLLISIONGROUP_GRENADES )
			return false;

		if ( iCollisionGroup == TFCOLLISION_GROUP_ROCKETS )
			return false;

		if ( iCollisionGroup == TFCOLLISION_GROUP_RESPAWNROOMS )
			return false;

		if ( iCollisionGroup == TFCOLLISION_GROUP_ARROWS )
			return false;

		return iCollisionGroup != COLLISION_GROUP_NONE;
	}

private:
	IHandleEntity *m_pArrow;
	IHandleEntity *m_pOwner;
};

IMPLEMENT_NETWORKCLASS_ALIASED( TFProjectile_Arrow, DT_TFProjectile_Arrow )
BEGIN_NETWORK_TABLE( CTFProjectile_Arrow, DT_TFProjectile_Arrow )
#ifdef CLIENT_DLL
	RecvPropBool( RECVINFO( m_bCritical ) ),
	RecvPropBool( RECVINFO( m_bArrowAlight ) ),
	RecvPropInt( RECVINFO( m_iProjectileType ) ),
#else
	SendPropBool( SENDINFO( m_bCritical ) ),
	SendPropBool( SENDINFO( m_bArrowAlight ) ),
	SendPropInt( SENDINFO( m_iProjectileType ) ),
#endif
END_NETWORK_TABLE()

#ifdef GAME_DLL
BEGIN_DATADESC( CTFProjectile_Arrow )
	DEFINE_ENTITYFUNC( ArrowTouch )
END_DATADESC()
#endif

LINK_ENTITY_TO_CLASS( tf_projectile_arrow, CTFProjectile_Arrow );
PRECACHE_REGISTER( tf_projectile_arrow );

CTFProjectile_Arrow::CTFProjectile_Arrow()
{
}

CTFProjectile_Arrow::~CTFProjectile_Arrow()
{
#ifdef CLIENT_DLL
	ParticleProp()->StopEmission();
#endif
}

#ifdef GAME_DLL

CTFProjectile_Arrow *CTFProjectile_Arrow::Create( const Vector &vecOrigin, const QAngle &vecAngles, float flSpeed, float flGravity, ProjectileType_t eType, CBaseEntity *pOwner, CBaseEntity *pWeapon )
{
	const char *pszEntClass = "tf_projectile_arrow";
	switch ( eType )
	{
		case TF_PROJECTILETYPE_HEALING_BOLT:
		case TF_PROJECTILETYPE_HEALING_BOLT_FESTIVE:
			pszEntClass = "tf_projectile_healing_bolt";
			break;
		case TF_PROJECTILETYPE_HOOK:
			pszEntClass = "tf_projectile_grapplinghook";
			break;
		default:
			pszEntClass = "tf_projectile_arrow";
			break;
	}

	CTFProjectile_Arrow *pArrow = static_cast<CTFProjectile_Arrow *>( CBaseEntity::CreateNoSpawn( pszEntClass, vecOrigin, vecAngles, pOwner ) );
	if ( pArrow )
	{
		// Set firing weapon.
		pArrow->SetLauncher( pWeapon );
		pArrow->SetScorer( pOwner );

		pArrow->InitArrow( vecAngles, flSpeed, flGravity, eType, pOwner, pWeapon );

		// Spawn.
		DispatchSpawn( pArrow );
		pArrow->m_flTrailReflectLifetime = 0;

		// Setup the initial velocity.
		Vector vecForward, vecRight, vecUp;
		AngleVectors( vecAngles, &vecForward, &vecRight, &vecUp );
	
		CALL_ATTRIB_HOOK_FLOAT_ON_OTHER( pWeapon, flSpeed, mult_projectile_speed );

		Vector vecVelocity = vecForward * flSpeed;
		pArrow->SetAbsVelocity( vecVelocity );
		pArrow->SetupInitialTransmittedGrenadeVelocity( vecVelocity );

		// Setup the initial angles.
		QAngle angles;
		VectorAngles( vecVelocity, angles );
		pArrow->SetAbsAngles( angles );

		pArrow->SetGravity( flGravity );

		return pArrow;
	}

	return pArrow;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFProjectile_Arrow::InitArrow( const QAngle &vecAngles, float flSpeed, float flGravity, ProjectileType_t eType, CBaseEntity *pOwner, CBaseEntity *pWeapon )
{
	ChangeTeam( pOwner->GetTeamNumber() );

	m_iProjectileType = eType;
	SetModel( g_pszArrowModels[eType] );
	string_t strModelOverride = NULL_STRING;
	if (strcmp(STRING(strModelOverride), "") != 0)
		SetModel(STRING(strModelOverride));

	m_nSkin = GetArrowSkin();

	CTFCompoundBow *pBow = dynamic_cast<CTFCompoundBow *>( pWeapon );
	if ( pBow && pBow->IsFlameArrow() )
		SetFlameArrow( true );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFProjectile_Arrow::Precache( void )
{
	// Precache all arrow models we have.
	for ( int i = 0; i < ARRAYSIZE( g_pszArrowModels ); i++ )
	{
		int iIndex = PrecacheModel( g_pszArrowModels[i] );
		PrecacheGibsForModel( iIndex );
	}

	for ( int i = FIRST_GAME_TEAM; i < TF_TEAM_GREEN; i++ )
	{
		PrecacheModel( ConstructTeamParticle( "effects/arrowtrail_%s.vmt", i, false, g_aTeamNamesShort ) );
		PrecacheModel( ConstructTeamParticle( "effects/repair_claw_trail_%s.vmt", i, false, g_aTeamParticleNames ) );
		PrecacheModel( ConstructTeamParticle( "cable/cable_%s.vmt", i, false, g_aTeamParticleNames ) );
	}

	PrecacheTeamParticles( "healshot_trail_%s" );

	// Precache flame effects
	PrecacheParticleSystem( "flying_flaming_arrow" );

	PrecacheScriptSound( "Weapon_Arrow.ImpactFlesh" );
	PrecacheScriptSound( "Weapon_Arrow.ImpactFleshCrossbowHeal" );
	PrecacheScriptSound( "Weapon_Arrow.ImpactMetal" );
	PrecacheScriptSound( "Weapon_Arrow.ImpactWood" );
	PrecacheScriptSound( "Weapon_Arrow.ImpactConcrete" );
	PrecacheScriptSound( "WeaponGrapplingHook.ImpactDefault" );
	PrecacheScriptSound( "WeaponGrapplingHook.ImpactFlesh" );
	PrecacheScriptSound( "Weapon_Arrow.Nearmiss" );

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFProjectile_Arrow::Spawn( void )
{
	BaseClass::Spawn();

#ifdef TF_ARROW_FIX
	SetSolidFlags( FSOLID_NOT_SOLID | FSOLID_TRIGGER );
#endif

	if ( GetProjectileType() == TF_PROJECTILETYPE_HOOK )
	{
		SetMoveType( MOVETYPE_FLY, MOVECOLLIDE_FLY_CUSTOM );
	}
	else
	{
		SetMoveType( MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_CUSTOM );
	}

	UTIL_SetSize( this, -Vector( 1, 1, 1 ), Vector( 1, 1, 1 ) );

	SetSolid( SOLID_BBOX );

	if ( m_iProjectileType == TF_PROJECTILETYPE_HEALING_BOLT || m_iProjectileType == TF_PROJECTILETYPE_HEALING_BOLT_FESTIVE )
		SetCollisionGroup( COLLISION_GROUP_PROJECTILE );
	else
		SetCollisionGroup( TFCOLLISION_GROUP_ROCKETS );

	AddEffects( EF_NOSHADOW );
	AddFlag( FL_GRENADE );

	m_flCreateTime = gpGlobals->curtime;

	CreateTrail();

	SetTouch( &CTFProjectile_Arrow::ArrowTouch );
	SetThink( &CTFProjectile_Arrow::FlyThink );
	SetNextThink( gpGlobals->curtime );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFProjectile_Arrow::SetScorer( CBaseEntity *pScorer )
{
	m_Scorer = pScorer;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CBasePlayer *CTFProjectile_Arrow::GetScorer( void )
{
	return dynamic_cast<CBasePlayer *>( m_Scorer.Get() );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFProjectile_Arrow::ArrowTouch( CBaseEntity *pOther )
{
	float flTimeAlive = gpGlobals->curtime - m_flCreateTime;
	if ( flTimeAlive >= 10.0 )
	{
		Warning( "Arrow alive for %f3.2 seconds\n", flTimeAlive );
		UTIL_Remove( this );
	}

	// Verify a correct "other."
	Assert( pOther );
	if ( m_bImpacted )
		return;

	bool bImpactedItem = false;
	if ( pOther->IsCombatItem() )
		bImpactedItem = !InSameTeam( pOther );

	CTFPumpkinBomb *pPumpkin = dynamic_cast<CTFPumpkinBomb *>( pOther );

	if ( pOther->IsSolidFlagSet( FSOLID_TRIGGER | FSOLID_VOLUME_CONTENTS ) && !pPumpkin && !bImpactedItem )
		return;

	CBaseEntity *pAttacker = GetOwnerEntity();
	IScorer *pScorerInterface = dynamic_cast<IScorer*>( pAttacker );
	if ( pScorerInterface )
	{
		pAttacker = pScorerInterface->GetScorer();
	}

	CBaseCombatCharacter *pActor = dynamic_cast<CBaseCombatCharacter *>( pOther );
	if ( pActor == nullptr )
	{
		pActor = dynamic_cast<CBaseCombatCharacter *>( pOther->GetOwnerEntity() );
	}

	//CTFRobotDestruction_Robot *pRobot = dynamic_cast<CTFRobotDestruction_Robot *>( pOther );
	//CTFMerasmusTrickOrTreatProp *pMerasProp = dynamic_cast<CTFMerasmusTrickOrTreatProp *>( pOther );

	if ( !FNullEnt( pOther->edict() ) )
	{
		if ( GetProjectileType() == TF_PROJECTILETYPE_HOOK )
		{
			if ( ToTFPlayer( pAttacker ) )
			{
				SetMoveType( MOVETYPE_NONE, MOVECOLLIDE_DEFAULT );
				SetAbsVelocity( vec3_origin );
				SetSolidFlags( FSOLID_NOT_SOLID );
				SetParent( pOther );
				ToTFPlayer( pAttacker )->SetGrapplingHookTarget( this );
				CTakeDamageInfo info( this, pAttacker, m_hLauncher, GetAbsOrigin(), GetAbsVelocity(), GetDamage(), GetDamageType() );
				pOther->TakeDamage( info );
				ImpactSound( "WeaponGrapplingHook.ImpactDefault", true );
			}
		}
		else
		{
			if ( pActor != nullptr || pPumpkin != nullptr/* || pMerasProp != nullptr || pRobot != nullptr*/ || bImpactedItem )
			{
				CBaseAnimating *pAnimating = dynamic_cast<CBaseAnimating *>( pOther );
				if ( !pAnimating )
				{
					UTIL_Remove( this );
					return;
				}

				CStudioHdr *pStudioHdr = pAnimating->GetModelPtr();
				if ( !pStudioHdr )
				{
					UTIL_Remove( this );
					return;
				}

				mstudiohitboxset_t *pSet = pStudioHdr->pHitboxSet( pAnimating->GetHitboxSet() );
				if ( !pSet )
				{
					UTIL_Remove( this );
					return;
				}

				// Determine where we should land
				Vector vecOrigin = GetAbsOrigin();
				Vector vecDir = GetAbsVelocity();

				trace_t tr;

				CTraceFilterCollisionArrows filter( this, GetOwnerEntity() );
				UTIL_TraceLine( vecOrigin, vecOrigin + vecDir * gpGlobals->frametime, MASK_TFARROWS, &filter, &tr );

				if ( tr.m_pEnt && tr.m_pEnt->GetTeamNumber() != GetTeamNumber() )
				{
					mstudiobbox_t *pBox = pSet->pHitbox( tr.hitbox );
					if ( pBox )
					{
						if ( !StrikeTarget( pBox, pOther ) )
							BreakArrow();

						if ( !m_bImpacted )
							SetAbsOrigin( vecOrigin );

						if ( bImpactedItem )
							BreakArrow();

						m_bImpacted = true;
						return;
					}
				}

				Vector vecFwd;
				AngleVectors( GetAbsAngles(), &vecFwd );
				Vector vecArrowEnd = GetAbsOrigin() + vecFwd * 16;

				// Find the closest hitbox we crossed
				float flClosest = 99999.f;
				mstudiobbox_t *pBox = NULL, *pCurrent = NULL;
				for ( int i = 0; i < pSet->numhitboxes; i++ )
				{
					pCurrent = pSet->pHitbox( i );

					Vector boxPosition;
					QAngle boxAngles;
					pAnimating->GetBonePosition( pCurrent->bone, boxPosition, boxAngles );

					Ray_t ray;
					ray.Init( boxPosition, vecArrowEnd );

					trace_t trace;
					IntersectRayWithBox( ray, boxPosition + pCurrent->bbmin, boxPosition + pCurrent->bbmax, 0, &trace );

					float flDistance = ( trace.endpos - vecArrowEnd ).Length();
					if ( flDistance < flClosest )
					{
						pBox = pCurrent;
						flClosest = flDistance;
					}
				}

				if ( pBox )
				{
					if ( !StrikeTarget( pBox, pOther ) )
						BreakArrow();

					if ( !m_bImpacted )
						SetAbsOrigin( vecOrigin );

					if ( bImpactedItem )
						BreakArrow();

					m_bImpacted = true;
				}
			}
			else
			{
				CTakeDamageInfo info( this, pAttacker, m_hLauncher, GetAbsOrigin(), GetAbsVelocity(), GetDamage(), GetDamageType() );
				pOther->TakeDamage( info );
				BreakArrow();
			}
		}
	}
	else
	{
		CheckSkyboxImpact( pOther );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFProjectile_Arrow::StrikeTarget( mstudiobbox_t *pBox, CBaseEntity *pTarget )
{
	if ( pTarget == nullptr )
		return false;

	if ( pTarget->IsBaseObject() && InSameTeam( pTarget ) )
		BuildingHealingArrow( pTarget );

	CTFPlayer *pPlayer = ToTFPlayer( pTarget );
	if ( pPlayer && pPlayer->m_Shared.IsInvulnerable() )
		return false;

	CBaseEntity *pAttacker = GetScorer();
	if ( pAttacker == nullptr )
	{
		pAttacker = GetOwnerEntity();
	}

	CBaseAnimating *pAnimating = dynamic_cast<CBaseAnimating *>( pTarget );
	if ( pAnimating == nullptr )
		return false;

	bool bBreakArrow = false;
	//if ( dynamic_cast<CHalloweenBaseBoss *>( pTarget )/* || dynamic_cast<CTFTankBoss *>( pTarget )*/ )
	//	bBreakArrow = true;

	if ( !bBreakArrow )
	{
		if ( !PositionArrowOnBone( pBox, pAnimating ) )
			return false;
	}

	bool bHeadshot = false;
	if ( pBox->group == HITGROUP_HEAD && CanHeadshot() )
		bHeadshot = true;

	Vector vecOrigin = GetAbsOrigin();
	Vector vecDir = GetAbsVelocity();
	VectorNormalizeFast( vecDir );

	int iDmgCustom = TF_DMG_CUSTOM_NONE;
	int iDmgType = GetDamageType();
	bool bImpact = true; // TODO: Some strange check involving a UtlVector on the arrow, possibly for pierce

	if( pAttacker )
	{
		if ( InSameTeam( pTarget ) )
		{
			CBaseCombatCharacter *pCombat = dynamic_cast<CBaseCombatCharacter *>( pTarget );
			if ( pCombat && bImpact )
				ImpactTeamPlayerOrNPC( pCombat );
		}
		else
		{
			IScorer *pScorer = dynamic_cast<IScorer *>( pAttacker );
			if ( pScorer )
				pAttacker = pScorer->GetScorer();

			if ( m_bArrowAlight )
			{
				iDmgType |= DMG_IGNITE;
				iDmgCustom = TF_DMG_CUSTOM_BURNING_ARROW;
			}

			if ( bHeadshot )
			{
				iDmgType |= DMG_CRITICAL;
				iDmgCustom = TF_DMG_CUSTOM_HEADSHOT;
			}

			if ( m_bCritical )
				iDmgType |= DMG_CRITICAL;

			if ( bImpact )
			{
				CTakeDamageInfo info( this, pAttacker, m_hLauncher, GetAbsOrigin(), GetAbsVelocity(), GetDamage(), iDmgType, iDmgCustom );
				pTarget->TakeDamage( info );

				ImpactSound( "Weapon_Arrow.ImpactFlesh", true );
			}
		}

		if( !m_bImpacted && !bBreakArrow )
		{
			Vector vecBoneOrigin;
			QAngle vecBoneAngles;
			int iBone, iPhysicsBone;
			GetBoneAttachmentInfo( pBox, pAnimating, vecBoneOrigin, vecBoneAngles, iBone, iPhysicsBone );

			if ( pPlayer && !pPlayer->IsAlive() )
			{
				if ( CheckRagdollPinned( vecOrigin, vecDir, iBone, iPhysicsBone, pPlayer->m_hRagdoll, pBox->group, pPlayer->entindex() ) )
				{
					pPlayer->StopRagdollDeathAnim();
				}
				else
				{
					IGameEvent *event = gameeventmanager->CreateEvent( "arrow_impact" );

					if ( event )
					{
						event->SetInt( "attachedEntity", pTarget->entindex() );
						event->SetInt( "shooter", pAttacker->entindex() );
						event->SetInt( "projectileType", GetProjectileType() );
						event->SetInt( "boneIndexAttached", iBone );
						event->SetFloat( "bonePositionX", vecBoneOrigin.x );
						event->SetFloat( "bonePositionY", vecBoneOrigin.y );
						event->SetFloat( "bonePositionZ", vecBoneOrigin.z );
						event->SetFloat( "boneAnglesX", vecBoneAngles.x );
						event->SetFloat( "boneAnglesY", vecBoneAngles.y );
						event->SetFloat( "boneAnglesZ", vecBoneAngles.z );

						gameeventmanager->FireEvent( event );
					}
				}
			}
			else
			{
				IGameEvent *event = gameeventmanager->CreateEvent( "arrow_impact" );

				if ( event )
				{
					event->SetInt( "attachedEntity", pTarget->entindex() );
					event->SetInt( "shooter", pAttacker->entindex() );
					event->SetInt( "projectileType", GetProjectileType() );
					event->SetInt( "boneIndexAttached", iBone );
					event->SetFloat( "bonePositionX", vecBoneOrigin.x );
					event->SetFloat( "bonePositionY", vecBoneOrigin.y );
					event->SetFloat( "bonePositionZ", vecBoneOrigin.z );
					event->SetFloat( "boneAnglesX", vecBoneAngles.x );
					event->SetFloat( "boneAnglesY", vecBoneAngles.y );
					event->SetFloat( "boneAnglesZ", vecBoneAngles.z );

					gameeventmanager->FireEvent( event );
				}
			}

			FadeOut( ARROW_FADE_TIME );
		}
	}

	trace_t tr;
	CTraceFilterCollisionArrows filter( this, GetOwnerEntity() );
	UTIL_TraceLine( vecOrigin, vecOrigin - vecDir * gpGlobals->frametime, MASK_TFARROWS, &filter, &tr );

	UTIL_ImpactTrace( &tr, DMG_GENERIC );

	return !bBreakArrow;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFProjectile_Arrow::CheckSkyboxImpact( CBaseEntity *pOther )
{
	Vector vecFwd = GetAbsVelocity();
	vecFwd.NormalizeInPlace();

	Vector vecOrigin = GetAbsOrigin();

	trace_t tr;
	UTIL_TraceLine( vecOrigin, vecOrigin + vecFwd * 32, MASK_SOLID, this, COLLISION_GROUP_DEBRIS, &tr );

	if ( tr.fraction < 1.0f && ( tr.surface.flags & SURF_SKY ) )
	{
		FadeOut( ARROW_FADE_TIME );
		return true;
	}

	if ( !FNullEnt( pOther->edict() ) )
	{
		BreakArrow();
		return false;
	}

	CEffectData	data;
	data.m_vOrigin = tr.endpos;
	data.m_vNormal = vecFwd;
	data.m_nEntIndex = pOther->entindex();
	data.m_fFlags = GetProjectileType();
	data.m_nColor = (GetTeamNumber() == TF_TEAM_BLUE); //Skin

	DispatchEffect( "TFBoltImpact", data );

	const char* pszImpactSound = "Weapon_Arrow.ImpactMetal";
	surfacedata_t *psurfaceData = physprops->GetSurfaceData( tr.surface.surfaceProps );
	if( psurfaceData )
	{
		switch ( psurfaceData->game.material )
		{
			case CHAR_TEX_CONCRETE:
				pszImpactSound = "Weapon_Arrow.ImpactConcrete";
				break;
			case CHAR_TEX_WOOD:
				pszImpactSound = "Weapon_Arrow.ImpactWood";
				break;
			default:
				pszImpactSound = "Weapon_Arrow.ImpactMetal";
				break;
		}
	}

	if ( GetProjectileType() == TF_PROJECTILETYPE_HOOK )
		pszImpactSound = "WeaponGrapplingHook.ImpactDefault";
	string_t strImpactReplaceSound = NULL_STRING;
	CALL_ATTRIB_HOOK_STRING_ON_OTHER(m_hLauncher, strImpactReplaceSound, arrow_override_impact_sound);

	// Play sound
	if (strcmp(STRING(strImpactReplaceSound), "") != 0)
	{
		pszImpactSound = STRING(strImpactReplaceSound);
	}
	if ( pszImpactSound )
	{
		ImpactSound( pszImpactSound );
	}

	if ( GetProjectileType() == TF_PROJECTILETYPE_HOOK )
	{
		CTFPlayer *pOwner = ToTFPlayer( GetOwnerEntity() );
		if ( pOwner )
		{
			SetMoveType( MOVETYPE_NONE, MOVECOLLIDE_DEFAULT );
			SetAbsVelocity( vec3_origin );
			SetSolidFlags( FSOLID_NOT_SOLID );
			pOwner->SetGrapplingHookTarget( this );
			return true;
		}
	}
	else
	{
		FadeOut( ARROW_FADE_TIME );
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFProjectile_Arrow::BuildingHealingArrow( CBaseEntity *pTarget )
{
	if ( !pTarget->IsBaseObject() )
		return;

	CBasePlayer *pOwner = GetScorer();
	if ( pOwner == nullptr )
		return;

	if ( GetTeamNumber() != pTarget->GetTeamNumber() )
		return;

	int nArrowHealsBuilding = 0;
	CALL_ATTRIB_HOOK_INT_ON_OTHER( pOwner, nArrowHealsBuilding, arrow_heals_buildings );
	if ( nArrowHealsBuilding == 0 )
		return;

	CBaseObject *pObject = dynamic_cast<CBaseObject *>( pTarget );
	if ( pObject == nullptr )
		return;

	if ( pObject->HasSapper() || pObject->IsBeingCarried() || pObject->IsRedeploying() )
		return;

	int nHealth = pObject->GetHealth();
	int nHealthToAdd = Min( nArrowHealsBuilding + nHealth, pObject->GetMaxHealth() );

	if ( ( nHealthToAdd - nHealth ) > 0 )
	{
		pObject->SetHealth( nHealthToAdd );

		IGameEvent *event = gameeventmanager->CreateEvent( "building_healed" );
		if ( event )
		{
			event->SetInt( "priority", 1 ); // HLTV priority
			event->SetInt( "building", pObject->entindex() );
			event->SetInt( "healer", pOwner->entindex() );
			event->SetInt( "amount", nHealthToAdd - nHealth );

			gameeventmanager->FireEvent( event );
		}

		CPVSFilter filter( GetAbsOrigin() );
		switch ( GetTeamNumber() )
		{
			case TF_TEAM_BLUE:
				TE_TFParticleEffect( filter, 0, "repair_claw_heal_blue", GetAbsOrigin(), vec3_angle );
				break;
			default:
				TE_TFParticleEffect( filter, 0, "repair_claw_heal_red", GetAbsOrigin(), vec3_angle );
				break;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFProjectile_Arrow::FlyThink(void)
{
	QAngle angles;

	VectorAngles( GetAbsVelocity(), angles );

	SetAbsAngles( angles );

	if ( GetProjectileType() == TF_PROJECTILETYPE_HOOK )
	{
		CBeam *pRope = dynamic_cast< CBeam* > ( m_hSpriteTrail.Get() );
		if ( pRope )
		{
			CTFPlayer *pPlayer = ToTFPlayer( GetOwnerEntity() );
			if ( pPlayer )
			{
				Vector vecOrigin = pPlayer->Weapon_ShootPosition();
				pPlayer->GetAttachment( pPlayer->LookupAttachment( "effect_hand_r" ), vecOrigin );
				pRope->SetStartPos( vecOrigin );
			}
		}
	}

	SetNextThink( gpGlobals->curtime + 0.1f );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int	CTFProjectile_Arrow::GetDamageType()
{
	int iDmgType = BaseClass::GetDamageType();

	// Buff banner mini-crit calculations
	CTFWeaponBase *pWeapon = ( CTFWeaponBase * )m_hLauncher.Get();
	if ( pWeapon )
	{
		int iCustomAddDmgType = -1;
		CALL_ATTRIB_HOOK_INT_ON_OTHER(pWeapon, iCustomAddDmgType, cw_add_dmgtype);
		if (iCustomAddDmgType >= 1)
		{
			DevMsg("CTFProjectile_Arrow: Damage type replaced with %i", iCustomAddDmgType);
			iDmgType = iCustomAddDmgType;
		}
		pWeapon->CalcIsAttackMiniCritical();
		if ( pWeapon->IsCurrentAttackAMiniCrit() )
		{
			iDmgType |= DMG_MINICRITICAL;
		}
	}

	if ( m_iProjectileType == TF_PROJECTILETYPE_HEALING_BOLT || m_iProjectileType == TF_PROJECTILETYPE_HEALING_BOLT_FESTIVE || m_iProjectileType == TF_PROJECTILETYPE_BUILDING_BOLT )
	{
		iDmgType |= DMG_USEDISTANCEMOD;
	}
	if ( m_bCritical )
	{
		iDmgType |= DMG_CRITICAL;
	}
	if ( CanHeadshot() )
	{
		iDmgType |= DMG_USE_HITLOCATIONS;
	}
	if ( m_bArrowAlight == true )
	{
		iDmgType |= DMG_IGNITE;	
	}
	if ( m_iDeflected > 0 )
	{
		iDmgType |= DMG_MINICRITICAL;
	}

	return iDmgType;
}

bool CTFProjectile_Arrow::IsDeflectable(void)
{
	// Don't deflect projectiles with non-deflect attributes.
	if ( m_hLauncher.Get() )
	{
		// Check to see if this is a non-deflectable projectile, like an energy projectile.
		int nCannotDeflect = 0;
		CALL_ATTRIB_HOOK_INT_ON_OTHER( m_hLauncher.Get(), nCannotDeflect, energy_weapon_no_deflect );
		if ( nCannotDeflect != 0 )
			return false;
	}
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFProjectile_Arrow::Deflected( CBaseEntity *pDeflectedBy, Vector &vecDir )
{
	CTFPlayer *pDeflector = ToTFPlayer( pDeflectedBy );
	if ( pDeflector == nullptr || m_iProjectileType == TF_PROJECTILETYPE_HOOK ) // Don't allow grappling hooks to be deflected.
		return;

	CTFPlayer *pOwner = ToTFPlayer( GetOwnerEntity() );
	if ( pOwner )
		pOwner->SpeakConceptIfAllowed( MP_CONCEPT_DEFLECTED, "projectile:1,victim:1" );

	// Get arrow's speed.
	float flVel = GetAbsVelocity().Length();

	QAngle angForward;
	VectorAngles( vecDir, angForward );

	// Now change arrow's direction.
	SetAbsAngles( angForward );
	SetAbsVelocity( vecDir * flVel );

	// And change owner.
	IncremenentDeflected();
	SetOwnerEntity( pDeflectedBy );
	SetScorer( pDeflectedBy );
	ChangeTeam( pDeflectedBy->GetTeamNumber() );

	if ( m_iProjectileType != TF_PROJECTILETYPE_ARROW && m_iProjectileType != TF_PROJECTILETYPE_ARROW_FESTIVE )
	{
		m_nSkin = ( pDeflectedBy->GetTeamNumber() - 2 );
	}

	if ( pDeflector->m_Shared.IsCritBoosted() )
		m_bCritical = true;

	// Change trail color.
	if ( m_hSpriteTrail.Get() )
	{
		m_hSpriteTrail->Remove();
	}

	CreateTrail();
}

void CTFProjectile_Arrow::IncremenentDeflected( void )
{
	m_iDeflected++;
	m_flTrailReflectLifetime = 1.0f;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFProjectile_Arrow::CanHeadshot( void )
{
	return ( m_iProjectileType == TF_PROJECTILETYPE_ARROW || m_iProjectileType == TF_PROJECTILETYPE_ARROW_FESTIVE );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const char *CTFProjectile_Arrow::GetTrailParticleName( void )
{
	const char *pszFormat = NULL;
	bool bLongTeamName = false;

	switch( GetProjectileType() )
	{
	case TF_PROJECTILETYPE_BUILDING_BOLT:
		pszFormat = "effects/repair_claw_trail_%s.vmt";
		bLongTeamName = true;
		break;
	case TF_PROJECTILETYPE_HOOK:
		pszFormat = "cable/cable_%s.vmt";
		bLongTeamName = true;
		break;
	default:
		pszFormat = "effects/arrowtrail_%s.vmt";
		break;
	}

	return ConstructTeamParticle( pszFormat, GetTeamNumber(), false, bLongTeamName ? g_aTeamParticleNames : g_aTeamNamesShort );

}

// ---------------------------------------------------------------------------- -
// Purpose: 
//-----------------------------------------------------------------------------
void CTFProjectile_Arrow::CreateTrail( void )
{
	if ( IsDormant() || m_hSpriteTrail != nullptr )
		return;

	if ( m_iProjectileType == TF_PROJECTILETYPE_HEALING_BOLT || m_iProjectileType == TF_PROJECTILETYPE_HEALING_BOLT_FESTIVE )
		return;

	if ( GetProjectileType() == TF_PROJECTILETYPE_HOOK )
	{
		Vector vecShootOrigin = GetAbsOrigin();

		CTFPlayer *pPlayer = ToTFPlayer( GetOwnerEntity() );
		if ( pPlayer )
			vecShootOrigin = pPlayer->Weapon_ShootPosition();

		CBeam *pBeam = CBeam::BeamCreate( GetTrailParticleName(), 3.0f );
		pBeam->PointEntInit( vecShootOrigin, this );

		CTFWeaponBase *pWeapon = dynamic_cast<CTFWeaponBase *>( m_hLauncher.Get() );
		if ( pWeapon )
			pBeam->SetStartAttachment( pWeapon->LookupAttachment( "muzzle" ) );

		pBeam->SetBrightness( 255 );
		pBeam->RelinkBeam();

		m_hSpriteTrail.Set( pBeam );
		return;
	}

	CSpriteTrail *pTrail = CSpriteTrail::SpriteTrailCreate( GetTrailParticleName(), GetAbsOrigin(), true );
	if ( pTrail )
	{
		pTrail->FollowEntity( this );
		pTrail->SetTransparency( kRenderTransAlpha, -1, -1, -1, 255, kRenderFxNone );
		pTrail->SetStartWidth( m_iProjectileType == TF_PROJECTILETYPE_BUILDING_BOLT ? 5.0f : 3.0f );
		pTrail->SetTextureResolution( 0.01f );
		pTrail->SetLifeTime( 0.3f );
		pTrail->SetAttachment( this, PATTACH_ABSORIGIN );
		pTrail->SetContextThink( &CTFProjectile_Arrow::RemoveTrail, gpGlobals->curtime + 3.0f, "FadeTrail" );

		m_hSpriteTrail = pTrail;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFProjectile_Arrow::RemoveTrail( void )
{
	if ( !m_hSpriteTrail )
		return;

	if( m_flTrailReflectLifetime <= 0 )
	{
		UTIL_Remove( m_hSpriteTrail.Get() );
		m_flTrailReflectLifetime = 1.0f;
	}
	else
	{
		CSpriteTrail *pSprite = dynamic_cast<CSpriteTrail *>( m_hSpriteTrail.Get() );
		if ( pSprite )
			pSprite->SetBrightness( m_flTrailReflectLifetime * 128.f );

		m_flTrailReflectLifetime -= 0.1;

		SetContextThink( &CTFProjectile_Arrow::RemoveTrail, gpGlobals->curtime, "FadeTrail" );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFProjectile_Arrow::UpdateOnRemove( void )
{
	RemoveTrail();

	CTFPlayer *pOwner = ToTFPlayer( GetOwnerEntity() );
	if ( pOwner )
		pOwner->SetGrapplingHookTarget( NULL );

	BaseClass::UpdateOnRemove();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFProjectile_Arrow::AdjustDamageDirection( CTakeDamageInfo const &info, Vector &vecDirection, CBaseEntity *pEntity )
{
	if ( pEntity )
		vecDirection = ( info.GetDamagePosition() - info.GetDamageForce() ) - pEntity->WorldSpaceCenter();
}

//-----------------------------------------------------------------------------
// Purpose: Setup to remove ourselves
//-----------------------------------------------------------------------------
void CTFProjectile_Arrow::FadeOut( int iTime )
{
	SetMoveType( MOVETYPE_NONE, MOVECOLLIDE_DEFAULT );
	SetAbsVelocity( vec3_origin );
	SetSolidFlags( FSOLID_NOT_SOLID );
	AddEffects( EF_NODRAW );

	SetContextThink( &CTFProjectile_Arrow::RemoveThink, gpGlobals->curtime + iTime, NULL );
}

//-----------------------------------------------------------------------------
// Purpose: Sends to the client information for arrow gibs
//-----------------------------------------------------------------------------
void CTFProjectile_Arrow::BreakArrow( void )
{
	FadeOut( ARROW_FADE_TIME );

	CPVSFilter filter( GetAbsOrigin() );
	UserMessageBegin( filter, "BreakModel" );
		WRITE_SHORT( GetModelIndex() );
		WRITE_VEC3COORD( GetAbsOrigin() );
		WRITE_ANGLES( GetAbsAngles() );
		WRITE_SHORT( m_nSkin );
	MessageEnd();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFProjectile_Arrow::PositionArrowOnBone( mstudiobbox_t *pbox, CBaseAnimating *pAnim )
{
	CStudioHdr *pStudioHdr = pAnim->GetModelPtr();	
	if ( !pStudioHdr )
		return false;

	mstudiohitboxset_t *set = pStudioHdr->pHitboxSet( pAnim->GetHitboxSet() );

	if ( !set->numhitboxes || pbox->bone > 127 )
		return false;

	CBoneCache *pCache = pAnim->GetBoneCache();
	if ( !pCache )
		return false;

	matrix3x4_t *pBone = pCache->GetCachedBone( pbox->bone );
	if ( pBone == nullptr )
		return false;
	
	Vector vecMins, vecMaxs, vecResult;
	TransformAABB( *pBone, pbox->bbmin, pbox->bbmax, vecMins, vecMaxs );
	vecResult = vecMaxs - vecMins;

	// This is a mess
	SetAbsOrigin( ( vecResult * 0.6f + vecMins ) - ( rand() / RAND_MAX * vecResult ) );

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFProjectile_Arrow::GetBoneAttachmentInfo( mstudiobbox_t *pbox, CBaseAnimating *pAnim, Vector &vecOrigin, QAngle &vecAngles, int &bone, int &iPhysicsBone )
{
	bone = pbox->bone;
	iPhysicsBone = pAnim->GetPhysicsBone( bone );
	//pAnim->GetBonePosition( bone, vecOrigin, vecAngles );

	matrix3x4_t arrowToWorld, boneToWorld, invBoneToWorld, boneToWorldTransform;
	MatrixCopy( EntityToWorldTransform(), arrowToWorld );
	pAnim->GetBoneTransform( bone, boneToWorld );

	MatrixInvert( boneToWorld, invBoneToWorld );
	ConcatTransforms( invBoneToWorld, arrowToWorld, boneToWorldTransform );
	MatrixAngles( boneToWorldTransform, vecAngles );
	MatrixGetColumn( boneToWorldTransform, 3, vecOrigin );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFProjectile_Arrow::CheckRagdollPinned( Vector const& vecOrigin, Vector const& vecDirection, int iBone, int iPhysBone, CBaseEntity *pOther, int iHitGroup, int iVictim )
{
	trace_t tr;
	UTIL_TraceLine( vecOrigin, vecOrigin + vecDirection * 120.f, MASK_BLOCKLOS, pOther, COLLISION_GROUP_NONE, &tr );

	if ( tr.fraction != 1.0f && tr.DidHitWorld() )
	{
		CEffectData data;
		data.m_vOrigin = tr.endpos;
		data.m_vNormal = vecDirection;
		data.m_nEntIndex = pOther->entindex();
		data.m_fFlags = GetProjectileType();
		data.m_nAttachmentIndex = iBone;
		data.m_nMaterial = iPhysBone;
		data.m_nDamageType = iHitGroup;
		data.m_nSurfaceProp = iVictim;
		data.m_nColor = (GetTeamNumber() == TF_TEAM_BLUE); //Skin

		if( GetScorer() )
			data.m_nHitBox = GetScorer()->entindex();

		DispatchEffect( "TFBoltImpact", data );

		return true;
	}

	return false;
}

// ----------------------------------------------------------------------------
// Purpose: Play the impact sound to nearby players of the recipient and the attacker
//-----------------------------------------------------------------------------
void CTFProjectile_Arrow::ImpactSound( const char *pszImpactSound, bool bIsPlayerImpact /*= false*/ )
{
	CTFPlayer *pAttacker = ToTFPlayer( GetOwnerEntity() );
	if ( pAttacker )
	{
		CRecipientFilter filter;
		filter.AddRecipientsByPAS( GetAbsOrigin() );

		// Only play the sound locally to the attacker if it's a player impact
		if ( bIsPlayerImpact )
		{
			filter.RemoveRecipient( pAttacker );

			CSingleUserRecipientFilter filterAttacker( pAttacker );
			EmitSound( filterAttacker, pAttacker->entindex(), pszImpactSound );
		}

		EmitSound( filter, entindex(), pszImpactSound );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CTFProjectile_Arrow::GetArrowSkin( void ) const
{
	if ( m_iProjectileType == TF_PROJECTILETYPE_HEALING_BOLT || m_iProjectileType == TF_PROJECTILETYPE_HEALING_BOLT_FESTIVE )
	{
		switch ( GetOwnerEntity()->GetTeamNumber() )
		{
		case TF_TEAM_BLUE:
			return 1;
			break;
		case TF_TEAM_RED:
		default:
			return 0;
			break;
		}
	}

	return 0;
}

#else
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_TFProjectile_Arrow::OnDataChanged( DataUpdateType_t updateType )
{
	BaseClass::OnDataChanged( updateType );

	if ( updateType == DATA_UPDATE_CREATED )
	{
		SetNextClientThink( CLIENT_THINK_ALWAYS );

		if ( m_bArrowAlight )
			ParticleProp()->Create( "flying_flaming_arrow", PATTACH_POINT_FOLLOW, "muzzle" );


		if ( m_iProjectileType == TF_PROJECTILETYPE_HEALING_BOLT || m_iProjectileType == TF_PROJECTILETYPE_HEALING_BOLT_FESTIVE )
		{
			char const *pszEffect = ConstructTeamParticle( "healshot_trail_%s", GetTeamNumber() );
			ParticleProp()->Create( pszEffect, PATTACH_ABSORIGIN_FOLLOW );
		}
	}

	if ( m_bCritical )
	{
		if ( updateType == DATA_UPDATE_CREATED || m_iDeflected != m_iDeflectedParity )
			CreateCritTrail();
	}

	m_iDeflectedParity = m_iDeflected;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_TFProjectile_Arrow::CreateCritTrail( void )
{
	if ( IsDormant() )
		return;

	if ( m_pCritEffect )
	{
		ParticleProp()->StopEmission( m_pCritEffect );
		m_pCritEffect = NULL;
	}

	char const *pszEffect = ConstructTeamParticle( "critical_rocket_%s", GetTeamNumber() );
	m_pCritEffect = ParticleProp()->Create( pszEffect, PATTACH_ABSORIGIN_FOLLOW );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_TFProjectile_Arrow::ClientThink( void )
{
	if ( !m_bWhizzed && gpGlobals->curtime > m_flCheckNearMiss )
	{
		CheckNearMiss();
		m_flCheckNearMiss = gpGlobals->curtime + 0.05f;
	}

	if ( !m_bCritical )
	{
		if ( m_pCritEffect )
		{
			ParticleProp()->StopEmission( m_pCritEffect );
			m_pCritEffect = NULL;
		}
	}

	if( m_pAttachedTo.Get() )
	{
		if ( gpGlobals->curtime < m_flDieTime )
		{
			Remove();
			return;
		}

		if ( m_pAttachedTo->GetEffects() & EF_NODRAW )
		{
			if ( !( GetEffects() & EF_NODRAW ) )
			{
				AddEffects( EF_NODRAW );
				UpdateVisibility();
			}
		}
	}

	if ( IsDormant() && !( GetEffects() & EF_NODRAW ) )
	{
		AddEffects( EF_NODRAW );
		UpdateVisibility();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_TFProjectile_Arrow::CheckNearMiss( void )
{
	C_TFPlayer *pLocal = C_TFPlayer::GetLocalTFPlayer();
	if ( pLocal == nullptr || !pLocal->IsAlive() )
		return;

	if ( pLocal->GetTeamNumber() == GetTeamNumber() )
		return;

	Vector vecOrigin = GetAbsOrigin();
	Vector vecTarget = pLocal->GetAbsOrigin();

	Vector vecFwd;
	AngleVectors( GetAbsAngles(), &vecFwd );

	Vector vecDirection = vecOrigin + vecFwd * 200;
	if ( ( vecDirection - vecTarget ).LengthSqr() > ( vecOrigin - vecTarget ).LengthSqr() )
	{
		// We passed right by him between frames, doh!
		m_bWhizzed = true;
		return;
	}

	Vector vecClosest; float flDistance;
	CalcClosestPointOnLineSegment( vecTarget, vecOrigin, vecDirection, vecClosest, &flDistance );

	flDistance = ( vecClosest - vecTarget ).Length();
	if ( flDistance <= 120.f )
	{
		m_bWhizzed = true;
		SetNextClientThink( CLIENT_THINK_NEVER );

		trace_t tr;
		UTIL_TraceLine( vecOrigin, vecOrigin + vecFwd * 400, MASK_TFARROWS, this, COLLISION_GROUP_NONE, &tr );

		if ( tr.DidHit() )
			return;

		EmitSound_t parm;
		parm.m_pSoundName = "Weapon_Arrow.Nearmiss";

		CSingleUserRecipientFilter filter( pLocal );
		C_BaseEntity::EmitSound( filter, entindex(), parm );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_TFProjectile_Arrow::NotifyBoneAttached( C_BaseAnimating* attachTarget )
{
	BaseClass::NotifyBoneAttached( attachTarget );

	m_bAttachment = true;

	SetNextClientThink( CLIENT_THINK_ALWAYS );
}

#endif

IMPLEMENT_NETWORKCLASS_ALIASED( TFProjectile_HealingBolt, DT_TFProjectile_HealingBolt )

BEGIN_NETWORK_TABLE( CTFProjectile_HealingBolt, DT_TFProjectile_HealingBolt )

END_NETWORK_TABLE()

LINK_ENTITY_TO_CLASS( tf_projectile_healing_bolt, CTFProjectile_HealingBolt );
PRECACHE_REGISTER( tf_projectile_healing_bolt );

CTFProjectile_HealingBolt::CTFProjectile_HealingBolt()
{
}

CTFProjectile_HealingBolt::~CTFProjectile_HealingBolt()
{
#ifdef CLIENT_DLL
	ParticleProp()->StopEmission();
#endif
}

#ifdef GAME_DLL
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFProjectile_HealingBolt::InitArrow( const QAngle &vecAngles, float flSpeed, float flGravity, ProjectileType_t eType, CBaseEntity *pOwner, CBaseEntity *pWeapon )
{
	

	float flAttribProjectileModelScale = 1.0f;
	CALL_ATTRIB_HOOK_FLOAT_ON_OTHER(pWeapon, flAttribProjectileModelScale, projectile_model_scale);
	string_t strAttribCustomProjModel = NULL_STRING;
	CALL_ATTRIB_HOOK_STRING_ON_OTHER(pWeapon, strAttribCustomProjModel, custom_projectile_model);
	if (strAttribCustomProjModel != NULL_STRING)
	{
		SetModel(STRING(strAttribCustomProjModel));
	}
	CALL_ATTRIB_HOOK_FLOAT_ON_OTHER(pWeapon, flGravity, mult_projectile_gravity);
	CALL_ATTRIB_HOOK_FLOAT_ON_OTHER(pWeapon, flSpeed, mult_projectile_speed);

	BaseClass::InitArrow(vecAngles, flSpeed, flGravity, eType, pOwner, pWeapon);
	SetModelScale( 3.0f * flAttribProjectileModelScale );
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFProjectile_HealingBolt::CanHeadshot( void )
{
	int iCanHeadshot = 0;
	CALL_ATTRIB_HOOK_INT_ON_OTHER(m_hLauncher, iCanHeadshot, can_headshot);
	if (iCanHeadshot != 0)
		return true;
	else
		return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFProjectile_HealingBolt::ImpactTeamPlayerOrNPC( CBaseCombatCharacter *pTarget )
{
	IGameEvent *event = NULL;

	if ( pTarget == nullptr )
		return;

	CTFPlayer *pOwner = ToTFPlayer( GetOwnerEntity() );
	if ( pOwner == nullptr )
		return;

	float flHealing = GetDamage();
	CALL_ATTRIB_HOOK_FLOAT_ON_OTHER( pTarget, flHealing, mult_healing_from_medics );

	CBaseCombatWeapon *pWeapon = pTarget->GetActiveWeapon();
	if ( pWeapon )
	{
		int nBlocksHealing = 0;
		CALL_ATTRIB_HOOK_INT_ON_OTHER( pWeapon, nBlocksHealing, weapon_blocks_healing );
		if ( nBlocksHealing == 1 )
			return;

		CALL_ATTRIB_HOOK_FLOAT_ON_OTHER( pWeapon, flHealing, mult_health_fromhealers_penalty_active );
	}

	if ( pTarget->TakeHealth( flHealing, DMG_GENERIC ) )
	{
		ImpactSound( "Weapon_Arrow.ImpactFleshCrossbowHeal" );

		CTF_GameStats.Event_PlayerHealedOther( pOwner, flHealing );

		if ( pTarget->IsNPC() )
			event = gameeventmanager->CreateEvent( "npc_healed" );
		else
			event = gameeventmanager->CreateEvent( "player_healed" );

		if ( event )
		{
			event->SetInt( "priority", 1 ); // HLTV priority
			event->SetInt( "patient", pTarget->entindex() );
			event->SetInt( "healer", pOwner->entindex() );
			event->SetInt( "amount", flHealing );

			gameeventmanager->FireEvent( event );
		}

		if ( pTarget->IsPlayer() )
		{
			event = gameeventmanager->CreateEvent( "player_healonhit" );
			if ( event )
			{
				event->SetInt( "entindex", pTarget->entindex() );
				event->SetInt( "amount", flHealing );

				gameeventmanager->FireEvent( event );
			}

			// Display health particles for a short duration
			CTFPlayer *pPlayer = ToTFPlayer( pTarget );
			if ( pPlayer )
				pPlayer->m_Shared.AddCond( TF_COND_HEALTH_OVERHEALED, 1.2f );

			CAI_BaseNPC *pNPC = dynamic_cast<CAI_BaseNPC*>( pTarget );
			if ( pNPC )
			{
				pNPC->AddCond( TF_COND_HEALTH_OVERHEALED, 1.2f );
			}
		}
	}
}

#endif

IMPLEMENT_NETWORKCLASS_ALIASED( TFProjectile_GrapplingHook, DT_TFProjectile_GrapplingHook )

BEGIN_NETWORK_TABLE( CTFProjectile_GrapplingHook, DT_TFProjectile_GrapplingHook )

END_NETWORK_TABLE()

LINK_ENTITY_TO_CLASS( tf_projectile_grapplinghook, CTFProjectile_GrapplingHook );
PRECACHE_REGISTER( tf_projectile_grapplinghook );

CTFProjectile_GrapplingHook::CTFProjectile_GrapplingHook()
{
}

CTFProjectile_GrapplingHook::~CTFProjectile_GrapplingHook()
{
#ifdef CLIENT_DLL
	ParticleProp()->StopEmission();
#endif
}