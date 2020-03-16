//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Workfile:     $
// $NoKeywords: $
//=============================================================================//
#if !defined( MAPDATA_H )
#define MAPDATA_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Panel.h>
#include <vgui/IImage.h>
#include "mathlib/vector.h"
#include "mapdata_shared.h"
#include "sharedInterface.h"

#define MAX_ZONES		32
#define MAX_CLASSES		32
#define MAX_MAP_TEAMS	8
#define MINIMAP_MATERIAL_STRING_SIZE 64
#define MINIMAP_STRING_SIZE 128

class Vector;
class C_BaseEntity;

class CMapData : public IMapData
{
public:
					CMapData( void );
	virtual			~CMapData( void );

	void			Init( void );
	void			Clear( void );
	void			LevelInit( const char *map );
	void			LevelShutdown( void );

	void			Update( );

	void			UseDefaults( void );

	void			UserCmd_ForceMapReload( void );

	// map dimensions
	void			GetMapBounds(Vector& mins, Vector& maxs);
	void			GetMapOrigin(Vector& org);
	void			GetMapSize(Vector& size);

	// 3d skybox
	void			Get3DSkyboxOrigin( Vector &vecOrigin );
	float			Get3DSkyboxScale( void );

	// Indicates the area currently visible in commander mode
	void			SetVisibleArea( const Vector& mins, const Vector& maxs );
	void			GetVisibleArea( Vector& mins, Vector& maxs );

private:
	char	m_szMap[ MINIMAP_STRING_SIZE ];

	// World size + visible size in commander mode
	Vector	m_WorldMins;
	Vector	m_WorldMaxs;
	Vector	m_VisibleMins;
	Vector	m_VisibleMaxs;
};


//-----------------------------------------------------------------------------
// Gets at the singleton map data 
//-----------------------------------------------------------------------------
CMapData& MapData();


#endif // MAPDATA_H
