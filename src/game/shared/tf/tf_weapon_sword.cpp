//====== Copyright ? 1996-2005, Valve Corporation, All rights reserved. =======
//
// Purpose: 
//
//=============================================================================

#include "cbase.h"
#include "tf_weapon_sword.h"
#include "decals.h"

// Client specific.
#ifdef CLIENT_DLL
#include "c_tf_player.h"
#include "c_ai_basenpc.h"
// Server specific.
#else
#include "tf_player.h"
#include "ai_basenpc.h"
#endif

//=============================================================================
//
// Weapon Sword tables.
//
CREATE_SIMPLE_WEAPON_TABLE( TFSword, tf_weapon_sword )

//=============================================================================
//
// Weapon Sword functions.
//

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CTFSword::CTFSword()
{
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CTFSword::~CTFSword()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFSword::Precache()
{
	BaseClass::Precache();
	PrecacheScriptSound( "Sword.Idle" );
	PrecacheScriptSound( "Sword.Hit" );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CTFSword::Deploy( void )
{
	bool orgResult = BaseClass::Deploy();
	if ( CanDecapitate() && orgResult )
	{
#ifdef GAME_DLL
		SetupGameEventListeners();
#endif
		return true;
	}
	else
	{
		CTFPlayer *pOwner = ToTFPlayer( GetOwner() );
		if (pOwner/* && DWORD( pOwner + 1917 )*/)
		{

		}

		return orgResult;
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
int CTFSword::GetSwingRange( void ) const
{
	CTFPlayer *pOwner = ToTFPlayer( GetOwner() );
	if ( !pOwner )
		return 72;

	int iBaseRange = BaseClass::GetSwingRange();
	return pOwner->m_Shared.InCond( TF_COND_SHIELD_CHARGE ) ? iBaseRange + 96 : iBaseRange + 24;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
int CTFSword::GetSwordHealthMod( void )
{
	CTFPlayer *pOwner = ToTFPlayer( GetOwner() );
	if (!pOwner)
		return 0;

	if (CanDecapitate())
		return Min( pOwner->m_Shared.GetDecapitationCount(), 4 ) * 15;

	return 0;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
float CTFSword::GetSwordSpeedMod( void )
{
	CTFPlayer *pOwner = ToTFPlayer( GetOwner() );
	if (!pOwner)
		return 1.0f;

	if (CanDecapitate())
		return ( Min( pOwner->m_Shared.GetDecapitationCount(), 4 ) * 0.08 ) + 1.0f;

	return 1.0f;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFSword::OnDecapitation( CBaseEntity *pVictim )
{
	CTFPlayer *pOwner = ToTFPlayer( GetOwner() );
	if ( !pOwner )
		return;

	int flDecapType = 0;
	CALL_ATTRIB_HOOK_INT( flDecapType, decapitate_type );
	if ( flDecapType == 1 )
	{
		int iHeadCount = pOwner->m_Shared.GetDecapitationCount() + 1;

		CTFPlayer *pPlayer = ToTFPlayer( pVictim );

		// steal their decap.
		if ( pPlayer )
			iHeadCount += pPlayer->m_Shared.GetDecapitationCount();

		pOwner->m_Shared.SetDecapitationCount( iHeadCount );
	}

	pOwner->TeamFortress_SetSpeed();

#ifdef GAME_DLL
	if ( pOwner->m_Shared.GetMaxBuffedHealth() > pOwner->GetHealth() )
		pOwner->TakeHealth( 15.0f, DMG_IGNORE_MAXHEALTH );

	if ( !pOwner->m_Shared.InCond( TF_COND_DEMO_BUFF ) )
		pOwner->m_Shared.AddCond( TF_COND_DEMO_BUFF );

	EmitSound( "Sword.Hit" );
#endif

}

#ifdef CLIENT_DLL
//-----------------------------------------------------------------------------
// Purpose:
// ----------------------------------------------------------------------------
int CTFSword::GetCount( void )
{
	CTFPlayer *pOwner = GetTFPlayerOwner();
	if ( !pOwner )
		return 0;
	
	return pOwner->m_Shared.GetDecapitationCount();
}


//-----------------------------------------------------------------------------
// Purpose: HEADS HEADS HEADS
//-----------------------------------------------------------------------------
void CTFSword::WeaponIdle( void )
{
	BaseClass::WeaponIdle();

	CTFPlayer *pOwner = GetTFPlayerOwner();
	if ( !pOwner )
		return;

	if ( !WeaponShouldBeLowered() && HasWeaponIdleTimeElapsed() )
	{
		EmitSound( "Sword.Idle" );
	}
}
#endif

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFSword::WeaponReset( void )
{
	BaseClass::WeaponReset();
}

//=============================================================================
//
// Weapon Katana tables.
//

IMPLEMENT_NETWORKCLASS_ALIASED( TFKatana, DT_TFKatana )
BEGIN_NETWORK_TABLE( CTFKatana, DT_TFKatana )
#ifdef CLIENT_DLL
	RecvPropBool( RECVINFO( m_bIsBloody ) ),
#else
	SendPropBool( SENDINFO( m_bIsBloody ) ),
#endif
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA( CTFKatana )
	DEFINE_PRED_FIELD( m_bIsBloody, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE )
END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS( tf_weapon_katana, CTFKatana );
PRECACHE_WEAPON_REGISTER( tf_weapon_katana );

//=============================================================================
//
// Weapon Katana functions.
//

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CTFKatana::CTFKatana()
{
	m_bIsBloody = false;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CTFKatana::~CTFKatana()
{
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CTFKatana::Deploy( void )
{
	bool orgResult = BaseClass::Deploy();
	if ( m_bIsBloody )
		m_bIsBloody = false;

	if ( CanDecapitate() )
	{
#if defined( GAME_DLL )
		if( orgResult )
			SetupGameEventListeners();
#endif
	}

	return orgResult;
}

// -----------------------------------------------------------------------------
// Purpose:
// -----------------------------------------------------------------------------
bool CTFKatana::CanHolster( void ) const
{
	CTFPlayer *pOwner = GetTFPlayerOwner();
	if ( pOwner )
	{
		if ( !m_bIsBloody && ( pOwner->GetHealth() <= 50 ) )
			return false;
	}

	return BaseClass::CanHolster();
}

// -----------------------------------------------------------------------------
// Purpose:
// -----------------------------------------------------------------------------
bool CTFKatana::Holster( CBaseCombatWeapon *pSwitchingTo )
{
	CTFPlayer *pOwner = GetTFPlayerOwner();
	if ( pOwner )
	{
		if ( !m_bIsBloody && ( pOwner->GetHealth() > 50 ) )
		{
#ifdef GAME_DLL
			CTakeDamageInfo info( this, pOwner, 50, DMG_SLASH | DMG_PREVENT_PHYSICS_FORCE, TF_DMG_CUSTOM_SUICIDE );
			pOwner->TakeDamage( info );
#endif
		}
	}

	return BaseClass::Holster( pSwitchingTo );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFKatana::OnDecapitation( CBaseEntity *pVictim )
{
	CTFPlayer *pOwner = ToTFPlayer( GetOwner() );
	if ( !pOwner )
		return;

	m_bIsBloody = true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
float CTFKatana::GetMeleeDamage( CBaseEntity *pTarget, int &iDamageType, int &iCustomDamage )
{
	float res = BaseClass::GetMeleeDamage( pTarget, iDamageType, iCustomDamage );

	CBaseCombatCharacter *pCombat = ToBaseCombatCharacter( pTarget );

	// instant kill if same weapon
	if ( pCombat && pCombat->GetActiveWeapon() && pCombat->GetActiveWeapon()->IsHonorBound() )
		return pCombat->GetHealth();

	iCustomDamage = TF_DMG_CUSTOM_DECAPITATION;
	return res;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
int CTFKatana::GetSkinOverride( void ) const
{
	if ( UTIL_IsLowViolence() )
		return -1;

	CTFPlayer *pOwner = GetTFPlayerOwner();
	if ( pOwner == nullptr )
		return -1;

	// bloody
	if ( m_bIsBloody )
	{
		switch ( pOwner->GetTeamNumber() )
		{
			case TF_TEAM_RED:
				return 2;
			case TF_TEAM_BLUE:
				return 3;
		}
	}

	return -1;
}

// -----------------------------------------------------------------------------
// Purpose:
// -----------------------------------------------------------------------------
int CTFKatana::GetActivityWeaponRole( void )
{
	CTFPlayer *pOwner = GetTFPlayerOwner();
	if ( pOwner && pOwner->IsPlayerClass( TF_CLASS_DEMOMAN ) )
		return TF_WPN_TYPE_ITEM1;

	return TF_WPN_TYPE_MELEE;
}
