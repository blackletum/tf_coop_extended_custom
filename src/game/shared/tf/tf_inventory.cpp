//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Simple Inventory
// by MrModez
// $NoKeywords: $
//=============================================================================//


#include "cbase.h"
#include "tf_shareddefs.h"
#include "tf_inventory.h"
#include "econ_item_system.h"
#include "tf_gamerules.h"
#include "filesystem.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define TF_INVENTORY_FILE "scripts/tf_inventory.txt"

static CTFInventory g_TFInventory;

CTFInventory *GetTFInventory()
{
	return &g_TFInventory;
}

#ifdef CLIENT_DLL
CON_COMMAND_F( econ_item_system_reload, "Reloads items_game.txt", FCVAR_CHEAT )
{
	GetTFInventory()->Reinit();
}
#endif

void CC_ItemWhitelist( IConVar *pConVar, const char *oldValue, float flOldValue )
{
	ConVarRef var( pConVar );

	if ( !GetTFInventory() )
		return;

	if ( !Q_strcmp( var.GetString(), "0" ) )
	{
		GetTFInventory()->Reinit();
		return;
	}

	if ( !Q_strcmp( var.GetString(), oldValue ) )
		return;
	else
		GetTFInventory()->LoadWhitelist();
}
ConVar  mp_tournament_whitelist( "mp_tournament_whitelist", "0", FCVAR_REPLICATED, "Specifies the item whitelist file to use.", CC_ItemWhitelist );

CTFInventory::CTFInventory() : CAutoGameSystemPerFrame( "CTFInventory" )
{
#ifdef CLIENT_DLL
	m_pInventory = NULL;
#endif

	// Generate dummy base items.
	for ( int iClass = 0; iClass < TF_CLASS_COUNT_ALL; iClass++ )
	{
		for ( int iSlot = 0; iSlot < LOADOUT_POSITION_COUNT; iSlot++ )
		{
			m_Items[iClass][iSlot].AddToTail( NULL );
		}
	}

#ifdef GAME_DLL
	m_Player.EnsureCapacity( MAX_PLAYERS );
	m_Player.SetLessFunc( invles );
#endif
};

