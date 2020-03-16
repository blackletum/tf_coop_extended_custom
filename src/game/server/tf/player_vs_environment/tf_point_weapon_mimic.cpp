//=============================================================================//
//
// Purpose: TF Projectile Launcher
//
//=============================================================================//

#include "cbase.h"
#include "particle_parse.h"
#include "te_particlesystem.h"
#include "tf_fx.h"

#include "tf_player.h"
#include "tf_projectile_rocket.h"
#include "tf_weapon_grenade_pipebomb.h"
#include "tf_projectile_arrow.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
class CTFPointWeaponMimic : public CPointEntity
{
	DECLARE_CLASS( CTFPointWeaponMimic, CPointEntity );
public:
	DECLARE_DATADESC();

	CTFPointWeaponMimic( void );
	
	virtual void		Spawn( void );
	virtual void		Precache( void );
	virtual void		UpdateOnRemove( void );

	bool				IsEnabled( void ) { return m_bEnabled; }

	void				FireRocket( void );
	void				FireGrenade( void );
	void				FireStickyGrenade( void );
	void				FireArrow( void );

	// Inputs
	void				InputDetonateStickies( inputdata_t &inputdata );
	void				InputFireMultiple( inputdata_t &inputdata );
	void				InputFireOnce( inputdata_t &inputdata );

	QAngle				GetFiringAngles( void ) const;

private:

	bool					m_bEnabled;

	int						m_nWeaponType;
	float					m_flDamage;
	float					m_flSplashRadius;
	float					m_flSpreadAngle;
	float					m_flSpeedMin;
	float					m_flSpeedMax;
	float					m_flModelScale;
	bool					m_bAlwaysCrit;
	string_t				m_iszParticleName;
	string_t				m_iszFireSound;
	string_t				m_iszModelOverride;
};

BEGIN_DATADESC( CTFPointWeaponMimic )
	// keys 
	DEFINE_KEYFIELD( m_nWeaponType, FIELD_INTEGER, "WeaponType" ),
	DEFINE_KEYFIELD( m_bAlwaysCrit, FIELD_BOOLEAN, "Crits" ),
	DEFINE_KEYFIELD( m_iszFireSound,	FIELD_SOUNDNAME,	"FireSound" ),
	DEFINE_KEYFIELD( m_iszParticleName,	FIELD_STRING,		"ParticleEffect"),
	DEFINE_KEYFIELD( m_iszModelOverride,	FIELD_STRING,	"ModelOverride"),
	DEFINE_KEYFIELD( m_flDamage,		FIELD_FLOAT,		"Damage" ),
	DEFINE_KEYFIELD( m_flSplashRadius,	FIELD_FLOAT,	"SplashRadius" ),
	DEFINE_KEYFIELD( m_flSpreadAngle,	FIELD_FLOAT,	"SpreadAngle" ),
	DEFINE_KEYFIELD( m_flModelScale,	FIELD_FLOAT,	"ModelScale" ),
	DEFINE_KEYFIELD( m_flSpeedMin,	FIELD_FLOAT,		"SpeedMin" ),
	DEFINE_KEYFIELD( m_flSpeedMax,	FIELD_FLOAT,		"SpeedMax" ),

	// Inputs
	DEFINE_INPUTFUNC( FIELD_VOID, "DetonateStickies", InputDetonateStickies ),
	DEFINE_INPUTFUNC( FIELD_VOID, "FireOnce", InputFireOnce ),
	DEFINE_INPUTFUNC( FIELD_INTEGER, "FireMultiple", InputFireMultiple ),

END_DATADESC()

LINK_ENTITY_TO_CLASS( tf_point_weapon_mimic, CTFPointWeaponMimic );

