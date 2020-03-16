//========= Copyright ? 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#include "cbase.h"
#include "tf_hud_itemeffectmeter.h"
#include <vgui_controls/AnimationController.h>
#include "engine/IEngineSound.h"
#include "tf_weapon_bat.h"
#include "tf_weapon_sword.h"
#include "tf_weapon_lunchbox.h"
#include "tf_weapon_jar.h"
#include "tf_weapon_buff_item.h"
#include "tf_weapon_shotgun.h"
#include "tf_weapon_rocketlauncher.h"
#include "tf_weapon_zombie_claw.h"
#include "tf_weapon_medigun.h"
#include "tf_weapon_sniperrifle.h"
#include "tf_weapon_flamethrower.h"
#include "tf_weapon_knife.h"
#include "tf_weapon_revolver.h"
//#include "tf_weapon_throwable.h"
#include "tf_weapon_raygun.h"
#include "tf_weapon_particle_cannon.h"
#include "halloween/tf_weapon_spellbook.h"
#include "tf_weapon_rocketpack.h"
#include "tf_item_powerup_bottle.h"
#include "tf_controls.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

extern ConVar lfe_episodic_flashlight;

template<typename Class>
class CHudItemEffectMeter_Weapon : public CHudItemEffectMeter
{
	DECLARE_CLASS( CHudItemEffectMeter_Weapon<Class>, CHudItemEffectMeter );
public:
	CHudItemEffectMeter_Weapon( const char *pElementName, const char *pResourceName = nullptr )
		: CHudItemEffectMeter( pElementName ), m_pszResource( pResourceName )
	{
	}

	virtual int     GetCount( void )			{ return -1; }
	virtual float   GetProgress( void );
	virtual const char* GetLabelText( void );
	virtual const char* GetResourceName( void );
	virtual const char* GetIconName( void );
	virtual bool    ShouldBeep( void );
	virtual bool    ShouldFlash( void )			{ return false; }
	virtual bool    IsEnabled( void );

	virtual Class*  GetWeapon( void );

private:
	const char *m_pszResource;
};

class CHudItemEffectMeter_Rune : public CHudItemEffectMeter
{
	DECLARE_CLASS( CHudItemEffectMeter_Rune, CHudItemEffectMeter );
public:
	CHudItemEffectMeter_Rune( const char *pElementName, const char *pResourceName = nullptr )
		: CHudItemEffectMeter( pElementName ), m_pszResource( pResourceName )
	{
	}

	virtual float	GetProgress( void );
	virtual const char* GetLabelText( void )			{ return "#TF_Powerup_Supernova"; }
	virtual const char* GetResourceName( void )			{ return "resource/UI/HudPowerupEffectMeter.res"; }
	virtual bool	ShouldFlash( void )					{ return true; }
	virtual bool	IsEnabled( void );

private:
	const char *m_pszResource;
};

class CHudItemEffectMeter_Flashlight : public CHudItemEffectMeter
{
	DECLARE_CLASS( CHudItemEffectMeter_Flashlight, CHudItemEffectMeter );
public:
	CHudItemEffectMeter_Flashlight( const char *pElementName, const char *pResourceName = nullptr )
		: CHudItemEffectMeter( pElementName ), m_pszResource( pResourceName )
	{
		SetHiddenBits( HIDEHUD_FLASHLIGHT );
	}

	virtual float	GetProgress( void );
	virtual const char* GetLabelText( void )			{ return "#TF_Flashlight"; }
	virtual const char* GetResourceName( void )			{ return "resource/UI/HudItemEffectMeter_Flashlight.res"; }
	virtual bool	ShouldBeep( void )					{ return false; }
	virtual bool	ShouldFlash( void )					{ return false; }
	virtual bool	IsEnabled( void );

private:
	const char *m_pszResource;
};

class CHudItemEffects : public CAutoGameSystemPerFrame, public CGameEventListener
{
public:
	CHudItemEffects();
	virtual ~CHudItemEffects();

	virtual bool Init( void );
	virtual void Shutdown( void );
	virtual void Update( float frametime );

	virtual void FireGameEvent( IGameEvent *event );

	void SetPlayer( void );

