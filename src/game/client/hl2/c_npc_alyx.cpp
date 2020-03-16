//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Client side frag
//
//=============================================================================

#include "cbase.h"
#include "dlight.h"
#include "iefx.h"
#include "c_ai_basenpc.h"
#include "tf_gamerules.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class C_NPC_Alyx : public C_AI_BaseNPC
{
public:
	C_NPC_Alyx();
	~C_NPC_Alyx();

	DECLARE_CLASS( C_NPC_Alyx, C_AI_BaseNPC );
	DECLARE_CLIENTCLASS();
 	DECLARE_DATADESC();

	virtual void OnDataChanged( DataUpdateType_t type );
	virtual void ClientThink();
};


//-----------------------------------------------------------------------------
// Save/restore
//-----------------------------------------------------------------------------
BEGIN_DATADESC( C_NPC_Alyx )
END_DATADESC()


//-----------------------------------------------------------------------------
// Networking
//-----------------------------------------------------------------------------
IMPLEMENT_CLIENTCLASS_DT( C_NPC_Alyx, DT_NPC_Alyx, CNPC_Alyx )
END_RECV_TABLE()

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_NPC_Alyx::C_NPC_Alyx()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_NPC_Alyx::~C_NPC_Alyx()
{
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void C_NPC_Alyx::OnDataChanged( DataUpdateType_t type )
{
	BaseClass::OnDataChanged( type );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void C_NPC_Alyx::ClientThink()
{
	BaseClass::ClientThink();

	if ( IsAlive() && ( TFGameRules()->IsInHL2EP1Map() || TFGameRules()->IsInHL2EP2Map() ) )
	{
		SetClientSideGlowEnabled( true );
		if ( GetHealth() < 60 )
		{
			GetGlowObject()->SetColor( Vector( 255, 128, 64 ) );
		}
		else if ( GetHealth() < 20 )
		{
			GetGlowObject()->SetColor( Vector( 255, 0, 0 ) );
		}
		else if ( GetHealth() > 60 )
		{
			SetClientSideGlowEnabled( false );
			DestroyGlowEffect();
		}
		else if ( GetHealth() > 20 )
		{
			GetGlowObject()->SetColor( Vector( 255, 128, 64 ) );
		}
		else
		{
			SetClientSideGlowEnabled( false );
			DestroyGlowEffect();
		}
	}

	if ( !IsAlive() )
	{
		SetClientSideGlowEnabled( false );
		DestroyGlowEffect();
	}
}
