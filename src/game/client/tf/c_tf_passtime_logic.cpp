//Copyright, Valve Corporation,Bad Robot,Escalation Studios, All rights reserved//
//
// Purpose: CTF Passtime Logic.
//
//=============================================================================//
#include "cbase.h"
#include "c_tf_passtime_logic.h"
#include "c_tf_player.h"
#include "engine/IEngineSound.h"
#include "soundenvelope.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// Network table.
IMPLEMENT_CLIENTCLASS_DT( C_TFPasstimeLogic, DT_TFPasstimeLogic, CTFPasstimeLogic )
END_RECV_TABLE()

C_TFPasstimeLogic::C_TFPasstimeLogic()
{
}

C_TFPasstimeLogic::~C_TFPasstimeLogic()
{
}
