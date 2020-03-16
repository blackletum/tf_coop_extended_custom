//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#include "cbase.h"
#include "baseentity.h"
#include "tf_gamerules.h"
#include "tf_team.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CTFHudNotify : public CPointEntity
{
public:
	DECLARE_CLASS( CTFHudNotify, CPointEntity );
	DECLARE_DATADESC();

	void InputDisplay( inputdata_t &inputdata );
	void Display( CBaseEntity *pActivator );

private:
	string_t m_iszMessage;
	string_t m_iszIcon;
	int m_iRecipientTeam;
	int m_iBackgroundTeam;
};

LINK_ENTITY_TO_CLASS( game_text_tf, CTFHudNotify );

BEGIN_DATADESC( CTFHudNotify )

DEFINE_KEYFIELD( m_iszMessage, FIELD_STRING, "message" ),
DEFINE_KEYFIELD( m_iszIcon, FIELD_STRING, "icon" ),
DEFINE_KEYFIELD( m_iRecipientTeam, FIELD_INTEGER, "display_to_team" ),
DEFINE_KEYFIELD( m_iBackgroundTeam, FIELD_INTEGER, "background" ),

// Inputs
DEFINE_INPUTFUNC( FIELD_VOID, "Display", InputDisplay ),

END_DATADESC()


void CTFHudNotify::InputDisplay( inputdata_t &inputdata )
{
	Display( inputdata.pActivator );
}

void CTFHudNotify::Display( CBaseEntity *pActivator )
{
	CBroadcastRecipientFilter filter;

	switch( m_iRecipientTeam )
	{
	case TF_TEAM_RED:
		filter.RemoveRecipientsByTeam( GetGlobalTeam(TF_TEAM_BLUE) );
		break;

	case TF_TEAM_BLUE:
		filter.RemoveRecipientsByTeam( GetGlobalTeam(TF_TEAM_RED) );
		break;
	}

	TFGameRules()->SendHudNotification( filter, STRING(m_iszMessage), STRING(m_iszIcon), m_iBackgroundTeam );
}

class CTFDeathNoticeNotify : public CPointEntity
{
public:
	DECLARE_CLASS( CTFDeathNoticeNotify, CPointEntity );
	DECLARE_DATADESC();

	void InputDisplay( inputdata_t &inputdata );
	void Display( CBaseEntity *pActivator );

private:
	string_t m_iszMessage;
	string_t m_iszIcon;
	int m_iBackgroundTeam;
	bool m_bCrit;
};

LINK_ENTITY_TO_CLASS( lfe_deathnotice_text, CTFDeathNoticeNotify );

BEGIN_DATADESC( CTFDeathNoticeNotify )

DEFINE_KEYFIELD( m_iszMessage, FIELD_STRING, "message" ),
DEFINE_KEYFIELD( m_iszIcon, FIELD_STRING, "icon" ),
DEFINE_KEYFIELD( m_iBackgroundTeam, FIELD_INTEGER, "background" ),
DEFINE_KEYFIELD( m_bCrit, FIELD_BOOLEAN, "background" ),

// Inputs
DEFINE_INPUTFUNC( FIELD_VOID, "Display", InputDisplay ),

END_DATADESC()


void CTFDeathNoticeNotify::InputDisplay( inputdata_t &inputdata )
{
	Display( inputdata.pActivator );
}

void CTFDeathNoticeNotify::Display( CBaseEntity *pActivator )
{
	IGameEvent *event = gameeventmanager->CreateEvent( "lfe_deathnotice_text" );
	if ( event )
	{
		event->SetInt( "id", entindex() );
		event->SetString( "text", STRING( m_iszMessage ) );
		event->SetString( "icon", STRING( m_iszIcon ) );
		event->SetInt( "team", m_iBackgroundTeam );
		event->SetInt( "crit", m_bCrit );
		event->SetInt( "priority", 9 );

		gameeventmanager->FireEvent( event );
	}
}
