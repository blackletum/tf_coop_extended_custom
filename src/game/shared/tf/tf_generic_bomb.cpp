//====== Copyright © 1996-2006, Valve Corporation, All rights reserved. =======//
//
// Purpose: 
//
//=============================================================================//


#include "cbase.h"
#include "tf_generic_bomb.h"
#include "tf_gamerules.h"
#include "particle_parse.h"
#include "tf_projectile_base.h"
#include "tf_weaponbase_rocket.h"
#include "tf_weaponbase_grenadeproj.h"

#ifdef CLIENT_DLL
#include "c_tf_player.h"
#else
#include "world.h"
#include "te_particlesystem.h"
#include "tf_fx.h"
#include "tf_player.h"
#include "baseanimating.h"
#include "util.h"
#include "props.h"
#include "triggers.h"
#endif


// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_AUTO_LIST( ITFGenericBomb );

LINK_ENTITY_TO_CLASS( tf_generic_bomb, CTFGenericBomb );

IMPLEMENT_NETWORKCLASS_ALIASED( TFGenericBomb, DT_TFGenericBomb )

BEGIN_NETWORK_TABLE( CTFGenericBomb, DT_TFGenericBomb )
END_NETWORK_TABLE()


#ifdef GAME_DLL
BEGIN_DATADESC( CTFGenericBomb )	
	DEFINE_KEYFIELD( m_flDamage,		FIELD_FLOAT,		"damage" ),
	DEFINE_KEYFIELD( m_flRadius,		FIELD_FLOAT,		"radius" ),
	DEFINE_KEYFIELD( m_iHealth,			FIELD_INTEGER,		"health" ),
	DEFINE_KEYFIELD( m_iszParticleName,	FIELD_STRING,		"explode_particle"),
	DEFINE_KEYFIELD( m_iszExplodeSound,	FIELD_SOUNDNAME,	"sound" ),
	DEFINE_KEYFIELD( m_bFriendlyFire,	FIELD_INTEGER,		"friendlyfire" ),

	DEFINE_INPUTFUNC( FIELD_VOID, "Detonate", InputDetonate ),
	DEFINE_OUTPUT( m_OnDetonate, "OnDetonate" ),

	DEFINE_FUNCTION( CTFGenericBombShim::Touch ),
END_DATADESC()
#endif

CTFGenericBomb::CTFGenericBomb()
{
#ifdef GAME_DLL
	SetMaxHealth( 1 );
	SetHealth( 1 );

	m_flDamage = 50.0f;
	m_flRadius = 100.0f;
	m_bFriendlyFire = false;
#endif
}

void CTFGenericBomb::Precache()
{
#ifdef GAME_DLL
	PrecacheParticleSystem( STRING( m_iszParticleName ) );
	PrecacheScriptSound( STRING( m_iszExplodeSound ) );
#endif
	BaseClass::Precache();
}

void CTFGenericBomb::Spawn()
{
	BaseClass::Spawn();
	m_takedamage = DAMAGE_YES;

	Precache();
#ifdef GAME_DLL
	char const *szModel = STRING( GetModelName() );
	if ( !szModel || !szModel[0] )
	{
		Warning( "prop at %.0f %.0f %0.f missing modelname\n", GetAbsOrigin().x, GetAbsOrigin().y, GetAbsOrigin().z );
		UTIL_Remove( this );
		return;
	}

	int index = PrecacheModel( szModel );
	PrecacheGibsForModel( index );

	SetModel( szModel );

	VPhysicsInitStatic();
	SetCollisionGroup( TFCOLLISION_GROUP_PUMPKIN_BOMB );
	SetMoveType( MOVETYPE_VPHYSICS );
	SetSolid( SOLID_VPHYSICS );
	SetSolid( SOLID_BBOX );
	SetElasticity( 0.01f );
	SetGravity( 1.0f );
#endif
	//VPhysicsInitNormal( SOLID_VPHYSICS, 0, false );

	SetTouch( &CTFGenericBomb::GenericTouch );
}

