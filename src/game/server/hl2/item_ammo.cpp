//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The various ammo types for HL2	
//
//=============================================================================//

#include "cbase.h"
#include "player.h"
#include "gamerules.h"
#include "items.h"
#include "ammodef.h"
#include "eventlist.h"
#include "npcevent.h"
extern ConVar sv_hl1_hd;

#ifdef TF_CLASSIC
#include "entity_ammopack.h"
#include "tf_player.h"
#include "tf_powerup.h"
#include "tf_weaponbase.h"
#include "tf_weapon_invis.h"

#define TF_AMMOPACK_PICKUP_SOUND	"AmmoPack.Touch"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

bool ITEM_GiveTFAmmo( CBasePlayer *pPlayer, float flCount, bool bSuppressSound = true )
{
	bool bSuccess = false;

	CTFPlayer *pTFPlayer = ToTFPlayer(pPlayer);
	if ( !pTFPlayer )
		return false;

	for ( int i = TF_AMMO_PRIMARY; i < TF_AMMO_COUNT; i++ )
	{
		if ( ( i == TF_AMMO_GRENADES1 ) || ( i == TF_AMMO_GRENADES2 ) || ( i == TF_AMMO_GRENADES3 ) )
			continue;

		int iMaxAmmo = pTFPlayer->GetMaxAmmo( i );
		if ( pTFPlayer->GiveAmmo( ceil( iMaxAmmo * flCount ), i, true ) )
			bSuccess = true;
	}

	if ( pTFPlayer->m_Shared.AddToSpyCloakMeter( ceil( 100.0f * flCount ) ) )
		bSuccess = true;

	int iGivesCharge = 0;
	CALL_ATTRIB_HOOK_FLOAT_ON_OTHER( pTFPlayer, iGivesCharge, ammo_gives_charge );
	if ( iGivesCharge && ( pTFPlayer->m_Shared.GetShieldChargeMeter() < 100.0f ) )
	{
		pTFPlayer->m_Shared.SetShieldChargeMeter( min( ( pTFPlayer->m_Shared.GetShieldChargeMeter() + ( flCount * 100 ) ), 100.0f ) );
		bSuccess = true;
	}

	return bSuccess;
}

class CLFItem : public CItem
{
public:
	DECLARE_CLASS( CLFItem, CItem);

	virtual powerupsize_t GetPowerupSize( void ) { return POWERUP_FULL; }
	virtual const char *GetPowerupModel( void ) { return "models/items/boxsrounds.mdl"; }

	void Spawn( void )
	{
		Precache();
		SetModel( GetPowerupModel() );
		if (sv_hl1_hd.GetBool())
		{
			if (!Q_strcmp(GetClassname(), "ammo_egonclip"))
			{
				SetModel("models/w_chainammo_hd.mdl");
			}
			else if (!Q_strcmp(GetClassname(), "ammo_gaussclip"))
			{
				SetModel("models/w_gaussammo_hd.mdl");
			}
			else if (!Q_strcmp(GetClassname(), "ammo_9mmar"))
			{
				SetModel("models/w_9mmARclip_hd.mdl");
			}
			else if (!Q_strcmp(GetClassname(), "ammo_9mmbox"))
			{
				SetModel("models/w_chainammo_hd.mdl");
			}
			else if (!Q_strcmp(GetClassname(), "ammo_argrenades"))
			{
				SetModel("models/w_ARgrenade_hd.mdl");
			}
			else if (!Q_strcmp(GetClassname(), "ammo_357"))
			{
				SetModel("models/w_357ammobox_hd.mdl");
			}
			else if (!Q_strcmp(GetClassname(), "ammo_rpgclip"))
			{
				SetModel("models/w_rpgammo_hd.mdl");
			}
			else if (!Q_strcmp(GetClassname(), "ammo_buckshot"))
			{
				SetModel("models/w_shotbox_hd.mdl");
			}
			
		}

		BaseClass::Spawn();
	}

