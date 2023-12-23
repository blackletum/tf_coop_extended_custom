//====== Copyright © 1996-2005, Valve Corporation, All rights reserved. =======
//
// Purpose: 
//
//=============================================================================

#include "cbase.h"
#include "tf_weapon_shotgun.h"
#include "decals.h"
#include "tf_fx_shared.h"
#include "tf_gamerules.h"

// Client specific.
#if defined( CLIENT_DLL )
#include "c_tf_player.h"
// Server specific.
#else
#include "tf_player.h"
#include "tf_obj_sentrygun.h"
#include "sceneentity.h"
#endif


//=============================================================================
//
// Weapon Shotgun tables.
//

CREATE_SIMPLE_WEAPON_TABLE( TFShotgun, tf_weapon_shotgun_primary )
CREATE_SIMPLE_WEAPON_TABLE( TFShotgun_Soldier, tf_weapon_shotgun_soldier )
CREATE_SIMPLE_WEAPON_TABLE( TFShotgun_HWG, tf_weapon_shotgun_hwg )
CREATE_SIMPLE_WEAPON_TABLE( TFShotgun_Pyro, tf_weapon_shotgun_pyro )
CREATE_SIMPLE_WEAPON_TABLE( TFScatterGun, tf_weapon_scattergun )
CREATE_SIMPLE_WEAPON_TABLE( TFSodaPopper, tf_weapon_soda_popper )
CREATE_SIMPLE_WEAPON_TABLE( TFPEPBrawlerBlaster, tf_weapon_pep_brawler_blaster )
CREATE_SIMPLE_WEAPON_TABLE( TFShotgunBuildingRescue, tf_weapon_shotgun_building_rescue )
CREATE_SIMPLE_WEAPON_TABLE( TFDRGPomson, tf_weapon_drg_pomson )