#ifdef GAME_DLL
void CTFGenericBomb::Event_Killed( const CTakeDamageInfo &info )
{
	m_takedamage = DAMAGE_NO;

	trace_t	tr;
	Vector vecForward = GetAbsVelocity();
	VectorNormalize( vecForward );
	UTIL_TraceLine ( GetAbsOrigin(), GetAbsOrigin() + 60*vecForward , MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);

	Vector vecOrigin = GetAbsOrigin(); QAngle vecAngles = GetAbsAngles();
	int iAttachment = LookupAttachment("alt-origin");
	if ( iAttachment > 0 )
		GetAttachment( iAttachment, vecOrigin, vecAngles );

	CPVSFilter filter( GetAbsOrigin() );

	if ( m_iszParticleName != NULL_STRING )
		TE_TFParticleEffect( filter, 0.0, STRING( m_iszParticleName ), vecOrigin, vecAngles, NULL, PATTACH_CUSTOMORIGIN );
	if ( m_iszExplodeSound != NULL_STRING )
		EmitSound( STRING( m_iszExplodeSound ) );

	SetSolid( SOLID_NONE ); 

	CBaseEntity *pAttacker = this;

	if ( info.GetAttacker() && info.GetAttacker()->IsPlayer() )
		pAttacker = info.GetAttacker();

	CTakeDamageInfo info_modified( this, pAttacker, m_flDamage, DMG_BLAST, GetCustomDamageType() );

	if ( m_bFriendlyFire )
		info_modified.SetForceFriendlyFire( true );

	TFGameRules()->RadiusDamage( info_modified, GetAbsOrigin(), m_flRadius, CLASS_NONE, this );

	if ( tr.m_pEnt && !tr.m_pEnt->IsPlayer() )
		UTIL_DecalTrace( &tr, "Scorch");

	UserMessageBegin( filter, "BreakModel" );
		WRITE_SHORT( GetModelIndex() );
		WRITE_VEC3COORD( GetAbsOrigin() );
		WRITE_ANGLES( GetAbsAngles() );
		WRITE_SHORT( m_nSkin );
	MessageEnd();

	m_OnDetonate.FireOutput( this, this );
	BaseClass::Event_Killed( info );
}

void CTFGenericBomb::InputDetonate( inputdata_t &inputdata )
{
	CTakeDamageInfo empty;
	Event_Killed( empty );
}
#endif

void CTFGenericBomb::GenericTouch( CBaseEntity *pOther )
{
	if ( !pOther || !( pOther->GetFlags() & FL_GRENADE ) )
		return;

#ifdef GAME_DLL
	CTFPlayer *pPlayer = NULL;
	CBaseGrenade *pGrenade = dynamic_cast<CBaseGrenade *>( pOther );
	if ( pGrenade )
	{
		pPlayer = ToTFPlayer( pGrenade->GetThrower() );
		pGrenade->ExplodeTouch( this );
	}
	
	CTFBaseRocket *pRocket = dynamic_cast<CTFBaseRocket *>( pOther );
	if ( pRocket )
	{
		pPlayer = ToTFPlayer( pRocket->GetOwnerEntity() );
		pRocket->RocketTouch( this );
	}
    
	if ( !pGrenade && !pRocket )
	{
		CBaseProjectile *pProj = dynamic_cast<CBaseProjectile *>( pOther );
		if ( pProj )
		{
			pPlayer = ToTFPlayer( pProj->GetOwnerEntity() );
		}
	}

	CTakeDamageInfo info( pOther, pPlayer, 10.0f, DMG_GENERIC );
	TakeDamage( info );
#endif
}

IMPLEMENT_AUTO_LIST( ITFPumpkinBomb );

LINK_ENTITY_TO_CLASS( tf_pumpkin_bomb, CTFPumpkinBomb );

IMPLEMENT_NETWORKCLASS_ALIASED( TFPumpkinBomb, DT_TFPumpkinBomb )

BEGIN_NETWORK_TABLE( CTFPumpkinBomb, DT_TFPumpkinBomb )
END_NETWORK_TABLE()

CTFPumpkinBomb::CTFPumpkinBomb()
{
#ifdef GAME_DLL
	m_bKilled = false;
	m_bPrecached = false;
	m_iTeam = TEAM_UNASSIGNED;
	m_flDamage = 150.0f;
	m_flScale = 1.0f;
	m_flRadius = 300.0f;
	m_flLifeTime = -1.0f;
#endif
}