CTFPointWeaponMimic::CTFPointWeaponMimic( void )
{
	m_bEnabled = true;
	m_bAlwaysCrit = false;

	m_flDamage = 50.0f;
	m_flSplashRadius = 100.0f;
	m_flSpreadAngle = 0;
	m_flSpeedMin = 1000.0f;
	m_flSpeedMax = 1000.0f;
	m_flModelScale = 1.0f;

	m_nWeaponType = 0;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPointWeaponMimic::Spawn( void )
{
	BaseClass::Spawn();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPointWeaponMimic::Precache()
{
	BaseClass::Precache();
	PrecacheParticleSystem( STRING( m_iszParticleName ) );
	PrecacheScriptSound( STRING( m_iszFireSound ) );
	PrecacheModel( STRING( m_iszModelOverride ) );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPointWeaponMimic::UpdateOnRemove( void )
{
	BaseClass::UpdateOnRemove();
}

QAngle CTFPointWeaponMimic::GetFiringAngles( void ) const
{
	QAngle angForward = GetAbsAngles();

	angForward.x += RandomFloat( -m_flSpreadAngle, m_flSpreadAngle );
	angForward.y += RandomFloat( -m_flSpreadAngle, m_flSpreadAngle );

	return angForward;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPointWeaponMimic::InputDetonateStickies( inputdata_t &inputdata )
{
	CBaseEntity *pSticky = gEntList.FindEntityByClassname( NULL, "tf_projectile_pipe_remote" );
	if ( pSticky && ( pSticky->GetOwnerEntity() == this ) )
	{
		variant_t emptyVariant;
		pSticky->AcceptInput( "Detonate", this, this, emptyVariant, 0 );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPointWeaponMimic::InputFireOnce( inputdata_t &inputdata )
{
	CPVSFilter filter( GetAbsOrigin() );
	if ( STRING( m_iszParticleName ) )
		TE_TFParticleEffect( filter, 0.0, STRING( m_iszParticleName ), GetAbsOrigin(), GetAbsAngles(), NULL, PATTACH_CUSTOMORIGIN );

	if ( STRING( m_iszFireSound ) )
		EmitSound( STRING( m_iszFireSound ) );

	switch ( m_nWeaponType )
	{
	case 0:
		FireRocket();
		break;
	case 1:
		FireGrenade();
		break;
	case 2:
		FireStickyGrenade();
		break;
	case 3:
		FireArrow();
		break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPointWeaponMimic::InputFireMultiple( inputdata_t &inputdata )
{
	int iCount = inputdata.value.Int();

	if ( iCount <= 0 )
		return;

	int i;

	for ( i = 0; i < iCount; i++ )
	{
		CPVSFilter filter( GetAbsOrigin() );
		if ( STRING( m_iszParticleName ) )
			TE_TFParticleEffect( filter, 0.0, STRING( m_iszParticleName ), GetAbsOrigin(), GetAbsAngles(), NULL, PATTACH_CUSTOMORIGIN );

		if ( STRING( m_iszFireSound ) )
			EmitSound( STRING( m_iszFireSound ) );

		switch ( m_nWeaponType )
		{
		case 0:
			FireRocket();
			break;
		case 1:
			FireGrenade();
			break;
		case 2:
			FireStickyGrenade();
			break;
		case 3:
			FireArrow();
			break;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Fire a rocket
//-----------------------------------------------------------------------------
void CTFPointWeaponMimic::FireRocket( void )
{
	Vector vecSrc = GetAbsOrigin();
	QAngle angForward = GetFiringAngles();

	CTFProjectile_Rocket *pProjectile = CTFProjectile_Rocket::Create( this, vecSrc, angForward, this, this );
	if ( pProjectile )
	{
		pProjectile->SetCritical( m_bAlwaysCrit );
		pProjectile->SetDamage( m_flDamage );
		pProjectile->SetDamageRadius( m_flSplashRadius );
		pProjectile->SetRocketSpeed( m_flSpeedMax );
		pProjectile->SetModel( STRING( m_iszModelOverride ) );
		pProjectile->SetModelScale( m_flModelScale );
		pProjectile->SetOwnerEntity( this );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Fire a pipe bomb
//-----------------------------------------------------------------------------
void CTFPointWeaponMimic::FireGrenade( void )
{
	Vector vecSrc = GetAbsOrigin();
	QAngle angForward = GetFiringAngles();

	Vector vecForward, vecRight, vecUp;
	AngleVectors( angForward, &vecForward, &vecRight, &vecUp );

	vecSrc +=  vecForward * 16.0f + vecRight * 8.0f + vecUp * -6.0f;

	Vector vecVelocity = ( vecForward * m_flSpeedMax ) + ( vecUp * 200.0f ) + ( random->RandomFloat( -10.0f, 10.0f ) * vecRight ) +		
		( random->RandomFloat( -10.0f, 10.0f ) * vecUp );

	AngularImpulse spin( 600, 0, 0 );

	spin = AngularImpulse( 600, random->RandomInt( -1200, 1200 ), 0 );

	CTFWeaponBaseGrenadeProj *pProjectile = pProjectile = CTFGrenadePipebombProjectile::Create( vecSrc, angForward, vecVelocity, spin, NULL, 0, 1, NULL );
	if ( pProjectile )
	{
		pProjectile->SetCritical( m_bAlwaysCrit );
		pProjectile->SetDamage( m_flDamage );
		pProjectile->SetDamageRadius( m_flSplashRadius );
		pProjectile->SetModel( STRING( m_iszModelOverride ) );
		pProjectile->SetModelScale( m_flModelScale );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Fire a stickybomb
//-----------------------------------------------------------------------------
void CTFPointWeaponMimic::FireStickyGrenade( void )
{
	Vector vecSrc = GetAbsOrigin();
	QAngle angForward = GetFiringAngles();

	Vector vecForward, vecRight, vecUp;
	AngleVectors( angForward, &vecForward, &vecRight, &vecUp );

	vecSrc +=  vecForward * 16.0f + vecRight * 8.0f + vecUp * -6.0f;

	Vector vecVelocity = ( vecForward * m_flSpeedMax ) + ( vecUp * 200.0f ) + ( random->RandomFloat( -10.0f, 10.0f ) * vecRight ) +		
		( random->RandomFloat( -10.0f, 10.0f ) * vecUp );

	AngularImpulse spin( 600, 0, 0 );

	spin = AngularImpulse( 600, random->RandomInt( -1200, 1200 ), 0 );

	CTFWeaponBaseGrenadeProj *pProjectile = pProjectile = CTFGrenadePipebombProjectile::Create( vecSrc, angForward, vecVelocity, spin, NULL, 1, 1, NULL );
	if ( pProjectile )
	{
		pProjectile->SetCritical( m_bAlwaysCrit );
		pProjectile->SetDamage( m_flDamage );
		pProjectile->SetDamageRadius( m_flSplashRadius );
		pProjectile->SetModel( STRING( m_iszModelOverride ) );
		pProjectile->SetModelScale( m_flModelScale );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Fire an Arrow
//-----------------------------------------------------------------------------
void CTFPointWeaponMimic::FireArrow( void )
{
	Vector vecSrc = GetAbsOrigin();
	QAngle angForward = GetFiringAngles();

	CTFProjectile_Arrow *pProjectile = CTFProjectile_Arrow::Create( vecSrc, angForward, m_flSpeedMax, 0, TF_PROJECTILETYPE_ARROW, this, this );
	if ( pProjectile )
	{
		pProjectile->SetCritical( m_bAlwaysCrit );
		pProjectile->SetDamage( m_flDamage );
		pProjectile->SetRocketSpeed( m_flSpeedMax );
		pProjectile->SetModel( STRING( m_iszModelOverride ) );
		pProjectile->SetModelScale( m_flModelScale );
	}
}
