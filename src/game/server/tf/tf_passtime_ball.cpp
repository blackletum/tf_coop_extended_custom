//Copyright, Valve Corporation,Bad Robot,Escalation Studios, All rights reserved//
//
// Purpose: CTF Passtime Ball.
//
//=============================================================================//

#include "cbase.h"
#include "tf_passtime_ball.h"
//#include "passtime_ballcontroller.h"
#include "tf_shareddefs.h"
#include "tf_gamerules.h"
#include "explode.h"
#include "func_passtime_goal.h"
#include "particle_parse.h"
#include "SpriteTrail.h"
#include "tf_weaponbase.h"
#include "econ_item_view.h"
#include "ilagcompensationmanager.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define TF_MODEL_PASSTIME_BALL				"models/passtime/ball/passtime_ball.mdl"
#define TF_MODEL_PASSTIME_BALL_HALLOWEEN	"models/passtime/ball/passtime_ball_halloween.mdl"

ConVar tf_passtime_ball_damping_scale( "tf_passtime_ball_damping_scale", "0", FCVAR_NOTIFY | FCVAR_REPLICATED );
ConVar tf_passtime_ball_drag_coefficient( "tf_passtime_ball_drag_coefficient", "0", FCVAR_NOTIFY | FCVAR_REPLICATED );
ConVar tf_passtime_ball_inertia_scale( "tf_passtime_ball_inertia_scale", "1", FCVAR_NOTIFY | FCVAR_REPLICATED );
ConVar tf_passtime_ball_mass( "tf_passtime_ball_mass", "1", FCVAR_NOTIFY | FCVAR_REPLICATED );
ConVar tf_passtime_ball_model( "tf_passtime_ball_model", "0", FCVAR_NOTIFY | FCVAR_REPLICATED, "Needs a model with collision info. Map change required." );
ConVar tf_passtime_ball_reset_time( "tf_passtime_ball_reset_time", "15", FCVAR_NOTIFY | FCVAR_REPLICATED, "How long the ball can be neutral before being automatically reset." );
ConVar tf_passtime_ball_rotdamping_scale( "tf_passtime_ball_rotdamping_scale", "1", FCVAR_NOTIFY | FCVAR_REPLICATED, "Higher values will prevent the ball from rolling on the ground." );
ConVar tf_passtime_ball_seek_range( "tf_passtime_ball_seek_range", "128", FCVAR_NOTIFY | FCVAR_REPLICATED, "How close players have to be for the ball to be drawn to them." );
ConVar tf_passtime_ball_seek_speed_factor( "tf_passtime_ball_seek_speed_factor", "3", FCVAR_NOTIFY | FCVAR_REPLICATED, "How fast the ball will move toward nearby players as a ratio of that player's max speed." );
ConVar tf_passtime_ball_sphere_collision( "tf_passtime_ball_sphere_collision", "1", FCVAR_NOTIFY | FCVAR_REPLICATED, "Boolean value. If nonzero, override mdl collision with a perfect sphere collider." );
ConVar tf_passtime_ball_sphere_radius( "tf_passtime_ball_sphere_radius", "7", FCVAR_NOTIFY | FCVAR_REPLICATED );
ConVar tf_passtime_ball_takedamage( "tf_passtime_ball_takedamage", "1", FCVAR_NOTIFY | FCVAR_REPLICATED, "Enables shooting the ball" );
ConVar tf_passtime_ball_takedamage_force( "tf_passtime_ball_takedamage_force", "800", FCVAR_NOTIFY | FCVAR_REPLICATED, "Controls how much the ball responds to being shot" );
ConVar tf_passtime_mode_homing_speed( "tf_passtime_mode_homing_speed", "1000", FCVAR_NOTIFY | FCVAR_REPLICATED, "How fast the ball moves during a pass." );

//----------------------------------------------

class CBallPlayerToucher : public CBaseEntity
{
public:
	DECLARE_CLASS( CBallPlayerToucher, CBaseEntity );
	DECLARE_DATADESC();

	~CBallPlayerToucher();

	virtual void			Spawn();
	virtual bool			ShouldCollide( int collisionGroup, int contentsMask ) const;
	virtual void			OnTouch( CBaseEntity *pOther );
};

BEGIN_DATADESC( CBallPlayerToucher )
	DEFINE_ENTITYFUNC( OnTouch ),
END_DATADESC();

LINK_ENTITY_TO_CLASS( _ballplayertoucher, CBallPlayerToucher );
PRECACHE_REGISTER( _ballplayertoucher );