void CTFPumpkinBomb::Precache( void )
{
	BaseClass::Precache();

	bool allowPrecache = CBaseEntity::IsPrecacheAllowed();
	CBaseEntity::SetAllowPrecache( true );

#ifdef GAME_DLL
	int iMdlIdx = PrecacheModel( "models/props_halloween/pumpkin_explode.mdl" );
	PrecacheGibsForModel( iMdlIdx );
#endif

	PrecacheModel( "models/props_halloween/pumpkin_explode_teamcolor.mdl" );

	PrecacheScriptSound( "Halloween.PumpkinExplode" );

	CBaseEntity::SetAllowPrecache( allowPrecache );

#ifdef GAME_DLL
	m_bPrecached = true;
#endif
}

void CTFPumpkinBomb::Spawn( void )
{
#ifdef GAME_DLL
	if ( !m_bPrecached )
		Precache();

	if ( m_iTeam == TEAM_UNASSIGNED )
	{
		SetModel( "models/props_halloween/pumpkin_explode.mdl" );
		SetMoveType( MOVETYPE_VPHYSICS );
		SetSolid( SOLID_VPHYSICS );
	}
	else
	{
		SetModel( "models/props_halloween/pumpkin_explode_teamcolor.mdl" );

		switch ( m_iTeam )
		{
			case TF_TEAM_RED:
				m_nSkin = 0;
				break;
			case TF_TEAM_BLUE:
				m_nSkin = 1;
				break;

			default:
				m_nSkin = 0;
				break;
		}

		SetCollisionGroup( TFCOLLISION_GROUP_PUMPKIN_BOMB );
		SetMoveType( MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_BOUNCE );
		SetSolid( SOLID_BBOX );
	}

#endif

	BaseClass::Spawn();

	m_iHealth = 1;
#ifdef GAME_DLL
	m_bKilled = false;
#endif
	m_takedamage = DAMAGE_YES;

#ifdef GAME_DLL
	if ( m_flLifeTime > 0.0f )
	{
		SetThink( &CTFPumpkinBomb::RemovePumpkin );
		SetNextThink( m_flLifeTime + gpGlobals->curtime );
	}
#endif
}

#ifdef GAME_DLL
int CTFPumpkinBomb::OnTakeDamage( CTakeDamageInfo const &info )
{
	if ( m_iTeam != TEAM_UNASSIGNED )
	{
		const char *szEffectName;
		switch ( m_iTeam )
		{
			case TF_TEAM_RED:
				szEffectName = "spell_pumpkin_mirv_goop_red";
				break;
			case TF_TEAM_BLUE:
				szEffectName = "spell_pumpkin_mirv_goop_blue";
				break;

			default:
				szEffectName = "spell_pumpkin_mirv_goop_blue";
				break;
		}

		CPVSFilter filter( GetAbsOrigin() );
		TE_TFParticleEffect( filter, 0, szEffectName, GetAbsOrigin(), vec3_angle );
	}

	CBaseEntity *pAttacker = info.GetAttacker();
	if ( pAttacker && pAttacker->GetTeamNumber() == m_iTeam )
	{
		SetHealth( 1 );
		return BaseClass::OnTakeDamage( info );
	}

	if ( m_iTeam != TEAM_UNASSIGNED )
	{
		RemovePumpkin();
		return 0;
	}

	return BaseClass::OnTakeDamage( info );
}


