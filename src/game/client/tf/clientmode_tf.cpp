//=========== (C) Copyright 1999 Valve, L.L.C. All rights reserved. ===========
//
// The copyright to the contents herein is the property of Valve, L.L.C.
// The contents may be used and/or copied only with the written permission of
// Valve, L.L.C., or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// $Header: $
// $NoKeywords: $
//
//=============================================================================
#include "cbase.h"
#include "hud.h"
#include "hud_macros.h"
#include "clientmode_tf.h"
#include "cdll_client_int.h"
#include "iinput.h"
#include <vgui/IInput.h>
#include <vgui/ISurface.h>
#include "vgui/IPanel.h"
#include "GameUI/IGameUI.h"
#include <vgui_controls/AnimationController.h>
#include "ivmodemanager.h"
#include "buymenu.h"
#include "filesystem.h"
#include "vgui/IVGui.h"
#include "view_shared.h"
#include "view.h"
#include "ivrenderview.h"
#include "model_types.h"
#include "iefx.h"
#include "dlight.h"
#include <imapoverview.h>
#include "c_playerresource.h"
#include <KeyValues.h>
#include "text_message.h"
#include "panelmetaclassmgr.h"
#include "c_tf_player.h"
#include "ienginevgui.h"
#include "in_buttons.h"
#include "voice_status.h"
#include "tf_hud_menu_engy_build.h"
#include "tf_hud_menu_engy_destroy.h"
#include "tf_hud_menu_spy_disguise.h"
#include "tf_hud_menu_taunt_selection.h"
#include "tf_statsummary.h"
#include "tf_hud_freezepanel.h"
#include "clienteffectprecachesystem.h"
#include "glow_outline_effect.h"
#include "cam_thirdperson.h"
#include "tf_gamerules.h"
#include "tf_hud_chat.h"
#include "c_tf_team.h"
#include "c_tf_playerresource.h"
#include "usermessages.h"
#include "utlvector.h"
#include "props_shared.h"
//#include "lfe_discordrpc.h"

#include "c_basehlplayer.h"

extern ConVar r_drawviewmodel;

ConVar default_fov( "default_fov", "75", FCVAR_CHEAT );
ConVar fov_desired( "fov_desired", "90", FCVAR_ARCHIVE | FCVAR_USERINFO, "Sets the base field-of-view.", true, 75.0, true, 130.0 );

ConVar tf_hud_no_crosshair_on_scope_zoom( "tf_hud_no_crosshair_on_scope_zoom", "1", FCVAR_ARCHIVE | FCVAR_CLIENTDLL );

void HUDMinModeChangedCallBack( IConVar *var, const char *pOldString, float flOldValue )
{
	engine->ExecuteClientCmd( "hud_reloadscheme" );
}

extern CBaseEntity *BreakModelCreateSingle( CBaseEntity *pOwner, breakmodel_t *pModel, const Vector &position, 
										   const QAngle &angles, const Vector &velocity, const AngularImpulse &angVelocity, int nSkin, const breakablepropparams_t &params );

ConVar cl_hud_minmode( "cl_hud_minmode", "0", FCVAR_ARCHIVE, "Set to 1 to turn on the advanced minimalist HUD mode.", HUDMinModeChangedCallBack );

IClientMode *g_pClientMode = NULL;
// --------------------------------------------------------------------------------- //
// CTFModeManager.
// --------------------------------------------------------------------------------- //

class CTFModeManager : public IVModeManager
{
public:
	virtual void	Init();
	virtual void	SwitchMode( bool commander, bool force ) {}
	virtual void	LevelInit( const char *newmap );
	virtual void	LevelShutdown( void );
	virtual void	ActivateMouse( bool isactive ) {}
};

static CTFModeManager g_ModeManager;
IVModeManager *modemanager = ( IVModeManager * )&g_ModeManager;