CBallPlayerToucher::~CBallPlayerToucher( void )
{
}

void CBallPlayerToucher::Spawn( void )
{
	BaseClass::Spawn();

	float flScale = tf_passtime_ball_sphere_radius.GetFloat() * 1.5f;

	SetSolid( SOLID_BBOX );
	SetSolidFlags( FSOLID_TRIGGER );
	SetCollisionBounds( -Vector(flScale,flScale,flScale), Vector(flScale,flScale,flScale) );

	SetTouch( &CBallPlayerToucher::OnTouch );
}

bool CBallPlayerToucher::ShouldCollide( int collisionGroup, int contentsMask ) const
{
	if ( collisionGroup == COLLISION_GROUP_PLAYER_MOVEMENT )
		return true;

	return BaseClass::ShouldCollide( collisionGroup, contentsMask );
}

void CBallPlayerToucher::OnTouch( CBaseEntity *pOther )
{
	Assert( pOther );

	CTFPlayer *pPlayer = ToTFPlayer( pOther );
	if ( !pPlayer )
		return;

	CPasstimeBall *pBall = dynamic_cast<CPasstimeBall *>( GetRootMoveParent() );
	if ( !pBall )
		return;

	pBall->TouchPlayer( pPlayer );
}



// Network table.
IMPLEMENT_SERVERCLASS_ST( CPasstimeBall, DT_PasstimeBall )
END_SEND_TABLE()

BEGIN_DATADESC( CPasstimeBall )
	DEFINE_THINKFUNC( DefaultThink ),
END_DATADESC();

LINK_ENTITY_TO_CLASS( passtime_ball, CPasstimeBall );
PRECACHE_REGISTER( passtime_ball );

CPasstimeBall::CPasstimeBall( void )
{
	SetModelName( AllocPooledString( TF_MODEL_PASSTIME_BALL ) );
	m_iCollisionCount = 0;
}

CPasstimeBall::~CPasstimeBall( void )
{
}

void CPasstimeBall::Spawn( void )
{
	Precache();
	SetModel( STRING( GetModelName() ) );
	BaseClass::Spawn();

	CreateModelCollider();
	CreateSphereCollider();

	ResetTrail();

	SetNextThink( gpGlobals->curtime );
	SetThink( &CPasstimeBall::DefaultThink );

	lagcompensation->AddAdditionalEntity( this );

	//g_PasstimeBallController->BallSpawned( this );
}

void CPasstimeBall::Precache( void )
{
	PrecacheModel( TF_MODEL_PASSTIME_BALL );
	PrecacheModel( "passtime/passtime_balltrail_blu.vmt" );
	PrecacheModel( "passtime/passtime_balltrail_red.vmt" );
	PrecacheModel( "passtime/passtime_balltrail_unassigned.vmt" );

	PrecacheScriptSound( "Passtime.BallCatch" );
	PrecacheScriptSound( "Passtime.BallDropped" );
	PrecacheScriptSound( "Passtime.BallGet" );
	PrecacheScriptSound( "Passtime.BallHoming" );
	PrecacheScriptSound( "Passtime.BallIdle" );
	PrecacheScriptSound( "Passtime.BallIntercepted" );
	PrecacheScriptSound( "Passtime.BallSmack" );
	PrecacheScriptSound( "Passtime.BallSpawn" );
	PrecacheScriptSound( "Passtime.BallStolen" );
}

CPasstimeBall *CPasstimeBall::Create( const Vector &vecOrigin, const QAngle &vecAngles )
{
	CPasstimeBall *pBall = static_cast<CPasstimeBall*>( CBaseAnimating::CreateNoSpawn( "passtime_ball", vecOrigin, vecAngles, NULL ) );
	if ( pBall )
	{
		pBall->ChangeTeam( TEAM_UNASSIGNED );
		DispatchSpawn( pBall );
	}

	return pBall;
}

void CPasstimeBall::DefaultThink( void )
{
	CBaseEntity *pTempEnt = NULL;
	while ( ( pTempEnt = gEntList.FindEntityByClassname( pTempEnt, "func_passtime_goal" ) ) != NULL )
	{
		CFuncPasstimeGoal *pZone = dynamic_cast<CFuncPasstimeGoal *>(pTempEnt);
		if ( !pZone->IsDisabled() && pZone->IsTouching( this ) && !InSameTeam( pZone ) && !( pZone->HasSpawnFlags( SF_PASSGOAL_DONTLETBALLSCORE ) || pZone->HasSpawnFlags( SF_PASSGOAL_CARRYBALLSCORE ) ) )
		{
			//passtimelogic->score(blahblah);
			UTIL_Remove( this );
		}
	}

	if ( m_iCollisionCount > 2 && GetTeamNumber() != TEAM_UNASSIGNED )
		ChangeTeam( TEAM_UNASSIGNED );

	SetNextThink( gpGlobals->curtime + 0.1f );
}