	void Precache( void )
	{
		PrecacheModel( GetPowerupModel() );
		if (!Q_strcmp(GetClassname(), "ammo_egonclip"))
		{
			PrecacheModel("models/w_chainammo_hd.mdl");
		}
		else if (!Q_strcmp(GetClassname(), "ammo_gaussclip"))
		{
			PrecacheModel("models/w_gaussammo_hd.mdl");
		}
		else if (!Q_strcmp(GetClassname(), "ammo_9mmclip"))
		{
			PrecacheModel("models/w_9mmclip_hd.mdl");
		}
		else if (!Q_strcmp(GetClassname(), "ammo_9mmar"))
		{
			PrecacheModel("models/w_9mmARclip_hd.mdl");
		}
		else if (!Q_strcmp(GetClassname(), "ammo_9mmbox"))
		{
			PrecacheModel("models/w_chainammo_hd.mdl");
		}
		else if (!Q_strcmp(GetClassname(), "ammo_argrenades"))
		{
			PrecacheModel("models/w_ARgrenade_hd.mdl");
		}
		else if (!Q_strcmp(GetClassname(), "ammo_357"))
		{
			PrecacheModel("models/w_357ammobox_hd.mdl");
		}
		else if (!Q_strcmp(GetClassname(), "ammo_rpgclip"))
		{
			PrecacheModel("models/w_rpgammo_hd.mdl");
		}
		else if (!Q_strcmp(GetClassname(), "ammo_buckshot"))
		{
			PrecacheModel("models/w_shotbox_hd.mdl");
		}
		PrecacheScriptSound( TF_AMMOPACK_PICKUP_SOUND );

		BaseClass::Precache();
	}

	bool MyTouch( CBasePlayer *pPlayer )
	{
		if ( ITEM_GiveTFAmmo( pPlayer, PackRatios[GetPowerupSize()]) )
		{
			CSingleUserRecipientFilter filter( pPlayer );
			EmitSound( filter, entindex(), TF_AMMOPACK_PICKUP_SOUND );
			return true;
		}

		return false;
	}
};

#define LF_ITEM_CLASS( entityName, className, powerupSize, powerupModel )	\
	class className : public CLFItem										\
	{																		\
	public:																	\
		DECLARE_CLASS( className, CLFItem );								\
		powerupsize_t GetPowerupSize( void ) { return powerupSize; }		\
		const char *GetPowerupModel( void ) { return powerupModel; }		\
	};																		\
	LINK_ENTITY_TO_CLASS( entityName, className );	


LF_ITEM_CLASS( item_ammo_pistol, CItem_BoxSRounds, POWERUP_SMALL, "models/items/boxsrounds.mdl" );
LINK_ENTITY_TO_CLASS( item_box_srounds, CItem_BoxSRounds );

LF_ITEM_CLASS( item_ammo_pistol_large, CItem_LargeBoxSRounds, POWERUP_MEDIUM, "models/items/boxsrounds.mdl" );
LINK_ENTITY_TO_CLASS( item_large_box_srounds, CItem_LargeBoxSRounds );

LF_ITEM_CLASS( item_ammo_smg1, CItem_BoxMRounds, POWERUP_SMALL, "models/items/boxmrounds.mdl" );
LINK_ENTITY_TO_CLASS( item_box_mrounds, CItem_BoxMRounds );

LF_ITEM_CLASS( item_ammo_smg1_large, CItem_LargeBoxMRounds, POWERUP_MEDIUM, "models/items/boxmrounds.mdl" );
LINK_ENTITY_TO_CLASS( item_large_box_mrounds, CItem_LargeBoxMRounds );

LF_ITEM_CLASS( item_ammo_ar2, CItem_BoxLRounds, POWERUP_MEDIUM, "models/items/combine_rifle_cartridge01.mdl" );
LINK_ENTITY_TO_CLASS( item_box_lrounds, CItem_BoxLRounds );

LF_ITEM_CLASS( item_ammo_ar2_large, CItem_LargeBoxLRounds, POWERUP_FULL, "models/items/combine_rifle_cartridge01.mdl" );
LINK_ENTITY_TO_CLASS( item_large_box_lrounds, CItem_LargeBoxLRounds );

