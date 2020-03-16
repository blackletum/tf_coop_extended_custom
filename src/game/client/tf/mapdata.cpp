//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Workfile:     $
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "mapdata.h"
#include "hud.h"
#include "hud_macros.h"
#include <KeyValues.h>
#include "iclientmode.h"
#include "c_world.h"
#include "c_baseplayer.h"
#include "c_team.h"

//-----------------------------------------------------------------------------
// Gets at the singleton map data 
//-----------------------------------------------------------------------------

static CMapData g_MapData;
CMapData& MapData()
{
	// Singleton object
	return g_MapData;
}

IMapData *g_pMapData = &g_MapData;

DECLARE_COMMAND( g_MapData, ForceMapReload );

//-----------------------------------------------------------------------------
// Purpose: This is a total hack, but should allow reloading the mapfile.txt stuff on the fly
//-----------------------------------------------------------------------------
void CMapData::UserCmd_ForceMapReload( void )
{
	if ( m_szMap[0] )
	{
		LevelInit( m_szMap );

		// Force any needed viewport fixups
		g_pClientMode->Disable();
		g_pClientMode->Enable();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CMapData::CMapData( void )
{
	m_szMap[0]=0;

	UseDefaults();
}

HOOK_COMMAND( forcemapreload, ForceMapReload );

//-----------------------------------------------------------------------------
// Purpose: One time init
//-----------------------------------------------------------------------------
void CMapData::Init( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: Delete all dynamic images, but leave rest of data structures
//-----------------------------------------------------------------------------
void CMapData::Clear( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CMapData::~CMapData( void )
{
	Clear();
}

//-----------------------------------------------------------------------------
// Purpose: Fill in placeholder colors, etc.
//-----------------------------------------------------------------------------
void CMapData::UseDefaults( void )
{
}


//-----------------------------------------------------------------------------
// Get the bounding box of the world
//-----------------------------------------------------------------------------
void CMapData::GetMapBounds(Vector &mins, Vector& maxs)
{
	C_BaseEntity *ent = cl_entitylist->GetEnt( 0 );
	C_World* pWorld = dynamic_cast<C_World*>(ent);
	if (pWorld)
	{
		VectorCopy( pWorld->m_WorldMins, mins );
		VectorCopy( pWorld->m_WorldMaxs, maxs );

		// Backward compatability...
		if ((mins.LengthSqr() == 0.0f) && (maxs.LengthSqr() == 0.0f))
		{
			mins.Init( -6500, -6500, -6500 );
			maxs.Init( 6500, 6500, 6500 );
		}
	}
	else
	{
		Assert(0);
		mins.Init( 0, 0, 0 );
		maxs.Init( 1, 1, 1 );
	}
}

void CMapData::GetMapOrigin(Vector &org)
{
	Vector mins, maxs;
	GetMapBounds( mins, maxs );
	VectorAdd( mins, maxs, org );
	VectorMultiply( org, 0.5, org );
}

void CMapData::GetMapSize(Vector &size) 
{
	Vector mins, maxs;
	GetMapBounds( mins, maxs );
	VectorSubtract( maxs, mins, size );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CMapData::Get3DSkyboxOrigin( Vector &vecOrigin )
{
	// NOTE: If the player hasn't been created yet -- this doesn't work!!!
	//       We need to pass the data along in the map - requires a tool change.
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if ( pPlayer )
	{
		CPlayerLocalData *pLocalData = &pPlayer->m_Local;
		VectorCopy( pLocalData->m_skybox3d.origin, vecOrigin );
	}
	else
	{
		// Debugging!
		Assert( 0 );
		vecOrigin.Init();
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
float CMapData::Get3DSkyboxScale( void )
{
	// NOTE: If the player hasn't been created yet -- this doesn't work!!!
	//       We need to pass the data along in the map - requires a tool change.
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if ( pPlayer )
	{
		CPlayerLocalData *pLocalData = &pPlayer->m_Local;
		return pLocalData->m_skybox3d.scale;
	}
	else
	{
		// Debugging!
		Assert( 0 );
		return ( 1.0f );
	}
}

//-----------------------------------------------------------------------------
// What's my visible area?
//-----------------------------------------------------------------------------
void CMapData::SetVisibleArea( const Vector& mins, const Vector& maxs )
{
	m_VisibleMins = mins;
	m_VisibleMaxs = maxs;
}

void CMapData::GetVisibleArea( Vector& mins, Vector& maxs )
{
	mins = m_VisibleMins;
	maxs = m_VisibleMaxs;
}


//-----------------------------------------------------------------------------
// Purpose: The client is about to change maps
// Input  : *map - name of the new map
//-----------------------------------------------------------------------------
void CMapData::LevelInit( const char *map )
{
	Q_strncpy( m_szMap, map, MINIMAP_STRING_SIZE );

	// Clear leftover data
	Clear();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CMapData::LevelShutdown( void )
{
	Clear();
}


//-----------------------------------------------------------------------------
// Update fog of war
//-----------------------------------------------------------------------------
void CMapData::Update( void ) 
{
}