void CPasstimeBall::CreateModelCollider( void )
{
	float flScale = tf_passtime_ball_sphere_radius.GetFloat() * GetModelScale();

	SetSolid( SOLID_BBOX );
	SetSolidFlags( FSOLID_NOT_STANDABLE );
	SetCollisionBounds( -Vector(flScale,flScale,flScale), Vector(flScale,flScale,flScale) );
	objectparams_t params = g_PhysDefaultObjectParams;
	params.pGameData = static_cast<void *>(this);
	IPhysicsObject *pPhysicsObject = physenv->CreateSphereObject( flScale, 0, GetAbsOrigin(), GetAbsAngles(), &params, false );
	if ( pPhysicsObject )
	{
		VPhysicsSetObject( pPhysicsObject );
		SetMoveType( MOVETYPE_VPHYSICS );
		pPhysicsObject->Wake();
	}
}

void CPasstimeBall::CreateSphereCollider( void )
{
	CBallPlayerToucher *pToucher = static_cast<CBallPlayerToucher*>( CBaseEntity::CreateNoSpawn( "_ballplayertoucher", GetAbsOrigin(), GetAbsAngles(), this ) );
	if ( pToucher )
	{
		DispatchSpawn( pToucher );
		pToucher->SetParent( this );
	}
}

