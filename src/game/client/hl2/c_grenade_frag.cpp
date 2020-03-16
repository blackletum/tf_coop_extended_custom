//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Client side frag
//
//=============================================================================

#include "cbase.h"
#include "dlight.h"
#include "iefx.h"
#include "basegrenade_shared.h"
#include "particle_parse.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class C_GrenadeFrag : public CBaseGrenade
{
public:
	C_GrenadeFrag() {}
	~C_GrenadeFrag();

	DECLARE_CLASS( C_GrenadeFrag, CBaseGrenade );
	DECLARE_CLIENTCLASS();
 	DECLARE_DATADESC();

	virtual void OnDataChanged( DataUpdateType_t type );
	virtual void ClientThink();
	virtual void CreateTrails( void );

private:
	bool m_bCritical;
};


//-----------------------------------------------------------------------------
// Save/restore
//-----------------------------------------------------------------------------
BEGIN_DATADESC( C_GrenadeFrag )
END_DATADESC()


//-----------------------------------------------------------------------------
// Networking
//-----------------------------------------------------------------------------
IMPLEMENT_CLIENTCLASS_DT(C_GrenadeFrag, DT_GrenadeFrag, CGrenadeFrag)
	RecvPropBool( RECVINFO( m_bCritical ) )
END_RECV_TABLE()

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_GrenadeFrag::~C_GrenadeFrag()
{
	ParticleProp()->StopEmission();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_GrenadeFrag::CreateTrails( void )
{
	const char *pszEffectName = ConstructTeamParticle( "critical_pipe_%s", GetTeamNumber() );
	ParticleProp()->Create( pszEffectName, PATTACH_ABSORIGIN_FOLLOW );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void C_GrenadeFrag::OnDataChanged( DataUpdateType_t type )
{
	BaseClass::OnDataChanged( type );

	if ( type == DATA_UPDATE_CREATED )
	{
		if ( m_bCritical )
			CreateTrails();
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void C_GrenadeFrag::ClientThink()
{
	BaseClass::ClientThink();
}
