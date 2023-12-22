//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Simple Inventory
// by MrModez
// $NoKeywords: $
//=============================================================================//
#ifndef TF_INVENTORY_H
#define TF_INVENTORY_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
//#include "server_class.h"
#include "igamesystem.h"
#include "tf_shareddefs.h"
#ifdef CLIENT_DLL
#include "c_tf_player.h"
#include "filesystem.h" 
#endif

class CTFInventory : public CAutoGameSystemPerFrame
{
public:
	static	bool invles( const uint64 &lhs, const uint64 &rhs )	{ return lhs != rhs; }

	CTFInventory();
	~CTFInventory();

	virtual char const *Name() { return "CTFInventory"; }

	virtual bool	Init( void );
	virtual bool	Reinit( void );
	virtual void	LevelInitPreEntity( void );
	virtual void	LevelInitPostEntity( void );

	int				GetNumPresets( int iClass, int iSlot );
	int				GetWeapon( int iClass, int iSlot );
	CEconItemView	*GetItem( int iClass, int iSlot, int iNum );
	CEconItemView	*GetItem( int iID );
	bool			CheckValidSlot( int iClass, int iSlot, bool bHudCheck = false );
	bool			CheckValidWeapon( int iClass, int iSlot, int iWeapon, bool bHudCheck = false );
	int				NumWeapons( int iClass, int iSlot );

	void			SetupOnRoundStart( void );
	void			RemoveAllItem( void );
	void			LoadWhitelist( void );

	void			AddItem( const char *pszItemName );
	bool			HasItem( const char *pszItemName );
	void			RemoveItem( const char *pszItemName );
#ifdef CLIENT_DLL
	int				GetWeaponPreset( int iClass, int iSlot );
	void			SetWeaponPreset( int iClass, int iSlot, int iPreset );
	const char		*GetSlotName( int iSlot );
	void			ResetInventory();
#else
	struct ClassLoadout {
		uint ItemIndex[MAX_WEAPON_SLOTS];
	};

	struct Loadout {
		ClassLoadout playerclass[TF_CLASS_COUNT];
	};

	CUtlMap<uint64, Loadout> m_Player;
#endif

private:
	static const int			Weapons[TF_CLASS_COUNT_ALL][LOADOUT_POSITION_BUFFER];
	static const int			WeaponsTFC[TF_CLASS_COUNT_ALL][LOADOUT_POSITION_BUFFER];
	CUtlVector<CEconItemView *>	m_Items[TF_CLASS_COUNT_ALL][LOADOUT_POSITION_COUNT];

#ifdef CLIENT_DLL
	void LoadInventory();
	//void ResetInventory();
	void SaveInventory();
	KeyValues* m_pInventory;
#endif
};

CTFInventory *GetTFInventory();

#endif // TF_INVENTORY_H
