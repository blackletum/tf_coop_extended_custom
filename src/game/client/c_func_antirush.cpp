//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: func_antirush clientside thing cpp
//
//=============================================================================//

#include "cbase.h"
#include "c_func_antirush.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_CLIENTCLASS_DT(C_FuncAntirush, DT_FuncAntirush, CFuncAntirush)
END_RECV_TABLE()


void C_FuncAntirush::OnDataChanged(DataUpdateType_t type)
{
	BaseClass::OnDataChanged(type);

	bool bCreate = (type == DATA_UPDATE_CREATED) ? true : false;
	VPhysicsShadowDataChanged(bCreate, this);
}


