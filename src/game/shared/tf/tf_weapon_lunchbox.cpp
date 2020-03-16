//====== Copyright ? 1996-2005, Valve Corporation, All rights reserved. =======
//
// Purpose: 
//
//=============================================================================
#include "cbase.h"
#include "tf_weapon_lunchbox.h"
#include "tf_viewmodel.h"

#ifdef GAME_DLL
#include "tf_player.h"
#include "tf_powerup.h"
#else
#include "c_tf_player.h"
#include "c_tf_viewmodeladdon.h"
#endif

CREATE_SIMPLE_WEAPON_TABLE( TFLunchBox, tf_weapon_lunchbox )

#define TF_SANDVICH_PLATE_MODEL		"models/items/plate.mdl"
#define TF_CHOCOLATE_PLATE_MODEL	"models/workshop/weapons/c_models/c_chocolate/plate_chocolate.mdl"
#define TF_STEAK_PLATE_MODEL		"models/workshop/weapons/c_models/c_buffalo_steak/plate_buffalo_steak.mdl"
#define TF_SANDVICH_BOT_PLATE_MODEL "models/items/plate_robo_sandwich.mdl"
#define TF_SANDVICH_XMAS_PLATE_MODEL "models/items/plate_sandwich_xmas.mdl"
#define TF_AMMO_MODEL				"models/items/ammopack_medium.mdl"
#define TF_BANANA_PLATE_MODEL		"models/items/banana/plate_banana.mdl"
#define TF_FISHCAKE_PLATE_MODEL		"models/workshop/weapons/c_models/c_fishcake/plate_fishcake.mdl"

#define SANDVICH_BODYGROUP_BITE 0
#define SANDVICH_STATE_BITTEN 1
#define SANDVICH_STATE_NORMAL 0

