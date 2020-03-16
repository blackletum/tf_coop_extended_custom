//====== Copyright © 1996-2020, Valve Corporation, All rights reserved. =======//
//
// Purpose: 
//
//=============================================================================//
#include "cbase.h"
#include "c_baseanimating.h"
#include "engine/ivdebugoverlay.h"
#include "c_tf_player.h"
#include "engine/IEngineSound.h"
#include "soundenvelope.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class C_CurrencyPack : public C_BaseAnimating, public ITargetIDProvidesHint
{
public:
	DECLARE_CLASS( C_CurrencyPack, C_BaseAnimating );
	DECLARE_CLIENTCLASS();

	~C_CurrencyPack();

	//virtual void	ClientThink( void );
	virtual void	OnDataChanged( DataUpdateType_t updateType );


	// ITargetIDProvidesHint
public:
	virtual void	DisplayHintTo( C_BasePlayer *pPlayer );
};

// Network table.
IMPLEMENT_CLIENTCLASS_DT( C_CurrencyPack, DT_CurrencyPack, CCurrencyPack )
END_RECV_TABLE()

C_CurrencyPack::~C_CurrencyPack()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : updateType - 
//-----------------------------------------------------------------------------
void C_CurrencyPack::OnDataChanged( DataUpdateType_t updateType )
{
	BaseClass::OnDataChanged( updateType );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pPlayer - 
//-----------------------------------------------------------------------------
void C_CurrencyPack::DisplayHintTo( C_BasePlayer *pPlayer )
{
	C_TFPlayer *pTFPlayer = ToTFPlayer(pPlayer);
	if ( pTFPlayer->IsPlayerClass( TF_CLASS_ENGINEER ) )
	{
		pTFPlayer->HintMessage( HINT_ENGINEER_PICKUP_METAL );
	}
	else
	{
		pTFPlayer->HintMessage( HINT_PICKUP_AMMO );
	}
}