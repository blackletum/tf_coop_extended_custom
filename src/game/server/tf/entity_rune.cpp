//====== Copyright © 1996-2013, Valve Corporation, All rights reserved. =======//
//
// Purpose: Base class for Mannpower powerups 
//
//=============================================================================//

#include "cbase.h"
#include "items.h"
#include "tf_gamerules.h"
#include "tf_shareddefs.h"
#include "tf_player.h"
#include "tf_team.h"
#include "engine/IEngineSound.h"
#include "entity_rune.h"
#include "ai_basenpc.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=============================================================================

BEGIN_DATADESC( CTFRune )
DEFINE_KEYFIELD( m_eRuneType, FIELD_INTEGER, "runetype"),
END_DATADESC()

LINK_ENTITY_TO_CLASS( item_powerup_rune, CTFRune );
LINK_ENTITY_TO_CLASS( item_powerup_rune_temp, CTFRune );

//=============================================================================

//-----------------------------------------------------------------------------
// Purpose: Constructor 
//-----------------------------------------------------------------------------
CTFRune::CTFRune()
{
	m_eRuneType = TF_RUNE_STRENGTH;
}

CTFRune *CTFRune::CreateRune( const Vector &vecOrigin, RuneTypes_t eRuneType, int iTeam, bool bSomething, bool bMorething, Vector vecSomething )
{
	//CTFRune *pPowerup = dynamic_cast<CTFRune *>( CBaseEntity::CreateNoSpawn( "item_powerup_rune", vecOrigin, vec3_angle, NULL ) );
	CTFRune *pPowerup = (CTFRune *)CreateEntityByName( "item_powerup_rune_temp" );
	if ( pPowerup )
	{
		pPowerup->SetAbsOrigin( vecOrigin );
		pPowerup->AddSpawnFlags( SF_NORESPAWN );
		pPowerup->ChangeTeam( iTeam );
		pPowerup->Spawn();
		pPowerup->m_eRuneType = eRuneType;

		if ( iTeam == TF_TEAM_RED )
			pPowerup->m_nSkin = 1;
		else if ( iTeam == TF_TEAM_BLUE )
			pPowerup->m_nSkin = 2;
		else
			pPowerup->m_nSkin = 0;

		DispatchSpawn( pPowerup );

		pPowerup->SetThink( &CTFRune::BlinkThink );
		pPowerup->SetNextThink( gpGlobals->curtime + 20.0f );
	}

	return pPowerup;
}