//=============================================================================
//
// Weapon Shotgun functions.
//

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CTFShotgun::CTFShotgun()
{
	m_bReloadsSingly = true;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFShotgun::PrimaryAttack()
{
	if ( !CanAttack() )
		return;

	// Set the weapon mode.
	m_iWeaponMode = TF_WEAPON_PRIMARY_MODE;

	BaseClass::PrimaryAttack();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFShotgun::UpdatePunchAngles( CTFPlayer *pPlayer )
{
	// Update the player's punch angle.
	QAngle angle = pPlayer->GetPunchAngle();
	float flPunchAngle = m_pWeaponInfo->GetWeaponData( m_iWeaponMode ).m_flPunchAngle;
	angle.x -= SharedRandomInt( "ShotgunPunchAngle", ( flPunchAngle - 1 ), ( flPunchAngle + 1 ));
	pPlayer->SetPunchAngle( angle );
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
bool CTFScatterGun::HasKnockback( void )
{
	int iSGKnockback = 0;
	CALL_ATTRIB_HOOK_INT( iSGKnockback, set_scattergun_has_knockback );
	if ( iSGKnockback )
		return true;

	return false;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
bool CTFScatterGun::Reload( void )
{
	int nScatterGunNoReloadSingle = 0;
	CALL_ATTRIB_HOOK_INT( nScatterGunNoReloadSingle, set_scattergun_no_reload_single );
	if ( nScatterGunNoReloadSingle == 1 )
		m_bReloadsSingly = false;

	return BaseClass::Reload();
}

//-----------------------------------------------------------------------------
// Purpose: Reload has finished.
//-----------------------------------------------------------------------------
void CTFScatterGun::FinishReload( void )
{
	CTFPlayer *pOwner = GetTFPlayerOwner();
	if ( !pOwner )
		return;

	if ( !UsesClipsForAmmo1() )
		return;

	if ( ReloadsSingly() )
		return;

	m_iClip1 += Min( GetMaxClip1() - m_iClip1, pOwner->GetAmmoCount( m_iPrimaryAmmoType ) );

	pOwner->RemoveAmmo( GetMaxClip1(), m_iPrimaryAmmoType );

	BaseClass::FinishReload();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFScatterGun::Equip( CBaseCombatCharacter *pEquipTo )
{
	BaseClass::Equip( pEquipTo );
}

//-----------------------------------------------------------------------------
// Purpose: Fire a bullet!
//-----------------------------------------------------------------------------
void CTFScatterGun::FireBullet( CTFPlayer *pPlayer )
{
	BaseClass::FireBullet( pPlayer );
#ifdef GAME_DLL
	if ( HasKnockback() && !pPlayer->m_bForceByNature && ( pPlayer->GetGroundEntity() == NULL ) )
	{
		Vector vecDir;
		QAngle angDir = pPlayer->EyeAngles();
		AngleVectors( angDir, &vecDir );
		QAngle angPushDir = angDir;

		AngleVectors( angPushDir, &vecDir );

		float flKnockbackMult = 350;
		CALL_ATTRIB_HOOK_FLOAT( flKnockbackMult, scattergun_knockback_mult );

		pPlayer->SetGroundEntity( NULL );

		if ( !pPlayer->m_bForceByNature )
			pPlayer->SetForceByNature( true );

		pPlayer->ApplyAbsVelocityImpulse( -vecDir * flKnockbackMult );
		pPlayer->m_Shared.StunPlayer( 0.3f, 1.0f, 1.0f, TF_STUNFLAG_NOSOUNDOREFFECT | TF_STUNFLAG_LIMITMOVEMENT | TF_STUNFLAG_SLOWDOWN, NULL );
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Fire a bullet!
//-----------------------------------------------------------------------------
bool CTFScatterGun::SendWeaponAnim( int iActivity )
{
	if ( GetTFPlayerOwner() && HasKnockback() )
	{
		switch ( iActivity )
		{
			case ACT_VM_DRAW:
				iActivity = ACT_ITEM2_VM_DRAW;
				break;
			case ACT_VM_HOLSTER:
				iActivity = ACT_ITEM2_VM_HOLSTER;
				break;
			case ACT_VM_IDLE:
				iActivity = ACT_ITEM2_VM_IDLE;
				break;
			case ACT_VM_PULLBACK:
				iActivity = ACT_ITEM2_VM_PULLBACK;
				break;
			case ACT_VM_PRIMARYATTACK:
				iActivity = ACT_ITEM2_VM_PRIMARYATTACK;
				break;
			case ACT_VM_SECONDARYATTACK:
				iActivity = ACT_ITEM2_VM_SECONDARYATTACK;
				break;
			case ACT_VM_RELOAD:
				iActivity = ACT_ITEM2_VM_RELOAD;
				break;
			case ACT_VM_DRYFIRE:
				iActivity = ACT_ITEM2_VM_DRYFIRE;
				break;
			case ACT_VM_IDLE_TO_LOWERED:
				iActivity = ACT_ITEM2_VM_IDLE_TO_LOWERED;
				break;
			case ACT_VM_IDLE_LOWERED:
				iActivity = ACT_ITEM2_VM_IDLE_LOWERED;
				break;
			case ACT_VM_LOWERED_TO_IDLE:
				iActivity = ACT_ITEM2_VM_LOWERED_TO_IDLE;
				break;
			default:
				return BaseClass::SendWeaponAnim( iActivity );
		}
	}

	return BaseClass::SendWeaponAnim( iActivity );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFSodaPopper::SecondaryAttack( void )
{
	CTFPlayer *pOwner = GetTFPlayerOwner();
	if ( !pOwner )
		return;

	if ( pOwner->m_Shared.GetScoutHypeMeter() == 100.0f )
	{
		pOwner->m_Shared.AddCond( TF_COND_SODAPOPPER_HYPE );
		pOwner->EmitSound( "DisciplineDevice.PowerUp" );
	}

	BaseClass::SecondaryAttack();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFSodaPopper::ItemBusyFrame( void )
{
	BaseClass::ItemBusyFrame();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
float CTFSodaPopper::GetEffectBarProgress( void )
{
	CTFPlayer *pOwner = GetTFPlayerOwner();
	if ( pOwner )
		return pOwner->m_Shared.GetScoutHypeMeter() / 100.0f;

	return 0.0f;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
float CTFPEPBrawlerBlaster::GetEffectBarProgress( void )
{
	CTFPlayer *pOwner = GetTFPlayerOwner();
	if ( pOwner )
		return pOwner->m_Shared.GetScoutHypeMeter() / 100.0f;

	return 0.0f;
}

IMPLEMENT_NETWORKCLASS_ALIASED( TFShotgun_Revenge, DT_TFShotgun_Revenge )

BEGIN_NETWORK_TABLE( CTFShotgun_Revenge, DT_TFShotgun_Revenge )
#ifndef CLIENT_DLL
	SendPropFloat( SENDINFO( m_iRevengeCrits ), 0, SPROP_NOSCALE | SPROP_CHANGES_OFTEN ),
#else
	RecvPropFloat( RECVINFO( m_iRevengeCrits ) ),
#endif
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA( CTFShotgun_Revenge )
	DEFINE_PRED_FIELD( m_iRevengeCrits, FIELD_INTEGER, FTYPEDESC_INSENDTABLE )
END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS( tf_weapon_sentry_revenge, CTFShotgun_Revenge );
PRECACHE_WEAPON_REGISTER( tf_weapon_sentry_revenge );


CTFShotgun_Revenge::CTFShotgun_Revenge()
{
	m_bReloadsSingly = true;
	m_iRevengeCrits = 0;
}

#if defined( CLIENT_DLL )
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CTFShotgun_Revenge::GetWorldModelIndex( void )
{
	CTFPlayer *pOwner = GetTFPlayerOwner();
	if ( pOwner && pOwner->IsAlive() )
	{
		if ( pOwner->IsPlayerClass( TF_CLASS_ENGINEER ) && pOwner->m_Shared.InCond( TF_COND_TAUNTING ) )
			return modelinfo->GetModelIndex( "models/player/items/engineer/guitar.mdl" );
	}

	return BaseClass::GetWorldModelIndex();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFShotgun_Revenge::SetWeaponVisible( bool visible )
{
	// DISPLACEMENT CRASHIE
	/*if ( !visible )
	{
		CTFPlayer *pOwner = GetTFPlayerOwner();
		if ( pOwner && pOwner->IsAlive() )
		{
			if ( pOwner->IsPlayerClass( TF_CLASS_ENGINEER ) && pOwner->m_Shared.InCond( TF_COND_TAUNTING ) )
			{
				const int iModelIndex = modelinfo->GetModelIndex( "models/player/items/engineer/guitar.mdl" );

				CUtlVector<breakmodel_t> list;

				BuildGibList( list, iModelIndex, 1.0f, COLLISION_GROUP_NONE );
				if ( !list.IsEmpty() )
				{
					QAngle vecAngles = CollisionProp()->GetCollisionAngles();

					Vector vecFwd, vecRight, vecUp;
					AngleVectors( vecAngles, &vecFwd, &vecRight, &vecUp );

					Vector vecOrigin = CollisionProp()->GetCollisionOrigin();
					vecOrigin = vecOrigin + vecFwd * 70.0f + vecUp * 10.0f;

					AngularImpulse angularImpulse( RandomFloat( 0.0f, 120.0f ), RandomFloat( 0.0f, 120.0f ), 0.0 );

					breakablepropparams_t params( vecOrigin, vecAngles, Vector( 0.0f, 0.0f, 200.0f ), angularImpulse );

					CreateGibsFromList( list, iModelIndex, NULL, params, NULL, -1, false, true );
				}
			}

		}
	}*/
	BaseClass::SetWeaponVisible( visible );
}
#endif

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFShotgun_Revenge::PrimaryAttack( void )
{
	if ( !CanAttack() )
		return;

	BaseClass::PrimaryAttack();

	m_iRevengeCrits = Max( m_iRevengeCrits - 1, 0 );

	CTFPlayer *pOwner = GetTFPlayerOwner();
	if ( pOwner && pOwner->IsAlive() )
	{
		if ( m_iRevengeCrits == 0 )
			pOwner->m_Shared.RemoveCond( TF_COND_CRITBOOSTED );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CTFShotgun_Revenge::GetCount( void ) const
{
	return m_iRevengeCrits;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CTFShotgun_Revenge::GetCustomDamageType( void ) const
{
	if ( m_iRevengeCrits > 0 )
		return TF_DMG_CUSTOM_SHOTGUN_REVENGE_CRIT;

	return TF_DMG_CUSTOM_NONE;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFShotgun_Revenge::Deploy( void )
{
	CTFPlayer *pOwner = GetTFPlayerOwner();
	if ( pOwner && BaseClass::Deploy() )
	{
		if ( m_iRevengeCrits > 0 )
			pOwner->m_Shared.AddCond( TF_COND_CRITBOOSTED );

		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFShotgun_Revenge::Holster( CBaseCombatWeapon *pSwitchTo )
{
	CTFPlayer *pOwner = GetTFPlayerOwner();
	if ( pOwner && BaseClass::Holster( pSwitchTo ) )
	{
		pOwner->m_Shared.RemoveCond( TF_COND_CRITBOOSTED );

		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFShotgun_Revenge::Detach( void )
{
	m_iRevengeCrits = 0;
	BaseClass::Detach();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFShotgun_Revenge::SentryKilled( int nKills )
{
	if ( !CanGetRevengeCrits() )
		return;

	m_iRevengeCrits = Min( m_iRevengeCrits + nKills, TF_WEAPON_MAX_REVENGE );

	CTFPlayer *pOwner = GetTFPlayerOwner();
	if ( pOwner && pOwner->GetActiveWeapon() == this )
	{
		if ( m_iRevengeCrits > 0 )
		{
			if ( !pOwner->m_Shared.InCond( TF_COND_CRITBOOSTED ) )
			{
				pOwner->m_Shared.AddCond( TF_COND_CRITBOOSTED );

#ifdef GAME_DLL
				if ( pOwner->IsPlayerClass( TF_CLASS_ENGINEER ) )
					InstancedScriptedScene( pOwner, "scenes/Player/Engineer/low/3648.vcd", NULL, 0.0f, false, NULL, true );
#endif
			}
		}
		else
		{
			if ( pOwner->m_Shared.InCond( TF_COND_CRITBOOSTED ) )
				pOwner->m_Shared.RemoveCond( TF_COND_CRITBOOSTED );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFShotgun_Revenge::Precache( void )
{
	int iMdlIndex = PrecacheModel( "models/player/items/engineer/guitar.mdl" );
	PrecacheGibsForModel( iMdlIndex );

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFShotgun_Revenge::CanGetRevengeCrits( void ) const
{
	int nSentryRevenge = 0;
	CALL_ATTRIB_HOOK_INT( nSentryRevenge, sentry_killed_revenge );
	return nSentryRevenge == 1;
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
CTFShotgunBuildingRescue::CTFShotgunBuildingRescue()
{
}

CTFShotgunBuildingRescue::~CTFShotgunBuildingRescue()
{
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
CTFDRGPomson::CTFDRGPomson()
{
}

CTFDRGPomson::~CTFDRGPomson()
{
}

void CTFDRGPomson::Precache()
{
	BaseClass::Precache();
}