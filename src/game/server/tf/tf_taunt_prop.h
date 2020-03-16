//====== Copyright © 1996-2005, Valve Corporation, All rights reserved. =======//
//
// Purpose:
//
//=============================================================================//
#ifndef TF_TAUNT_PROP
#define TF_TAUNT_PROP
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include "basecombatcharacter.h"

class CTFTauntProp : public CBaseCombatCharacter
{
public:
	DECLARE_CLASS( CTFTauntProp, CBaseCombatCharacter );
	DECLARE_SERVERCLASS();

	CTFTauntProp();
	~CTFTauntProp();

	//virtual void	UpdateOnRemove( void );

	virtual float	PlayScene( const char *pszScene, float flDelay = 0.0f, AI_Response *response = NULL, IRecipientFilter *filter = NULL );
	virtual	bool	StartSceneEvent( CSceneEventInfo *info, CChoreoScene *scene, CChoreoEvent *event, CChoreoActor *actor, CBaseEntity *pTarget );
	virtual bool	ProcessSceneEvent( CSceneEventInfo *info, CChoreoScene *scene, CChoreoEvent *event );
};

#endif