void CTFPumpkinBomb::Event_Killed( CTakeDamageInfo const &info )
{
	CBaseEntity *pAttacker = info.GetAttacker();

	if ( m_bKilled ) return;
	m_bKilled = true;

	if ( m_iTeam != TEAM_UNASSIGNED )
	{
		if ( pAttacker && pAttacker->GetTeamNumber() != m_iTeam )
		{
			RemovePumpkin();
			return;
		}
	}

	trace_t	tr;
	UTIL_TraceLine( GetAbsOrigin(), GetAbsOrigin() + Vector( 0, 0, 8.0f ), MASK_SHOT_HULL, this, COLLISION_GROUP_NONE, &tr );

	CPVSFilter filter( GetAbsOrigin() );
	TE_TFExplosion( filter, 0, GetAbsOrigin(), tr.plane.normal, TF_WEAPON_PUMPKIN_BOMB, -1 );
	TE_TFParticleEffect( filter, 0, "pumpkin_explode", GetAbsOrigin(), vec3_angle );

	if ( tr.m_pEnt && !tr.m_pEnt->IsPlayer() )
		UTIL_DecalTrace( &tr, "Scorch" );

	SetSolid( SOLID_NONE );

	CTakeDamageInfo newInfo( this, info.GetAttacker(), m_flDamage, DMG_BLAST | DMG_HALF_FALLOFF, GetCustomDamageType() );
	CTFRadiusDamageInfo radiusInfo;
	radiusInfo.info = &newInfo;
	newInfo.SetDamageCustom( m_bSpell ? TF_DMG_CUSTOM_SPELL_MIRV : TF_DMG_CUSTOM_PUMPKIN_BOMB );
	radiusInfo.m_flRadius = m_flRadius;
	radiusInfo.m_flSelfDamageRadius = 0.0f;
	radiusInfo.m_vecSrc = GetAbsOrigin();

	if ( TFGameRules() )
		TFGameRules()->RadiusDamage( radiusInfo );

	UserMessageBegin( filter, "BreakModel" );
		WRITE_SHORT( GetModelIndex() );
		WRITE_VEC3COORD( GetAbsOrigin() );
		WRITE_ANGLES( GetAbsAngles() );
		WRITE_SHORT( m_nSkin );
	MessageEnd();

	BaseClass::Event_Killed( info );
}

void CTFPumpkinBomb::RemovePumpkin( void )
{
	const char *szEffectName;
	switch ( m_iTeam )
	{
		case TF_TEAM_RED:
			szEffectName = "spell_pumpkin_mirv_goop_red";
			break;
		case TF_TEAM_BLUE:
			szEffectName = "spell_pumpkin_mirv_goop_blue";
			break;

		default:
			szEffectName = "spell_pumpkin_mirv_goop_blue";
			break;
	}

	CPVSFilter filter( GetAbsOrigin() );
	TE_TFParticleEffect( filter, 0, szEffectName, GetAbsOrigin(), vec3_angle );

	UTIL_Remove( this );
}

void CTFPumpkinBomb::Break()
{
	CPVSFilter filter( GetAbsOrigin() );
	UserMessageBegin( filter, "BreakModel" );
		WRITE_SHORT( GetModelIndex() );
		WRITE_VEC3COORD( GetAbsOrigin() );
		WRITE_ANGLES( GetAbsAngles() );
		WRITE_SHORT( m_nSkin );
	MessageEnd();
}
#endif

void CTFPumpkinBomb::PumpkinTouch( CBaseEntity *pOther )
{
	if ( pOther == nullptr )
		return;

#ifdef GAME_DLL
	if ( !( pOther->GetFlags() & FL_GRENADE ) )
	{
		if ( FStrEq( pOther->GetClassname(), "trigger_hurt" ) )
			RemovePumpkin();

		return;
	}

	CTFPlayer *pPlayer = NULL;
	CBaseGrenade *pGrenade = dynamic_cast<CBaseGrenade *>( pOther );
	if ( pGrenade )
	{
		pPlayer = ToTFPlayer( pGrenade->GetThrower() );
		pGrenade->ExplodeTouch( this );
	}

	CTFBaseRocket *pRocket = dynamic_cast<CTFBaseRocket *>( pOther );
	if ( pRocket )
	{
		pPlayer = ToTFPlayer( pRocket->GetOwnerEntity() );
		pRocket->RocketTouch( this );
	}

	if ( !pGrenade && !pRocket )
	{
		CTFBaseProjectile *pProj = dynamic_cast<CTFBaseProjectile *>( pOther );
		if ( pProj )
		{
			pPlayer = ToTFPlayer( pProj->GetScorer() );
		}
	}

	if ( m_iTeam != TEAM_UNASSIGNED && pOther->GetTeamNumber() != m_iTeam )
	{
		RemovePumpkin();
		return;
	}

	CTakeDamageInfo info( pOther, pPlayer, 10.0f, DMG_GENERIC );
	TakeDamage( info );
#endif
}
