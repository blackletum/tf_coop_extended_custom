//=========================                         ==========================//
//
// Purpose: Custom Weapon with Custom Attributes.
//
//=============================================================================//
#include "cbase.h"
#include "tf_gamerules.h"
#include "tf_shareddefs.h"
#include "tf_player.h"
#include "tf_team.h"
#include "engine/IEngineSound.h"
#include "entity_weapon_custom.h"
#include "tf_weaponbase.h"
#include "basecombatcharacter.h"
#include "in_buttons.h"
#include "tf_fx.h"
#include "tf_dropped_weapon.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

BEGIN_DATADESC( CTFLFEWeaponCustom )
	DEFINE_KEYFIELD( m_nItemID, 			FIELD_INTEGER,	"itemid" ),
	DEFINE_KEYFIELD( m_szItemClassname, 	FIELD_STRING,	"itemclassname" ),
	DEFINE_KEYFIELD( m_szItemModelW, 		FIELD_STRING,	"itemmodelw" ),
	DEFINE_KEYFIELD( m_szItemModelV, 		FIELD_STRING,	"itemmodelv" ),

	DEFINE_KEYFIELD( m_bSkipBaseAttributes,	FIELD_BOOLEAN,	"skipbaseattribute"),
END_DATADESC()

LINK_ENTITY_TO_CLASS( lfe_weapon_custom, CTFLFEWeaponCustom );


CTFLFEWeaponCustom::CTFLFEWeaponCustom()
{
	m_nItemID = -1;
	m_bSkipBaseAttributes = false;

	for ( int i = 0; i < MAX_NUM_CUSTOM_ATTRIBUTES; i++ )
	{
		m_nAttributes[i] = 0;
		m_nAttributesValue[i] = 0;
	}
}


//-----------------------------------------------------------------------------
// Purpose: Spawn function 
//-----------------------------------------------------------------------------
void CTFLFEWeaponCustom::Spawn( void )
{
	CEconItemDefinition *pItemDef = GetItemSchema()->GetItemDefinition( m_nItemID );
	if ( !pItemDef )
	{
		Warning( "lfe_weapon_custom has incorrect item ID %d. DELETED\n", m_nItemID );
		UTIL_Remove( this );
		return;
	}

	m_Item.SetItemDefIndex( m_nItemID );

	Precache();

	/*if ( m_szItemModelW != "" )
		SetModel( m_szItemModelW );
	else*/
		SetModel( m_Item.GetWorldDisplayModel() );

	BaseClass::Spawn();

	SetSolid( SOLID_BBOX );
	SetCollisionBounds( -Vector( 22, 22, 15 ), Vector( 22, 22, 15 ) );

	//AddEffects( EF_ITEM_BLINK );
}

//-----------------------------------------------------------------------------
// Purpose: Precache function 
//-----------------------------------------------------------------------------
void CTFLFEWeaponCustom::Precache( void )
{
	PrecacheModel( m_Item.GetWorldDisplayModel() );
	//PrecacheModel( m_szItemModelW );
	//PrecacheModel( m_szItemModelV );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CTFLFEWeaponCustom::KeyValue( const char *szKeyName, const char *szValue )
{
	if ( szValue && szValue[0] )
	{
		for ( int i=0; i < MAX_NUM_CUSTOM_ATTRIBUTES; i++ )
		{
			if ( FStrEq( szKeyName, UTIL_VarArgs( "at%i", i ) ) )
			{
				m_nAttributes[i] = atoi( szValue );
				return true;
			}

			if ( FStrEq( szKeyName, UTIL_VarArgs( "atv%d", i ) ) )
			{
				m_nAttributesValue[i] = atof( szValue );
				return true;
			}
		}
	}

	return BaseClass::KeyValue( szKeyName, szValue );
}

//-----------------------------------------------------------------------------
// Purpose:  Override to get rid of EF_NODRAW
//-----------------------------------------------------------------------------
CBaseEntity* CTFLFEWeaponCustom::Respawn( void )
{
	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFLFEWeaponCustom::Materialize( void )
{

}

//-----------------------------------------------------------------------------
// Purpose:  
//-----------------------------------------------------------------------------
void CTFLFEWeaponCustom::EndTouch( CBaseEntity *pOther )
{
	CTFPlayer *pTFPlayer = dynamic_cast<CTFPlayer*>( pOther );

	if ( ValidTouch( pTFPlayer ) )
	{
		int iCurrentWeaponID = pTFPlayer->m_Shared.GetDesiredWeaponIndex();
		if ( iCurrentWeaponID == m_nItemID )
		{
			pTFPlayer->m_Shared.SetDesiredWeaponIndex( -1 );
		}
	}

}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFLFEWeaponCustom::MyTouch( CBasePlayer *pPlayer )
{
	bool bSuccess = false;

	CTFPlayer *pTFPlayer = ToTFPlayer( pPlayer );

	if ( ValidTouch( pTFPlayer ) )
	{
		int iSlot = m_Item.GetStaticData()->GetLoadoutSlot( TF_CLASS_COUNT_ALL );
		CTFWeaponBase *pWeapon = (CTFWeaponBase *)pTFPlayer->GetEntityForLoadoutSlot( iSlot );

		CTFWeaponBase *pActiveWeapon = (CTFWeaponBase *)pTFPlayer->GetActiveTFWeapon();
		if ( pActiveWeapon )
		{
			// Drop a usable weapon
			pTFPlayer->DropWeapon( pActiveWeapon );

			pActiveWeapon->UnEquip( pTFPlayer );
			pActiveWeapon = NULL;
		}

		if ( !pWeapon )
		{
			for ( int i = 0; i < MAX_NUM_CUSTOM_ATTRIBUTES; i++ )
			{
				CEconItemAttribute econAttribute( m_nAttributes[i], m_nAttributesValue[i] );
				m_bSkipBaseAttributes = m_Item.AddAttribute( &econAttribute );
			}

			m_Item.SkipBaseAttributes( m_bSkipBaseAttributes );

			const char* pszWeaponName = m_szItemClassname; // m_Item.GetEntityName();
			CTFWeaponBase *pNewWeapon = (CTFWeaponBase *)pTFPlayer->GiveNamedItem( pszWeaponName, 0, &m_Item );

			if ( pNewWeapon )
			{
				int iAmmoType = pNewWeapon->GetPrimaryAmmoType();
				pTFPlayer->SetAmmoCount( pTFPlayer->GetMaxAmmo( iAmmoType ), iAmmoType );
				//pNewWeapon->SetModel( m_szItemModelW );
				//pNewWeapon->DetermineViewModelType( m_szItemModelV );
				pNewWeapon->GiveTo( pTFPlayer );
				pTFPlayer->m_Shared.SetDesiredWeaponIndex( -1 );
				bSuccess = true;
			}
		}

		if ( bSuccess )
		{
			CSingleUserRecipientFilter user( pPlayer );
			user.MakeReliable();

			UserMessageBegin( user, "ItemPickup" );
				WRITE_STRING( GetClassname() );
			MessageEnd();

			pPlayer->EmitSound( "BaseCombatCharacter.AmmoPickup" );

			UTIL_Remove( this );
		}
	}

	return bSuccess;
}