LF_ITEM_CLASS( item_ammo_357, CItem_Box357Rounds, POWERUP_MEDIUM, "models/items/357ammo.mdl" );

LF_ITEM_CLASS( item_ammo_357_large, CItem_LargeBox357Rounds, POWERUP_MEDIUM, "models/items/357ammobox.mdl" );

LF_ITEM_CLASS( item_ammo_crossbow, CItem_BoxXbowRounds, POWERUP_MEDIUM, "models/items/crossbowrounds.mdl" );

LF_ITEM_CLASS( item_rpg_round, CItem_RPG_Round, POWERUP_SMALL, "models/weapons/w_missile_closed.mdl" );
LINK_ENTITY_TO_CLASS( item_ml_grenade, CItem_RPG_Round );

LF_ITEM_CLASS( item_ammo_smg1_grenade, CItem_AR2_Grenade, POWERUP_SMALL, "models/items/ar2_grenade.mdl" );
LINK_ENTITY_TO_CLASS( item_ar2_grenade, CItem_AR2_Grenade );

LF_ITEM_CLASS( item_box_buckshot, CItem_BoxBuckshot, POWERUP_MEDIUM, "models/items/boxbuckshot.mdl" );

LF_ITEM_CLASS( item_ammo_ar2_altfire, CItem_AR2AltFireRound, POWERUP_SMALL, "models/items/combine_rifle_ammo01.mdl" );

LF_ITEM_CLASS( ammo_crossbow, CCrossbowAmmo, POWERUP_MEDIUM, "models/w_crossbow_clip.mdl" );

// HL1 Ammo

LF_ITEM_CLASS( ammo_egonclip, CEgonAmmo, POWERUP_MEDIUM, "models/w_chainammo.mdl" );

LF_ITEM_CLASS( ammo_gaussclip, CGaussAmmo, POWERUP_MEDIUM, "models/w_gaussammo.mdl" );

LF_ITEM_CLASS( ammo_glockclip, CGlockAmmo, POWERUP_SMALL, "models/w_9mmclip.mdl" );

LINK_ENTITY_TO_CLASS( ammo_9mmclip, CGlockAmmo )

LF_ITEM_CLASS( ammo_mp5clip, CMP5AmmoClip, POWERUP_MEDIUM, "models/w_9mmARclip.mdl" );

LINK_ENTITY_TO_CLASS( ammo_9mmar, CMP5AmmoClip )

LF_ITEM_CLASS( ammo_9mmbox, CMP5Chainammo, POWERUP_MEDIUM, "models/w_chainammo.mdl");

LF_ITEM_CLASS( ammo_mp5grenades, CMP5AmmoGrenade, POWERUP_MEDIUM, "models/w_ARgrenade.mdl");

LINK_ENTITY_TO_CLASS( ammo_argrenades, CMP5AmmoGrenade)

LF_ITEM_CLASS( ammo_357, CPythonAmmo, POWERUP_MEDIUM, "models/w_357ammobox.mdl" );

LF_ITEM_CLASS( ammo_rpgclip, CRpgAmmo, POWERUP_SMALL, "models/w_rpgammo.mdl" );

LF_ITEM_CLASS( ammo_buckshot, CShotgunAmmo, POWERUP_MEDIUM, "models/w_shotbox.mdl" );

// ==================================================================
// Ammo crate which will supply infinite ammo of the specified type
// ==================================================================

// Ammo types
enum
{
	AMMOCRATE_SMALL_ROUNDS,
	AMMOCRATE_MEDIUM_ROUNDS,
	AMMOCRATE_LARGE_ROUNDS,
	AMMOCRATE_RPG_ROUNDS,
	AMMOCRATE_BUCKSHOT,
	AMMOCRATE_GRENADES,
	AMMOCRATE_357,
	AMMOCRATE_CROSSBOW,
	AMMOCRATE_AR2_ALTFIRE,
	AMMOCRATE_SMG_ALTFIRE,
	NUM_AMMO_CRATE_TYPES,
};

// Ammo crate

