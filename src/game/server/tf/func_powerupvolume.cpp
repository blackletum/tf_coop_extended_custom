//=============================================================================//
//
// Purpose: Mannpower Imbalance crits
//
//=============================================================================//

#include "cbase.h"
#include "func_powerupvolume.h"
#include "tf_player.h"
#include "ai_basenpc.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


BEGIN_DATADESC( CPowerupVolume )

END_DATADESC()

IMPLEMENT_AUTO_LIST( IFuncPowerupVolumeAutoList );

LINK_ENTITY_TO_CLASS( func_powerupvolume, CPowerupVolume );


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CPowerupVolume::Spawn( void )
{
	BaseClass::Spawn();

	AddSpawnFlags( SF_TRIGGER_ALLOW_CLIENTS | SF_TRIGGER_ALLOW_NPCS );

	InitTrigger();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPowerupVolume::Precache( void )
{
	PrecacheScriptSound( "Powerup.Volume.Use" );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CPowerupVolume::Touch( CBaseEntity *pOther )
{
	if ( !m_bDisabled && pOther->InSameTeam( this ) )
	{
		CTFPlayer *pPlayer = ToTFPlayer( pOther );
		CAI_BaseNPC *pNPC = dynamic_cast<CAI_BaseNPC *>( pOther );
		if ( pPlayer )
		{
			pPlayer->m_Shared.AddTempCritBonus( 30 );
		}
		else if ( pNPC )
		{
			pNPC->AddTempCritBonus( 30 );
		}

		EmitSound( "Powerup.Volume.Use" );
	}
}