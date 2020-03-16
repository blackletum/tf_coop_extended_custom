//====== Copyright © 1996-2020, Valve Corporation, All rights reserved. =======
//
// Purpose: 
//
//=============================================================================

#include "cbase.h"
#include "tf_weapon_spellbook.h"
#include "decals.h"

// Client specific.
#ifdef CLIENT_DLL
#include "c_tf_player.h"
#include "particles_simple.h"
// Server specific.
#else
#include "tf_player.h"
#include "tf_team.h"
#endif

//=============================================================================
//
// Weapon Spell Book tables.
//
IMPLEMENT_NETWORKCLASS_ALIASED( TFSpellBook, DT_TFWeaponSpellBook )

BEGIN_NETWORK_TABLE( CTFSpellBook, DT_TFWeaponSpellBook )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CTFSpellBook )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( tf_weapon_spellbook, CTFSpellBook );
PRECACHE_WEAPON_REGISTER( tf_weapon_spellbook );

//=============================================================================
//
// Weapon Spell Book functions.
//

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CTFSpellBook::CTFSpellBook()
{
#ifdef CLIENT_DLL
	m_pSpellEffect = NULL;
#endif
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CTFSpellBook::~CTFSpellBook()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFSpellBook::Precache()
{
	BaseClass::Precache();

	PrecacheScriptSound( "Halloween.spell_athletic" );
	PrecacheScriptSound( "Halloween.spell_bat_cast" );
	PrecacheScriptSound( "Halloween.spell_blastjump" );
	PrecacheScriptSound( "Halloween.spell_fireball_cast" );
	PrecacheScriptSound( "Halloween.spell_lightning_cast" );
	PrecacheScriptSound( "Halloween.spell_meteor_cast" );
	PrecacheScriptSound( "Halloween.spell_mirv_cast" );
	PrecacheScriptSound( "Halloween.spell_overheal" );
	PrecacheScriptSound( "Halloween.spell_skeleton_horde_cast" );
	PrecacheScriptSound( "Halloween.spell_spawn_boss" );
	PrecacheScriptSound( "Halloween.spell_stealth" );
	PrecacheScriptSound( "Halloween.spell_teleport" );

	PrecacheParticleSystem( "spellbook_minor_burning" );
	PrecacheParticleSystem( "spellbook_major_burning" );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CTFSpellBook::Deploy( void )
{
	return BaseClass::Deploy();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFSpellBook::PrimaryAttack( void )
{
	BaseClass::PrimaryAttack();

	CTFPlayer *pOwner = GetTFPlayerOwner();
	if ( !pOwner )
		return;

#ifdef GAME_DLL
	pOwner->SpeakConceptIfAllowed( MP_CONCEPT_PLAYER_CAST_FIREBALL );
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFSpellBook::ItemBusyFrame( void )
{
#ifdef CLIENT_DLL
	CTFPlayer *pOwner = GetTFPlayerOwner();
	if ( pOwner )
	{
		if ( m_pSpellEffect != NULL )
		{
			if ( pOwner )
			{
				pOwner->ParticleProp()->StopEmission( m_pSpellEffect );
			}

			m_pSpellEffect = NULL;
		}
	}
#endif

	BaseClass::ItemBusyFrame();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFSpellBook::ItemPostFrame( void )
{
#ifdef CLIENT_DLL
	CTFPlayer *pOwner = GetTFPlayerOwner();
	if ( pOwner )
	{
		if ( !m_bLowered )
		{
			if (  m_pSpellEffect == NULL )
			{
				m_pSpellEffect = pOwner->ParticleProp()->Create( "spellbook_minor_burning", PATTACH_POINT_FOLLOW, "effect_hand_r" );
			}
		}
		else
		{
			if ( m_pSpellEffect != NULL )
			{
				if ( pOwner )
				{
					pOwner->ParticleProp()->StopEmission( m_pSpellEffect );
				}

				m_pSpellEffect = NULL;
			}
		}
	}
#endif

	BaseClass::ItemPostFrame();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFSpellBook::ItemHolsterFrame( void )
{
#ifdef CLIENT_DLL
	CTFPlayer *pOwner = GetTFPlayerOwner();
	if ( pOwner )
	{
		if ( m_pSpellEffect != NULL )
		{
			if ( pOwner )
			{
				pOwner->ParticleProp()->StopEmission( m_pSpellEffect );
			}

			m_pSpellEffect = NULL;
		}
	}
#endif

	BaseClass::ItemHolsterFrame();
}