CTFInventory::~CTFInventory()
{
#ifdef CLIENT_DLL
	m_pInventory->deleteThis();
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Fill the item arrays with data from item schema.
//-----------------------------------------------------------------------------
bool CTFInventory::Init( void )
{
	GetItemSchema()->Init();

	// Generate item list.
	FOR_EACH_MAP( GetItemSchema()->m_Items, i )
	{
		int iItemID = GetItemSchema()->m_Items.Key( i );
		CEconItemDefinition *pItemDef = GetItemSchema()->m_Items.Element( i );

		if ( pItemDef->item_slot == -1 )
			continue;

		// Add it to each class that uses it.
		for ( int iClass = 0; iClass < TF_CLASS_COUNT_ALL; iClass++ )
		{
			if ( pItemDef->used_by_classes & ( 1 << iClass ) )
			{
				// Show it if it's either base item or has show_in_armory flag.
				int iSlot = pItemDef->GetLoadoutSlot( iClass );

				bool bHolidayRestrictedItem = false;
				if ( pItemDef->holiday_restriction[0] != '\0' )
				{
					if ( ( UTIL_IsHolidayActive( kHoliday_Halloween ) ) && ( strcmp( pItemDef->holiday_restriction, "halloween" ) == 0 ) )
						bHolidayRestrictedItem = true;
					else if ( ( UTIL_IsHolidayActive( kHoliday_HalloweenOrFullMoon ) ) && ( strcmp( pItemDef->holiday_restriction, "halloween_or_fullmoon" ) == 0 ) )
						bHolidayRestrictedItem = true;
					else if ( ( UTIL_IsHolidayActive( kHoliday_HalloweenOrFullMoonOrValentines ) ) && ( strcmp( pItemDef->holiday_restriction, "halloween_or_fullmoon_or_valentines" ) == 0 ) )
						bHolidayRestrictedItem = true;
					else if ( ( UTIL_IsHolidayActive( kHoliday_Christmas ) ) && ( strcmp( pItemDef->holiday_restriction, "christmas" ) == 0 ) )
						bHolidayRestrictedItem = true;
					else if ( ( UTIL_IsHolidayActive( kHoliday_TF2Birthday ) ) && ( strcmp( pItemDef->holiday_restriction, "birthday" ) == 0 ) )
						bHolidayRestrictedItem = true;
				}

				if ( pItemDef->baseitem )
				{
					CEconItemView *pBaseItem = m_Items[iClass][iSlot][0];
					if ( pBaseItem != NULL )
					{
						Warning( "Duplicate base item %d for class %s in slot %s!\n", iItemID, g_aPlayerClassNames_NonLocalized[iClass], g_LoadoutSlots[iSlot] );
						delete pBaseItem;
					}

					m_Items[iClass][iSlot][0] = new CEconItemView( iItemID );
				}
				else if ( pItemDef->show_in_armory && !bHolidayRestrictedItem )
				{
					CEconItemView *pNewItem = new CEconItemView( iItemID );
					m_Items[iClass][iSlot].AddToTail( pNewItem );
				}
			}
		}
	}

#if defined( CLIENT_DLL )
	LoadInventory();
#endif

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Fill the item arrays with data from item schema.
//-----------------------------------------------------------------------------
bool CTFInventory::Reinit( void )
{
	RemoveAllItem();

	GetItemSchema()->Init();

	// Generate item list.
	FOR_EACH_MAP( GetItemSchema()->m_Items, i )
	{
		int iItemID = GetItemSchema()->m_Items.Key( i );
		CEconItemDefinition *pItemDef = GetItemSchema()->m_Items.Element( i );

		if ( pItemDef->item_slot == -1 )
			continue;

		// Add it to each class that uses it.
		for ( int iClass = 0; iClass < TF_CLASS_COUNT_ALL; iClass++ )
		{
			if ( pItemDef->used_by_classes & ( 1 << iClass ) )
			{
				// Show it if it's either base item or has show_in_armory flag.
				int iSlot = pItemDef->GetLoadoutSlot( iClass );

				bool bHolidayRestrictedItem = false;
				if ( pItemDef->holiday_restriction[0] != '\0' )
				{
					if ( ( UTIL_IsHolidayActive( kHoliday_Halloween ) ) && ( strcmp( pItemDef->holiday_restriction, "halloween" ) == 0 ) )
						bHolidayRestrictedItem = true;
					else if ( ( UTIL_IsHolidayActive( kHoliday_HalloweenOrFullMoon ) ) && ( strcmp( pItemDef->holiday_restriction, "halloween_or_fullmoon" ) == 0 ) )
						bHolidayRestrictedItem = true;
					else if ( ( UTIL_IsHolidayActive( kHoliday_HalloweenOrFullMoonOrValentines ) ) && ( strcmp( pItemDef->holiday_restriction, "halloween_or_fullmoon_or_valentines" ) == 0 ) )
						bHolidayRestrictedItem = true;
					else if ( ( UTIL_IsHolidayActive( kHoliday_Christmas ) ) && ( strcmp( pItemDef->holiday_restriction, "christmas" ) == 0 ) )
						bHolidayRestrictedItem = true;
					else if ( ( UTIL_IsHolidayActive( kHoliday_TF2Birthday ) ) && ( strcmp( pItemDef->holiday_restriction, "birthday" ) == 0 ) )
						bHolidayRestrictedItem = true;
				}

				if ( pItemDef->baseitem )
				{
					CEconItemView *pBaseItem = m_Items[iClass][iSlot][0];
					m_Items[iClass][iSlot].AddToHead( pBaseItem );
				}
				else if ( pItemDef->show_in_armory && !bHolidayRestrictedItem )
				{
					CEconItemView *pNewItem = new CEconItemView( iItemID );
					m_Items[iClass][iSlot].AddToTail( pNewItem );
				}
			}
		}
	}

#if defined( CLIENT_DLL )
	LoadInventory();
#endif

	return true;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFInventory::LevelInitPreEntity( void )
{
	GetItemSchema()->Precache();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFInventory::LevelInitPostEntity( void )
{
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFInventory::SetupOnRoundStart( void )
{
	LoadWhitelist();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFInventory::RemoveAllItem( void )
{
	for ( int iClass = 0; iClass < TF_CLASS_COUNT_ALL; iClass++ )
	{
		for ( int iSlot = 0; iSlot < LOADOUT_POSITION_COUNT; iSlot++ )
		{
			m_Items[iClass][iSlot].RemoveAll();
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: allowing spooky weapons
//-----------------------------------------------------------------------------
void CTFInventory::LoadWhitelist( void )
{
	const char *pszFileName = mp_tournament_whitelist.GetString();
	if ( !Q_strcmp( pszFileName, "0" ) )
		return;

	KeyValues *pWhitelist = new KeyValues( "item_whitelist" );
	if ( pWhitelist->LoadFromFile( filesystem, pszFileName, "MOD" ) )
	{
		ConColorMsg( Color( 77, 116, 85, 255 ), "Parsing item whitelist (default: %s).\n", pszFileName );
		RemoveAllItem();

		GetItemSchema()->Init();

		KeyValues *pvItem;
		for ( pvItem = pWhitelist->GetFirstSubKey(); pvItem != NULL; pvItem = pvItem->GetNextKey() )
		{
			// Generate item list.
			FOR_EACH_MAP( GetItemSchema()->m_Items, i )
			{
				int iItemID = GetItemSchema()->m_Items.Key( i );
				CEconItemDefinition *pItemDef = GetItemSchema()->m_Items.Element( i );

				if ( pItemDef->item_slot == -1 )
					continue;

				if ( pItemDef->item_quality == QUALITY_VALVE )
					continue;

				for ( int iClass = 0; iClass < TF_CLASS_COUNT_ALL; iClass++ )
				{
					if ( pItemDef->used_by_classes & ( 1 << iClass ) )
					{
						int iSlot = pItemDef->GetLoadoutSlot( iClass );

						if ( pItemDef->baseitem )
						{
							CEconItemView *pBaseItem = m_Items[iClass][iSlot][0];
							m_Items[iClass][iSlot].AddToHead( pBaseItem );
						}
						else
						{
							if ( !Q_stricmp( pvItem->GetName(), pItemDef->name ) )
							{
								if ( pvItem->GetInt() > 0 )
								{
									CEconItemView *pNewItem = new CEconItemView( iItemID );
									m_Items[iClass][iSlot].AddToTail( pNewItem );
								}
							}
							/*else
							{
								if ( !Q_stricmp( pvItem->GetName(), "unlisted_items_default_to" ) )
								{
									if ( pvItem->GetInt() > 0 )
									{
										CEconItemView *pNewItem = new CEconItemView( iItemID );
										m_Items[iClass][iSlot].AddToTail( pNewItem );
									}
								}
							}*/
						}
					}
				}
			}
		}

#ifdef CLIENT_DLL
		LoadInventory();
#endif
	}
	else
	{
		ConColorMsg( Color( 77, 116, 85, 255 ), "Item Whitelist file '%s' could not be found. All items will be allowed.\n", pszFileName );
	}

	pWhitelist->deleteThis();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFInventory::AddItem( const char *pszItemName )
{
	FOR_EACH_MAP( GetItemSchema()->m_Items, i )
	{
		int iItemID = GetItemSchema()->m_Items.Key( i );
		CEconItemDefinition *pItemDef = GetItemSchema()->m_Items.Element( i );

		if ( pItemDef->item_slot == -1 )
			continue;

		for ( int iClass = 0; iClass < TF_CLASS_COUNT_ALL; iClass++ )
		{
			if ( pItemDef->used_by_classes & ( 1 << iClass ) )
			{
				int iSlot = pItemDef->GetLoadoutSlot( iClass );
				if ( FStrEq( pszItemName, pItemDef->name ) )
				{
					CEconItemView *pNewItem = new CEconItemView( iItemID );
					m_Items[iClass][iSlot].AddToTail( pNewItem );
				}
			}
		}
#ifdef CLIENT_DLL
		LoadInventory();
#endif
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFInventory::RemoveItem( const char *pszItemName )
{
	FOR_EACH_MAP( GetItemSchema()->m_Items, i )
	{
		int iItemID = GetItemSchema()->m_Items.Key( i );
		CEconItemDefinition *pItemDef = GetItemSchema()->m_Items.Element( i );

		for ( int iClass = 0; iClass < TF_CLASS_COUNT_ALL; iClass++ )
		{
			if ( pItemDef->used_by_classes & ( 1 << iClass ) )
			{
				int iSlot = pItemDef->GetLoadoutSlot( iClass );
				if ( FStrEq( pszItemName, pItemDef->name ) )
				{
					m_Items[iClass][iSlot].Remove( iItemID );
				}
			}
		}
#ifdef CLIENT_DLL
		LoadInventory();
#endif
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CTFInventory::HasItem( const char *pszItemName )
{
	FOR_EACH_MAP( GetItemSchema()->m_Items, i )
	{
		int iItemID = GetItemSchema()->m_Items.Key( i );
		CEconItemDefinition *pItemDef = GetItemSchema()->m_Items.Element( i );

		for ( int iClass = 0; iClass < TF_CLASS_COUNT_ALL; iClass++ )
		{
			if ( pItemDef->used_by_classes & ( 1 << iClass ) )
			{
				int iSlot = pItemDef->GetLoadoutSlot( iClass );
				if ( FStrEq( pszItemName, pItemDef->name ) )
				{
					CEconItemView *pNewItem = new CEconItemView( iItemID );
					return ( m_Items[iClass][iSlot].HasElement( pNewItem ) );
				}
			}
		}
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
int CTFInventory::GetNumPresets( int iClass, int iSlot )
{
	return m_Items[iClass][iSlot].Count();
};

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
int CTFInventory::GetWeapon( int iClass, int iSlot )
{
	if ( TFGameRules()->IsTFCAllowed() )
		return WeaponsTFC[iClass][iSlot];
	else
		return Weapons[iClass][iSlot];
};

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CEconItemView *CTFInventory::GetItem( int iClass, int iSlot, int iNum )
{
	if ( CheckValidWeapon( iClass, iSlot, iNum ) == false )
		return NULL;

	return m_Items[iClass][iSlot][iNum];
};

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CEconItemView *CTFInventory::GetItem( int iID )
{
	for ( int iClass = 0; iClass < TF_CLASS_COUNT_ALL; iClass++ )
	{
		for ( int iSlot = 0; iSlot < LOADOUT_POSITION_COUNT; iSlot++ )
		{
			for ( int iItem = 0; iItem < m_Items[iClass][iSlot].Count(); iItem++ )
			{
				CEconItemView *pItem = m_Items[iClass][iSlot].Element( iItem );
				if ( pItem && pItem->GetItemDefIndex() == iID )
					return pItem;
			}
		}

	}
	return nullptr;
};

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CTFInventory::CheckValidSlot( int iClass, int iSlot, bool bHudCheck /*= false*/ )
{
	if ( iClass < TF_CLASS_UNDEFINED || iClass > TF_CLASS_COUNT )
		return false;

	int iCount = LOADOUT_POSITION_COUNT;

	// Array bounds check.
	if ( iSlot >= iCount || iSlot < 0 )
		return false;

	// Slot must contain a base item.
	if ( m_Items[iClass][iSlot][0] == NULL )
		return false;

	return true;
};

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CTFInventory::CheckValidWeapon( int iClass, int iSlot, int iWeapon, bool bHudCheck /*= false*/ )
{
	if ( iClass < TF_CLASS_UNDEFINED || iClass > TF_CLASS_COUNT )
		return false;

	int iCount = m_Items[iClass][iSlot].Count();

	// Array bounds check.
	if ( iWeapon >= iCount || iWeapon < 0 )
		return false;

	// Don't allow switching if this class has no weapon at this position.
	if ( m_Items[iClass][iSlot][iWeapon] == NULL )
		return false;

	return true;
};

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
int CTFInventory::NumWeapons( int iClass, int iSlot )
{
	// Slot must contain a base item.
	if ( m_Items[iClass][iSlot][0] == NULL )
		return 0;

	return m_Items[iClass][iSlot].Count();
}

#if defined( CLIENT_DLL )
//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFInventory::LoadInventory()
{
	bool bExist = filesystem->FileExists( TF_INVENTORY_FILE, "MOD" );
	if (bExist)
	{
		if (!m_pInventory)
		{
			m_pInventory = new KeyValues("Inventory");
		}
		m_pInventory->LoadFromFile( filesystem, TF_INVENTORY_FILE, "MOD" );
	}
	else
	{
		ResetInventory();
	}
};

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFInventory::SaveInventory()
{
	m_pInventory->SaveToFile( filesystem, TF_INVENTORY_FILE );
};

//-----------------------------------------------------------------------------
// Purpose: Create a default inventory file.
//-----------------------------------------------------------------------------
void CTFInventory::ResetInventory()
{
	if ( m_pInventory )
	{
		m_pInventory->deleteThis();
	}

	m_pInventory = new KeyValues( "Inventory" );

	for ( int i = TF_FIRST_NORMAL_CLASS; i <= TF_CLASS_COUNT_ALL; i++ )
	{
		KeyValues *pClassInv = new KeyValues( g_aPlayerClassNames_NonLocalized[i] );
		for ( int j = 0; j < LOADOUT_POSITION_BUFFER; j++ )
		{
			if ( j == LOADOUT_POSITION_UTILITY )
				continue;

			pClassInv->SetInt( g_LoadoutSlots[j], 0 );
		}
		m_pInventory->AddSubKey( pClassInv );
	}

	SaveInventory();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
int CTFInventory::GetWeaponPreset( int iClass, int iSlot )
{
	KeyValues *pClass = m_pInventory->FindKey( g_aPlayerClassNames_NonLocalized[iClass] );
	if ( !pClass )	//cannot find class node
	{
		//ResetInventory();
		return 0;
	}
	int iPreset = pClass->GetInt( g_LoadoutSlots[iSlot], -1 );
	if ( iPreset == -1 )	//cannot find slot node
	{
		//ResetInventory();
		return 0;
	}

	if ( CheckValidWeapon( iClass, iSlot, iPreset ) == false )
		return 0;

	return iPreset;
};

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFInventory::SetWeaponPreset( int iClass, int iSlot, int iPreset )
{
	KeyValues* pClass = m_pInventory->FindKey( g_aPlayerClassNames_NonLocalized[iClass] );
	if ( !pClass )	//cannot find class node
	{
		//ResetInventory();
		pClass = m_pInventory->FindKey( g_aPlayerClassNames_NonLocalized[iClass] );
	}
	pClass->SetInt( GetSlotName( iSlot ), iPreset );
	SaveInventory();
}

const char* CTFInventory::GetSlotName( int iSlot )
{
	return g_LoadoutSlots[iSlot];
};

#endif

//-----------------------------------------------------------------------------
// Purpose: Legacy array, used when we're forced to use old method of giving out weapons.
//-----------------------------------------------------------------------------
const int CTFInventory::Weapons[TF_CLASS_COUNT_ALL][LOADOUT_POSITION_BUFFER] =
{
	{

	},
	{
		TF_WEAPON_SCATTERGUN,
		TF_WEAPON_PISTOL_SCOUT,
		TF_WEAPON_BAT
	},
	{
		TF_WEAPON_SNIPERRIFLE,
		TF_WEAPON_SMG,
		TF_WEAPON_CLUB
	},
	{
		TF_WEAPON_ROCKETLAUNCHER,
		TF_WEAPON_SHOTGUN_SOLDIER,
		TF_WEAPON_SHOVEL
	},
	{
		TF_WEAPON_GRENADELAUNCHER,
		TF_WEAPON_PIPEBOMBLAUNCHER,
		TF_WEAPON_BOTTLE
	},
	{
		TF_WEAPON_SYRINGEGUN_MEDIC,
		TF_WEAPON_MEDIGUN,
		TF_WEAPON_BONESAW
	},
	{
		TF_WEAPON_MINIGUN,
		TF_WEAPON_SHOTGUN_HWG,
		TF_WEAPON_FISTS
	},
	{
		TF_WEAPON_FLAMETHROWER,
		TF_WEAPON_SHOTGUN_PYRO,
		TF_WEAPON_FIREAXE
	},
	{
		TF_WEAPON_REVOLVER,
		TF_WEAPON_NONE,
		TF_WEAPON_KNIFE,
		TF_WEAPON_PDA_SPY,
		TF_WEAPON_INVIS
	},
	{
		TF_WEAPON_SHOTGUN_PRIMARY,
		TF_WEAPON_PISTOL,
		TF_WEAPON_WRENCH,
		TF_WEAPON_PDA_ENGINEER_BUILD,
		TF_WEAPON_PDA_ENGINEER_DESTROY
	}
};

//-----------------------------------------------------------------------------
// Purpose: Used when we're in tfc mode.
//-----------------------------------------------------------------------------
const int CTFInventory::WeaponsTFC[TF_CLASS_COUNT_ALL][LOADOUT_POSITION_BUFFER] =
{
	{

	},
	{
		TF_WEAPON_TFC_NAILGUN,
		TF_WEAPON_TFC_SHOTGUN,
		TF_WEAPON_TFC_CROWBAR
	},
	{
		TF_WEAPON_TFC_SNIPERRIFLE,
		TF_WEAPON_TFC_AUTOMATICRIFLE,
		TF_WEAPON_TFC_CROWBAR,
		TF_WEAPON_TFC_NAILGUN
		
	},
	{
		TF_WEAPON_TFC_ROCKETLAUNCHER,
		TF_WEAPON_TFC_SUPER_SHOTGUN,
		TF_WEAPON_TFC_CROWBAR,
		TF_WEAPON_TFC_SHOTGUN
	},
	{
		TF_WEAPON_TFC_GRENADELAUNCHER,
		TF_WEAPON_TFC_PIPEBOMBLAUNCHER,
		TF_WEAPON_TFC_CROWBAR,
		TF_WEAPON_TFC_SHOTGUN
		
	},
	{
		TF_WEAPON_TFC_SUPER_NAILGUN,
		TF_WEAPON_TFC_SUPER_SHOTGUN,
		TF_WEAPON_TFC_MEDIKIT,
		TF_WEAPON_TFC_SHOTGUN
	},
	{
		TF_WEAPON_TFC_MINIGUN,
		TF_WEAPON_TFC_SUPER_SHOTGUN,
		TF_WEAPON_TFC_CROWBAR,
		TF_WEAPON_TFC_SHOTGUN
	},
	{
		TF_WEAPON_TFC_FLAMETHROWER,
		TF_WEAPON_TFC_INCENDIARYCANNON,
		TF_WEAPON_TFC_CROWBAR,
		TF_WEAPON_TFC_SHOTGUN
	},
	{
		TF_WEAPON_TFC_TRANQ,
		TF_WEAPON_TFC_SUPER_SHOTGUN,
		TF_WEAPON_TFC_KNIFE,
		TF_WEAPON_TFC_NAILGUN,
		TF_WEAPON_PDA_SPY
	},
	{
		TF_WEAPON_TFC_RAILGUN,
		TF_WEAPON_TFC_SUPER_SHOTGUN,
		TF_WEAPON_TFC_SPANNER,
		TF_WEAPON_PDA_ENGINEER_BUILD,
		TF_WEAPON_PDA_ENGINEER_DESTROY
	}/*,
	{
		TF_WEAPON_TFC_UMBRELLA,
	}*/
};
