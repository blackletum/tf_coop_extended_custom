//=========== (C) Copyright 1999 Valve, L.L.C. All rights reserved. ===========
//
// Purpose: This is the camo version of the combine soldier
//
//=============================================================================

#include "cbase.h"
#include "ai_hull.h"
#include "npc_talker.h"
#include "npc_combinee.h"
#include "tf_gamerules.h"

ConVar	sk_combine_elite_health( "sk_combine_elite_health","0");
ConVar	sk_combine_elite_kick( "sk_combine_elite_kick","0");

LINK_ENTITY_TO_CLASS( npc_combineelite, CNPC_CombineE );

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_CombineE::Spawn( void )
{
	if ( TFGameRules() && TFGameRules()->IsHL2Beta() )
	{
		Precache();
		SetModel( "models/Combine_Elite.mdl" );

		m_iHealth = sk_combine_elite_health.GetFloat();
		m_nKickDamage = sk_combine_elite_kick.GetFloat();

		BaseClass::Spawn();
	}
	else
	{
		UTIL_Remove( this );
	}
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CNPC_CombineE::Precache()
{
	PrecacheModel( "models/combine_elite.mdl" );

	BaseClass::Precache();
}