CLIENTEFFECT_REGISTER_BEGIN( PrecachePostProcessingEffectsGlow )
	CLIENTEFFECT_MATERIAL( "dev/glow_blur_x" )
	CLIENTEFFECT_MATERIAL( "dev/glow_blur_y" )
	CLIENTEFFECT_MATERIAL( "dev/glow_color" )
	CLIENTEFFECT_MATERIAL( "dev/glow_downsample" )
	CLIENTEFFECT_MATERIAL( "dev/halo_add_to_screen" )
	CLIENTEFFECT_MATERIAL( "debug/debugfbtexture1" )
CLIENTEFFECT_REGISTER_END_CONDITIONAL(	engine->GetDXSupportLevel() >= 90 )

// --------------------------------------------------------------------------------- //
// CTFModeManager implementation.
// --------------------------------------------------------------------------------- //

#define SCREEN_FILE		"scripts/vgui_screens.txt"

void CTFModeManager::Init()
{
	g_pClientMode = GetClientModeNormal();
	
	PanelMetaClassMgr()->LoadMetaClassDefinitionFile( SCREEN_FILE );

	// Load the objects.txt file.
	LoadObjectInfos( ::filesystem );

	//MapData().Init();

	GetClientVoiceMgr()->SetHeadLabelOffset( 40 );
}

void CTFModeManager::LevelInit( const char *newmap )
{
	g_pClientMode->LevelInit( newmap );

	ConVarRef voice_steal( "voice_steal" );

	if ( voice_steal.IsValid() )
	{
		voice_steal.SetValue( 1 );
	}

	g_ThirdPersonManager.Init();

	//g_discordrpc.LevelInit( newmap );
}