class CItem_AmmoCrate : public CBaseAnimating
{
public:
	DECLARE_CLASS( CItem_AmmoCrate, CBaseAnimating );

	void	Spawn( void );
	void	Precache( void );
	bool	CreateVPhysics( void );

	virtual void HandleAnimEvent( animevent_t *pEvent );

	void	SetupCrate( void );
	void	OnRestore( void );

	//FIXME: May not want to have this used in a radius
	int		ObjectCaps( void ) { return (BaseClass::ObjectCaps() | (FCAP_IMPULSE_USE|FCAP_USE_IN_RADIUS)); };
	void	Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

	void	InputKill( inputdata_t &data );
	void	CrateThink( void );
	
	virtual int OnTakeDamage( const CTakeDamageInfo &info );

protected:

	int		m_nAmmoType;
	int		m_nAmmoIndex;

	static const char *m_lpzModelNames[NUM_AMMO_CRATE_TYPES];
	static const char *m_lpzAmmoNames[NUM_AMMO_CRATE_TYPES];
	static int m_nAmmoAmounts[NUM_AMMO_CRATE_TYPES];
	static const char *m_pGiveWeapon[NUM_AMMO_CRATE_TYPES];

	float	m_flCloseTime;
	COutputEvent	m_OnUsed;
	CHandle< CBasePlayer > m_hActivator;

	DECLARE_DATADESC();
};

LINK_ENTITY_TO_CLASS( item_ammo_crate, CItem_AmmoCrate );

BEGIN_DATADESC( CItem_AmmoCrate )

	DEFINE_KEYFIELD( m_nAmmoType,	FIELD_INTEGER, "AmmoType" ),	

	DEFINE_FIELD( m_flCloseTime, FIELD_FLOAT ),
	DEFINE_FIELD( m_hActivator, FIELD_EHANDLE ),

	// These can be recreated
	//DEFINE_FIELD( m_nAmmoIndex,		FIELD_INTEGER ),
	//DEFINE_FIELD( m_lpzModelNames,	FIELD_ ),
	//DEFINE_FIELD( m_lpzAmmoNames,	FIELD_ ),
	//DEFINE_FIELD( m_nAmmoAmounts,	FIELD_INTEGER ),

	DEFINE_OUTPUT( m_OnUsed, "OnUsed" ),

	DEFINE_INPUTFUNC( FIELD_VOID, "Kill", InputKill ),

	DEFINE_THINKFUNC( CrateThink ),

END_DATADESC()

//-----------------------------------------------------------------------------
// Animation events.
//-----------------------------------------------------------------------------

// Models names
const char *CItem_AmmoCrate::m_lpzModelNames[NUM_AMMO_CRATE_TYPES] =
{
	"models/items/ammocrate_pistol.mdl",	// Small rounds
	"models/items/ammocrate_smg1.mdl",		// Medium rounds
	"models/items/ammocrate_ar2.mdl",		// Large rounds
	"models/items/ammocrate_rockets.mdl",	// RPG rounds
	"models/items/ammocrate_buckshot.mdl",	// Buckshot
	"models/items/ammocrate_grenade.mdl",	// Grenades
	"models/items/ammocrate_smg1.mdl",		// 357
	"models/items/ammocrate_smg1.mdl",	// Crossbow
	
	//FIXME: This model is incorrect!
	"models/items/ammocrate_ar2.mdl",		// Combine Ball 
	"models/items/ammocrate_smg2.mdl",	    // smg grenade
};

// Ammo type names
const char *CItem_AmmoCrate::m_lpzAmmoNames[NUM_AMMO_CRATE_TYPES] =
{
	"Pistol",		
	"SMG1",			
	"AR2",			
	"RPG_Round",	
	"Buckshot",		
	"Grenade",
	"357",
	"XBowBolt",
	"AR2AltFire",
	"SMG1_Grenade",
};

// Ammo amount given per +use
int CItem_AmmoCrate::m_nAmmoAmounts[NUM_AMMO_CRATE_TYPES] =
{
	300,	// Pistol
	300,	// SMG1
	300,	// AR2
	3,		// RPG rounds
	120,	// Buckshot
	5,		// Grenades
	50,		// 357
	50,		// Crossbow
	3,		// AR2 alt-fire
	5,
};

