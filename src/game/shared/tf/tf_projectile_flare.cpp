//====== Copyright © 1996-2013, Valve Corporation, All rights reserved. ========//
//
// Purpose: Flare used by the flaregun.
//
//=============================================================================//
#include "cbase.h"
#include "tf_projectile_flare.h"
#include "tf_weapon_compound_bow.h"
#include "tf_projectile_arrow.h"
#include "tf_weapon_flaregun.h"
#include "tf_gamerules.h"
#include "effect_dispatch_data.h"
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

#define TF_WEAPON_FLARE_MODEL		"models/weapons/w_models/w_flaregun_shell.mdl"
#define TF_WEAPON_FLARE_MODEL_TFC	"models/rpgrocket.mdl"
#define TF_DETONATOR_RADIUS			110.0f	// Matches live tf2.

BEGIN_DATADESC( CTFProjectile_Flare )
END_DATADESC()

LINK_ENTITY_TO_CLASS( tf_projectile_flare, CTFProjectile_Flare );
PRECACHE_REGISTER( tf_projectile_flare );

IMPLEMENT_NETWORKCLASS_ALIASED( TFProjectile_Flare, DT_TFProjectile_Flare )
BEGIN_NETWORK_TABLE( CTFProjectile_Flare, DT_TFProjectile_Flare )
#ifdef GAME_DLL
	SendPropBool( SENDINFO( m_bCritical ) ),
#else
	RecvPropBool( RECVINFO( m_bCritical ) ),