	inline void AddItemMeter( CHudItemEffectMeter *pMeter )
	{
		DHANDLE<CHudItemEffectMeter> hndl;
		hndl = pMeter;
		if ( hndl )
		{
			gHUD.AddHudElement( hndl.Get() );
			m_pEffectBars.AddToTail( hndl );
			hndl->SetVisible( false );
		}
	}

	static CUtlVector< DHANDLE<CHudItemEffectMeter> > m_pEffectBars;
};
CUtlVector< DHANDLE<CHudItemEffectMeter> > CHudItemEffects::m_pEffectBars;

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CHudItemEffectMeter::CHudItemEffectMeter( const char *pElementName )
	: BaseClass( NULL, pElementName ), CHudElement( pElementName )
{
	SetParent( g_pClientMode->GetViewport() );

	m_pEffectMeter = new ContinuousProgressBar( this, "ItemEffectMeter" );
	m_pEffectMeterLabel = new CExLabel( this, "ItemEffectMeterLabel", "" );
	m_pEffectMeterIcon = new CTFImagePanel( this, "ItemEffectIcon" );

	SetHiddenBits( HIDEHUD_MISCSTATUS );

	m_hItem = NULL;
	m_bEnabled = true;
	m_flOldCharge = 1.0f;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudItemEffectMeter::ApplySchemeSettings( IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	// load control settings...
	LoadControlSettings( GetResourceName() );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudItemEffectMeter::PerformLayout( void )
{
	int iNumPanels = 0;
	CHudItemEffectMeter *pLastMeter = nullptr;
	for ( int i = 0; i < CHudItemEffects::m_pEffectBars.Count(); ++i )
	{
		CHudItemEffectMeter *pMeter = CHudItemEffects::m_pEffectBars[i];
		if ( pMeter && pMeter->IsEnabled() )
		{
			iNumPanels++;
			pLastMeter = pMeter;
		}
	}

	if ( iNumPanels > 1 && pLastMeter )
	{
		int x = 0, y = 0;
		GetPos( x, y );
		SetPos( x - m_iXOffset, y );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CHudItemEffectMeter::ShouldDraw( void )
{
	// Investigate for optimzations that may have happened to this logic
	bool bResult = false;

	C_TFPlayer *pPlayer = C_TFPlayer::GetLocalTFPlayer();
	if ( pPlayer && pPlayer->IsAlive() && IsEnabled() )
	{
		bResult = CHudElement::ShouldDraw();
	}

	if ( bResult ^ IsVisible() )
	{
		SetVisible( bResult );
		if ( bResult )
			InvalidateLayout( false, true );
	}

	return bResult;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudItemEffectMeter::Update( C_TFPlayer *pPlayer )
{
	if ( !pPlayer || !pPlayer->IsAlive() )
	{
		m_flOldCharge = 1.0f;
		return;
	}

	if ( m_pEffectMeter && IsEnabled() )
	{
		if ( GetCount() >= 0 )
			SetDialogVariable( "progresscount", GetCount() );

		float flCharge = GetProgress();
		m_pEffectMeter->SetProgress( flCharge );

		// Play a ding when full charged.
		if ( m_flOldCharge < 1.0f && flCharge >= 1.0f && ShouldBeep() )
		{
			CLocalPlayerFilter filter;
			C_BaseEntity::EmitSound( filter, SOUND_FROM_LOCAL_PLAYER, "TFPlayer.Recharged" );
		}

		if ( ShouldFlash() )
		{
			// Meter is depleting
			int r = 10 * ( (int)( gpGlobals->realtime * 10.0f ) % 10 ) - 96;
			m_pEffectMeter->SetFgColor( Color( r, 0, 0, 255 ) );
		}
		else
		{
			m_pEffectMeter->SetFgColor( COLOR_WHITE );
		}

		m_flOldCharge = flCharge;

		if ( m_pEffectMeterLabel )
		{
			const char *pszLabel = GetLabelText();
			wchar_t *pszLocalized = g_pVGuiLocalize->Find( pszLabel );
			if ( pszLocalized )
				m_pEffectMeterLabel->SetText( pszLocalized );
			else
				m_pEffectMeterLabel->SetText( pszLabel );
		}

		if ( m_pEffectMeterIcon )
		{
			const char *pszImage = GetIconName();
			if ( pszImage && pszImage[0] != '\0' )
			{
				m_pEffectMeterIcon->SetImage( GetIconName() );
				m_pEffectMeterIcon->SetVisible( true );
			}
			else
			{
				m_pEffectMeterIcon->SetImage( "" );
				m_pEffectMeterIcon->SetVisible( false );
			}
		}

		// set the %use_action_slot_item% dialog var to our binding
		/*const char *psUseAction = engine->Key_LookupBinding( "+use_action_slot_item" );
		if ( !psUseAction )
		{
			psUseAction = "< not bound >";
		}
		else
		{
			SetDialogVariable( "use_action_slot_item", psUseAction );
		}*/
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
float CHudItemEffectMeter::GetProgress( void )
{
	C_TFPlayer *pPlayer = C_TFPlayer::GetLocalTFPlayer();
	if ( pPlayer && pPlayer->IsAlive() )
	{
		return pPlayer->m_Shared.GetSpyCloakMeter() / 100;
	}

	return 1.0f;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const char *CHudItemEffectMeter::GetLabelText( void )
{
	C_TFPlayer *pPlayer = C_TFPlayer::GetLocalTFPlayer();
	if ( pPlayer && pPlayer->IsAlive() )
	{
		C_TFWeaponInvis *pWatch = static_cast<C_TFWeaponInvis *>( pPlayer->Weapon_OwnsThisID( TF_WEAPON_INVIS ) );
		if ( pWatch )
			return pWatch->GetEffectLabelText();
	}

	return "#TF_Cloak";
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const char *CHudItemEffectMeter::GetIconName( void )
{
	return "";
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CHudItemEffectMeter::ShouldBeep( void )
{
	C_TFPlayer *pPlayer = C_TFPlayer::GetLocalTFPlayer();
	if ( pPlayer && pPlayer->IsAlive() )
	{
		C_TFWeaponInvis *pWatch = static_cast<C_TFWeaponInvis *>( pPlayer->Weapon_OwnsThisID( TF_WEAPON_INVIS ) );
		if ( pWatch && !pWatch->HasFeignDeath() )
			return false;
	}

	return true;
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
template<typename Class>
float CHudItemEffectMeter_Weapon<Class>::GetProgress( void )
{
	C_TFWeaponBase *pWeapon = GetWeapon();
	if ( pWeapon )
		return pWeapon->GetEffectBarProgress();

	return CHudItemEffectMeter::GetProgress();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
template<typename Class>
const char *CHudItemEffectMeter_Weapon<Class>::GetLabelText( void )
{
	C_TFWeaponBase *pWeapon = GetWeapon();
	if ( pWeapon )
		return pWeapon->GetEffectLabelText();

	return CHudItemEffectMeter::GetLabelText();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
template<typename Class>
const char *CHudItemEffectMeter_Weapon<Class>::GetIconName( void )
{
	return CHudItemEffectMeter::GetIconName();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
template<typename Class>
const char *CHudItemEffectMeter_Weapon<Class>::GetResourceName( void )
{
	if ( m_pszResource )
		return m_pszResource;

	return CHudItemEffectMeter::GetResourceName();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
template<typename Class>
bool CHudItemEffectMeter_Weapon<Class>::ShouldBeep( void )
{
	return CHudItemEffectMeter::ShouldBeep();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
template<typename Class>
bool CHudItemEffectMeter_Weapon<Class>::IsEnabled( void )
{
	if ( GetWeapon() )
		return true;

	return CHudItemEffectMeter::IsEnabled();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
template<typename Class>
Class *CHudItemEffectMeter_Weapon<Class>::GetWeapon( void )
{
	if ( !GetItem() )
	{
		C_TFPlayer *pPlayer = C_TFPlayer::GetLocalTFPlayer();
		if ( pPlayer )
		{
			for ( int i = 0; i < pPlayer->WeaponCount(); ++i )
			{
				Class *pWeapon = dynamic_cast<Class *>( pPlayer->GetWeapon( i ) );
				if ( pWeapon )
				{
					SetItem( pWeapon );
					break;
				}
			}
		}

		if ( !GetItem() )
			m_bEnabled = false;
	}

	return static_cast<Class *>( GetItem() );
}

//-----------------------------------------------------------------------------
// C_TFSword Specialization
//-----------------------------------------------------------------------------
template<>
bool CHudItemEffectMeter_Weapon<C_TFSword>::IsEnabled( void )
{
	if ( GetWeapon() && GetWeapon()->CanDecapitate() )
		return true;

	return false;
}

template<>
int CHudItemEffectMeter_Weapon<C_TFSword>::GetCount( void )
{
	C_TFPlayer *pPlayer = C_TFPlayer::GetLocalTFPlayer();
	if ( pPlayer )
	{
		C_TFSword *pSword = GetWeapon();
		if ( pSword && pSword->CanDecapitate() )
			return pPlayer->m_Shared.GetDecapitationCount();
	}

	return -1;
}

template<>
bool CHudItemEffectMeter_Weapon<C_TFSword>::ShouldBeep( void )
{
	return false;
}

//-----------------------------------------------------------------------------
// C_TFShotgun_Revenge Specialization
//-----------------------------------------------------------------------------
template<>
int CHudItemEffectMeter_Weapon<C_TFShotgun_Revenge>::GetCount( void )
{
	C_TFPlayer *pPlayer = C_TFPlayer::GetLocalTFPlayer();
	if ( pPlayer )
	{
		C_TFShotgun_Revenge *pRevenge = GetWeapon();
		if ( pRevenge && pRevenge->CanGetRevengeCrits() )
			return pRevenge->GetCount();
	}

	return -1;
}

//-----------------------------------------------------------------------------
// C_TFRocketLauncher_AirStrike Specialization
//-----------------------------------------------------------------------------
template<>
int CHudItemEffectMeter_Weapon<C_TFRocketLauncher_AirStrike>::GetCount( void )
{
	C_TFPlayer *pPlayer = C_TFPlayer::GetLocalTFPlayer();
	if ( pPlayer )
	{
		C_TFRocketLauncher_AirStrike *pLauncher = GetWeapon();
		if ( pLauncher )
			return pLauncher->GetCount();
	}

	return -1;
}

//-----------------------------------------------------------------------------
// C_TFSniperRifleDecap Specialization
//-----------------------------------------------------------------------------
template<>
int CHudItemEffectMeter_Weapon<C_TFSniperRifleDecap>::GetCount( void )
{
	C_TFPlayer *pPlayer = C_TFPlayer::GetLocalTFPlayer();
	if ( pPlayer )
	{
		C_TFSniperRifleDecap *pRifle = GetWeapon();
		if ( pRifle )
			return pRifle->GetCount();
	}

	return -1;
}

//-----------------------------------------------------------------------------
// C_TFBuffItem Specialization
//-----------------------------------------------------------------------------
template<>
bool CHudItemEffectMeter_Weapon<C_TFBuffItem>::ShouldFlash( void )
{
	C_TFPlayer *pPlayer = C_TFPlayer::GetLocalTFPlayer();
	if ( !pPlayer )
		return false;

	if ( pPlayer->m_Shared.GetRageProgress() >= 100.0f )
		return true;

	return pPlayer->m_Shared.IsRageActive();
}

//-----------------------------------------------------------------------------
// C_TFFlameThrower Specialization
//-----------------------------------------------------------------------------
template<>
bool CHudItemEffectMeter_Weapon<C_TFFlameThrower>::ShouldFlash( void )
{
	C_TFPlayer *pPlayer = C_TFPlayer::GetLocalTFPlayer();
	if ( !pPlayer )
		return false;

	if ( pPlayer->m_Shared.GetRageProgress() >= 100.0f )
		return true;

	return pPlayer->m_Shared.IsRageActive();
}

template<>
bool CHudItemEffectMeter_Weapon<C_TFFlameThrower>::IsEnabled( void )
{
	return ( GetWeapon() && GetWeapon()->HasChargeBar() );
}

//-----------------------------------------------------------------------------
// C_TFKnife Specialization
//-----------------------------------------------------------------------------
template<>
bool CHudItemEffectMeter_Weapon<C_TFKnife>::IsEnabled( void )
{
	if ( GetWeapon() )
	{
		int iType = 0;
		CALL_ATTRIB_HOOK_INT_ON_OTHER( GetWeapon(), iType, set_weapon_mode );
		if ( iType == 3 )
			return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// C_TFRevolver Specialization
//-----------------------------------------------------------------------------
template<>
bool CHudItemEffectMeter_Weapon<C_TFRevolver>::IsEnabled( void )
{
	if ( GetWeapon() )
	{
		int iKillsCollectCrit = 0;
		CALL_ATTRIB_HOOK_INT_ON_OTHER( GetWeapon(), iKillsCollectCrit, sapper_kills_collect_crits );
		if ( iKillsCollectCrit > 0 )
			return true;
	}

	return false;
}

template<>
int CHudItemEffectMeter_Weapon<C_TFRevolver>::GetCount( void )
{
	C_TFPlayer *pPlayer = C_TFPlayer::GetLocalTFPlayer();
	if ( pPlayer )
	{
		C_TFRevolver *pDiamond = GetWeapon();
		if ( pDiamond )
			return pDiamond->GetCount();
	}

	return -1;
}

//-----------------------------------------------------------------------------
// C_TFSodaPopper Specialization
//-----------------------------------------------------------------------------
template<>
bool CHudItemEffectMeter_Weapon<C_TFSodaPopper>::ShouldFlash( void )
{
	C_TFPlayer *pPlayer = C_TFPlayer::GetLocalTFPlayer();
	if ( !pPlayer )
		return false;

	return ( pPlayer->m_Shared.GetScoutHypeMeter() >= 100.0f );
}

//-----------------------------------------------------------------------------
// C_TFPEPBrawlerBlaster Specialization
//-----------------------------------------------------------------------------
template<>
bool CHudItemEffectMeter_Weapon<C_TFPEPBrawlerBlaster>::ShouldFlash( void )
{
	C_TFPlayer *pPlayer = C_TFPlayer::GetLocalTFPlayer();
	if ( !pPlayer )
		return false;

	return ( pPlayer->m_Shared.GetScoutHypeMeter() >= 100.0f );
}

//-----------------------------------------------------------------------------
// C_WeaponMedigun Specialization
//-----------------------------------------------------------------------------
template<>
bool CHudItemEffectMeter_Weapon<C_WeaponMedigun>::ShouldFlash( void )
{
	C_TFPlayer *pPlayer = C_TFPlayer::GetLocalTFPlayer();
	if ( !pPlayer )
		return false;

	return ( GetWeapon() && GetWeapon()->GetEnergyLevel() >= 100.0f );
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
template<>
bool CHudItemEffectMeter_Weapon<C_WeaponMedigun>::IsEnabled( void )
{
	if ( GetWeapon() )
	{
		int nRageOnHeal = 0;
		CALL_ATTRIB_HOOK_INT_ON_OTHER( GetWeapon(), nRageOnHeal, generate_rage_on_heal );
		if ( nRageOnHeal )
			return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// C_TFPowerupBottle Specialization
//-----------------------------------------------------------------------------
template<>
C_TFPowerupBottle *CHudItemEffectMeter_Weapon<C_TFPowerupBottle>::GetWeapon( void )
{
	if ( !GetItem() )
	{
		C_TFPlayer *pPlayer = C_TFPlayer::GetLocalTFPlayer();
		if ( pPlayer )
		{
			for ( int i = 0; i < pPlayer->GetNumWearables(); i++ )
			{
				C_TFPowerupBottle *pBottle = dynamic_cast<C_TFPowerupBottle *>( pPlayer->GetWearable( i ) );
				if ( pBottle )
				{
					SetItem( pBottle );
					break;
				}
			}
		}

		if ( !GetItem() )
			m_bEnabled = false;
	}

	return static_cast<C_TFPowerupBottle *>( GetItem() );
}

template<>
bool CHudItemEffectMeter_Weapon<C_TFPowerupBottle>::IsEnabled( void )
{
	C_TFPlayer *pPlayer = C_TFPlayer::GetLocalTFPlayer();
	if ( pPlayer )
	{
		C_TFPowerupBottle *pBottle = GetWeapon();
		if ( pBottle )
			return true;
	}

	return false;
}

template<>
float CHudItemEffectMeter_Weapon<C_TFPowerupBottle>::GetProgress( void )
{
	return 0.0f;
}

template<>
int CHudItemEffectMeter_Weapon<C_TFPowerupBottle>::GetCount( void )
{
	C_TFPlayer *pPlayer = C_TFPlayer::GetLocalTFPlayer();
	if ( pPlayer )
	{
		C_TFPowerupBottle *pBottle = GetWeapon();
		if ( pBottle )
			return pBottle->GetNumCharges();
	}

	return -1;
}

template<>
const char *CHudItemEffectMeter_Weapon<C_TFPowerupBottle>::GetLabelText( void )
{
	C_TFPowerupBottle *pBottle = GetWeapon();
	if ( pBottle )
		return pBottle->GetEffectLabelText();

	return CHudItemEffectMeter::GetLabelText();
}

template<>
const char *CHudItemEffectMeter_Weapon<C_TFPowerupBottle>::GetIconName( void )
{
	C_TFPowerupBottle *pBottle = GetWeapon();
	if ( pBottle )
		return pBottle->GetEffectIconName();

	return CHudItemEffectMeter::GetIconName();
}

//-----------------------------------------------------------------------------
// C_TFParticleCannon Specialization
//-----------------------------------------------------------------------------
template<>
float CHudItemEffectMeter_Weapon<C_TFParticleCannon>::GetProgress( void )
{
	C_TFWeaponBase *pWeapon = GetWeapon();
	if ( pWeapon )
		return pWeapon->GetProgress();

	return CHudItemEffectMeter::GetProgress();
}

template<>
bool CHudItemEffectMeter_Weapon<C_TFParticleCannon>::ShouldBeep( void )
{
	return false;
}

template<>
bool CHudItemEffectMeter_Weapon<C_TFParticleCannon>::ShouldFlash( void )
{
	C_TFWeaponBase *pWeapon = GetWeapon();
	if ( pWeapon )
	{
		if ( pWeapon->GetProgress() < 1.0f )
			return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// C_TFRocketPack Specialization
//-----------------------------------------------------------------------------
template<>
float CHudItemEffectMeter_Weapon<C_TFRocketPack>::GetProgress( void )
{
	C_TFWeaponBase *pWeapon = GetWeapon();
	if ( pWeapon )
		return pWeapon->GetEffectBarProgress();

	return CHudItemEffectMeter::GetProgress();
}

//-----------------------------------------------------------------------------
// C_TFSpellBook Specialization
//-----------------------------------------------------------------------------
template<>
int CHudItemEffectMeter_Weapon<C_TFSpellBook>::GetCount( void )
{
	return -1;
}

//-----------------------------------------------------------------------------
// Super Nova
//-----------------------------------------------------------------------------
float CHudItemEffectMeter_Rune::GetProgress( void )
{
	C_TFPlayer *pPlayer = C_TFPlayer::GetLocalTFPlayer();
	if ( !pPlayer )
		return false;

	return pPlayer->m_Shared.GetRuneProgress() / 100;
}

bool CHudItemEffectMeter_Rune::IsEnabled( void )
{
	C_TFPlayer *pPlayer = C_TFPlayer::GetLocalTFPlayer();
	if ( !pPlayer )
		return false;

	return pPlayer->m_Shared.CanRuneCharge();
}

//-----------------------------------------------------------------------------
// Flashlight
//-----------------------------------------------------------------------------
float CHudItemEffectMeter_Flashlight::GetProgress( void )
{
	C_TFPlayer *pPlayer = C_TFPlayer::GetLocalTFPlayer();
	if ( !pPlayer )
		return false;

	return pPlayer->m_Shared.GetFlashlightBattery() / 100;
}

bool CHudItemEffectMeter_Flashlight::IsEnabled( void )
{
	C_TFPlayer *pPlayer = C_TFPlayer::GetLocalTFPlayer();
	if ( !pPlayer )
		return false;

	if ( pPlayer->m_Shared.IsInCutScene() )
		return false;

	return ( lfe_episodic_flashlight.GetBool() );
}

CHudItemEffects::CHudItemEffects()
	: CAutoGameSystemPerFrame( "CHudItemEffects" )
{
}

CHudItemEffects::~CHudItemEffects()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CHudItemEffects::Init( void )
{
	ListenForGameEvent( "localplayer_respawn" );
	ListenForGameEvent( "post_inventory_application" );

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudItemEffects::Shutdown( void )
{
	for ( int i = 0; i < m_pEffectBars.Count(); ++i )
	{
		if ( m_pEffectBars[i] )
			delete m_pEffectBars[i].Get();
	}

	StopListeningForAllEvents();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudItemEffects::Update( float frametime )
{
	C_TFPlayer *pPlayer = C_TFPlayer::GetLocalTFPlayer();
	if ( !pPlayer )
		return;

	for ( int i = 0; i < m_pEffectBars.Count(); ++i )
	{
		CHudItemEffectMeter *pMeter = m_pEffectBars[i];
		if ( pMeter )
			pMeter->Update( pPlayer );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Setup elements on player setup
//-----------------------------------------------------------------------------
void CHudItemEffects::FireGameEvent( IGameEvent *event )
{
	if ( !Q_strcmp( event->GetName(), "post_inventory_application" ) )
	{
		int iUserID = event->GetInt( "userid" );
		C_TFPlayer *pPlayer = C_TFPlayer::GetLocalTFPlayer();

		if ( pPlayer && pPlayer->GetUserID() == iUserID )
		{
			SetPlayer();
		}
	}
	else if ( !Q_strcmp( event->GetName(), "localplayer_respawn" ) )
	{
		SetPlayer();
	}
}

void CHudItemEffects::SetPlayer( void )
{
	C_TFPlayer *pPlayer = C_TFPlayer::GetLocalTFPlayer();
	if ( !pPlayer )
	{
		return;
	}

	for ( int i = 0; i < m_pEffectBars.Count(); ++i )
	{
		if ( m_pEffectBars[i] )
			delete m_pEffectBars[i].Get();
	}

	AddItemMeter( new CHudItemEffectMeter_Weapon<C_TFPowerupBottle>( "HudItemEffectMeter", "resource/UI/HudItemEffectMeter_PowerupBottle.res" ) );
	AddItemMeter( new CHudItemEffectMeter_Weapon<C_TFSpellBook>( "HudItemEffectMeter", "resource/UI/HudSpellSelection.res" ) );
	AddItemMeter( new CHudItemEffectMeter_Rune( "HudItemEffectMeter" ) );
	AddItemMeter( new CHudItemEffectMeter_Flashlight( "HudItemEffectMeter" ) );

	switch ( pPlayer->GetPlayerClass()->GetClassIndex() )
	{
		case TF_CLASS_SCOUT:
			AddItemMeter( new CHudItemEffectMeter_Weapon<C_TFBat_Wood>( "HudItemEffectMeter" ) );
			AddItemMeter( new CHudItemEffectMeter_Weapon<C_TFLunchBox_Drink>( "HudItemEffectMeter", "resource/UI/HudItemEffectMeter_Scout.res" ) );
			AddItemMeter( new CHudItemEffectMeter_Weapon<C_TFJarMilk>( "HudItemEffectMeter", "resource/UI/HudItemEffectMeter_Scout.res" ) );
			AddItemMeter( new CHudItemEffectMeter_Weapon<C_TFSodaPopper>( "HudItemEffectMeter", "resource/UI/HudItemEffectMeter_SodaPopper.res" ) );
			AddItemMeter( new CHudItemEffectMeter_Weapon<C_TFPEPBrawlerBlaster>( "HudItemEffectMeter", "resource/UI/HudItemEffectMeter_SodaPopper.res" ) );
			//AddItemMeter( new CHudItemEffectMeter_Weapon<C_TFCleaver>( "HudItemEffectMeter", "resource/UI/HudItemEffectMeter_Cleaver.res" ) );
			break;
		case TF_CLASS_SNIPER:
			AddItemMeter( new CHudItemEffectMeter_Weapon<C_TFJar>( "HudItemEffectMeter" ) );
			AddItemMeter( new CHudItemEffectMeter_Weapon<C_TFSniperRifleDecap>( "HudItemEffectMeter", "resource/UI/HudItemEffectMeter_Sniper.res" ) );
			break;
		case TF_CLASS_SOLDIER:
			AddItemMeter( new CHudItemEffectMeter_Weapon<C_TFBuffItem>( "HudItemEffectMeter" ) );
			//AddItemMeter( new CHudItemEffectMeter_Weapon<C_TFRayGun>( "HudItemEffectMeter", "resource/UI/HudItemEffectMeter_RayGun.res" ) );
			AddItemMeter( new CHudItemEffectMeter_Weapon<C_TFBison>( "HudItemEffectMeter", "resource/UI/HudItemEffectMeter_RayGun.res" ) );
			AddItemMeter( new CHudItemEffectMeter_Weapon<C_TFParticleCannon>( "HudItemEffectMeter", "resource/UI/HudItemEffectMeter_ParticleCannon.res" ) );
			AddItemMeter( new CHudItemEffectMeter_Weapon<C_TFRocketLauncher_AirStrike>( "HudItemEffectMeter", "resource/UI/HudItemEffectMeter_Demoman.res" ) );
			break;
		case TF_CLASS_DEMOMAN:
			AddItemMeter( new CHudItemEffectMeter_Weapon<C_TFSword>( "HudItemEffectMeter", "resource/UI/HudItemEffectMeter_Demoman.res" ) );
			break;
		case TF_CLASS_MEDIC:
			AddItemMeter( new CHudItemEffectMeter_Weapon<C_WeaponMedigun>( "HudItemEffectMeter", "resource/UI/HudItemEffectMeter_Scout.res" ) );
			break;
		case TF_CLASS_HEAVYWEAPONS:
			AddItemMeter( new CHudItemEffectMeter_Weapon<C_TFLunchBox>( "HudItemEffectMeter" ) );
			break;
		case TF_CLASS_PYRO:
			AddItemMeter( new CHudItemEffectMeter_Weapon<C_TFFlameThrower>( "HudItemEffectMeter" ) );
			//AddItemMeter( new CHudItemEffectMeter_Weapon<C_TFFlareGun_Revenge>( "HudItemEffectMeter", "resource/UI/HudItemEffectMeter_Pyro.res" ) );
			AddItemMeter( new CHudItemEffectMeter_Weapon<C_TFRocketPack>( "HudItemEffectMeter", "resource/UI/HudRocketPack.res" ) );
			break;
		case TF_CLASS_SPY:
			AddItemMeter( new CHudItemEffectMeter( "HudItemEffectMeter" ) );
			AddItemMeter( new CHudItemEffectMeter_Weapon<C_TFKnife>( "HudItemEffectMeter", "resource/UI/HudItemEffectMeter_SpyKnife.res" ) );
			AddItemMeter( new CHudItemEffectMeter_Weapon<C_TFRevolver>( "HudItemEffectMeter", "resource/UI/HudItemEffectMeter_Spy.res" ) );
			//AddItemMeter( new CHudItemEffectMeter_Weapon<C_TFWeaponBuilder>( "HudItemEffectMeter", "resource/UI/HudItemEffectMeter_Sapper.res" ) );
			break;
		case TF_CLASS_ENGINEER:
			AddItemMeter( new CHudItemEffectMeter_Weapon<C_TFShotgun_Revenge>( "HudItemEffectMeter", "resource/UI/HudItemEffectMeter_Engineer.res" ) );
			AddItemMeter( new CHudItemEffectMeter_Weapon<C_TFDRGPomson>( "HudItemEffectMeter", "resource/UI/HudItemEffectMeter_RayGun.res" ) );
			break;
		case TF_CLASS_ZOMBIEFAST:
			AddItemMeter( new CHudItemEffectMeter_Weapon<C_TFZombieClaw>( "HudItemEffectMeter" ) );
			break;
		case TF_CLASS_CIVILIAN:
		case TF_CLASS_COMBINE:
		case TF_CLASS_ANTLION:
		default:
			break;
	}
}

static CHudItemEffects g_pItemMeters;