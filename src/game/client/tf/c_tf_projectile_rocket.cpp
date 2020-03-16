//====== Copyright © 1996-2005, Valve Corporation, All rights reserved. =======
//
// Purpose: 
//
//=============================================================================

#include "cbase.h"
#include "c_tf_projectile_rocket.h"
#include "particles_new.h"
#include "tf_gamerules.h"
#include "tempent.h"
#include "iefx.h"
#include "dlight.h"
#include "c_te_legacytempents.h"
#include "tf_gamerules.h"

extern ConVar lfe_muzzlelight;

IMPLEMENT_NETWORKCLASS_ALIASED( TFProjectile_Rocket, DT_TFProjectile_Rocket )

BEGIN_NETWORK_TABLE( C_TFProjectile_Rocket, DT_TFProjectile_Rocket )
END_NETWORK_TABLE()

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_TFProjectile_Rocket::C_TFProjectile_Rocket( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_TFProjectile_Rocket::~C_TFProjectile_Rocket( void )
{
	ParticleProp()->StopEmission();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_TFProjectile_Rocket::OnDataChanged( DataUpdateType_t updateType )
{
	BaseClass::OnDataChanged( updateType );

	if ( updateType == DATA_UPDATE_CREATED )
	{
		CreateRocketTrails();
		CreateLightEffects();
	}

	// Watch team changes and change trail accordingly.
	if ( m_iOldTeamNum && m_iOldTeamNum != m_iTeamNum )
	{
		ParticleProp()->StopEmission();
		CreateRocketTrails();
		CreateLightEffects();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_TFProjectile_Rocket::CreateRocketTrails( void )
{
	if ( IsDormant() )
		return;

	ParticleProp()->Create( GetTrailParticleName(), PATTACH_POINT_FOLLOW, "trail" );

	if ( m_bCritical )
	{
		const char *pszEffectName = "";
		switch ( GetTeamNumber() )
		{
			case TF_TEAM_RED:
				pszEffectName = "critical_rocket_red";
				break;
			case TF_TEAM_BLUE:
				pszEffectName = "critical_rocket_blue";
				break;
			default:
				pszEffectName = "eyeboss_projectile";
				break;
		}

		ParticleProp()->Create( pszEffectName, PATTACH_ABSORIGIN_FOLLOW );
	}
}

const char *C_TFProjectile_Rocket::GetTrailParticleName( void )
{
	// bubbles
	if ( enginetrace->GetPointContents( GetAbsOrigin() ) & MASK_WATER )
		return "rockettrail_underwater";

	// yes
	if ( GetModelScale() >= 4.0f )
		return "rockettrail_doomsday";
	//else if ( GetModelScale() >= 6.0f )
	//	return "choreo_launch_rocket_jet";

	// thanos
	int iHalloweenSpell = 0;
	CALL_ATTRIB_HOOK_INT_ON_OTHER( GetOwnerEntity(), iHalloweenSpell, halloween_pumpkin_explosions );
	if ( iHalloweenSpell != 0 )
		return "halloween_rockettrail";

	// white trail
	int iMiniRockets = 0;
	CALL_ATTRIB_HOOK_INT_ON_OTHER( GetOwnerEntity(), iMiniRockets, mini_rockets );
	if ( iMiniRockets != 0 )
		return "rockettrail_airstrike_line";

	return "rockettrail";
}

void C_TFProjectile_Rocket::CreateLightEffects( void )
{
	// Handle the dynamic light
	if ( lfe_muzzlelight.GetBool() )
	{
		AddEffects( EF_DIMLIGHT );

		dlight_t *dl;
		if ( IsEffectActive( EF_DIMLIGHT ) )
		{	
			dl = effects->CL_AllocDlight( LIGHT_INDEX_TE_DYNAMIC + index );
			dl->origin = GetAbsOrigin();
			dl->color.r = 255;
			dl->color.g = 100;
			dl->color.b = 10;

			if ( IsLocalPlayerUsingVisionFilterFlags( TF_VISION_FILTER_PYRO ) )
			{
				if ( !m_bCritical ) {
				dl->color.r = 230; dl->color.g = 65; dl->color.b = 245; }
				else {
				dl->color.r = 250; dl->color.g = 180; dl->color.b = 255; }

			}
			else if ( TFGameRules()->IsTFCAllowed() )
			{
				dl->color.r = 255; dl->color.g = 255; dl->color.b = 255;
			}
			else
			{
				switch ( GetTeamNumber() )
				{
				case TF_TEAM_RED:
					if ( !m_bCritical ) {
					dl->color.r = 255; dl->color.g = 100; dl->color.b = 10; }
					else {
					dl->color.r = 255; dl->color.g = 10; dl->color.b = 10; }
				break;

				case TF_TEAM_BLUE:
					if ( !m_bCritical ) {
					dl->color.r = 255; dl->color.g = 100; dl->color.b = 10; }
					else {
					dl->color.r = 10; dl->color.g = 10; dl->color.b = 255; }
					break;
				}
			}
			dl->die = gpGlobals->curtime + 0.01f;
			dl->radius = 340.f;
			dl->decay = 512.0f;

			tempents->RocketFlare( GetAbsOrigin() );
		}
	}
}
