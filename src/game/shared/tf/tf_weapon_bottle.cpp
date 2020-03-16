//====== Copyright ? 1996-2020, Valve Corporation, All rights reserved. =======
//
// Purpose: 
//
//=============================================================================

#include "cbase.h"
#include "tf_weapon_bottle.h"
#include "decals.h"
#include "tf_viewmodel.h"

// Client specific.
#ifdef CLIENT_DLL
#include "c_tf_player.h"
#include "c_tf_viewmodeladdon.h"
// Server specific.
#else
#include "tf_player.h"
#include "tf_fx.h"
#include "takedamageinfo.h"
#include "tf_gamerules.h"
#include "soundent.h"
#endif

//=============================================================================
//
// Weapon Breakable Melee Base tables.
//
IMPLEMENT_NETWORKCLASS_ALIASED( TFBreakableMelee, DT_TFWeaponBreakableMelee )

BEGIN_NETWORK_TABLE( CTFBreakableMelee, DT_TFWeaponBreakableMelee )
#ifdef CLIENT_DLL
	RecvPropBool( RECVINFO( m_bBroken ) )
#else
	SendPropBool( SENDINFO( m_bBroken ) )
#endif
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CTFBreakableMelee )
END_PREDICTION_DATA()

#define TF_MELEE_NOTBROKEN		0
#define TF_MELEE_BROKEN		1

//=============================================================================
//
// Weapon Breakable Melee functions.
//

void CTFBreakableMelee::WeaponRegenerate()
{
	m_bBroken = false;
	SetContextThink( &CTFBreakableMelee::SwitchBodyGroups, gpGlobals->curtime + 0.01f, "SwitchBodyGroups" );
}

void CTFBreakableMelee::WeaponReset( void )
{
	m_bBroken = false;
	BaseClass::WeaponReset();
}

bool CTFBreakableMelee::DefaultDeploy( char *szViewModel, char *szWeaponModel, int iActivity, char *szAnimExt )
{
	bool bRet = BaseClass::DefaultDeploy( szViewModel, szWeaponModel, iActivity, szAnimExt );

	if ( bRet )
	{
		SwitchBodyGroups();
	}

	return bRet;
}

void CTFBreakableMelee::SwitchBodyGroups( void )
{
	int iState = TF_MELEE_NOTBROKEN;

	if ( m_bBroken )
		iState = TF_MELEE_BROKEN;

	int iBroken = FindBodygroupByName( "broken" );
	SetBodygroup( iBroken, iState );

#ifdef CLIENT_DLL
	C_ViewmodelAttachmentModel *pAttach = GetViewmodelAddon();
	if ( pAttach )
	{
		iBroken = pAttach->FindBodygroupByName( "broken" );
		pAttach->SetBodygroup( iBroken, iState );
	}
#endif
}

void CTFBreakableMelee::Smack( void )
{
	BaseClass::Smack();

	if ( !m_bBroken && ConnectedHit() && IsCurrentAttackACritical() )
	{
		m_bBroken = true;
		SwitchBodyGroups();
	}
}

//=============================================================================
//
// Weapon Bottle tables.
//
IMPLEMENT_NETWORKCLASS_ALIASED( TFBottle, DT_TFWeaponBottle )

BEGIN_NETWORK_TABLE( CTFBottle, DT_TFWeaponBottle )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CTFBottle )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( tf_weapon_bottle, CTFBottle );
PRECACHE_WEAPON_REGISTER( tf_weapon_bottle );

#define TF_BOTTLE_MODEL  		"models/weapons/c_models/c_bottle/c_bottle.mdl"
#define TF_BOTTLE_MODEL_BROKEN	"models/weapons/c_models/c_bottle/c_bottle_broken.mdl"

//=============================================================================
//
// Weapon Bottle functions.
//

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CTFBottle::CTFBottle()
{
}

const char *CTFBottle::GetWorldModel() const
{
	if ( m_bBroken && FStrEq( BaseClass::GetWorldModel(), "models/weapons/c_models/c_bottle/c_bottle.mdl" ) )
		return TF_BOTTLE_MODEL_BROKEN;

	return BaseClass::GetWorldModel();
}

void CTFBottle::Precache()
{
	BaseClass::Precache();

	PrecacheModel( TF_BOTTLE_MODEL );
	PrecacheModel( TF_BOTTLE_MODEL_BROKEN );
}

#ifdef CLIENT_DLL
int C_TFBottle::GetWorldModelIndex()
{
	if ( !FStrEq( BaseClass::GetWorldModel(), "models/weapons/c_models/c_bottle/c_bottle.mdl" ) )
		return BaseClass::GetWorldModelIndex();

	if ( !modelinfo )
		return BaseClass::GetWorldModelIndex();

	int index = modelinfo->GetModelIndex( IsBroken() ? TF_BOTTLE_MODEL_BROKEN : TF_BOTTLE_MODEL );
	m_iWorldModelIndex = index;
	return index;
}
#endif


IMPLEMENT_NETWORKCLASS_ALIASED( TFStickBomb, DT_TFWeaponStickBomb );

BEGIN_NETWORK_TABLE( CTFStickBomb, DT_TFWeaponStickBomb )
#ifdef CLIENT_DLL
	RecvPropInt( RECVINFO( m_iDetonated ) )
#else
	SendPropInt( SENDINFO( m_iDetonated ) )
#endif
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CTFStickBomb )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( tf_weapon_stickbomb, CTFStickBomb );
PRECACHE_WEAPON_REGISTER( tf_weapon_stickbomb );


#define MODEL_NORMAL   "models/workshop/weapons/c_models/c_caber/c_caber.mdl"
#define MODEL_EXPLODED "models/workshop/weapons/c_models/c_caber/c_caber_exploded.mdl"