void CPasstimeBall::ResetTrail( void )
{
	if ( m_hSpriteTrail.Get() )
		UTIL_Remove( m_hSpriteTrail );

	const char *pszTrail = "passtime/passtime_balltrail_unassigned.vmt";
	if ( GetTeamNumber() == TF_TEAM_RED )
		pszTrail = "passtime/passtime_balltrail_red.vmt";
	else if ( GetTeamNumber() == TF_TEAM_BLUE )
		pszTrail = "passtime/passtime_balltrail_blu.vmt";

	CSpriteTrail *pTrail = CSpriteTrail::SpriteTrailCreate( pszTrail, GetAbsOrigin(), true );
	if ( pTrail )
	{
		pTrail->FollowEntity( this );
		pTrail->SetTransparency( kRenderTransAlpha, -1, -1, -1, 255, kRenderFxNone );
		pTrail->SetStartWidth( 10.0f * GetModelScale() );
		pTrail->SetTextureResolution( 0.01f );
		pTrail->SetLifeTime( 1.0f );
		pTrail->TurnOn();
		m_hSpriteTrail.Set( pTrail );
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CPasstimeBall::BIgnorePlayer( CTFPlayer *pTarget )
{
	// don't allow taunt
	if ( pTarget->IsTaunting() )
		return true;

	// don't allow engithere
	if ( pTarget->m_Shared.IsCarryingObject() )
		return true;

	// don't allow jumper
	if ( pTarget->IsAllowedToPickUpFlag() )
		return true;

	// don't allow spies
	if ( pTarget->m_Shared.InCond( TF_COND_DISGUISED ) )
		return true;

	if ( pTarget->m_Shared.IsStealthed() )
		return true;

	// don't allow bonk
	if ( pTarget->m_Shared.InCond( TF_COND_PHASE ) )
		return true;

	// don't allow moar ball
	if ( pTarget->GetActiveTFWeapon() && pTarget->GetActiveTFWeapon()->IsWeapon( TF_WEAPON_PASSTIME_GUN ) )
		return true;

	// allow gravity gun
	if ( pTarget->GetHeldObject() == this )
		return true;

	return false;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CPasstimeBall::TouchPlayer( CTFPlayer *pPlayer )
{
	if ( !pPlayer )
		return;

	if ( BIgnorePlayer( pPlayer ) )
		return;

	ChangeTeam( pPlayer->GetTeamNumber() );

	//g_PasstimeBallController->BallPickedUp( this, pPlayer );

	SetStateCarried( pPlayer );

	CEconItemView econItem( 1155 );
	CTFWeaponBase *pBallWeapon = (CTFWeaponBase *)pPlayer->GiveNamedItem( econItem.GetEntityName(), 0, &econItem );
	if ( pBallWeapon )
	{
		pBallWeapon->GiveTo( pPlayer );
		pPlayer->Weapon_Switch( pBallWeapon );
		pPlayer->EmitSound( "Passtime.BallGet" );
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CPasstimeBall::MoveTo( const Vector &vecOrigin, const Vector &vecVelocity )
{
	IPhysicsObject *pPhysicsObject = VPhysicsGetObject();
	if ( pPhysicsObject )
	{
		Vector vecSpeed = vecOrigin * tf_passtime_mode_homing_speed.GetFloat();
		pPhysicsObject->SetVelocityInstantaneous( &vecSpeed, NULL );
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CPasstimeBall::ChangeTeam( int iTeamNum )
{
	SetStateFree();

	int skinForTeam = 2;
	if ( iTeamNum == TF_TEAM_RED )
		skinForTeam = 0;
	else if ( iTeamNum == TF_TEAM_BLUE )
		skinForTeam = 1;

	m_nSkin = skinForTeam;
	ResetTrail();

	BaseClass::ChangeTeam( iTeamNum );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CPasstimeBall::SetStateCarried( CTFPlayer *pCarrier )
{
	if ( !pCarrier )
		return;

	SetMoveType( MOVETYPE_NONE, MOVECOLLIDE_DEFAULT );
	SetAbsVelocity( vec3_origin );
	SetSolidFlags( FSOLID_NOT_SOLID );
	AddEffects( EF_NODRAW );

	SetParent( pCarrier, pCarrier->LookupAttachment( "effect_hand_r" ) );
}
//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CPasstimeBall::SetStateFree( void )
{
	SetParent( NULL );
	RemoveEffects( EF_NODRAW );
	CreateModelCollider();
	m_iCollisionCount = 0;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
unsigned int CPasstimeBall::PhysicsSolidMaskForEntity( void ) const
{ 
	return BaseClass::PhysicsSolidMaskForEntity() | CONTENTS_REDTEAM | CONTENTS_BLUETEAM | CONTENTS_GREENTEAM | CONTENTS_YELLOWTEAM;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CPasstimeBall::ShouldCollide( int collisionGroup, int contentsMask ) const
{
	return BaseClass::ShouldCollide( collisionGroup, contentsMask );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPasstimeBall::VPhysicsCollision( int index, gamevcollisionevent_t *pEvent )
{
	BaseClass::VPhysicsCollision( index, pEvent );

	int otherIndex = !index;
	CBaseEntity *pWorld = pEvent->pEntities[otherIndex];
	if ( !pWorld )
		return;

	if ( pWorld->ClassMatches( "func_passtime_goal" ) )
	{
		CFuncPasstimeGoal *pZone = dynamic_cast<CFuncPasstimeGoal *>( pWorld );
		if ( !pZone->IsDisabled() && !InSameTeam( pZone ) && !( pZone->HasSpawnFlags( SF_PASSGOAL_DONTLETBALLSCORE ) || pZone->HasSpawnFlags( SF_PASSGOAL_CARRYBALLSCORE ) ) )
		{
			//passtimelogic->score();
			UTIL_Remove( this );
		}
	}

	if ( !pWorld->IsWorld() )
		return;

	//g_PasstimeBallController->BallCollision( this, index, pEvent );
	OnCollision();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPasstimeBall::OnCollision( void )
{
	EmitSound( "Passtime.BallSmack" );
	m_iCollisionCount++;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CPasstimeBall::OnTakeDamage( const CTakeDamageInfo &info )
{
	//g_PasstimeBallController->BallDamaged( this );

	if ( !tf_passtime_ball_takedamage.GetBool() )
		return 0;

	return BaseClass::OnTakeDamage( info );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPasstimeBall::Deflected( CBaseEntity *pDeflectedBy, Vector &vecDir )
{
	IPhysicsObject *pPhysicsObject = VPhysicsGetObject();
	if ( pPhysicsObject )
	{
		Vector vecOldVelocity, vecVelocity;

		pPhysicsObject->GetVelocity( &vecOldVelocity, NULL );

		float flSpeed = vecOldVelocity.Length();

		vecVelocity = vecDir;
		vecVelocity *= flSpeed;
		AngularImpulse angVelocity( ( 600, random->RandomInt( -1200, 1200 ), 0 ) );

		// Now change grenade's direction.
		pPhysicsObject->SetVelocityInstantaneous( &vecVelocity, &angVelocity );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CPasstimeBall::UpdateTransmitState()
{
	return SetTransmitState( FL_EDICT_ALWAYS );
}
