//====== Copyright © 1996-2013, Valve Corporation, All rights reserved. ========//
//
// Purpose: Flare used by the flaregun.
//
//=============================================================================//
#include "cbase.h"
#include "tf_tfc_flame.h"
#include "tf_weapon_compound_bow.h"
#include "tf_projectile_arrow.h"
// Client specific.
#ifdef CLIENT_DLL
#include "c_tf_player.h"
#include "particles_new.h"
#include "iefx.h"
#include "dlight.h"
#include "tempent.h"
#include "c_te_legacytempents.h"
#else
#include "tf_player.h"
#include "tf_fx.h"
#include "ai_basenpc.h"
#include "effect_dispatch_data.h"
#include "collisionutils.h"
#include "tf_team.h"
#include "props.h"
#endif

#ifdef CLIENT_DLL
	extern ConVar lfe_muzzlelight;
#endif

#ifdef GAME_DLL
#define TFC_FLAME_DISTANCE	300
#define TFC_FLAME_RADIUS	22.5
#define TFC_FLAME_SPEED		1150
extern ConVar  tf_fireball_draw_debug_radius;
#endif

#define TF_WEAPON_FIREBALL_MODEL	"models/empty.mdl"

//=============================================================================
//
// Dragon's Fury Projectile
//

BEGIN_DATADESC( CTFCFlame )
END_DATADESC()

LINK_ENTITY_TO_CLASS( tf_tfc_flame, CTFCFlame );
PRECACHE_REGISTER( tf_tfc_flame );

IMPLEMENT_NETWORKCLASS_ALIASED( TFCFlame, DT_TFCFlame )
BEGIN_NETWORK_TABLE( CTFCFlame, DT_TFCFlame )
#ifdef GAME_DLL
	SendPropBool( SENDINFO( m_bCritical ) ),
#else
	RecvPropBool( RECVINFO( m_bCritical ) ),
#endif
END_NETWORK_TABLE()

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CTFCFlame::CTFCFlame()
{

}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CTFCFlame::~CTFCFlame()
{
#ifdef CLIENT_DLL
	ParticleProp()->StopEmission();
#else
	m_bCollideWithTeammates = false;
#endif
}

