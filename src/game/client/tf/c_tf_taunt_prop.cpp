//====== Copyright © 1996-2019, Valve Corporation, All rights reserved. =======
//
// Purpose: 
//
//=============================================================================

#include "cbase.h"
#include "c_tf_taunt_prop.h"
#include "particles_new.h"
#include "tf_gamerules.h"
#include "tempent.h"
#include "iefx.h"
#include "dlight.h"

IMPLEMENT_NETWORKCLASS_ALIASED( TFTauntProp, DT_TFTauntProp )

BEGIN_NETWORK_TABLE( C_TFTauntProp, DT_TFTauntProp )
END_NETWORK_TABLE()

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_TFTauntProp::C_TFTauntProp( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_TFTauntProp::~C_TFTauntProp( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool C_TFTauntProp::StartSceneEvent( CSceneEventInfo *info, CChoreoScene *scene, CChoreoEvent *event, CChoreoActor *actor, CBaseEntity *pTarget )
{
	if ( GetOwnerEntity() )
		info->m_hTarget = GetOwnerEntity();
	else
		info->m_hTarget = this;

	return BaseClass::StartSceneEvent( info, scene, event, actor, pTarget );
}