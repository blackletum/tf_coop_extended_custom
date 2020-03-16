//=============================================================================
//
//
//
//=============================================================================
#include "cbase.h"
#include "tf_weapon_cheatgun.h"
#include "tf_fx_shared.h"
#include "in_buttons.h"
#include "tf_gamerules.h"
#include "prop_portal_shared.h"
// Client specific.
#ifdef CLIENT_DLL
#include "c_tf_player.h"
#include "c_ai_basenpc.h"
// Server specific.
#else
#include "tf_player.h"
#include "ai_basenpc.h"
#include "soundent.h"
#include "tf_fx.h"
#include "mapentities.h"
#endif

//=============================================================================
//
// Weapon Cheat gun tables.
//

IMPLEMENT_NETWORKCLASS_ALIASED( TFCheatGun, DT_TFCheatGun )

BEGIN_NETWORK_TABLE( CTFCheatGun, DT_TFCheatGun )
#ifdef GAME_DLL
	SendPropEHandle( SENDINFO( m_hTargetEnt ) ),
	SendPropEHandle( SENDINFO( m_hTraceEnt ) ),
	SendPropInt( SENDINFO( m_iCheatMode ) ),
	SendPropInt( SENDINFO( m_iKitchenProjectile ) ),
	SendPropFloat( SENDINFO( m_flTraceDistance ) ),
	SendPropVector( SENDINFO( m_vecTraceEnt ) ),
#else
	RecvPropEHandle( RECVINFO( m_hTargetEnt ) ),
	RecvPropEHandle( RECVINFO( m_hTraceEnt ) ),
	RecvPropInt( RECVINFO( m_iCheatMode ) ),
	RecvPropInt( RECVINFO( m_iKitchenProjectile ) ),
	RecvPropFloat( RECVINFO( m_flTraceDistance ) ),
	RecvPropVector( RECVINFO( m_vecTraceEnt ) ),
#endif
END_NETWORK_TABLE()

LINK_ENTITY_TO_CLASS( tf_weapon_cheatgun, CTFCheatGun );
PRECACHE_WEAPON_REGISTER( tf_weapon_cheatgun );