const char *CItem_AmmoCrate::m_pGiveWeapon[NUM_AMMO_CRATE_TYPES] =
{
	NULL,	// Pistol
	NULL,	// SMG1
	NULL,	// AR2
	NULL,		// RPG rounds
	NULL,	// Buckshot
	"weapon_frag",		// Grenades
	NULL,		// 357
	NULL,		// Crossbow
	NULL,		// AR2 alt-fire
	NULL,		// SMG alt-fire
};

#define	AMMO_CRATE_CLOSE_DELAY	1.5f

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CItem_AmmoCrate::Spawn( void )
{
	Precache();

	BaseClass::Spawn();

	SetModel( STRING( GetModelName() ) );
	SetMoveType( MOVETYPE_NONE );
	SetSolid( SOLID_VPHYSICS );
	CreateVPhysics();

	ResetSequence( LookupSequence( "Idle" ) );
	SetBodygroup( 1, true );

	m_flCloseTime = gpGlobals->curtime;
	m_flAnimTime = gpGlobals->curtime;
	m_flPlaybackRate = 0.0;
	SetCycle( 0 );

	m_takedamage = DAMAGE_EVENTS_ONLY;

}

//------------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------------
bool CItem_AmmoCrate::CreateVPhysics( void )
{
	return ( VPhysicsInitStatic() != NULL );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CItem_AmmoCrate::Precache( void )
{
	SetupCrate();
	PrecacheModel( STRING( GetModelName() ) );

	PrecacheScriptSound( "AmmoCrate.Open" );
	PrecacheScriptSound( "AmmoCrate.Close" );
#ifdef TF_CLASSIC
	PrecacheScriptSound( TF_AMMOPACK_PICKUP_SOUND );
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CItem_AmmoCrate::SetupCrate( void )
{
	SetModelName( AllocPooledString( m_lpzModelNames[m_nAmmoType] ) );
	
	m_nAmmoIndex = GetAmmoDef()->Index( m_lpzAmmoNames[m_nAmmoType] );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CItem_AmmoCrate::OnRestore( void )
{
	BaseClass::OnRestore();

	// Restore our internal state
	SetupCrate();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pActivator - 
//			*pCaller - 
//			useType - 
//			value - 
//-----------------------------------------------------------------------------
void CItem_AmmoCrate::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	CBasePlayer *pPlayer = ToBasePlayer( pActivator );

	if ( pPlayer == NULL )
		return;

	m_OnUsed.FireOutput( pActivator, this );

	int iSequence = LookupSequence( "Open" );

	// See if we're not opening already
	if ( GetSequence() != iSequence )
	{
		Vector mins, maxs;
		trace_t tr;

		CollisionProp()->WorldSpaceAABB( &mins, &maxs );

		Vector vOrigin = GetAbsOrigin();
		vOrigin.z += ( maxs.z - mins.z );
		mins = (mins - GetAbsOrigin()) * 0.2f;
		maxs = (maxs - GetAbsOrigin()) * 0.2f;
		mins.z = ( GetAbsOrigin().z - vOrigin.z );  
		
		UTIL_TraceHull( vOrigin, vOrigin, mins, maxs, MASK_SOLID, this, COLLISION_GROUP_NONE, &tr );

		if ( tr.startsolid || tr.allsolid )
			 return;
			
		m_hActivator = pPlayer;

		// Animate!
		ResetSequence( iSequence );

		// Make sound
		CPASAttenuationFilter sndFilter( this, "AmmoCrate.Open" );
		EmitSound( sndFilter, entindex(), "AmmoCrate.Open" );

		// Start thinking to make it return
		SetThink( &CItem_AmmoCrate::CrateThink );
		SetNextThink( gpGlobals->curtime + 0.1f );
	}

	// Don't close again for two seconds
	m_flCloseTime = gpGlobals->curtime + AMMO_CRATE_CLOSE_DELAY;
}

//-----------------------------------------------------------------------------
// Purpose: allows the crate to open up when hit by a crowbar
//-----------------------------------------------------------------------------
int CItem_AmmoCrate::OnTakeDamage( const CTakeDamageInfo &info )
{
	// if it's the player hitting us with a crowbar, open up
	CBasePlayer *player = ToBasePlayer(info.GetAttacker());
	if (player)
	{
		CBaseCombatWeapon *weapon = player->GetActiveWeapon();

		if (weapon && !stricmp(weapon->GetName(), "weapon_crowbar"))
		{
			// play the normal use sound
			player->EmitSound( "HL2Player.Use" );
			// open the crate
			Use(info.GetAttacker(), info.GetAttacker(), USE_TOGGLE, 0.0f);
		}
	}

	// don't actually take any damage
	return 0;
}


//-----------------------------------------------------------------------------
// Purpose: Catches the monster-specific messages that occur when tagged
//			animation frames are played.
// Input  : *pEvent - 
//-----------------------------------------------------------------------------
void CItem_AmmoCrate::HandleAnimEvent( animevent_t *pEvent )
{
	if ( pEvent->event == AE_AMMOCRATE_PICKUP_AMMO )
	{
		if ( m_hActivator )
		{
#ifndef TF_CLASSIC
			if ( m_pGiveWeapon[m_nAmmoType] && !m_hActivator->Weapon_OwnsThisType( m_pGiveWeapon[m_nAmmoType] ) )
			{
				CBaseEntity *pEntity = CreateEntityByName( m_pGiveWeapon[m_nAmmoType] );
				CBaseCombatWeapon *pWeapon = dynamic_cast<CBaseCombatWeapon*>(pEntity);
				if ( pWeapon )
				{
					pWeapon->SetAbsOrigin( m_hActivator->GetAbsOrigin() );
					pWeapon->m_iPrimaryAmmoType = 0;
					pWeapon->m_iSecondaryAmmoType = 0;
					pWeapon->Spawn();
					if ( !m_hActivator->BumpWeapon( pWeapon ) )
					{
						UTIL_Remove( pEntity );
					}
					else
					{
						SetBodygroup( 1, false );
					}
				}
			}

			if ( m_hActivator->GiveAmmo( m_nAmmoAmounts[m_nAmmoType], m_nAmmoIndex ) != 0 )
			{
				SetBodygroup( 1, false );
			}
#else
			if ( ITEM_GiveTFAmmo( m_hActivator, PackRatios[POWERUP_FULL] ) )
			{
				SetBodygroup( 1, false );
				CSingleUserRecipientFilter filter( m_hActivator );
				EmitSound( filter, entindex(), TF_AMMOPACK_PICKUP_SOUND );
			}
#endif
			m_hActivator = NULL;
		}
		return;
	}

	BaseClass::HandleAnimEvent( pEvent );
}

	
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CItem_AmmoCrate::CrateThink( void )
{
	StudioFrameAdvance();
	DispatchAnimEvents( this );

	SetNextThink( gpGlobals->curtime + 0.1f );

	// Start closing if we're not already
	if ( GetSequence() != LookupSequence( "Close" ) )
	{
		// Not ready to close?
		if ( m_flCloseTime <= gpGlobals->curtime )
		{
			m_hActivator = NULL;

			ResetSequence( LookupSequence( "Close" ) );
		}
	}
	else
	{
		// See if we're fully closed
		if ( IsSequenceFinished() )
		{
			// Stop thinking
			SetThink( NULL );
			CPASAttenuationFilter sndFilter( this, "AmmoCrate.Close" );
			EmitSound( sndFilter, entindex(), "AmmoCrate.Close" );

			// FIXME: We're resetting the sequence here
			// but setting Think to NULL will cause this to never have
			// StudioFrameAdvance called. What are the consequences of that?
			ResetSequence( LookupSequence( "Idle" ) );
			SetBodygroup( 1, true );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &data - 
//-----------------------------------------------------------------------------
void CItem_AmmoCrate::InputKill( inputdata_t &data )
{
	UTIL_Remove( this );
}

