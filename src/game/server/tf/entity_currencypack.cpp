//====== Copyright © 1996-2020, Valve Corporation, All rights reserved. =======//
//
// Purpose: CTF Currencypack.
//
//=============================================================================//
#include "cbase.h"
#include "items.h"
#include "tf_shareddefs.h"
#include "tf_player.h"
#include "tf_team.h"
#include "engine/IEngineSound.h"
#include "entity_currencypack.h"
#include "tf_gamestats.h"
#include "ai_basenpc.h"

//=============================================================================
//
// CTF Currencypack defines.
//

#define TF_CURRENCY_PICKUP_SOUND	"MVM.MoneyPickup"
#define TF_CURRENCY_VANISH_SOUND	"MVM.MoneyVanish"

IMPLEMENT_SERVERCLASS_ST( CCurrencyPack, DT_CurrencyPack )
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( item_currencypack_custom, CCurrencyPack );
LINK_ENTITY_TO_CLASS( item_currencypack_large, CCurrencyPack );
LINK_ENTITY_TO_CLASS( item_currencypack_small, CCurrencyPackSmall );
LINK_ENTITY_TO_CLASS( item_currencypack_medium, CCurrencyPackMedium );

IMPLEMENT_AUTO_LIST( ICurrencyPackAutoList );

CCurrencyPack::CCurrencyPack()
{
}

//-----------------------------------------------------------------------------
// Purpose: Spawn function for the currencypack
//-----------------------------------------------------------------------------
void CCurrencyPack::Spawn( void )
{
	Precache();
	SetModel( GetDefaultPowerupModel() );

	BaseClass::Spawn();
}

//-----------------------------------------------------------------------------
// Purpose: Precache function for the currencypack
//-----------------------------------------------------------------------------
void CCurrencyPack::Precache( void )
{
	PrecacheModel( GetDefaultPowerupModel() );

	PrecacheScriptSound( TF_CURRENCY_PICKUP_SOUND );
	PrecacheScriptSound( TF_CURRENCY_VANISH_SOUND );
}

//-----------------------------------------------------------------------------
// Purpose: MyTouch function for the currencypack
//-----------------------------------------------------------------------------
bool CCurrencyPack::MyTouch( CBasePlayer *pPlayer )
{
	bool bSuccess = false;

	if ( ValidTouch( pPlayer ) )
	{
		CTFPlayer *pTFPlayer = ToTFPlayer( pPlayer );
		Assert( pTFPlayer );

		int iMoneyToAdd = ceil( 500 * PackRatios[GetPowerupSize()] );
		CALL_ATTRIB_HOOK_INT_ON_OTHER( pTFPlayer, iMoneyToAdd, currency_bonus );

		if ( pTFPlayer != GetOwnerEntity() )
		{
			pTFPlayer->m_Shared.SetCurrency( pTFPlayer->m_Shared.GetCurrency() + iMoneyToAdd );
			bSuccess = true;
		}

		if ( bSuccess )
		{
			CSingleUserRecipientFilter user( pPlayer );
			user.MakeReliable();

			UserMessageBegin( user, "ItemPickup" );
			WRITE_STRING( GetClassname() );
			MessageEnd();

			EmitSound( user, entindex(), TF_CURRENCY_PICKUP_SOUND );

			IGameEvent *event = gameeventmanager->CreateEvent( "mvm_pickup_currency" );
			if ( event )
			{
				event->SetInt( "player", pPlayer->entindex() );
				event->SetInt( "currency", iMoneyToAdd );
				gameeventmanager->FireEvent( event );
			}
		}
	}

	return bSuccess;
}

//-----------------------------------------------------------------------------
// Purpose: MyTouch function for npc
//-----------------------------------------------------------------------------
bool CCurrencyPack::NPCTouch( CAI_BaseNPC *pNPC )
{
	return false;
}