void CTFModeManager::LevelShutdown( void )
{
	g_pClientMode->LevelShutdown();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
ClientModeTFNormal::ClientModeTFNormal()
{
	m_pMenuEngyBuild = NULL;
	m_pMenuEngyDestroy = NULL;
	m_pMenuSpyDisguise = NULL;
	m_pGameUI = NULL;
	m_pFreezePanel = NULL;
	m_pTauntSelection = NULL;
	MessageHooks();
}

//-----------------------------------------------------------------------------
// Purpose: If you don't know what a destructor is by now, you are probably going to get fired
//-----------------------------------------------------------------------------
ClientModeTFNormal::~ClientModeTFNormal()
{
}

// See interface.h/.cpp for specifics:  basically this ensures that we actually Sys_UnloadModule the dll and that we don't call Sys_LoadModule 
//  over and over again.
static CDllDemandLoader g_GameUI( "gameui" );

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void ClientModeTFNormal::Init()
{
	m_pMenuEngyBuild = ( CHudMenuEngyBuild * )GET_HUDELEMENT( CHudMenuEngyBuild );
	Assert( m_pMenuEngyBuild );

	m_pMenuEngyDestroy = ( CHudMenuEngyDestroy * )GET_HUDELEMENT( CHudMenuEngyDestroy );
	Assert( m_pMenuEngyDestroy );

	m_pMenuSpyDisguise = ( CHudMenuSpyDisguise * )GET_HUDELEMENT( CHudMenuSpyDisguise );
	Assert( m_pMenuSpyDisguise );

	m_pFreezePanel = ( CTFFreezePanel * )GET_HUDELEMENT( CTFFreezePanel );
	Assert( m_pFreezePanel );

	m_pTauntSelection = ( CHudMenuTauntSelection * )GET_HUDELEMENT( CHudMenuTauntSelection );
	Assert( m_pTauntSelection );

	CreateInterfaceFn gameUIFactory = g_GameUI.GetFactory();
	if ( gameUIFactory )
	{
		m_pGameUI = (IGameUI *) gameUIFactory(GAMEUI_INTERFACE_VERSION, NULL );
		if ( NULL != m_pGameUI )
		{
			// insert stats summary panel.
			CTFStatsSummaryPanel *pStatsPanel = GStatsSummaryPanel();
			pStatsPanel->InvalidateLayout( false, true );
			pStatsPanel->SetVisible( false );
			pStatsPanel->MakePopup( false );
			m_pGameUI->SetLoadingBackgroundDialog( pStatsPanel->GetVPanel() );
		}		
	}

	BaseClass::Init();

	ListenForGameEvent( "pumpkin_lord_summoned" );
	ListenForGameEvent( "pumpkin_lord_killed" );
	ListenForGameEvent( "eyeball_boss_summoned" );
	ListenForGameEvent( "eyeball_boss_stunned" );
	ListenForGameEvent( "eyeball_boss_killed" );
	ListenForGameEvent( "eyeball_boss_killer" );
	ListenForGameEvent( "eyeball_boss_escape_imminent" );
	ListenForGameEvent( "eyeball_boss_escaped" );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void ClientModeTFNormal::Shutdown()
{
	DestroyStatsSummaryPanel();
}

void ClientModeTFNormal::InitViewport()
{
	m_pViewport = new TFViewport();
	m_pViewport->Start( gameuifuncs, gameeventmanager );
}

ClientModeTFNormal g_ClientModeNormal;

IClientMode *GetClientModeNormal()
{
	return &g_ClientModeNormal;
}


ClientModeTFNormal* GetClientModeTFNormal()
{
	Assert( dynamic_cast< ClientModeTFNormal* >( GetClientModeNormal() ) );

	return static_cast< ClientModeTFNormal* >( GetClientModeNormal() );
}

// let us drive the fixed camera position around
static bool s_bFixedInputActive = false;
static int s_nOldCursor[2] = {0, 0};
void Handle_Fixed_Input( bool active )
{
	if ( s_bFixedInputActive ^ active )
	{
		if ( s_bFixedInputActive && !active )
		{
			// Restore mouse
			vgui::input()->SetCursorPos( s_nOldCursor[0], s_nOldCursor[1] );
		}
		else
		{
			// todO: set the initial fixed vectors here..
			vgui::input()->GetCursorPos( s_nOldCursor[0], s_nOldCursor[1] );
		}
	}

	if ( active )
	{
		int mx, my;
		int dx, dy;

		vgui::input()->GetCursorPos( mx, my );

		dx = mx - s_nOldCursor[0];
		dy = my - s_nOldCursor[1];

		vgui::input()->SetCursorPos( s_nOldCursor[0], s_nOldCursor[1] );
	}

	s_bFixedInputActive = active;
}

//-----------------------------------------------------------------------------
// Purpose: Fixes some bugs from base class.
//-----------------------------------------------------------------------------
void ClientModeTFNormal::OverrideView( CViewSetup *pSetup )
{
	QAngle camAngles;

	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if ( !pPlayer )
		return;

	// Let the player override the view.
	pPlayer->OverrideView( pSetup );

	if ( ::input->CAM_IsThirdPerson() && !pPlayer->IsObserver() )
	{
		const Vector& cam_ofs = g_ThirdPersonManager.GetCameraOffsetAngles();
		Vector cam_ofs_distance;

		if ( g_ThirdPersonManager.IsOverridingThirdPerson() )
		{
			cam_ofs_distance = g_ThirdPersonManager.GetDesiredCameraOffset();
		}
		else
		{
			cam_ofs_distance = g_ThirdPersonManager.GetFinalCameraOffset();
		}

		cam_ofs_distance *= g_ThirdPersonManager.GetDistanceFraction();

		camAngles[PITCH] = cam_ofs[PITCH];
		camAngles[YAW] = cam_ofs[YAW];
		camAngles[ROLL] = 0;

		Vector camForward, camRight, camUp;

		if ( g_ThirdPersonManager.IsOverridingThirdPerson() == false )
		{
			engine->GetViewAngles( camAngles );
		}

		// get the forward vector
		AngleVectors( camAngles, &camForward, &camRight, &camUp );

		VectorMA( pSetup->origin, -cam_ofs_distance[0], camForward, pSetup->origin );
		VectorMA( pSetup->origin, cam_ofs_distance[1], camRight, pSetup->origin );
		VectorMA( pSetup->origin, cam_ofs_distance[2], camUp, pSetup->origin );

		// Override angles from third person camera
		VectorCopy( camAngles, pSetup->angles );
	}
	else if ( ::input->CAM_IsOrthographic() )
	{
		pSetup->m_bOrtho = true;
		float w, h;
		::input->CAM_OrthographicSize( w, h );
		w *= 0.5f;
		h *= 0.5f;
		pSetup->m_OrthoLeft = -w;
		pSetup->m_OrthoTop = -h;
		pSetup->m_OrthoRight = w;
		pSetup->m_OrthoBottom = h;
	}

	Handle_Fixed_Input( vgui::input()->IsKeyDown( KEY_LSHIFT ) && vgui::input()->IsMouseDown( MOUSE_LEFT ) );
}

extern ConVar v_viewmodel_fov;
float g_flViewModelFOV = 75;
float ClientModeTFNormal::GetViewModelFOV( void )
{
	return v_viewmodel_fov.GetFloat();
//	return g_flViewModelFOV;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool ClientModeTFNormal::ShouldDrawViewModel()
{
	C_TFPlayer *pPlayer = C_TFPlayer::GetLocalTFPlayer();
	if ( pPlayer )
	{
		if ( pPlayer->m_Shared.InCond( TF_COND_ZOOMED ) )
			return false;
	}

	return r_drawviewmodel.GetBool();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool ClientModeTFNormal::ShouldDrawCrosshair( void )
{
	C_TFPlayer *pPlayer = C_TFPlayer::GetLocalTFPlayer();
	if ( pPlayer )
	{
		if ( pPlayer->m_Shared.InCond( TF_COND_ZOOMED ) )
			return tf_hud_no_crosshair_on_scope_zoom.GetBool();
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool ClientModeTFNormal::DoPostScreenSpaceEffects( const CViewSetup *pSetup )
{
	if ( !IsInFreezeCam() )
		g_GlowObjectManager.RenderGlowEffects( pSetup, 0 );

	return BaseClass::DoPostScreenSpaceEffects( pSetup );
}

int ClientModeTFNormal::GetDeathMessageStartHeight( void )
{
	return m_pViewport->GetDeathMessageStartHeight();
}


bool IsInCommentaryMode( void );
bool PlayerNameNotSetYet( const char *pszName );

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void ClientModeTFNormal::FireGameEvent( IGameEvent *event )
{
	const char *eventname = event->GetName();

	if ( !eventname || !eventname[0] )
		return;

	if (FStrEq( eventname, "pumpkin_lord_summoned" ))
	{
		C_BasePlayer *pLocal = C_BasePlayer::GetLocalPlayer();
		if ( pLocal )
			pLocal->EmitSound( "Halloween.HeadlessBossSpawn" );
	}
	else if (FStrEq( eventname, "pumpkin_lord_killed" ))
	{
		C_BasePlayer *pLocal = C_BasePlayer::GetLocalPlayer();
		if ( pLocal )
			pLocal->EmitSound( "Halloween.HeadlessBossDeath" );
	}
	else if ( FStrEq( eventname, "eyeball_boss_summoned" ) )
	{
		C_BasePlayer *pLocal = C_BasePlayer::GetLocalPlayer();
		if ( pLocal )
			pLocal->EmitSound( "Halloween.MonoculusBossSpawn" );
	}
	else if ( FStrEq( eventname, "eyeball_boss_stunned" ) )
	{
		CHudChat *pChat = GET_HUDELEMENT( CHudChat );
		int iLevel = event->GetInt( "level" );

		if ( pChat )
		{
			pChat->SetCustomColor( COLOR_EYEBALLBOSS_TEXT );

			KeyValues *kv = new KeyValues( "data" );
			kv->SetString( "player", g_PR->GetPlayerName( event->GetInt( "player_entindex" ) ) );
			wchar_t *loc;

			if ( iLevel < 2 )
			{
				loc = g_pVGuiLocalize->Find( "#TF_Halloween_Eyeball_Boss_Stun" );
			}
			else
			{
				kv->SetInt( "level", iLevel );
				loc = g_pVGuiLocalize->Find( "#TF_Halloween_Eyeball_Boss_LevelUp_Stun" );
			}

			wchar_t unicode[256];
			char ansi[256];
			g_pVGuiLocalize->ConstructString( unicode, sizeof( unicode ), loc, kv );
			g_pVGuiLocalize->ConvertUnicodeToANSI( unicode, ansi, sizeof( ansi ) );

			pChat->ChatPrintf( event->GetInt( "player_entindex" ), CHAT_FILTER_NONE, "%s", ansi );

			if ( kv )
			{
				kv->deleteThis();
			}
		}
	}
	else if ( FStrEq( eventname, "eyeball_boss_killed" ) )
	{
		C_BasePlayer *pLocal = C_BasePlayer::GetLocalPlayer();
		if ( pLocal )
			pLocal->EmitSound( "Halloween.HeadlessBossDeath" );
	}
	else if ( FStrEq( eventname, "eyeball_boss_killer" ) )
	{
		CHudChat *pChat = GET_HUDELEMENT( CHudChat );
		int iLevel = event->GetInt( "level" );

		if ( pChat )
		{
			pChat->SetCustomColor( COLOR_EYEBALLBOSS_TEXT );

			KeyValues *kv = new KeyValues( "data" );
			kv->SetString( "player", g_PR->GetPlayerName( event->GetInt( "player_entindex" ) ) );
			wchar_t *loc;

			if ( iLevel < 2 )
			{
				loc = g_pVGuiLocalize->Find( "#TF_Halloween_Eyeball_Boss_Killers" );
			}
			else
			{
				kv->SetInt( "level", iLevel );
				loc = g_pVGuiLocalize->Find( "#TF_Halloween_Eyeball_Boss_LevelUp_Killers" );
			}

			wchar_t unicode[256];
			char ansi[256];
			g_pVGuiLocalize->ConstructString( unicode, sizeof( unicode ), loc, kv );
			g_pVGuiLocalize->ConvertUnicodeToANSI( unicode, ansi, sizeof( ansi ) );

			pChat->ChatPrintf( event->GetInt( "player_entindex" ), CHAT_FILTER_NONE, "%s", ansi );

			if ( kv )
			{
				kv->deleteThis();
			}
		}
	}
	else if ( FStrEq( eventname, "eyeball_boss_escape_imminent" ) )
	{
		int iTimeLeft = event->GetInt( "time_remaining" );
		C_BasePlayer *pLocal = C_BasePlayer::GetLocalPlayer();
		if ( pLocal )
		{
			if ( iTimeLeft > 10 )
				pLocal->EmitSound( "Halloween.EyeballBossEscapeSoon" );
			else
				pLocal->EmitSound( "Halloween.EyeballBossEscapeImminent" );
		}

		CHudChat *pChat = GET_HUDELEMENT( CHudChat );
		int iLevel = event->GetInt( "level" );

		pChat->SetCustomColor( COLOR_EYEBALLBOSS_TEXT );

		KeyValues *kv = new KeyValues( "data" );
		kv->SetString( "player", g_PR->GetPlayerName( event->GetInt( "player_entindex" ) ) );
		wchar_t *loc;
		wchar_t unicode[256];
		char buf[64], ansi[256];

		if ( iLevel >= 2 )
		{
			kv->SetInt( "level", iLevel );
		}

		// Construct a string for the time left
		Q_snprintf( buf, sizeof( buf ), "#TF_Halloween_Eyeball_Boss%s_Escaping_In_%i", ( iLevel < 2 ) ? "" : "_LevelUp", iTimeLeft );
		loc = g_pVGuiLocalize->Find( buf );

		g_pVGuiLocalize->ConstructString( unicode, sizeof( unicode ), loc, kv );
		g_pVGuiLocalize->ConvertUnicodeToANSI( unicode, ansi, sizeof( ansi ) );

		pChat->ChatPrintf( 0, CHAT_FILTER_NONE, "%s", ansi );

		if ( kv )
		{
			kv->deleteThis();
		}

	}
	else if ( FStrEq( eventname, "eyeball_boss_escaped" ) )
	{
		C_BasePlayer *pLocal = C_BasePlayer::GetLocalPlayer();
		if ( pLocal )
			pLocal->EmitSound( "Halloween.EyeballBossEscaped" );
	}
	else if ( FStrEq( eventname, "player_team" ) )
	{
		// Using our own strings here.
		int iPlayerIndex = engine->GetPlayerForUserID( event->GetInt( "userid" ) );

		CHudChat *pChat = GetTFChatHud();
		if ( !pChat )
			return;

		bool bDisconnected = event->GetBool( "disconnect" );

		if ( bDisconnected )
			return;

		int iTeam = event->GetInt( "team" );
		bool bAutoTeamed = event->GetBool( "autoteam" );
		bool bSilent = event->GetBool( "silent" );

		const char *pszName = event->GetString( "name" );
		if ( PlayerNameNotSetYet( pszName ) )
			return;

		if ( !bSilent )
		{
			wchar_t wszPlayerName[MAX_PLAYER_NAME_LENGTH];
			g_pVGuiLocalize->ConvertANSIToUnicode( pszName, wszPlayerName, sizeof( wszPlayerName ) );

			wchar_t wszTeam[64];
			C_Team *pTeam = GetGlobalTeam( iTeam );
			if ( pTeam )
			{
				g_pVGuiLocalize->ConvertANSIToUnicode( pTeam->Get_Name(), wszTeam, sizeof( wszTeam ) );
			}
			else
			{
				_snwprintf( wszTeam, sizeof( wszTeam ) / sizeof( wchar_t ), L"%d", iTeam );
			}

			if ( !IsInCommentaryMode() )
			{
				// Client isn't going to catch up on team change so we have to set the color manually here.
				Color col = pChat->GetTeamColor( iTeam );

				wchar_t wszLocalized[100];
				if ( bAutoTeamed )
				{
					g_pVGuiLocalize->ConstructString( wszLocalized, sizeof( wszLocalized ), g_pVGuiLocalize->Find( "#TF_Joined_AutoTeam" ), 2, wszPlayerName, wszTeam );
				}
				else
				{
					g_pVGuiLocalize->ConstructString( wszLocalized, sizeof( wszLocalized ), g_pVGuiLocalize->Find( "#TF_Joined_Team" ), 2, wszPlayerName, wszTeam );
				}

				char szLocalized[100];
				g_pVGuiLocalize->ConvertUnicodeToANSI( wszLocalized, szLocalized, sizeof( szLocalized ) );

				pChat->SetCustomColor( col );
				pChat->ChatPrintf( iPlayerIndex, CHAT_FILTER_TEAMCHANGE, "%s", szLocalized );
			}
		}

		C_BasePlayer *pPlayer = UTIL_PlayerByIndex( iPlayerIndex );

		if ( pPlayer && pPlayer->IsLocalPlayer() )
		{
			// that's me
			pPlayer->TeamChange( iTeam );
		}

		return;
	}
	else if ( FStrEq( eventname, "player_changename" ) )
	{
		return; // server sends a colorized text string for this
	}

	BaseClass::FireGameEvent( event );
}


void ClientModeTFNormal::PostRenderVGui()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool ClientModeTFNormal::CreateMove( float flInputSampleTime, CUserCmd *cmd )
{
	return BaseClass::CreateMove( flInputSampleTime, cmd );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void ClientModeTFNormal::Update( void )
{
	BaseClass::Update();
	
	//MapData().Update();
}

//-----------------------------------------------------------------------------
// Purpose: See if hud elements want key input. Return 0 if the key is swallowed
//-----------------------------------------------------------------------------
int	ClientModeTFNormal::HudElementKeyInput( int down, ButtonCode_t keynum, const char *pszCurrentBinding )
{
	C_TFPlayer *pPlayer = ToTFPlayer( C_BasePlayer::GetLocalPlayer() );
	if( pPlayer )
	{
		if ( pPlayer->m_Shared.InCond( TF_COND_HALLOWEEN_THRILLER ) )
			return 0;
	}

	// check for hud menus
	if ( m_pMenuEngyBuild )
	{
		if ( !m_pMenuEngyBuild->HudElementKeyInput( down, keynum, pszCurrentBinding ) )
			return 0;
	}

	if ( m_pMenuEngyDestroy )
	{
		if ( !m_pMenuEngyDestroy->HudElementKeyInput( down, keynum, pszCurrentBinding ) )
			return 0;
	}

	if ( m_pMenuSpyDisguise )
	{
		if ( !m_pMenuSpyDisguise->HudElementKeyInput( down, keynum, pszCurrentBinding ) )
			return 0;
	}

	if ( m_pFreezePanel )
	{
		m_pFreezePanel->HudElementKeyInput( down, keynum, pszCurrentBinding );
	}

	if ( m_pTauntSelection )
	{
		m_pTauntSelection->HudElementKeyInput( down, keynum, pszCurrentBinding );
	}

	return BaseClass::HudElementKeyInput( down, keynum, pszCurrentBinding );
}

//-----------------------------------------------------------------------------
// Purpose: See if spectator input occurred. Return 0 if the key is swallowed.
//-----------------------------------------------------------------------------
int ClientModeTFNormal::HandleSpectatorKeyInput( int down, ButtonCode_t keynum, const char *pszCurrentBinding )
{
	return BaseClass::HandleSpectatorKeyInput( down, keynum, pszCurrentBinding );
}

void ClientModeTFNormal::OnColorCorrectionWeightsReset( void )
{
	C_ColorCorrection *pNewColorCorrection = NULL;
	C_ColorCorrection *pOldColorCorrection = m_pCurrentColorCorrection;
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if ( pPlayer )
	{
		pNewColorCorrection = pPlayer->GetActiveColorCorrection();
	}

 	// Only blend between environmental color corrections if there is no failure/infested-induced color correction
	if ( pNewColorCorrection != pOldColorCorrection )
	{
		if ( pOldColorCorrection )
		{
			pOldColorCorrection->EnableOnClient( false );
		}
		if ( pNewColorCorrection )
		{
			pNewColorCorrection->EnableOnClient( true, pOldColorCorrection == NULL );
		}
		m_pCurrentColorCorrection = pNewColorCorrection;
	}
}


void __MsgFunc_BreakModel_Pumpkin( bf_read &msg )
{
	CUtlVector<breakmodel_t> list;
	int iModelIndex = msg.ReadShort();
	BuildGibList( list, iModelIndex, 1.0f, COLLISION_GROUP_NONE );

	if ( list.IsEmpty() )
		return;

	for ( int i=0; i < list.Count(); ++i )
	{
		breakmodel_t *model = &list[ i ];
		model->burstScale = 1000.f;
	}

	Vector vecOrigin = vec3_origin;
	msg.ReadBitVec3Coord( vecOrigin );

	QAngle vecAngles = vec3_angle;
	msg.ReadBitAngles( vecAngles );

	AngularImpulse angularImpulse( RandomFloat( 0.0f, 120.0f ), RandomFloat( 0.0f, 120.0f ), 0.0 );
	breakablepropparams_t params( vecOrigin, vecAngles, Vector( 0.0f ), angularImpulse );

	CUtlVector<EHANDLE> gibList;
	CreateGibsFromList( list, iModelIndex, NULL, params, NULL, -1, false, true, &gibList );

	for ( int i=0; i < gibList.Count(); ++i )
	{
		C_BaseEntity *pGiblet = gibList[ i ];
		if ( pGiblet == nullptr )
			continue;

		if ( pGiblet->VPhysicsGetObject() == nullptr )
			continue;

		Vector vecVelocity; AngularImpulse vecImpulse;
		pGiblet->VPhysicsGetObject()->GetVelocity( &vecVelocity, &vecImpulse );

		vecImpulse.x *= 3.0f;
		vecImpulse.y *= 3.0f;
		vecImpulse.z = i == 3 ? 300.0f : 400.0f;

		pGiblet->VPhysicsGetObject()->SetVelocity( &vecVelocity, &vecImpulse );
	}
}

void __MsgFunc_BreakModelRocketDud( bf_read &msg )
{
	CUtlVector<breakmodel_t> list;
	int iModelIndex = msg.ReadShort();
	BuildGibList( list, iModelIndex, 1.0f, COLLISION_GROUP_NONE );

	Vector vecOrigin = vec3_origin;
	msg.ReadBitVec3Coord( vecOrigin );

	QAngle vecAngles = vec3_angle;
	msg.ReadBitAngles( vecAngles );

	AngularImpulse angularImpulse( 600, random->RandomInt( -1200, 1200 ), 0 );
	Vector vecVelocity( 0.0f, 200.0f, 200.0f );
	breakablepropparams_t params( vecOrigin, vecAngles, vecVelocity, angularImpulse );
	params.impactEnergyScale = 1.0f;

	CreateGibsFromList( list, iModelIndex, NULL, params, NULL, -1, false );
}

void __MsgFunc_PlayerJarated( bf_read &msg )
{
	int iAttacker = msg.ReadByte();
	int iVictim = msg.ReadByte();

	IGameEvent *event = gameeventmanager->CreateEvent( "player_jarated" );
	if ( event )
	{
		event->SetInt( "thrower_entindex", iAttacker );
		event->SetInt( "victim_entindex", iVictim );

		gameeventmanager->FireEventClientSide( event );
	}
}

void __MsgFunc_PlayerJaratedFade( bf_read &msg )
{
	int iAttacker = msg.ReadByte();
	int iVictim = msg.ReadByte();

	IGameEvent *event = gameeventmanager->CreateEvent( "player_jarated_fade" );
	if ( event )
	{
		event->SetInt( "thrower_entindex", iAttacker );
		event->SetInt( "victim_entindex", iVictim );

		gameeventmanager->FireEventClientSide( event );
	}
}

void __MsgFunc_PlayerExtinguished( bf_read &msg )
{
	int iVictim = msg.ReadByte();
	int iHealer = msg.ReadByte();

	IGameEvent *event = gameeventmanager->CreateEvent( "player_extinguished" );
	if ( event )
	{
		event->SetInt( "victim", iVictim );
		event->SetInt( "healer", iHealer );

		gameeventmanager->FireEventClientSide( event );
	}
}

void __MsgFunc_PlayerShieldBlocked( bf_read &msg )
{
	int iAttacker = msg.ReadByte();
	int iVictim = msg.ReadByte();

	IGameEvent *event = gameeventmanager->CreateEvent( "player_shield_blocked" );
	if ( event )
	{
		event->SetInt( "attacker_index", iAttacker );
		event->SetInt( "blocker_index", iVictim );

		gameeventmanager->FireEventClientSide( event );
	}
}

void __MsgFunc_BreakModel( bf_read &msg )
{
	HandleBreakModel( msg, false );
}

void __MsgFunc_CheapBreakModel( bf_read &msg )
{
	// Cheap gibs don't use angle vectors
	HandleBreakModel( msg, true );
}

void ClientModeTFNormal::MessageHooks( void )
{
	HOOK_MESSAGE( BreakModel );
	HOOK_MESSAGE( CheapBreakModel );
	HOOK_MESSAGE( BreakModel_Pumpkin );
	HOOK_MESSAGE( BreakModelRocketDud );
	HOOK_MESSAGE( PlayerJarated );
	HOOK_MESSAGE( PlayerJaratedFade );
	HOOK_MESSAGE( PlayerExtinguished );
	HOOK_MESSAGE( PlayerShieldBlocked );
}

void HandleBreakModel( bf_read &msg, bool bCheap )
{
	CUtlVector<breakmodel_t> list;
	int iModelIndex = msg.ReadShort();
	BuildGibList( list, iModelIndex, 1.0f, COLLISION_GROUP_NONE );

	if ( list.IsEmpty() )
		return;

	Vector vecOrigin = vec3_origin;
	msg.ReadBitVec3Coord( vecOrigin );

	QAngle vecAngles = vec3_angle;
	int nSkin = 0;
	if ( !bCheap )
	{
		msg.ReadBitAngles( vecAngles );
		nSkin = msg.ReadShort();
	}

	AngularImpulse angularImpulse( RandomFloat( 0.0f, 120.0f ), RandomFloat( 0.0f, 120.0f ), 0.0 );
	Vector vecVelocity( 0.0f, 0.0f, 200.0f );
	breakablepropparams_t params( vecOrigin, vecAngles, vecVelocity, angularImpulse );
	params.impactEnergyScale = 1.0f;
	params.nDefaultSkin = nSkin;

	CreateGibsFromList( list, iModelIndex, NULL, params, NULL, -1, false );
}