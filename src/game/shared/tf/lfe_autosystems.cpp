#include "cbase.h"
#include "GameEventListener.h"
#include <ctime>
#ifdef GAME_DLL
#include "networkstringtable_gamedll.h"
#endif

#ifdef CLIENT_DLL
	#include "physpropclientside.h"
	#include "c_te_legacytempents.h"
	#include "cdll_client_int.h"
	#include "c_soundscape.h"
	#include <engine/IEngineSound.h>
	#include "c_tf_player.h"

#ifdef _WIN32
	#define _WINREG_
	#undef ReadConsoleInput
	#undef INVALID_HANDLE_VALUE
	#undef GetCommandLine
	#include <Windows.h>
#endif // _WIN32
#else
	#include "info_camera_link.h"
	#include "point_camera.h"
	#include "utllinkedlist.h"
#endif // CLIENT_DLL

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#ifdef CLIENT_DLL
class CTFAutoSystems : public CAutoGameSystemPerFrame, public CGameEventListener
#else
class CTFAutoSystems : public CAutoGameSystem, public CGameEventListener
#endif
{
public:
#ifdef CLIENT_DLL
	CTFAutoSystems() : CAutoGameSystemPerFrame( "TFAutoSystems" ) 
	{
		fps_count = 0;
	}
#else
    CTFAutoSystems() : CAutoGameSystem( "TFAutoSystems" ) {}
#endif

    virtual void PostInit() OVERRIDE;
    virtual void LevelInitPostEntity() OVERRIDE;

#ifdef CLIENT_DLL
    void DoWindowsEffect();
	virtual void Update( float frametime ) OVERRIDE;
#else
	virtual void PreClientUpdate();
#endif

    virtual void FireGameEvent( IGameEvent* pEvent ) OVERRIDE;

#ifdef CLIENT_DLL
private:
	int fps_count;
	float init_timer;
#endif
};

void CTFAutoSystems::PostInit()
{
#ifdef CLIENT_DLL
    ListenForGameEvent( "teamplay_restart_round" );
    ListenForGameEvent( "teamplay_ready_restart" );
	ListenForGameEvent( "teamplay_game_over" );
	ListenForGameEvent( "server_spawn" );
	ListenForGameEvent( "tf_game_over" );
	ListenForGameEvent( "tf_changelevel" );
	ListenForGameEvent( "credits_outro_roll" );
#endif
}

void CTFAutoSystems::LevelInitPostEntity()
{
#ifdef CLIENT_DLL
	fps_count = 0;
#endif
}

void CTFAutoSystems::FireGameEvent( IGameEvent* pEvent )
{
#ifdef CLIENT_DLL
	if ( Q_strcmp( pEvent->GetName(), "teamplay_restart_round" ) == 0 || 
		 Q_strcmp( pEvent->GetName(), "teamplay_ready_restart" ) == 0 || 
		 Q_strcmp( pEvent->GetName(), "teamplay_game_over" ) == 0 ||
		 Q_strcmp( pEvent->GetName(), "tf_game_over" ) == 0 )
    {
        // Read client-side phys props from map and recreate em.
        C_PhysPropClientside::RecreateAll();

        tempents->Clear();
        
        // Stop sounds.
        enginesound->StopAllSounds( true );
        Soundscape_OnStopAllSounds();

        // Clear decals.
        engine->ClientCmd( "r_cleardecals" );

        // Remove client ragdolls since they don't like getting removed.
        C_ClientRagdoll* pRagdoll;
        for ( C_BaseEntity* pEnt = ClientEntityList().FirstBaseEntity(); pEnt; pEnt = ClientEntityList().NextBaseEntity( pEnt ) )
        {
            pRagdoll = dynamic_cast<C_ClientRagdoll*>( pEnt );
            if ( pRagdoll )
            {
                // This will make them fade out.
                pRagdoll->SUB_Remove();
            }
        }

		DoWindowsEffect();
    }
	else if ( Q_strcmp( pEvent->GetName(), "server_spawn" ) == 0 ||
			  Q_strcmp( pEvent->GetName(), "tf_changelevel" ) == 0 ||
			  Q_strcmp( pEvent->GetName(), "credits_outro_roll" ) == 0 )
		DoWindowsEffect();
#endif
}

#ifdef CLIENT_DLL
void CTFAutoSystems::DoWindowsEffect()
{
    bool bActive = engine->IsActiveApp();
	if ( !bActive )
	{
#ifdef _WIN32
		PlaySound( (LPCTSTR)SND_ALIAS_SYSTEMEXCLAMATION, NULL, SND_ALIAS_ID | SND_ASYNC );
#endif
		engine->FlashWindow();
	}
}

void CTFAutoSystems::Update( float frametime )
{
	// Do this only in-game
	if ( !gpGlobals->maxClients )
		return;
}
#else
void CTFAutoSystems::PreClientUpdate()
{
	for ( CPointCamera *pCameraEnt = GetPointCameraList(); pCameraEnt != NULL; pCameraEnt = pCameraEnt->m_pNext )
		pCameraEnt->SetActive( false );
}
#endif

CTFAutoSystems g_TFAutoSystems;