#ifdef GAME_DLL
//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFCFlame::Precache()
{
	PrecacheModel( TF_WEAPON_FIREBALL_MODEL );

	PrecacheParticleSystem( "superrare_burning1" );
	PrecacheParticleSystem( "rockettrail_waterbubbles" );
	PrecacheTeamParticles( "projectile_fireball_crit_%s" );

	PrecacheScriptSound( "Weapon_DragonsFury.Impact" );
	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: Spawn function
//-----------------------------------------------------------------------------
void CTFCFlame::Spawn()
{
	SetModel( TF_WEAPON_FIREBALL_MODEL );
	BaseClass::Spawn();
	SetMoveType( MOVETYPE_FLY, MOVECOLLIDE_FLY_CUSTOM );
	AddEFlags( EFL_NO_WATER_VELOCITY_CHANGE );
	SetGravity( 0.0f );

	SetSolid( SOLID_NONE );
	SetSolidFlags( FSOLID_NOT_SOLID );
	SetCollisionGroup( COLLISION_GROUP_NONE );

	//float flRadius = GetDamageRadius();
	float flRadius = GetFireballScale();
	CALL_ATTRIB_HOOK_FLOAT_ON_OTHER( GetOwnerEntity(), flRadius, mult_flame_size );
	UTIL_SetSize( this, -Vector( flRadius, flRadius, flRadius ), Vector( flRadius, flRadius, flRadius ) );

	// Setup attributes.
	m_takedamage = DAMAGE_NO;
	m_vecInitialPos = GetOwnerEntity()->GetAbsOrigin();
	m_vecPrevPos = m_vecInitialPos;

	// Setup the think function.
	SetThink( &CTFCFlame::ExpireDelayThink );
	SetNextThink( gpGlobals->curtime );
}

//-----------------------------------------------------------------------------
// Purpose: Think method
//-----------------------------------------------------------------------------
void CTFCFlame::ExpireDelayThink( void )
{
	// Render debug visualization if convar on
	if ( tf_fireball_draw_debug_radius.GetBool() )
	{
		NDebugOverlay::Sphere( GetAbsOrigin(), GetAbsAngles(), GetDamageRadius(), 0, 255, 0, 0, false, 0 );
	}

	SetNextThink( gpGlobals->curtime );

	m_vecPrevPos = GetAbsOrigin();

 	CBaseEntity *pEntity = NULL;
	Vector vecOrigin = GetAbsOrigin();
	CBaseEntity *pBaseAttacker = GetOwnerEntity();
	IScorer *pScorerInterface = dynamic_cast<IScorer*>( pBaseAttacker );
	if ( pScorerInterface )
		pBaseAttacker = pScorerInterface->GetScorer();

	CTFPlayer *pAttacker = dynamic_cast<CTFPlayer *>( pBaseAttacker );
	if ( !pAttacker )
		return;

 	for ( CEntitySphereQuery sphere( vecOrigin, GetDamageRadius() ); ( pEntity = sphere.GetCurrentEntity() ) != NULL; sphere.NextEntity() )
	{
		if ( !pEntity )
			continue;

		// if we've already burnt this entity, don't do more damage, so skip even checking for collision with the entity
		int iIndex = m_hEntitiesBurnt.Find( pEntity );
		if ( iIndex != m_hEntitiesBurnt.InvalidIndex() )
			continue;

 		Vector vecHitPoint;
		pEntity->CollisionProp()->CalcNearestPoint( vecOrigin, &vecHitPoint );
		Vector vecDir = vecHitPoint - vecOrigin;
 		if ( vecDir.LengthSqr() < ( GetDamageRadius() * GetDamageRadius() ) )
		{
			if ( pEntity && !pEntity->InSameTeam( pAttacker ) )
			{
				Burn( pEntity );
			}
		}
	}

	// Render debug visualization if convar on
	if ( tf_fireball_draw_debug_radius.GetBool() )
	{
		if ( m_hEntitiesBurnt.Count() > 0 )
		{
			int val = ( (int) ( gpGlobals->curtime * 10 ) ) % 255;
			NDebugOverlay::Sphere( GetAbsOrigin(), GetAbsAngles(), GetDamageRadius(), val, 255, val, 0, false, 0 );
		} 
		else 
		{
			NDebugOverlay::Sphere( GetAbsOrigin(), GetAbsAngles(), GetDamageRadius(), 0, 100, 0, 0, false, 0 );
		}
	}

	// if we've expired, remove ourselves
	float flDistance = GetAbsOrigin().DistTo( m_vecInitialPos );
	if ( flDistance >= TFC_FLAME_DISTANCE )
	{
		UTIL_Remove( this );
		if ( tf_fireball_draw_debug_radius.GetBool() )
			NDebugOverlay::Sphere( GetAbsOrigin(), GetAbsAngles(), GetDamageRadius(), 0, 100, 0, 0, false, 1 );

		return;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFCFlame::RocketTouch( CBaseEntity *pOther )
{
	BaseClass::RocketTouch( pOther );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFCFlame::SetScorer( CBaseEntity *pScorer )
{
	m_Scorer = pScorer;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CBasePlayer *CTFCFlame::GetScorer( void )
{
	return dynamic_cast<CBasePlayer *>( m_Scorer.Get() );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int	CTFCFlame::GetDamageType() 
{ 
	int iDmgType = BaseClass::GetDamageType();

	// Buff banner mini-crit calculations
	CTFWeaponBase *pWeapon = ( CTFWeaponBase * )m_hLauncher.Get();
	if ( pWeapon )
	{
		pWeapon->CalcIsAttackMiniCritical();
		if ( pWeapon->IsCurrentAttackAMiniCrit() )
		{
			iDmgType |= DMG_MINICRITICAL;
		}
	}

	if ( m_bCritical )
	{
		iDmgType |= DMG_CRITICAL;
	}
	if ( m_iDeflected > 0 )
	{
		iDmgType |= DMG_MINICRITICAL;
	}

	return iDmgType;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFCFlame::Deflected( CBaseEntity *pDeflectedBy, Vector &vecDir )
{
	// Get rocket's speed.
	float flVel = GetAbsVelocity().Length();

	QAngle angForward;
	VectorAngles( vecDir, angForward );

	// Now change rocket's direction.
	SetAbsAngles( angForward );
	SetAbsVelocity( vecDir * flVel );

	// And change owner.
	IncremenentDeflected();
	SetOwnerEntity( pDeflectedBy );
	ChangeTeam( pDeflectedBy->GetTeamNumber() );
	SetScorer( pDeflectedBy );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFCFlame::Explode( trace_t *pTrace, CBaseEntity *pOther )
{
	// Save this entity as enemy, they will take 100% damage.
	m_hEnemy = pOther;

	// Invisible.
	SetModelName( NULL_STRING );
	AddSolidFlags( FSOLID_NOT_SOLID );
	m_takedamage = DAMAGE_NO;

	// Pull out a bit.
	if ( pTrace->fraction != 1.0 )
	{
		SetAbsOrigin( pTrace->endpos + ( pTrace->plane.normal * 1.0f ) );
	}

	// Don't decal players with scorch.
	if ( !pOther->IsPlayer() )
	{
		UTIL_DecalTrace( pTrace, "Scorch" );
	}

	// Remove.
	UTIL_Remove( this );
	if ( tf_fireball_draw_debug_radius.GetBool() )
		NDebugOverlay::Sphere( GetAbsOrigin(), GetAbsAngles(), GetDamageRadius(), 0, 100, 0, 0, false, 1 );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CTFCFlame *CTFCFlame::Create( CBaseEntity *pWeapon, const Vector &vecOrigin, const QAngle &vecAngles, CBaseEntity *pOwner, CBaseEntity *pScorer )
{
	CTFCFlame *pFireball = static_cast<CTFCFlame*>( CBaseEntity::CreateNoSpawn( "tf_tfc_flame", vecOrigin, vecAngles, pOwner ) );

	if ( pFireball )
	{
		// Set team.
		pFireball->ChangeTeam( pOwner->GetTeamNumber() );

		// Set scorer.
		pFireball->SetScorer( pScorer );

		// Set firing weapon.
		pFireball->SetLauncher( pWeapon );

		// Spawn.
		DispatchSpawn( pFireball );

		// Setup the initial velocity.
		Vector vecForward, vecRight, vecUp;
		AngleVectors( vecAngles, &vecForward, &vecRight, &vecUp );

		float flVelocity = TFC_FLAME_SPEED;
		CALL_ATTRIB_HOOK_FLOAT_ON_OTHER( pWeapon, flVelocity, mult_projectile_speed );

		Vector vecVelocity = vecForward * flVelocity;
		pFireball->SetAbsVelocity( vecVelocity );
		pFireball->SetupInitialTransmittedGrenadeVelocity( vecVelocity );

		// Setup the initial angles.
		QAngle angles;
		VectorAngles( vecVelocity, angles );
		pFireball->SetAbsAngles( angles );
		
		float flGravity = 0.0f;
		CALL_ATTRIB_HOOK_FLOAT_ON_OTHER( pWeapon, flGravity, mod_rocket_gravity );
		if ( flGravity )
		{
			pFireball->SetMoveType( MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_CUSTOM );
			pFireball->SetGravity( flGravity );
		}

		return pFireball;
	}

	return pFireball;
}
#else

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFCFlame::OnDataChanged( DataUpdateType_t updateType )
{
	BaseClass::OnDataChanged( updateType );

	if ( updateType == DATA_UPDATE_CREATED )
	{
		CreateTrails();
		CreateLightEffects();
	}

	// Watch team changes and change trail accordingly.
	if ( m_iOldTeamNum && m_iOldTeamNum != m_iTeamNum )
	{
		ParticleProp()->StopEmission();
		CreateTrails();
		CreateLightEffects();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFCFlame::CreateTrails( void )
{
	if ( IsDormant() )
		return;

	if ( enginetrace->GetPointContents( GetAbsOrigin() ) & MASK_WATER )
	{
		ParticleProp()->Create( "rockettrail_waterbubbles", PATTACH_POINT_FOLLOW, "empty" );
	}
	else
	{
		const char *pszFormat = m_bCritical ? "projectile_fireball_crit_%s" : "superrare_burning1";
		const char *pszEffectName = ConstructTeamParticle( pszFormat, GetTeamNumber() );

		ParticleProp()->Create( pszEffectName, PATTACH_POINT_FOLLOW, "empty" );
	}
}

void CTFCFlame::CreateLightEffects( void )
{
	// Handle the dynamic light
	if (lfe_muzzlelight.GetBool())
	{
		AddEffects( EF_DIMLIGHT );

		dlight_t *dl;
		if ( IsEffectActive( EF_DIMLIGHT ) )
		{	
			dl = effects->CL_AllocDlight( LIGHT_INDEX_TE_DYNAMIC + index );
			dl->origin = GetAbsOrigin();
			switch ( GetTeamNumber() )
			{
			case TF_TEAM_RED:
				if ( !m_bCritical )
				{	dl->color.r = 255; dl->color.g = 30; dl->color.b = 10; }
				else
				{	dl->color.r = 255; dl->color.g = 10; dl->color.b = 10; }
				break;

			case TF_TEAM_BLUE:
				if ( !m_bCritical )
				{	dl->color.r = 10; dl->color.g = 30; dl->color.b = 255; }
				else
				{	dl->color.r = 10; dl->color.g = 10; dl->color.b = 255; }
				break;
			}
			dl->radius = 256.0f;
			dl->die = gpGlobals->curtime + 0.1;

			tempents->RocketFlare( GetAbsOrigin() );
		}
	}
}
#endif

#ifdef GAME_DLL
//-----------------------------------------------------------------------------
// Purpose: Called when we've collided with another entity
//-----------------------------------------------------------------------------
void CTFCFlame::Burn( CBaseEntity *pOther )
{
	// remember that we've burnt this one
 	m_hEntitiesBurnt.AddToTail( pOther );

	// Save this entity as enemy, they will take 100% damage.
	m_hEnemy = pOther;

	// Invisible.
	SetModelName( NULL_STRING );
	AddSolidFlags( FSOLID_NOT_SOLID );
	m_takedamage = DAMAGE_NO;

	// Damage.
	CBaseEntity *pAttacker = GetOwnerEntity();
	IScorer *pScorerInterface = dynamic_cast<IScorer*>( pAttacker );
	if ( pScorerInterface )
	{
		pAttacker = pScorerInterface->GetScorer();
	}

	// Play explosion sound and effect.
	Vector vecOrigin = GetAbsOrigin();
	CTFPlayer *pPlayer = ToTFPlayer( pOther );
	CAI_BaseNPC *pNPC = pOther->MyNPCPointer();
	CBreakableProp *pProp = dynamic_cast< CBreakableProp * >( pOther );

	float flDamage = 20;
	int iDamageCustom = TF_DMG_CUSTOM_DRAGONS_FURY_IGNITE;

	if ( pPlayer )
	{
		if ( !pPlayer )
			return;

		// Hit player, do impact sound and more damage
		if (  m_hEntitiesBurnt.Count() > 0 )
		{
			// gotta recharge fast
			//SetHitTarget();
		}

		iDamageCustom = TF_DMG_CUSTOM_DRAGONS_FURY_IGNITE;
	}
	else if ( pNPC )
	{
		if ( !pNPC )
			return;

		// Hit npc, also do impact sound and more damage
		if ( m_hEntitiesBurnt.Count() > 0 )
		{
			// i said gotta recharge fast
			//SetHitTarget();
		}

		iDamageCustom = TF_DMG_CUSTOM_DRAGONS_FURY_IGNITE;
	}
	else if ( pProp )
	{
		if ( !pProp )
			return;

		// If we won't be able to break it, don't burn
		if ( pProp->m_takedamage == DAMAGE_YES )
		{
			pProp->IgniteLifetime( TF_BURNING_FLAME_LIFE );
			ApplyMultiDamage();
		}
	}
	else
	{
		// Hit world, delet this.
		//UTIL_Remove( this );
		ApplyMultiDamage();
	}

	CTakeDamageInfo info( GetOwnerEntity(), pAttacker, m_hLauncher.Get(), flDamage, GetDamageType(), iDamageCustom );
	pOther->TakeDamage( info );

	// Remove.
	//UTIL_Remove( this );

	//SetHitTarget();

	info.SetReportedPosition( pAttacker->GetAbsOrigin() );	
		
	// We collided with pOther, so try to find a place on their surface to show blood
	trace_t pTrace;
	Ray_t ray; ray.Init( WorldSpaceCenter(), pOther->WorldSpaceCenter() );
	UTIL_Portal_TraceRay( ray, /*MASK_SOLID*/ MASK_SHOT|CONTENTS_HITBOX, this, COLLISION_GROUP_DEBRIS, &pTrace );

	pOther->DispatchTraceAttack( info, GetAbsVelocity(), &pTrace );

	ApplyMultiDamage();
}
#endif

#ifdef GAME_DLL
float CTFCFlame::GetDamageRadius( void ) const
{
	return TFC_FLAME_RADIUS;
}

// need correct number
float CTFCFlame::GetFireballScale( void ) const
{
	return 0.05f;
}

#endif