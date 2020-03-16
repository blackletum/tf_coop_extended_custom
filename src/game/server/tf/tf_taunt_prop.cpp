//====== Copyright © 1996-2019, Valve Corporation, All rights reserved. =======//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "tf_taunt_prop.h"
#include "tf_gamerules.h"
#include "in_buttons.h"
#include "sceneentity.h"
#include "choreoactor.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_SERVERCLASS_ST( CTFTauntProp, DT_TFTauntProp )
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( tf_taunt_prop, CTFTauntProp );

CTFTauntProp::CTFTauntProp()
{
}

CTFTauntProp::~CTFTauntProp()
{
}

float CTFTauntProp::PlayScene( const char *pszScene, float flDelay, AI_Response *response, IRecipientFilter *filter )
{
	return InstancedScriptedScene( this, pszScene, NULL, flDelay, false, response, true, filter );
}

bool CTFTauntProp::StartSceneEvent( CSceneEventInfo *info, CChoreoScene *scene, CChoreoEvent *event, CChoreoActor *actor, CBaseEntity *pTarget )
{
	if ( GetOwnerEntity() )
		info->m_hTarget = GetOwnerEntity();
	else
		info->m_hTarget = this;

	SetName( MAKE_STRING( info->m_pActor->GetName() ) );
	return BaseClass::StartSceneEvent( info, scene, event, actor, pTarget );
}

bool CTFTauntProp::ProcessSceneEvent( CSceneEventInfo *info, CChoreoScene *scene, CChoreoEvent *event )
{
	if ( GetOwnerEntity() )
		info->m_hTarget = GetOwnerEntity();
	else
		info->m_hTarget = this;

	return BaseClass::ProcessSceneEvent( info, scene, event );
}