//-----------------------------------------------------------------------------
// Purpose: Precache 
//-----------------------------------------------------------------------------
void CTFRune::Precache( void )
{
	PrecacheModel( GetDefaultPowerupModel() );
	PrecacheModel( TF_MODEL_RUNE_STRENGTH );
	PrecacheModel( TF_MODEL_RUNE_HASTE );
	PrecacheModel( TF_MODEL_RUNE_REGEN );
	PrecacheModel( TF_MODEL_RUNE_RESIST );
	PrecacheModel( TF_MODEL_RUNE_VAMPIRE );
	PrecacheModel( TF_MODEL_RUNE_WARLOCK );
	PrecacheModel( TF_MODEL_RUNE_PRECISION );
	PrecacheModel( TF_MODEL_RUNE_AGILITY );
	PrecacheModel( TF_MODEL_RUNE_KNOCKOUT );
	PrecacheModel( TF_MODEL_RUNE_KING );
	PrecacheModel( TF_MODEL_RUNE_PLAGUE );
	PrecacheModel( TF_MODEL_RUNE_SUPERNOVA );

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: Spawn function 
//-----------------------------------------------------------------------------
void CTFRune::Spawn( void )
{
	Precache();

	switch ( m_eRuneType )
	{
	case TF_RUNE_STRENGTH:
		SetModel( TF_MODEL_RUNE_STRENGTH );
		break;
	case TF_RUNE_HASTE:
		SetModel( TF_MODEL_RUNE_HASTE );
		break;
	case TF_RUNE_REGEN:
		SetModel( TF_MODEL_RUNE_REGEN );
		break;
	case TF_RUNE_RESIST:
		SetModel( TF_MODEL_RUNE_RESIST );
		break;
	case TF_RUNE_VAMPIRE:
		SetModel( TF_MODEL_RUNE_VAMPIRE );
		break;
	case TF_RUNE_WARLOCK:
		SetModel( TF_MODEL_RUNE_WARLOCK );
		break;
	case TF_RUNE_PRECISION:
		SetModel( TF_MODEL_RUNE_PRECISION );
		break;
	case TF_RUNE_AGILITY:
		SetModel( TF_MODEL_RUNE_AGILITY );
		break;
	case TF_RUNE_KNOCKOUT:
		SetModel( TF_MODEL_RUNE_KNOCKOUT );
		break;
	case TF_RUNE_KING:
		SetModel( TF_MODEL_RUNE_KING );
		break;
	case TF_RUNE_PLAGUE:
		SetModel( TF_MODEL_RUNE_PLAGUE );
		break;
	case TF_RUNE_SUPERNOVA:
		SetModel( TF_MODEL_RUNE_SUPERNOVA );
		break;

	case TF_RUNE_NONE:
	default:
		SetModel( GetDefaultPowerupModel() );
		break;
	}

	BaseClass::Spawn();

	SetMoveType( MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_BOUNCE );
	//UTIL_DropToFloor( this, MASK_SOLID );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFRune::Materialize( void )
{
	BaseClass::Materialize();

	if ( !IsDisabled() )
	{
		EmitSound( "Item.Materialize" );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Touch function
//-----------------------------------------------------------------------------
bool CTFRune::MyTouch( CBasePlayer *pPlayer )
{
	bool bSuccess = false;

	CTFPlayer *pTFPlayer = ToTFPlayer( pPlayer );
	if ( pTFPlayer && ValidTouch( pTFPlayer ) && pTFPlayer->m_Shared.GetCarryingRuneType() == TF_RUNE_NONE )
	{
		pTFPlayer->m_Shared.SetCarryingRuneType( m_eRuneType );

		CSingleUserRecipientFilter user( pTFPlayer );
		user.MakeReliable();

		UserMessageBegin( user, "ItemPickup" );
			WRITE_STRING( GetClassname() );
		MessageEnd();

		bSuccess = true;
	}

	return bSuccess;
}

//-----------------------------------------------------------------------------
// Purpose: NPC Touch function
//-----------------------------------------------------------------------------
bool CTFRune::NPCTouch( CAI_BaseNPC *pNPC )
{
	bool bSuccess = false;

	if ( pNPC && ValidTouch( pNPC ) && pNPC->GetCarryingRuneType() == TF_RUNE_NONE )
	{
		pNPC->SetCarryingRuneType( m_eRuneType );
		bSuccess = true;
	}

	return bSuccess;
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFRune::BlinkThink( void )
{
	m_nRenderFX = kRenderFxStrobeSlow;

	// it suppose to be respawn if info_powerup_spawn is available
	// lets delete it
	SetContextThink( &CBaseEntity::SUB_Remove, gpGlobals->curtime + 10.0f, "RuneRemoveThink" ); // RepositionRune()
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFRune::RepositionRune( RuneTypes_t eRuneType, int nNumber )
{

}


//=============================================================================

BEGIN_DATADESC( CTFRuneTemp )
	DEFINE_KEYFIELD( m_flRespawnTime, FIELD_FLOAT, "RespawnTime" ),
END_DATADESC()

//LINK_ENTITY_TO_CLASS( item_powerup_rune_temp, CTFRuneTemp );

//-----------------------------------------------------------------------------
// Purpose: Constructor 
//-----------------------------------------------------------------------------
CTFRuneTemp::CTFRuneTemp()
{	
	m_flRespawnTime = 60.0f;
}

//-----------------------------------------------------------------------------
// Purpose: Spawn function 
//-----------------------------------------------------------------------------
void CTFRuneTemp::Spawn( void )
{
	Precache();
	SetModel( GetDefaultPowerupModel() );
	AddEffects( EF_ITEM_BLINK );

	BaseClass::BaseClass::Spawn();
}

//-----------------------------------------------------------------------------
// Purpose:  Override to get rid of EF_NODRAW
//-----------------------------------------------------------------------------
CBaseEntity* CTFRuneTemp::Respawn( void )
{
	CBaseEntity *pRet = BaseClass::Respawn();

	RemoveEffects( EF_NODRAW );
	RemoveEffects( EF_ITEM_BLINK );
	SetRenderColorA( 80 );

	m_flRespawnAtTime = GetNextThink();

	return pRet;
}

//-----------------------------------------------------------------------------
// Purpose:  
//-----------------------------------------------------------------------------
float CTFRuneTemp::GetRespawnDelay( void )
{
	return m_flRespawnTime;
}

//=============================================================================

BEGIN_DATADESC( CTFRuneTempCrit )
END_DATADESC()

LINK_ENTITY_TO_CLASS( item_powerup_crit, CTFRuneTempCrit );

//-----------------------------------------------------------------------------
// Purpose: Constructor 
//-----------------------------------------------------------------------------
CTFRuneTempCrit::CTFRuneTempCrit()
{	
}

//-----------------------------------------------------------------------------
// Purpose: Touch function
//-----------------------------------------------------------------------------
bool CTFRuneTempCrit::MyTouch( CBasePlayer *pPlayer )
{
	bool bSuccess = false;

	CTFPlayer *pTFPlayer = ToTFPlayer( pPlayer );
	if ( pTFPlayer && ValidTouch( pPlayer ) )
	{
		// Also add this too
		pTFPlayer->m_Shared.AddCond( TF_COND_RUNE_IMBALANCE, 30.0f );
		pTFPlayer->m_Shared.AddCond( TF_COND_CRITBOOSTED_RUNE_TEMP, 30.0f );

		CSingleUserRecipientFilter user( pTFPlayer );
		user.MakeReliable();

		UserMessageBegin( user, "ItemPickup" );
			WRITE_STRING( GetClassname() );
		MessageEnd();

		pTFPlayer->EmitSound( "Powerup.PickUpTemp.Crit" );

		bSuccess = true;
	}

	return bSuccess;
}

//=============================================================================

BEGIN_DATADESC( CTFRuneTempUber )
END_DATADESC()

LINK_ENTITY_TO_CLASS( item_powerup_uber, CTFRuneTempUber );

//-----------------------------------------------------------------------------
// Purpose: Constructor 
//-----------------------------------------------------------------------------
CTFRuneTempUber::CTFRuneTempUber()
{	
}

//-----------------------------------------------------------------------------
// Purpose: Touch function
//-----------------------------------------------------------------------------
bool CTFRuneTempUber::MyTouch( CBasePlayer *pPlayer )
{
	bool bSuccess = false;

	CTFPlayer *pTFPlayer = ToTFPlayer( pPlayer );
	if ( pTFPlayer && ValidTouch( pPlayer ) )
	{
		// Also add this too
		pTFPlayer->m_Shared.AddCond( TF_COND_RUNE_IMBALANCE, 30.0f );
		pTFPlayer->m_Shared.AddCond( TF_COND_INVULNERABLE_USER_BUFF, 30.0f );

		CSingleUserRecipientFilter user( pTFPlayer );
		user.MakeReliable();

		UserMessageBegin( user, "ItemPickup" );
			WRITE_STRING( GetClassname() );
		MessageEnd();

		pTFPlayer->EmitSound( "Powerup.PickUpTemp.Uber" );

		bSuccess = true;
	}

	return bSuccess;
}

//=============================================================================

BEGIN_DATADESC( CTFRuneCustom )

	DEFINE_KEYFIELD( m_iPowerupCondition, FIELD_INTEGER, "condition" ),
	DEFINE_KEYFIELD( m_flEffectDuration, FIELD_FLOAT, "EffectDuration" ),
	DEFINE_KEYFIELD( m_strPickupSound, FIELD_SOUNDNAME, "PickupSound" ),

END_DATADESC()

LINK_ENTITY_TO_CLASS( item_powerup_rune_custom, CTFRuneCustom );



//-----------------------------------------------------------------------------
// Purpose: Constructor 
//-----------------------------------------------------------------------------
CTFRuneCustom::CTFRuneCustom()
{	
	m_iPowerupCondition = TF_COND_INVULNERABLE;
	m_flEffectDuration = 15.0f;
}

//-----------------------------------------------------------------------------
// Purpose: Precache 
//-----------------------------------------------------------------------------
void CTFRuneCustom::Precache( void )
{
	PrecacheScriptSound( "Powerup.PickUpStrength" );

	UTIL_ValidateSoundName( m_strPickupSound, "Powerup.PickUpStrength" );
	if ( GetModelName() == NULL_STRING )
		SetModelName( AllocPooledString( GetDefaultPowerupModel() ) );

	PrecacheModel( STRING( GetModelName() ) );
	PrecacheScriptSound( STRING( m_strPickupSound ) );

	BaseClass::Precache();
}


//-----------------------------------------------------------------------------
// Purpose: Touch function
//-----------------------------------------------------------------------------
bool CTFRuneCustom::MyTouch( CBasePlayer *pPlayer )
{
	bool bSuccess = false;

	CTFPlayer *pTFPlayer = ToTFPlayer( pPlayer );
	if ( pTFPlayer && ValidTouch( pPlayer ) )
	{
		// Also add this too
		pTFPlayer->m_Shared.AddCond( GetCondition(), GetEffectDuration() );

		CSingleUserRecipientFilter user( pTFPlayer );
		user.MakeReliable();

		UserMessageBegin( user, "ItemPickup" );
			WRITE_STRING( GetClassname() );
		MessageEnd();

		if ( m_strPickupSound != NULL_STRING )
			pTFPlayer->EmitSound( STRING( m_strPickupSound ) );

		bSuccess = true;
	}

	return bSuccess;
}

BEGIN_DATADESC( CTFInfoPowerupSpawn )
END_DATADESC()

IMPLEMENT_AUTO_LIST( IInfoPowerupSpawnAutoList );

LINK_ENTITY_TO_CLASS( info_powerup_spawn, CTFInfoPowerupSpawn );

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
CTFInfoPowerupSpawn::CTFInfoPowerupSpawn( void )
{
}

void CTFInfoPowerupSpawn::Spawn( void )
{
	BaseClass::Spawn();
}

CTFInfoPowerupSpawn::~CTFInfoPowerupSpawn()
{
}