#define TF_STICKBOMB_NORMAL    0
#define TF_STICKBOMB_DETONATED 1

//=============================================================================
//
// Weapon Stick Bomb functions.
//
CTFStickBomb::CTFStickBomb()
{
	m_iDetonated = TF_STICKBOMB_NORMAL;
}

const char *CTFStickBomb::GetWorldModel() const
{
	if ( m_iDetonated == TF_STICKBOMB_DETONATED )
		return MODEL_EXPLODED;

	return BaseClass::GetWorldModel();
}

void CTFStickBomb::Precache()
{
	BaseClass::Precache();

	PrecacheModel( MODEL_NORMAL );
	PrecacheModel( MODEL_EXPLODED );
}

void CTFStickBomb::Smack()
{
	BaseClass::Smack();
	
	if ( ( m_iDetonated == TF_STICKBOMB_NORMAL ) && ConnectedHit() ) 
	{
		m_iDetonated = TF_STICKBOMB_DETONATED;

		SwitchBodyGroups();

#ifdef GAME_DLL
		CTFPlayer *owner = GetTFPlayerOwner();
		if ( owner ) 
		{
			// TF2 does these things and doesn't use the results:
			// calls owner->EyeAngles() and then AngleVectors()
			// calls this->GetSwingRange()
			// my bet: they meant to multiply the fwd vector by the swing range
			// and then use that for the damage force, but they typo'd it and
			// just reused the shoot position instead

			Vector where = owner->Weapon_ShootPosition();

			CPVSFilter filter( where );
			TE_TFExplosion( filter, 0.0f, where, Vector( 0.0f, 0.0f, 1.0f ),
				TF_WEAPON_GRENADELAUNCHER, ENTINDEX( owner ) );

			/* why is the damage force vector set to Weapon_ShootPosition()?
			 * I dunno! */
			CTakeDamageInfo dmginfo( owner, owner, this, where, where, 75.0f,
				DMG_BLAST | DMG_CRITICAL | ( IsCurrentAttackACrit() ? DMG_USEDISTANCEMOD : 0 ),
				TF_DMG_CUSTOM_STICKBOMB_EXPLOSION, &where );

			CTFRadiusDamageInfo radius;
			radius.info       = &dmginfo;
			radius.m_vecSrc   = where;
			radius.m_flRadius = 100.0f;
			TFGameRules()->RadiusDamage( radius );
		}
#endif
	}
}

void CTFStickBomb::SwitchBodyGroups()
{
	int iBroken = FindBodygroupByName( "broken" );
	SetBodygroup( iBroken, m_iDetonated );

#ifdef CLIENT_DLL
	C_ViewmodelAttachmentModel *pAttach = GetViewmodelAddon();
	if ( pAttach )
	{
		iBroken = pAttach->FindBodygroupByName( "broken" );
		pAttach->SetBodygroup( iBroken, m_iDetonated );
	}
#endif
}

bool CTFStickBomb::IsBroken()
{
	return ( m_iDetonated == TF_STICKBOMB_DETONATED );
}

void CTFStickBomb::WeaponRegenerate()
{
	m_iDetonated = TF_STICKBOMB_NORMAL;
	SetContextThink( &CTFStickBomb::SwitchBodyGroups, gpGlobals->curtime + 0.01f, "SwitchBodyGroups" );
}

void CTFStickBomb::WeaponReset()
{
	m_iDetonated = TF_STICKBOMB_NORMAL;
	BaseClass::WeaponReset();
}

#ifdef CLIENT_DLL
int C_TFStickBomb::GetWorldModelIndex()
{
	if ( !modelinfo )
		return BaseClass::GetWorldModelIndex();
	
	int index = modelinfo->GetModelIndex( m_iDetonated == TF_STICKBOMB_DETONATED ? MODEL_EXPLODED : MODEL_NORMAL );
	m_iWorldModelIndex = index;
	return index;
}
#endif

//=============================================================================
//
// Weapon Neon Sign tables.
//
IMPLEMENT_NETWORKCLASS_ALIASED( TFBreakableSign, DT_TFWeaponBreakableSign )

BEGIN_NETWORK_TABLE( CTFBreakableSign, DT_TFWeaponBreakableSign )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CTFBreakableSign )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( tf_weapon_breakable_sign, CTFBreakableSign );
PRECACHE_WEAPON_REGISTER( tf_weapon_breakable_sign );

#define TF_SIGN_MODEL  			"models/workshop_partner/weapons/c_models/c_sd_neonsign/c_sd_neonsign.mdl"
#define TF_SIGN_MODEL_BROKEN  	"models/workshop_partner/weapons/c_models/c_sd_neonsign/c_sd_neonsign.mdl"
//#define TF_SIGN_MODEL_BROKEN	"models/workshop_partner/weapons/c_models/c_sd_neonsign/c_sd_neonsign_broken.mdl"

//=============================================================================
//
// Weapon Neon Sign functions.
//

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CTFBreakableSign::CTFBreakableSign()
{
}

const char *CTFBreakableSign::GetWorldModel() const
{
	if ( m_bBroken )
		return TF_SIGN_MODEL_BROKEN;

	return BaseClass::GetWorldModel();
}

void CTFBreakableSign::Precache()
{
	BaseClass::Precache();

	PrecacheModel( TF_SIGN_MODEL );
	PrecacheModel( TF_SIGN_MODEL_BROKEN );
}

#ifdef CLIENT_DLL
int C_TFBreakableSign::GetWorldModelIndex()
{
	if ( !modelinfo )
		return BaseClass::GetWorldModelIndex();

	int index = modelinfo->GetModelIndex( IsBroken() ? TF_SIGN_MODEL_BROKEN : TF_SIGN_MODEL );
	m_iWorldModelIndex = index;
	return index;
}
#endif