//=============================================================================
//
// Weapon Cheat gun functions.
//
CTFCheatGun::CTFCheatGun()
{
	m_hTargetEnt = NULL;
	m_hTraceEnt = NULL;
	m_iCheatMode = CHEATMODE_KITCHEN;
	m_iKitchenProjectile = TF_PROJECTILE_BULLET;
#ifdef GAME_DLL
	m_vecTraceEnt = vec3_origin;
#endif
	m_flTraceDistance = 1024.0f;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFCheatGun::Precache()
{
	BaseClass::Precache();

	PrecacheScriptSound( "WeaponMedigun_Vaccinator.Toggle" );
	PrecacheScriptSound( "Airboat.FireGunRevDown" );
	PrecacheScriptSound( "Panel.SlideUp" );
	PrecacheScriptSound( "Panel.SlideDown" );
	PrecacheScriptSound( "Hud.Hint" );

	PrecacheModel( "models/props_doomsday/dd_woodfence_128.mdl" );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFCheatGun::PrimaryAttack( void )
{
	CTFPlayer *pOwner = GetTFPlayerOwner();
	if ( !pOwner || !pOwner->IsAlive() )
		return;

	if ( !CanAttack() )
		return;

	if ( m_flNextPrimaryAttack < gpGlobals->curtime )
	{
		m_iWeaponMode = TF_WEAPON_PRIMARY_MODE;
		if ( m_iCheatMode == CHEATMODE_KITCHEN )
		{
			FireProjectile( pOwner );
			EmitSound( "Airboat.FireGunRevDown" );
		}
		else if ( m_iCheatMode == CHEATMODE_MOVE )
		{
			RemoveTarget();
		}
		else if ( m_iCheatMode == CHEATMODE_SPAWN )
		{
#ifdef CLIENT_DLL
			ParticleTrace();
#endif
			EmitSound( "Airboat.FireGunRevDown" );
			CBaseEntity *pWall = (CBaseEntity *)CreateEntityByName( "prop_dynamic" );
			if ( pWall )
			{
				pWall->SetModel( "models/props_doomsday/dd_woodfence_128.mdl" );
				pWall->KeyValue( "solid", "6" );
				pWall->SetAbsOrigin( m_hTargetEnt->GetAbsOrigin() );
				pWall->SetAbsAngles( m_hTargetEnt->GetAbsAngles() );
#ifdef GAME_DLL
				DispatchSpawn( pWall );
#endif
			}
		}
		else if ( m_iCheatMode == CHEATMODE_RESIZER )
		{
			if ( m_hTraceEnt && m_hTraceEnt->GetBaseAnimating() )
			{
				m_hTraceEnt->GetBaseAnimating()->SetModelScale( m_hTraceEnt->GetBaseAnimating()->GetModelScale() + 0.2f );

#ifdef CLIENT_DLL
				ParticleTrace();
#endif
				EmitSound( "Airboat.FireGunRevDown" );
				EmitSound( "Panel.SlideUp" );
			}
		}
		else if ( m_iCheatMode == CHEATMODE_DECAL )
		{
#ifdef CLIENT_DLL
			ParticleTrace();
#else
			EmitSound( "Airboat.FireGunRevDown" );
			CBaseEntity *pDecal = (CBaseEntity *)CreateEntityByName( "infodecal" );
			if ( pDecal )
			{
				pDecal->KeyValue( "texture", "overlays/blood_splat001" );
				pDecal->SetAbsOrigin( m_vecTraceEnt );
				DispatchSpawn( pDecal );
			}
#endif
		}
		else if ( m_iCheatMode == CHEATMODE_KILL )
		{
			if ( m_hTraceEnt && !m_hTraceEnt->IsPlayer() )
				m_hTraceEnt->SetHealth( 0 );

#ifdef CLIENT_DLL
			ParticleTrace();
#endif
			EmitSound( "Airboat.FireGunRevDown" );
		}
		else if ( m_iCheatMode == CHEATMODE_ADDCOND )
		{
			if ( m_hTraceEnt )
			{
				CTFPlayer *pTracePlayer = ToTFPlayer( m_hTraceEnt );
				if ( pTracePlayer )
				{
					pTracePlayer->m_Shared.AddCond( TF_COND_INVULNERABLE_USER_BUFF );
				}

				CAI_BaseNPC *pTraceNPC = dynamic_cast<CAI_BaseNPC*>( m_hTraceEnt.Get() );
				if ( pTraceNPC )
				{
					pTraceNPC->AddCond( TF_COND_INVULNERABLE_USER_BUFF );
				}
			}

#ifdef CLIENT_DLL
			ParticleTrace();
#endif
			EmitSound( "Airboat.FireGunRevDown" );
		}

		m_flNextPrimaryAttack = gpGlobals->curtime + 0.2f;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFCheatGun::SecondaryAttack( void )
{
	if ( !CanAttack() )
		return;

	CTFPlayer *pOwner = GetTFPlayerOwner();
	if ( !pOwner || !pOwner->IsAlive() )
		return;

	if ( m_flNextSecondaryAttack <= gpGlobals->curtime )
	{
		if ( m_iCheatMode == CHEATMODE_KITCHEN )
		{
			CycleKitchenType();
		}
		else if ( m_iCheatMode == CHEATMODE_MOVE )
		{
			m_hTargetEnt = m_hTraceEnt;
			if ( m_hTargetEnt )
				EmitSound( "Hud.Hint" );
		}
		else if ( m_iCheatMode == CHEATMODE_SPAWN )
		{
#ifdef GAME_DLL
			engine->ClientCommand( pOwner->edict(), "gameui_activate\n" );
			engine->ClientCommand( pOwner->edict(), "mdlpickerdialog\n" );
#endif
		}
		else if ( m_iCheatMode == CHEATMODE_RESIZER )
		{
			if ( m_hTraceEnt && m_hTraceEnt->GetBaseAnimating() )
			{
				m_hTraceEnt->GetBaseAnimating()->SetModelScale( m_hTraceEnt->GetBaseAnimating()->GetModelScale() - 0.2f );

#ifdef CLIENT_DLL
				ParticleTrace();
#endif
				EmitSound( "Airboat.FireGunRevDown" );
				EmitSound( "Panel.SlideDown" );
			}
		}
		else if ( m_iCheatMode == CHEATMODE_DECAL )
		{
#ifdef CLIENT_DLL
			ParticleTrace();
#endif
			EmitSound( "Airboat.FireGunRevDown" );
			EmitSound( "Panel.SlideDown" );
		}
		else if ( m_iCheatMode == CHEATMODE_KILL )
		{
			if ( m_hTraceEnt && !m_hTraceEnt->IsPlayer() )
			{
#ifdef GAME_DLL
				CSingleUserRecipientFilter filter( pOwner );
				TFGameRules()->SendHudNotification( filter, m_hTraceEnt->GetClassname(), "ico_notify_flag_moving", TEAM_UNASSIGNED );
				UTIL_Remove( m_hTraceEnt );
#endif
			}

#ifdef CLIENT_DLL
			ParticleTrace();
#endif
			EmitSound( "Airboat.FireGunRevDown" );
		}
		else if ( m_iCheatMode == CHEATMODE_ADDCOND )
		{
			if ( m_hTraceEnt )
			{
				CTFPlayer *pTracePlayer = ToTFPlayer( m_hTraceEnt );
				if ( pTracePlayer )
				{
					pTracePlayer->m_Shared.RemoveCond( TF_COND_INVULNERABLE_USER_BUFF );
				}

				CAI_BaseNPC *pTraceNPC = dynamic_cast<CAI_BaseNPC*>( m_hTraceEnt.Get() );
				if ( pTraceNPC )
				{
					pTraceNPC->RemoveCond( TF_COND_INVULNERABLE_USER_BUFF );
				}
			}

#ifdef CLIENT_DLL
			ParticleTrace();
#endif
			EmitSound( "Airboat.FireGunRevDown" );
		}

		m_flNextSecondaryAttack = gpGlobals->curtime + 0.2f;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFCheatGun::TertiaryAttack( void )
{
	if ( !CanAttack() )
		return;

	CTFPlayer *pOwner = GetTFPlayerOwner();
	if ( !pOwner )
		return;

	if ( m_flNextSecondaryAttack <= gpGlobals->curtime )
	{
		if ( m_iCheatMode == CHEATMODE_KITCHEN )
		{
			pOwner->m_Shared.AddCond( TF_COND_CRITBOOSTED, 0.4f );
		}
		else
		{
			if ( m_hTraceEnt )
			{
				CopyTargetEnt( m_hTraceEnt );
				pOwner->m_afButtonPressed &= ~IN_ATTACK3;
			}
		}
		m_flNextSecondaryAttack = gpGlobals->curtime + 0.2f;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFCheatGun::CycleCheatMode( void )
{
	CTFPlayer *pOwner = GetTFPlayerOwner();
	if ( !pOwner )
		return;

	CSingleUserRecipientFilter filter( pOwner );
#ifdef CLIENT_DLL
	EmitSound( filter, entindex(), "WeaponMedigun_Vaccinator.Toggle" );
#endif

	m_iCheatMode += 1;
	if ( m_iCheatMode >= CHEATMODE_COUNT )
		m_iCheatMode = CHEATMODE_KITCHEN;

#ifdef GAME_DLL
	if ( m_iCheatMode == CHEATMODE_KITCHEN )
	{
		TFGameRules()->SendHudNotification( filter, "GUN", "obj_status_sentrygun_1", TEAM_UNASSIGNED );
	}
	else if ( m_iCheatMode == CHEATMODE_MOVE )
	{
		TFGameRules()->SendHudNotification( filter, "Move", "obj_weapon_pickup", TEAM_UNASSIGNED );
	}
	else if ( m_iCheatMode == CHEATMODE_SPAWN )
	{
		TFGameRules()->SendHudNotification( filter, "Build", "obj_status_tele_exit", TEAM_UNASSIGNED );
	}
	else if ( m_iCheatMode == CHEATMODE_RESIZER )
	{
		TFGameRules()->SendHudNotification( filter, "Resizer", "ico_build", TEAM_UNASSIGNED );
	}
	else if ( m_iCheatMode == CHEATMODE_DECAL )
	{
		TFGameRules()->SendHudNotification( filter, "Decal", "ico_build", TEAM_UNASSIGNED );
	}
	else if ( m_iCheatMode == CHEATMODE_KILL )
	{
		TFGameRules()->SendHudNotification( filter, "Kill", "ico_notify_flag_moving", TEAM_UNASSIGNED );
	}
	else if ( m_iCheatMode == CHEATMODE_ADDCOND )
	{
		TFGameRules()->SendHudNotification( filter, "Uber", "ico_notify_flag_moving", TEAM_UNASSIGNED );
	}

	if ( m_hTargetEnt && m_hTargetEnt->ClassMatches( "prop_dynamic" ) && m_hTargetEnt->GetOwnerEntity() == pOwner )
	{
		UTIL_Remove( m_hTargetEnt );
	}
#endif
	RemoveTarget();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFCheatGun::CycleKitchenType( void )
{
	CTFPlayer *pOwner = GetTFPlayerOwner();
	if ( !pOwner )
		return;

	CSingleUserRecipientFilter filter( pOwner );
#ifdef CLIENT_DLL
	EmitSound( filter, entindex(), "WeaponMedigun_Vaccinator.Toggle" );
#endif

	m_iKitchenProjectile += 1;
	if ( m_iKitchenProjectile >= TF_NUM_PROJECTILES )
		m_iKitchenProjectile = TF_PROJECTILE_BULLET;

#ifdef GAME_DLL
	const char *pszProjectile = "Bullet";

	if ( m_iKitchenProjectile == TF_PROJECTILE_BULLET )
		pszProjectile = "Bullet";
	else if ( m_iKitchenProjectile == TF_PROJECTILE_ROCKET )
		pszProjectile = "Rocket";
	else if ( m_iKitchenProjectile == TF_PROJECTILE_PIPEBOMB )
		pszProjectile = "Pipebomb";
	else if ( m_iKitchenProjectile == TF_PROJECTILE_PIPEBOMB_REMOTE )
		pszProjectile = "Stickybomb";
	else if ( m_iKitchenProjectile == TF_PROJECTILE_SYRINGE )
		pszProjectile = "Syringe";
	else if ( m_iKitchenProjectile == TF_PROJECTILE_FLARE )
		pszProjectile = "Flare";
	else if ( m_iKitchenProjectile == TF_PROJECTILE_JAR )
		pszProjectile = "Jarate";
	else if ( m_iKitchenProjectile == TF_PROJECTILE_ARROW )
		pszProjectile = "Arrow";
	else if ( m_iKitchenProjectile == TF_PROJECTILE_FLAME_ROCKET )
		pszProjectile = "Flame Rocket";
	else if ( m_iKitchenProjectile == TF_PROJECTILE_JAR_MILK )
		pszProjectile = "Mad Milk";
	else if ( m_iKitchenProjectile == TF_PROJECTILE_HEALING_BOLT )
		pszProjectile = "Crossbow";
	else if ( m_iKitchenProjectile == TF_PROJECTILE_ENERGY_BALL )
		pszProjectile = "Cow Mangler";
	else if ( m_iKitchenProjectile == TF_PROJECTILE_ENERGY_RING )
		pszProjectile = "Bison";
	else if ( m_iKitchenProjectile == TF_PROJECTILE_PIPEBOMB_REMOTE_PRACTICE )
		pszProjectile = "Stickybomb Jumper";
	else if ( m_iKitchenProjectile == TF_PROJECTILE_CLEAVER )
		pszProjectile = "Cleaver";
	else if ( m_iKitchenProjectile == TF_PROJECTILE_STICKY_BALL )
		pszProjectile = "Sticky Ball";
	else if ( m_iKitchenProjectile == TF_PROJECTILE_CANNONBALL )
		pszProjectile = "Cannon Ball";
	else if ( m_iKitchenProjectile == TF_PROJECTILE_BUILDING_REPAIR_BOLT )
		pszProjectile = "Rescue Ranger";
	else if ( m_iKitchenProjectile == TF_PROJECTILE_FESTITIVE_ARROW )
		pszProjectile = "Festive Arrow";
	else if ( m_iKitchenProjectile == TF_PROJECTILE_THROWABLE )
		pszProjectile = "Throwable";
	else if ( m_iKitchenProjectile == TF_PROJECTILE_SPELLFIREBALL )
		pszProjectile = "Spell: Fireball";
	else if ( m_iKitchenProjectile == TF_PROJECTILE_FESTITIVE_URINE )
		pszProjectile = "Festive Jarate";
	else if ( m_iKitchenProjectile == TF_PROJECTILE_FESTITIVE_HEALING_BOLT )
		pszProjectile = "Festive Crossbow";
	else if ( m_iKitchenProjectile == TF_PROJECTILE_BREADMONSTER_JARATE )
		pszProjectile = "Bread Jarate";
	else if ( m_iKitchenProjectile == TF_PROJECTILE_BREADMONSTER_MADMILK )
		pszProjectile = "Bread Mad Milk";
	else if ( m_iKitchenProjectile == TF_PROJECTILE_GRAPPLINGHOOK )
		pszProjectile = "Grappling Hook";
	else if ( m_iKitchenProjectile == TF_PROJECTILE_SENTRY_ROCKET )
		pszProjectile = "Sentry Rocket";
	else if ( m_iKitchenProjectile == TF_PROJECTILE_BREAD_MONSTER )
		pszProjectile = "Bread Monster";
	else if ( m_iKitchenProjectile == TF_PROJECTILE_JAR_GAS )
		pszProjectile = "Gas Passer";
	else if ( m_iKitchenProjectile == TF_PROJECTILE_BALLOFFIRE )
		pszProjectile = "Dragon's Fury";
	else if ( m_iKitchenProjectile == LFE_HL2_PROJECTILE_FRAG )
		pszProjectile = "Frag Grenade";
	else if ( m_iKitchenProjectile == LFE_HL2_PROJECTILE_COMBINEBALL )
		pszProjectile = "Combine Ball";
	else if ( m_iKitchenProjectile == LFE_HL2_PROJECTILE_SPIT )
		pszProjectile = "Worker Acid";
	else if ( m_iKitchenProjectile == LFE_HL2_PROJECTILE_CROSSBOW_BOLT )
		pszProjectile = "HL2 Bolt";
	else if ( m_iKitchenProjectile == LFE_HL2_PROJECTILE_RPG_MISSILE )
		pszProjectile = "RPG Missile";
	else if ( m_iKitchenProjectile == LFE_HL1_PROJECTILE_RPG_ROCKET )
		pszProjectile = "RPG Rocket";
	else if ( m_iKitchenProjectile == LFE_HL1_PROJECTILE_HORNET )
		pszProjectile = "Hornet";
	else if ( m_iKitchenProjectile == LFE_HL1_PROJECTILE_CROSSBOW_BOLT )
		pszProjectile = "HL1 Bolt";
	else if ( m_iKitchenProjectile == LFE_HL1_PROJECTILE_GRENADE )
		pszProjectile = "HL1 Grenade";
	else if ( m_iKitchenProjectile == LFE_TFC_PROJECTILE_NAIL )
		pszProjectile = "TFC Nail";
	else if ( m_iKitchenProjectile == LFE_TFC_PROJECTILE_NAIL_SUPER )
		pszProjectile = "TFC Super Nail";
	else if ( m_iKitchenProjectile == LFE_TFC_PROJECTILE_NAIL_TRANQ )
		pszProjectile = "TFC Tranq Nail";
	else if ( m_iKitchenProjectile == LFE_TFC_PROJECTILE_NAIL_RAILGUN )
		pszProjectile = "TFC Railgun Nail";
	else if ( m_iKitchenProjectile == LFE_TFC_PROJECTILE_NAIL_GRENADE )
		pszProjectile = "TFC Nail Grenade";
	else if ( m_iKitchenProjectile == LFE_TFC_PROJECTILE_GRENADE )
		pszProjectile = "TFC Grenade";
	else if ( m_iKitchenProjectile == LFE_TFC_PROJECTILE_ROCKET )
		pszProjectile = "TFC Rocket";
	else if ( m_iKitchenProjectile == LFE_TFC_PROJECTILE_IC )
		pszProjectile = "TFC Incendiary Cannon";

	if ( m_iCheatMode == CHEATMODE_KITCHEN )
	{
		TFGameRules()->SendHudNotification( filter, pszProjectile, "obj_status_sentrygun_1", TEAM_UNASSIGNED );
	}
#endif

	pOwner->m_afButtonPressed &= ~IN_ATTACK2;
}

//-----------------------------------------------------------------------------
// Purpose: Accessor for damage
//-----------------------------------------------------------------------------
float CTFCheatGun::GetProjectileDamage( void )
{
	float flDamage = 0;
	if ( m_iCheatMode == CHEATMODE_KITCHEN )
	{
		flDamage = 50;
		CALL_ATTRIB_HOOK_FLOAT( flDamage, mult_dmg );
		if ( GetTFPlayerOwner() && GetTFPlayerOwner()->m_Shared.InCond( TF_COND_RUNE_STRENGTH ) )
			flDamage *= 2;
	}

	return flDamage;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
int CTFCheatGun::GetWeaponProjectileType( void ) const
{
	return m_iKitchenProjectile;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFCheatGun::RemoveTarget( void )
{
	m_hTargetEnt = NULL;
	m_hTraceEnt = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFCheatGun::Deploy( void )
{
	CTFPlayer *pOwner = GetTFPlayerOwner();
	if ( !pOwner )
		return false;

	if ( m_iCheatMode == CHEATMODE_SPAWN )
	{
/*#ifdef GAME_DLL
		engine->ClientCommand( pOwner->edict(), "gameui_activate\n" );
		engine->ClientCommand( pOwner->edict(), "mdlpickerdialog\n" );
#endif*/
	}

	return BaseClass::Deploy();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFCheatGun::UpdateOnRemove( void )
{
	BaseClass::UpdateOnRemove();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFCheatGun::ItemPostFrame( void )
{
	CTFPlayer *pOwner = GetTFPlayerOwner();
	if ( !pOwner )
		return;

	if ( !pOwner->IsAlive() )
		return;

	UpdateTargetTrace();

	if ( pOwner->m_nButtons & IN_ATTACK )
		PrimaryAttack();

	if ( pOwner->m_nButtons & IN_ATTACK2 )
		SecondaryAttack();

	if ( pOwner->m_nButtons & IN_ATTACK3 )
		TertiaryAttack();

	if ( pOwner->m_afButtonPressed & IN_RELOAD )
		CycleCheatMode();

	if ( m_iCheatMode == CHEATMODE_MOVE )
	{
		if ( pOwner->m_nButtons & IN_WEAPON1 )
		{
			m_flTraceDistance = Approach( 1024.0f, m_flTraceDistance, m_flTraceDistance * 0.1 );
#ifdef CLIENT_DLL
			if ( gpGlobals->maxClients > 1 )
			{
				gHUD.m_bSkipClear = false;
			}
#endif
		}
		if ( pOwner->m_nButtons & IN_WEAPON2 )
		{
			m_flTraceDistance = Approach( 40, m_flTraceDistance, m_flTraceDistance * 0.1 );
#ifdef CLIENT_DLL
			if ( gpGlobals->maxClients > 1 )
			{
				gHUD.m_bSkipClear = false;
			}
#endif
		}
	}

	if ( pOwner->m_nButtons & IN_USE )
	{
		if ( m_hTargetEnt )
		{
			QAngle useAngles = m_hTargetEnt->GetAbsAngles();
			useAngles.y += 10;
			m_hTargetEnt->SetAbsAngles( useAngles );
		}
	}

	WeaponIdle();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFCheatGun::Holster( CBaseCombatWeapon *pSwitchingTo )
{
	RemoveTarget();

	return BaseClass::Holster( pSwitchingTo );
}

#ifdef CLIENT_DLL
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CTFCheatGun::KeyInput( int down, ButtonCode_t keynum, const char *pszCurrentBinding )
{
	if ( m_iCheatMode == CHEATMODE_MOVE )
	{
		switch ( keynum )
		{
		case MOUSE_WHEEL_UP:
			gHUD.m_iKeyBits |= IN_WEAPON1;
			return 0;

		case MOUSE_WHEEL_DOWN:
			gHUD.m_iKeyBits |= IN_WEAPON2;
			return 0;
		}
	}

	return BaseClass::KeyInput( down, keynum, pszCurrentBinding );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFCheatGun::ParticleTrace( int iCheatMode )
{
	CTFPlayer *pOwner = GetTFPlayerOwner();
	if ( !pOwner )
		return;

	C_BaseEntity *pEffectOwner = GetWeaponForEffect();
	if ( pEffectOwner )
 	{
		m_pTracer = pEffectOwner->ParticleProp()->Create( ConstructTeamParticle( "dxhr_sniper_rail_%s", pOwner->GetTeamNumber() ), PATTACH_POINT_FOLLOW, "muzzle" );
		if ( m_pTracer )
			m_pTracer->SetControlPoint( 1, m_vecTraceEnt );

		if ( pEffectOwner )
			pEffectOwner->ParticleProp()->Create( "dxhr_sniper_fizzle", PATTACH_POINT_FOLLOW, "muzzle" );
	}

	SendWeaponAnim( ACT_VM_PRIMARYATTACK );
}
#endif

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFCheatGun::SetSpawnEnt( CBaseEntity *pTarget )
{
	CTFPlayer *pOwner = GetTFPlayerOwner();
	if ( !pOwner )
		return;

	if ( m_hTargetEnt )
	{
#ifdef GAME_DLL
		if ( m_hTargetEnt->ClassMatches( "prop_dynamic" ) && m_hTargetEnt->GetOwnerEntity() == pOwner )
		{
			UTIL_Remove( m_hTargetEnt );
		}
#endif

		m_hTargetEnt = NULL;
	}

	m_iCheatMode = CHEATMODE_MOVE;

	m_hTargetEnt = pTarget;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFCheatGun::CopyTargetEnt( CBaseEntity *pTarget )
{
	if ( pTarget->IsPlayer() )
		return;

	if ( m_hTargetEnt )
		m_hTargetEnt = NULL;

	m_iCheatMode = CHEATMODE_MOVE;
#ifdef GAME_DLL
	m_hTargetEnt = (CBaseEntity *)CreateEntityByName( pTarget->GetClassname() );
	if ( m_hTargetEnt )
	{
		for ( datamap_t *dmap = pTarget->GetDataDescMap(); dmap != NULL; dmap = dmap->baseMap )
		{
			// search through all the actions in the data description, printing out details
			for ( int i = 0; i < dmap->dataNumFields; i++ )
			{
				variant_t var;
				if ( m_hTargetEnt->ReadKeyField( dmap->dataDesc[i].externalName, &var) )
				{
					char buf[256];
					buf[0] = 0;
					switch( var.FieldType() )
					{
					case FIELD_STRING:
						Q_strncpy( buf, var.String() ,sizeof(buf));
						break;
					case FIELD_INTEGER:
						if ( var.Int() )
							Q_snprintf( buf,sizeof(buf), "%d", var.Int() );
						break;
					case FIELD_FLOAT:
						if ( var.Float() )
							Q_snprintf( buf,sizeof(buf), "%.2f", var.Float() );
						break;
					case FIELD_EHANDLE:
						{
							if ( var.Entity() )
							{
								Q_snprintf( buf,sizeof(buf), "%s", STRING(var.Entity()->GetEntityName()) );
							}
						}
						break;
					}

					if ( buf[0] )
					{
						m_hTargetEnt->KeyValue( dmap->dataDesc[i].externalName, buf );
					}
				}
			}
		}

		m_hTargetEnt->SetModel( STRING( pTarget->GetModelName() ) );

		if ( m_hTargetEnt->GetBaseAnimating() )
			m_hTargetEnt->GetBaseAnimating()->m_nSkin = pTarget->GetBaseAnimating()->m_nSkin;

		DispatchSpawn( m_hTargetEnt );
		EmitSound( "Airboat.FireGunRevDown" );

		if ( GetTFPlayerOwner() )
		{
			CSingleUserRecipientFilter filter( GetTFPlayerOwner() );
			TFGameRules()->SendHudNotification( filter, pTarget->GetClassname(), "obj_weapon_pickup", TEAM_UNASSIGNED );
		}
	}
#else
	ParticleTrace();
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Allow weapons to override mouse input to viewangles
//-----------------------------------------------------------------------------
bool CTFCheatGun::OverrideViewAngles( void )
{
	CTFPlayer *pPlayer = GetTFPlayerOwner();
	if( !pPlayer )
		return false;

	return false;
}


// -----------------------------------------------------------------------------
// Purpose:
// -----------------------------------------------------------------------------
int CTFCheatGun::GetActivityWeaponRole( void )
{
	CTFPlayer *pOwner = ToTFPlayer( GetOwner() );
	if ( pOwner )
	{
		if ( pOwner->IsPlayerClass( TF_CLASS_SCOUT ) || pOwner->IsPlayerClass( TF_CLASS_ENGINEER ) || pOwner->IsPlayerClass( TF_CLASS_HEAVYWEAPONS ) || pOwner->IsPlayerClass( TF_CLASS_SNIPER ) )
			return TF_WPN_TYPE_SECONDARY;
		else if ( pOwner->IsPlayerClass( TF_CLASS_PYRO ) )
			return TF_WPN_TYPE_ITEM1;
	}

	return TF_WPN_TYPE_PRIMARY;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFCheatGun::UpdateTargetTrace( void )
{
	CTFPlayer *pOwner = GetTFPlayerOwner();
	if ( !pOwner )
		return;

	if ( m_iCheatMode == CHEATMODE_SPAWN )
	{
		if ( m_hTargetEnt == NULL )
		{
#ifdef GAME_DLL
			m_hTargetEnt = (CBaseEntity *)CreateEntityByName( "prop_dynamic" );
			if ( m_hTargetEnt && m_hTargetEnt->GetBaseAnimating() )
			{
				m_hTargetEnt->SetModel( "models/props_doomsday/dd_woodfence_128.mdl" );
				m_hTargetEnt->KeyValue( "disableshadows", "1" );
				DispatchSpawn( m_hTargetEnt );
				m_hTargetEnt->SetOwnerEntity( pOwner );
				m_hTargetEnt->GetBaseAnimating()->MaterialOverride( "models/effects/resist_shield/resist_shield_blue" );
			}
#endif
		}
	}
	else
	{
#ifdef GAME_DLL
		if ( m_hTargetEnt && m_hTargetEnt->ClassMatches( "prop_dynamic" ) && m_hTargetEnt->GetOwnerEntity() == pOwner )
		{
			UTIL_Remove( m_hTargetEnt );
			RemoveTarget();
		}
#endif
	}

	trace_t tr;
	Vector vecStart, vecEnd, vecForward, vecRight, vecUp;
	pOwner->EyeVectors( &vecForward, &vecRight, &vecUp );

	vecStart = pOwner->EyePosition();

	if ( m_iCheatMode == CHEATMODE_MOVE )
		vecEnd = vecStart + ( vecForward * m_flTraceDistance );
	else
		vecEnd = vecStart + ( vecForward * MAX_TRACE_LENGTH );

	CTraceFilterSkipTwoEntities filter( pOwner, m_hTargetEnt, COLLISION_GROUP_NONE );

	Ray_t rayLaser;
	rayLaser.Init( vecStart, vecEnd );
	UTIL_Portal_TraceRay( rayLaser, MASK_SHOT, &filter, &tr );

	if ( tr.DidHitNonWorldEntity() && tr.m_pEnt )
	{
#ifdef GAME_DLL
		CProp_Portal *pPortal = pOwner->FInViewConeThroughPortal( tr.m_pEnt );
		if ( pPortal )
			UTIL_Portal_PointTransform( pPortal->m_hLinkedPortal->MatrixThisToLinked(), vecEnd, vecEnd );
#endif

		UTIL_Portal_TraceRay( rayLaser, MASK_SHOT, &filter, &tr );
		if ( tr.DidHitNonWorldEntity() && tr.m_pEnt && tr.m_pEnt != m_hTargetEnt )
		{
			if ( m_iCheatMode == CHEATMODE_ADDCOND )
			{
				if ( tr.m_pEnt->IsPlayer() || tr.m_pEnt->IsNPC() )
				{
#ifdef GAME_DLL
					tr.m_pEnt->DrawBBoxOverlay( 0.4f );
#endif
					m_hTraceEnt = tr.m_pEnt;
				}
			}
			else
			{
#ifdef GAME_DLL
				tr.m_pEnt->DrawBBoxOverlay( 0.4f );
#endif
				m_hTraceEnt = tr.m_pEnt;
			}
		}
	}
	else
	{
		m_hTraceEnt = NULL;
	}

	Ray_t rayLaser2; rayLaser2.Init( vecStart, tr.endpos );
	UTIL_Portal_TraceRay( rayLaser2, MASK_SHOT, &filter, &tr );

	vecEnd = tr.endpos;

#ifdef GAME_DLL
	m_vecTraceEnt = vecEnd;
#endif

	if ( m_hTargetEnt )
	{
#ifdef GAME_DLL
		if ( ( m_iCheatMode == CHEATMODE_SPAWN ) && m_hTraceEnt && m_hTraceEnt->ClassMatches( "prop_dynamic" ) && ( FStrEq( STRING( m_hTraceEnt->GetModelName() ), "models/props_doomsday/dd_woodfence_128.mdl" ) ) )
		{
			QAngle vecAngles = pOwner->GetAbsAngles();
			vecAngles[PITCH] = 0.0;
			Vector vecTraceAlign = m_hTraceEnt->GetAbsOrigin();
			vecTraceAlign.y += m_hTraceEnt->WorldAlignSize().y;
			m_hTargetEnt->Teleport( &vecTraceAlign, &vecAngles, NULL );
		}
		else
		{
			QAngle vecAngles = pOwner->GetAbsAngles();
			vecAngles[PITCH] = 0.0;
			m_hTargetEnt->Teleport( &vecEnd, &vecAngles, NULL );
		}
#endif
	}
}