//-----------------------------------------------------------------------------
// Purpose: Give us a fresh sandwich.
//-----------------------------------------------------------------------------
CTFLunchBox::CTFLunchBox()
{
	m_bBitten = false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFLunchBox::PrimaryAttack( void )
{
	CTFPlayer *pOwner = GetTFPlayerOwner();
	if ( !pOwner )
		return;

#ifdef GAME_DLL
	pOwner->Taunt();
#endif
	m_bBitten = true;
	SwitchBodyGroups();

	m_flNextPrimaryAttack = gpGlobals->curtime + 0.5f;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFLunchBox::SecondaryAttack( void )
{
	CTFPlayer *pOwner = GetTFPlayerOwner();
	if ( !pOwner )
		return;

	if ( !CanAttack() )
		return;

#ifdef GAME_DLL
	// Remove the previous dropped lunch box.
	if ( m_hDroppedLunch.Get() )
	{
		UTIL_Remove( m_hDroppedLunch.Get() );
		m_hDroppedLunch = NULL;
	}

	// Throw a sandvich plate down on the ground.
	Vector vecSrc, vecThrow;
	QAngle angThrow;
	vecSrc = pOwner->EyePosition();

	// A bit below the eye position.
	vecSrc.z -= 8.0f;

	const char *pszItemName = "item_healthkit_medium";

	int iLunchboxAddsMaxHealth = 0;
	CALL_ATTRIB_HOOK_INT( iLunchboxAddsMaxHealth, set_weapon_mode );
	if ( ( iLunchboxAddsMaxHealth == 1 ) || ( iLunchboxAddsMaxHealth == 6 ) || ( iLunchboxAddsMaxHealth == 7 ) ) // Chocolate, Fishcake and Banana are small health drops
		pszItemName = "item_healthkit_small";
	else if ( iLunchboxAddsMaxHealth == 5 ) // unused ammo sandvich
		pszItemName = "item_ammopack_medium";

	CTFPowerup *pPowerup = static_cast<CTFPowerup *>( CBaseEntity::Create( pszItemName, vecSrc, vec3_angle, pOwner ) );
	if ( !pPowerup )
		return;

	// Don't collide with the player owner for the first portion of its life
	pPowerup->m_flNextCollideTime = gpGlobals->curtime + 0.5f;

	int iType = 0;
	CALL_ATTRIB_HOOK_INT( iType, set_weapon_mode );
	if ( iType == 1 )
	{
		pPowerup->SetModel( TF_CHOCOLATE_PLATE_MODEL );
		pPowerup->SetModelIndexOverride( VISION_MODE_NONE, modelinfo->GetModelIndex( TF_CHOCOLATE_PLATE_MODEL ) );
	}
	else if ( iType == 2 )
	{
		pPowerup->SetModel( TF_STEAK_PLATE_MODEL );
		pPowerup->SetModelIndexOverride( VISION_MODE_NONE, modelinfo->GetModelIndex( TF_STEAK_PLATE_MODEL ) );
	}
	else if ( iType == 3 )
	{
		pPowerup->SetModel( TF_SANDVICH_BOT_PLATE_MODEL );
		pPowerup->SetModelIndexOverride( VISION_MODE_NONE, modelinfo->GetModelIndex( TF_SANDVICH_BOT_PLATE_MODEL ) );
	}
	else if ( iType == 4 )
	{
		pPowerup->SetModel( TF_SANDVICH_XMAS_PLATE_MODEL );
		pPowerup->SetModelIndexOverride( VISION_MODE_NONE, modelinfo->GetModelIndex( TF_SANDVICH_XMAS_PLATE_MODEL ) );
	}
	else if ( iType == 6 )
	{
		pPowerup->SetModel( TF_BANANA_PLATE_MODEL );
		pPowerup->SetModelIndexOverride( VISION_MODE_NONE, modelinfo->GetModelIndex( TF_BANANA_PLATE_MODEL ) );
	}
	else if ( iType == 5 )
	{
		pPowerup->SetModel( TF_AMMO_MODEL );
		pPowerup->SetModelIndexOverride( VISION_MODE_NONE, modelinfo->GetModelIndex( TF_AMMO_MODEL ) );
	}
	else if ( iType == 7 )
	{
		pPowerup->SetModel( TF_FISHCAKE_PLATE_MODEL );
		pPowerup->SetModelIndexOverride( VISION_MODE_NONE, modelinfo->GetModelIndex( TF_FISHCAKE_PLATE_MODEL ) );
	}
	else
	{
		pPowerup->SetModel( TF_SANDVICH_PLATE_MODEL );
		pPowerup->SetModelIndexOverride( VISION_MODE_NONE, modelinfo->GetModelIndex( TF_SANDVICH_PLATE_MODEL ) );
	}

	UTIL_SetSize( pPowerup, -Vector( 17, 17, 10 ), Vector( 17, 17, 10 ) );

	// Throw it down.
	angThrow = pOwner->EyeAngles();
	angThrow[PITCH] -= 10.0f;
	AngleVectors( angThrow, &vecThrow );
	vecThrow *= 500;

	pPowerup->DropSingleInstance( vecThrow, pOwner, 0.3f, 0.1f );

	m_hDroppedLunch = pPowerup;
#endif

	// Switch away from it immediately, don't want it to stick around.
	pOwner->RemoveAmmo( 1, m_iPrimaryAmmoType );
	pOwner->SwitchToNextBestWeapon( this );

	StartEffectBarRegen();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFLunchBox::DepleteAmmo( void )
{
	CTFPlayer *pOwner = GetTFPlayerOwner();
	if ( !pOwner )
		return;

	if ( pOwner->HealthFraction() >= 1.0f )
		return;

	// Switch away from it immediately, don't want it to stick around.
	pOwner->RemoveAmmo( 1, m_iPrimaryAmmoType );
	pOwner->SwitchToNextBestWeapon( this );

	StartEffectBarRegen();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
float CTFLunchBox::InternalGetEffectBarRechargeTime( void )
{
	if ( CAttributeManager::AttribHookValue<float>( 0, "item_meter_charge_rate", this ) > 0 )
		return ( CAttributeManager::AttribHookValue<float>( 0, "item_meter_charge_rate", this ) );

	// If we're using the Dalokoh, regen in 10 seconds.
	if ( ( CAttributeManager::AttribHookValue<int>( 0, "set_weapon_mode", this ) == 1 ) || ( CAttributeManager::AttribHookValue<int>( 0, "set_weapon_mode", this ) == 7 ) )
		return 10.0f;

	// Everything else is 30 seconds.
	return 30.0f;
}

//-----------------------------------------------------------------------------
// Purpose: Update the sandvich bite effects
//-----------------------------------------------------------------------------
void CTFLunchBox::SwitchBodyGroups( void )
{
#ifdef CLIENT_DLL
	int iState = m_bBitten ? SANDVICH_STATE_BITTEN : SANDVICH_STATE_NORMAL;
	C_ViewmodelAttachmentModel *pAttach = GetViewmodelAddon();
	if ( pAttach )
		pAttach->SetBodygroup( SANDVICH_BODYGROUP_BITE, iState );

	SetBodygroup( SANDVICH_BODYGROUP_BITE, iState );
#endif
}

#ifdef GAME_DLL

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFLunchBox::Precache( void )
{
	UTIL_PrecacheOther( "item_healthkit_medium" );
	UTIL_PrecacheOther( "item_ammopack_medium" );

	PrecacheModel( TF_SANDVICH_PLATE_MODEL );
	PrecacheModel( TF_CHOCOLATE_PLATE_MODEL );
	PrecacheModel( TF_STEAK_PLATE_MODEL );
	PrecacheModel( TF_SANDVICH_BOT_PLATE_MODEL );
	PrecacheModel( TF_SANDVICH_XMAS_PLATE_MODEL );
	PrecacheModel( TF_BANANA_PLATE_MODEL );
	PrecacheModel( TF_FISHCAKE_PLATE_MODEL );

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFLunchBox::ApplyBiteEffects( bool bHurt )
{
	if ( !bHurt )
		return;

	// Heal 25% of the player's max health per second for a total of 100%.
	CTFPlayer *pOwner = GetTFPlayerOwner();
	if ( !pOwner )
		return;

	int iLunchboxAddsMaxHealth = 0;
	CALL_ATTRIB_HOOK_INT( iLunchboxAddsMaxHealth, set_weapon_mode );

	if ( pOwner->GetMaxHealth() < pOwner->GetHealth() && ( iLunchboxAddsMaxHealth != 2 || iLunchboxAddsMaxHealth != 5 ) )
		return;

	int iPlayerMaxHealth = pOwner->GetMaxHealth();

	float flAmt = iPlayerMaxHealth / 4; // 25%.
	if ( ( iLunchboxAddsMaxHealth == 1 ) || ( iLunchboxAddsMaxHealth == 7 ) )
		flAmt /= 12;

	CALL_ATTRIB_HOOK_FLOAT( flAmt, lunchbox_healing_scale );

	if ( iLunchboxAddsMaxHealth == 2 )
	{
		pOwner->m_Shared.AddCond( TF_COND_CANNOT_SWITCH_FROM_MELEE, 16.0f );
		pOwner->m_Shared.AddCond( TF_COND_ENERGY_BUFF, 16.0f );
#ifdef GAME_DLL
		pOwner->AddCustomAttribute( "hidden maxhealth non buffed", 5, 16.0f );
		pOwner->AddCustomAttribute( "move speed bonus", 1.03, 16.0f );
#endif
		pOwner->Weapon_Switch( pOwner->Weapon_GetSlot( LOADOUT_POSITION_MELEE ) );
	}
	else if ( iLunchboxAddsMaxHealth == 5 )
	{
		int iAmmoAmt = pOwner->GetMaxAmmo( TF_AMMO_PRIMARY ) / 4;
		pOwner->GiveAmmo( iAmmoAmt, TF_AMMO_PRIMARY, false, TF_AMMO_SOURCE_RESUPPLY );
	}
	else
	{
		pOwner->TakeHealth( flAmt, DMG_GENERIC );
	}
}

#endif

//=============================================================================
//
// Weapon BONK! tables.
//
CREATE_SIMPLE_WEAPON_TABLE( TFLunchBox_Drink, tf_weapon_lunchbox_drink )

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFLunchBox_Drink::PrimaryAttack( void )
{
	CTFPlayer *pOwner = GetTFPlayerOwner();
	if ( !pOwner )
		return;

#ifdef GAME_DLL
	pOwner->Taunt();
#endif

	m_flNextPrimaryAttack = gpGlobals->curtime + 0.5f;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFLunchBox_Drink::SecondaryAttack( void )
{
	BaseClass::BaseClass::SecondaryAttack();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFLunchBox_Drink::DepleteAmmo( void )
{
	CTFPlayer *pOwner = GetTFPlayerOwner();
	if ( !pOwner )
		return;

	// Switch away from it immediately, don't want it to stick around.
	pOwner->RemoveAmmo( 1, m_iPrimaryAmmoType );
	pOwner->SwitchToNextBestWeapon( this );

	StartEffectBarRegen();
}

#ifdef CLIENT_DLL
//-----------------------------------------------------------------------------
// Purpose: Kill splash particles 
//-----------------------------------------------------------------------------
bool C_TFLunchBox_Drink::Holster( CBaseCombatWeapon *pSwitchingTo )
{
	C_TFPlayer *pPlayer = GetTFPlayerOwner();
	if ( pPlayer && pPlayer->IsLocalPlayer() )
	{
		C_BaseViewModel *vm = pPlayer->GetViewModel();
		if ( vm )
		{
			pPlayer->StopViewModelParticles( vm );
		}
	}

	return BaseClass::Holster( pSwitchingTo );
}

//-----------------------------------------------------------------------------
// Purpose: Override energydrink particles
//-----------------------------------------------------------------------------
const char* C_TFLunchBox_Drink::ModifyEventParticles( const char* token )
{
	if ( !V_stricmp( token, "energydrink_splash" ) )
	{
		CEconItemDefinition *pStatic = GetItem()->GetStaticData();
		if ( pStatic )
		{
			PerTeamVisuals_t *pVisuals =	pStatic->GetVisuals( GetTeamNumber() );
			if ( pVisuals )
			{
				const char *pszCustomEffectName = pVisuals->custom_particlesystem;
				if ( pszCustomEffectName[0] == '\0' )
					return "energydrink_splash";
				else
					return pszCustomEffectName;
			}
		}
	}

	return token;
}
#endif