#endif
END_NETWORK_TABLE()

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CTFProjectile_Flare::CTFProjectile_Flare()
{

}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CTFProjectile_Flare::~CTFProjectile_Flare()
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
void CTFProjectile_Flare::Precache()
{
	PrecacheModel( TF_WEAPON_FLARE_MODEL_TFC );
	PrecacheModel( TF_WEAPON_FLARE_MODEL );

	PrecacheParticleSystem( "ExplosionCore_MidAir" );
	PrecacheParticleSystem( "flaregun_destroyed" );
	PrecacheParticleSystem( "pyrovision_flaregun_destroyed" );

	PrecacheParticleSystem( "rockettrail_waterbubbles" );
	PrecacheTeamParticles( "flaregun_trail_%s" );
	PrecacheTeamParticles( "flaregun_trail_crit_%s" );
	PrecacheTeamParticles( "pyrovision_flaregun_trail_%s" );
	PrecacheTeamParticles( "pyrovision_flaregun_trail_crit_%s" );

	PrecacheScriptSound( "TFPlayer.FlareImpact" );
	PrecacheScriptSound( "Weapon_Detonator.Detonate" );
	PrecacheScriptSound( "Weapon_Detonator.DetonateWorld" );
	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: Spawn function
//-----------------------------------------------------------------------------
void CTFProjectile_Flare::Spawn()
{
	if ( TFGameRules()->IsTFCAllowed() )
	{
		SetModel( TF_WEAPON_FLARE_MODEL_TFC );
	}
	else
	{
		SetModel( TF_WEAPON_FLARE_MODEL );
	}

	BaseClass::Spawn();
	SetMoveType( MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_CUSTOM );
	SetGravity( 0.3f );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFProjectile_Flare::SetScorer( CBaseEntity *pScorer )
{
	m_Scorer = pScorer;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CBasePlayer *CTFProjectile_Flare::GetScorer( void )
{
	return dynamic_cast<CBasePlayer *>( m_Scorer.Get() );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int	CTFProjectile_Flare::GetDamageType() 
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
void CTFProjectile_Flare::Deflected( CBaseEntity *pDeflectedBy, Vector &vecDir )
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
void CTFProjectile_Flare::Explode( trace_t *pTrace, CBaseEntity *pOther )
{
	// Save this entity as enemy, they will take 100% damage.
	m_hEnemy = pOther;

	// Invisible.
	SetModelName( NULL_STRING );
	AddSolidFlags( FSOLID_NOT_SOLID );
	m_takedamage = DAMAGE_NO;

	// Pull out a bit.
	if ( pTrace->fraction != 1.0 )
		SetAbsOrigin( pTrace->endpos + ( pTrace->plane.normal * 1.0f ) );

	// Don't decal players with scorch.
	if ( !pOther->IsPlayer() )
		UTIL_DecalTrace( pTrace, "Scorch" );

	// Damage.
	CBaseEntity *pAttacker = GetOwnerEntity();
	IScorer *pScorerInterface = dynamic_cast<IScorer*>( pAttacker );
	if ( pScorerInterface )
	{
		pAttacker = pScorerInterface->GetScorer();
	}

	// Figure out Econ ID.
	int iItemID = -1;
	if ( m_hLauncher.Get() )
	{
		CTFWeaponBase *pWeapon = dynamic_cast<CTFWeaponBase *>( m_hLauncher.Get() );
		if ( pWeapon )
		{
			iItemID = pWeapon->GetItemID();
		}
	}

	int iDamageType = GetDamageType();
	const char *pszSound = "TFPlayer.FlareImpact";

	int iType = 0;
	bool bDetonator = false;
	CALL_ATTRIB_HOOK_INT_ON_OTHER( m_hLauncher.Get(), iType, set_weapon_mode );
	if ( iType == 1 )
		bDetonator = true;

	// Play explosion sound and effect.
	Vector vecOrigin = GetAbsOrigin();
	CTFPlayer *pPlayer = ToTFPlayer( pOther );
	CAI_BaseNPC *pNPC = pOther->MyNPCPointer();
	CBreakableProp *pProp = dynamic_cast< CBreakableProp * >( pOther );

	CPVSFilter filter( vecOrigin );
	if ( pPlayer && ( pPlayer != pAttacker ) )
	{
		if ( !pPlayer )
			return;

		// Hit player, do impact sound
		if ( pPlayer->m_Shared.InCond( TF_COND_BURNING ) )
		{
			if ( bDetonator )
				iDamageType |= DMG_MINICRITICAL;
			else
				m_bCritical = true;
		}

		if ( !bDetonator )
			EmitSound( filter, pPlayer->entindex(), pszSound );

		TE_TFExplosion( filter, 0.0f, vecOrigin, pTrace->plane.normal, GetWeaponID(), pPlayer->entindex(), iItemID );
	}
	else if ( pNPC )
	{
		if ( !pNPC )
			return;

		// Hit npc, also do impact sound
		if ( pNPC->InCond( TF_COND_BURNING ) )
		{
			if ( bDetonator )
				iDamageType |= DMG_MINICRITICAL;
			else
				m_bCritical = true;
		}

		if ( !bDetonator )
			EmitSound( filter, pNPC->entindex(), pszSound );

		TE_TFExplosion( filter, 0.0f, vecOrigin, pTrace->plane.normal, GetWeaponID(), pNPC->entindex(), iItemID );
	}
	else if ( pProp )
	{
		if ( !pProp )
			return;

		// If we won't be able to break it, don't burn
		if ( pProp->m_takedamage == DAMAGE_YES )
			pProp->IgniteLifetime( TF_BURNING_FLAME_LIFE );

		if ( !bDetonator )
			EmitSound( filter, pProp->entindex(), pszSound );

		TE_TFExplosion( filter, 0.0f, vecOrigin, pTrace->plane.normal, GetWeaponID(), pProp->entindex(), iItemID );
	}
	else
	{
		// Hit world, do the explosion effect.
		TE_TFExplosion( filter, 0.0f, vecOrigin, pTrace->plane.normal, GetWeaponID(), pOther->entindex(), iItemID );
	}

	if ( m_bCritical )
		iDamageType |= DMG_CRITICAL;

	CTakeDamageInfo info( this, pAttacker, m_hLauncher.Get(), GetDamage(), iDamageType /*, TF_DMG_CUSTOM_BURNING_FLARE*/ );
	info.SetReportedPosition( pAttacker ? pAttacker->GetAbsOrigin() : vec3_origin );
	pOther->TakeDamage( info );

	if ( bDetonator )
	{
		Explode_Air( pTrace, 0, false );
	}
	else
	{
		SendDeathNotice();
		UTIL_Remove( this );
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
float CTFProjectile_Flare::GetRadius( void )
{
	float flRadius = TF_DETONATOR_RADIUS;
	CALL_ATTRIB_HOOK_FLOAT_ON_OTHER( m_hLauncher.Get(), flRadius, mult_explosion_radius );
	return flRadius;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFProjectile_Flare::SendDeathNotice( void )
{
	CTFFlareGun *pLauncher = dynamic_cast<CTFFlareGun*>( m_hLauncher.Get() );
	if ( pLauncher )
		pLauncher->DeathNotice( this );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFProjectile_Flare::Explode_Air( trace_t *pTrace, int nSomething, bool bDetonate )
{
	CDisablePredictionFiltering disabler;

	CBaseEntity *pAttacker = GetOwnerEntity();
	IScorer *pScorerInterface = dynamic_cast<IScorer*>( pAttacker );
	if ( pScorerInterface )
		pAttacker = pScorerInterface->GetScorer();

	if ( !pAttacker )
		return;

	// Figure out Econ ID.
	int iItemID = -1;
	if ( m_hLauncher.Get() )
	{
		CTFWeaponBase *pWeapon = dynamic_cast<CTFWeaponBase *>( m_hLauncher.Get() );
		if ( pWeapon )
		{
			iItemID = pWeapon->GetItemID();
		}
	}

	// Play explosion sound and effect.
	Vector vecOrigin = GetAbsOrigin();
	CPVSFilter filter( vecOrigin );
	TE_TFExplosion( filter, 0.0f, vecOrigin, pTrace->plane.normal, GetWeaponID(), pAttacker->entindex(), iItemID );

	// Damage.
	float flRadius = GetRadius();
	CTakeDamageInfo newInfo( this, pAttacker, m_hLauncher.Get(), vec3_origin, vecOrigin, GetDamage(), GetDamageType(), TF_DMG_CUSTOM_FLARE_EXPLOSION );
	CTFRadiusDamageInfo radiusInfo;
	radiusInfo.info = &newInfo;
	radiusInfo.m_vecSrc = vecOrigin;

	if ( bDetonate )
	{
		DispatchParticleEffect( "ExplosionCore_MidAir", vecOrigin, vec3_angle );
		EmitSound( filter, pAttacker->entindex(), "Weapon_Detonator.Detonate" );
		radiusInfo.m_flRadius = flRadius;
		radiusInfo.m_flSelfDamageRadius = flRadius;
	}
	else
	{
		DispatchParticleEffect( "flaregun_destroyed", vecOrigin, vec3_angle );
		EmitSound( filter, pAttacker->entindex(), "Weapon_Detonator.DetonateWorld" );
		radiusInfo.m_pEntityIgnore = pTrace->m_pEnt; // ignore everything except owner
		radiusInfo.m_flRadius = 0;
		radiusInfo.m_flSelfDamageRadius = flRadius;
	}

	CSoundEnt::InsertSound( SOUND_DANGER, vecOrigin, 150, 3.0 );

	TFGameRules()->RadiusDamage( radiusInfo );

	SendDeathNotice();
	UTIL_Remove( this );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFProjectile_Flare::Detonate( bool bMidAir )
{
	trace_t		tr;
	Vector		vecSpot;// trace starts here!

	SetThink( NULL );

	vecSpot = GetAbsOrigin() + Vector ( 0 , 0 , 8 );
	Ray_t ray; ray.Init( vecSpot, vecSpot + Vector ( 0, 0, -32 ) );
	UTIL_Portal_TraceRay( ray, MASK_SHOT_HULL, this, COLLISION_GROUP_NONE, &tr );

	if ( bMidAir )
		Explode_Air( &tr, 0, true );
	else
		Explode( &tr, tr.m_pEnt );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CTFProjectile_Flare *CTFProjectile_Flare::Create( CBaseEntity *pWeapon, const Vector &vecOrigin, const QAngle &vecAngles, CBaseEntity *pOwner, CBaseEntity *pScorer )
{
	CTFProjectile_Flare *pFlare = static_cast<CTFProjectile_Flare*>( CBaseEntity::CreateNoSpawn( "tf_projectile_flare", vecOrigin, vecAngles, pOwner ) );

	if ( pFlare )
	{
		// Set team.
		pFlare->ChangeTeam( pOwner->GetTeamNumber() );

		// Set scorer.
		pFlare->SetScorer( pScorer );

		// Set firing weapon.
		pFlare->SetLauncher( pWeapon );

		// Spawn.
		DispatchSpawn( pFlare );

		// Setup the initial velocity.
		Vector vecForward, vecRight, vecUp;
		AngleVectors( vecAngles, &vecForward, &vecRight, &vecUp );

		float flVelocity = 2000.0f;
		CALL_ATTRIB_HOOK_FLOAT_ON_OTHER( pWeapon, flVelocity, mult_projectile_speed );

		Vector vecVelocity = vecForward * flVelocity;
		pFlare->SetAbsVelocity( vecVelocity );
		pFlare->SetupInitialTransmittedGrenadeVelocity( vecVelocity );

		// Setup the initial angles.
		QAngle angles;
		VectorAngles( vecVelocity, angles );
		pFlare->SetAbsAngles( angles );
		return pFlare;
	}

	return pFlare;
}
#else

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFProjectile_Flare::OnDataChanged( DataUpdateType_t updateType )
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
void CTFProjectile_Flare::CreateTrails( void )
{
	if ( IsDormant() )
		return;

	if ( enginetrace->GetPointContents( GetAbsOrigin() ) & MASK_WATER )
	{
		ParticleProp()->Create( "rockettrail_waterbubbles", PATTACH_ABSORIGIN_FOLLOW );
	}
	else
	{
		const char *pszFormat = m_bCritical ? "flaregun_trail_crit_%s" : "flaregun_trail_%s";
		const char *pszEffectName = ConstructTeamParticle( pszFormat, GetTeamNumber() );

		ParticleProp()->Create( pszEffectName, PATTACH_ABSORIGIN_FOLLOW );
	}
}

void CTFProjectile_Flare::CreateLightEffects( void )
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
				{	dl->color.r = 255; dl->color.g = 100; dl->color.b = 10; }
				else
				{	dl->color.r = 255; dl->color.g = 10; dl->color.b = 10; }
				break;

			case TF_TEAM_BLUE:
				if ( !m_bCritical )
				{	dl->color.r = 255; dl->color.g = 100; dl->color.b = 10; }
				else
				{	dl->color.r = 10; dl->color.g = 10; dl->color.b = 255; }
				break;
			}
			dl->radius = 128.0f;
			dl->die = gpGlobals->curtime + 0.001;

			tempents->RocketFlare( GetAbsOrigin() );
		}
	}
}

#endif