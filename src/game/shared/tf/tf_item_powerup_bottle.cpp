#include "cbase.h"

#ifdef CLIENT_DLL
#include "c_tf_player.h"
#else
#include "tf_player.h"
#endif

#include "tf_gamerules.h"
#include "tf_item_powerup_bottle.h"
#include "tf_weapon_medigun.h"

IMPLEMENT_NETWORKCLASS_ALIASED( TFPowerupBottle, DT_TFPowerupBottle );

BEGIN_NETWORK_TABLE( CTFPowerupBottle, DT_TFPowerupBottle )
END_NETWORK_TABLE()

LINK_ENTITY_TO_CLASS( tf_powerup_bottle, CTFPowerupBottle );
PRECACHE_REGISTER( tf_powerup_bottle );

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CTFPowerupBottle::CTFPowerupBottle()
{
	m_usNumCharges = 3;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CTFPowerupBottle::~CTFPowerupBottle()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPowerupBottle::Precache( void )
{
	BaseClass::Precache();

	PrecacheScriptSound( "MVM.PlayerUsedPowerup" );
}

//-----------------------------------------------------------------------------
// Purpose: Add or remove this from owner's attribute providers list.
//-----------------------------------------------------------------------------
void CTFPowerupBottle::ReapplyProvision( void )
{
	BaseClass::ReapplyProvision();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CTFPowerupBottle::GetSkin( void )
{
	return GetPowerupType();
}

#ifdef GAME_DLL
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPowerupBottle::UnEquip( CBaseEntity *pPlayer )
{
	Reset();
	RemoveEffect();
	BaseClass::UnEquip( pPlayer );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFPowerupBottle::Use( void )
{
	CTFPlayer *pOwner = ToTFPlayer( GetOwnerEntity() );
	if ( !pOwner )
		return false;

	if ( GetPowerupType() == EMPTY )
		return false;

	if ( m_usNumCharges < 1 )
		return false;

	bool bSucess = false;
	const char *pszChat = "#TF_PVE_Player_UsedCritsBottle";
	float flTime = 5.0f;

	if ( m_itCooldown.IsLessThen( flTime ) )
		return false;

	if ( GetPowerupType() == CRITBOOST )
	{
		pOwner->m_Shared.AddCond( TF_COND_CRITBOOSTED_USER_BUFF, flTime );
		pszChat = "#TF_PVE_Player_UsedCritsBottle";
		bSucess = true;
	}
	else if ( GetPowerupType() == UBERCHARGE )
	{
		pOwner->m_Shared.AddCond( TF_COND_INVULNERABLE_USER_BUFF, flTime );
		pszChat = "#TF_PVE_Player_UsedUberBottle";
		bSucess = true;
	}
	else if ( GetPowerupType() == RECALL )
	{
		pOwner->ForceRespawn();
		pszChat = "#TF_PVE_Player_UsedRecallBottle";
		bSucess = true;
	}
	else if ( GetPowerupType() == REFILL_AMMO )
	{
		for ( int i = TF_AMMO_PRIMARY; i < TF_AMMO_COUNT; i++ )
		{
			pOwner->GiveAmmo( pOwner->GetMaxAmmo( i ), i, false, TF_AMMO_SOURCE_RESUPPLY );
		}

		pszChat = "#TF_PVE_Player_UsedRefillAmmoBottle";
		bSucess = true;
	}
	else if ( GetPowerupType() == BUILDING_UPGRADE )
	{
		pOwner->Regenerate();
		pszChat = "#TF_PVE_Player_UsedBuildingUpgrade";
		bSucess = true;
	}

	if ( bSucess )
	{
		CReliableBroadcastRecipientFilter filter;
		UTIL_SayText2Filter( filter, pOwner, false, pszChat, pOwner->GetPlayerName() );
		pOwner->EmitSound( "MVM.PlayerUsedPowerup" );
		m_itCooldown.Reset();
		m_itCooldown.Start();
		m_usNumCharges--;

		IGameEvent *event = gameeventmanager->CreateEvent( "player_used_powerup_bottle" );
		if ( event )
		{
			event->SetInt( "player", pOwner->GetUserID() );
			event->SetInt( "type", GetPowerupType() );
			event->SetFloat( "time", flTime );

			gameeventmanager->FireEvent( event );
		}

		CWeaponMedigun *pMedigun = pOwner->GetMedigun();
		if ( pMedigun )
		{
			int iSpecialist = 0;
			CALL_ATTRIB_HOOK_INT_ON_OTHER( pMedigun, iSpecialist, canteen_specialist );
			if ( iSpecialist > 0 )
			{
				CTFPlayer *pHealTarget = ToTFPlayer( pOwner->MedicGetHealTarget() );
				if ( pHealTarget )
				{
					bool bShared = false;
					if ( iSpecialist == 1 )
					{
						flTime *= 0.5;
					}
					else if ( iSpecialist == 2 )
					{
						flTime *= 0.8;
					}

					if ( GetPowerupType() == CRITBOOST )
					{
						pHealTarget->m_Shared.AddCond( TF_COND_CRITBOOSTED_USER_BUFF, flTime );
						bShared = true;
					}
					else if ( GetPowerupType() == UBERCHARGE )
					{
						pHealTarget->m_Shared.AddCond( TF_COND_INVULNERABLE_USER_BUFF, flTime );
						bShared = true;
					}
					else if ( GetPowerupType() == RECALL )
					{
						pHealTarget->ForceRespawn();
						bShared = true;
					}
					else if ( GetPowerupType() == REFILL_AMMO )
					{
						for ( int i = TF_AMMO_PRIMARY; i < TF_AMMO_COUNT; i++ )
						{
							if ( iSpecialist == 1 )
								pHealTarget->GiveAmmo( pHealTarget->GetMaxAmmo( i ) / 2.5, i, false, TF_AMMO_SOURCE_RESUPPLY );
							else if ( iSpecialist == 1 )
								pHealTarget->GiveAmmo( pHealTarget->GetMaxAmmo( i ) / 2, i, false, TF_AMMO_SOURCE_RESUPPLY );
							else if ( iSpecialist >= 3 )
								pHealTarget->GiveAmmo( pHealTarget->GetMaxAmmo( i ), i, false, TF_AMMO_SOURCE_RESUPPLY );
						}

						bShared = true;
					}
					else if ( GetPowerupType() == BUILDING_UPGRADE )
					{
						pHealTarget->Regenerate();
						bShared = true;
					}

					if ( bShared )
					{
						IGameEvent *shareevent = gameeventmanager->CreateEvent( "mvm_medic_powerup_shared" );
						if ( shareevent )
						{
							shareevent->SetInt( "player", pOwner->GetUserID() );
							gameeventmanager->FireEvent( shareevent );
						}
					}
				}
			}
		}

	}

	return bSucess;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPowerupBottle::RemoveEffect( void )
{
	CTFPlayer *pOwner = ToTFPlayer( GetOwnerEntity() );
	if ( !pOwner )
		return;

	pOwner->m_Shared.RemoveCond( TF_COND_INVULNERABLE_USER_BUFF );
	pOwner->m_Shared.RemoveCond( TF_COND_CRITBOOSTED_USER_BUFF );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPowerupBottle::Reset( void )
{
	m_usNumCharges = 0;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPowerupBottle::SetNumCharges( int iNum )
{
	m_usNumCharges = iNum;
	m_nSkin = GetSkin();
}
#else

extern ConVar cl_hud_minmode;

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const char *C_TFPowerupBottle::GetEffectLabelText( void )
{
	if ( cl_hud_minmode.GetBool() )
		return "#TF_PVE_UsePowerup_MinMode";

	if ( GetPowerupType() == CRITBOOST )
	{
		return "#TF_PVE_UsePowerup_CritBoost";
	}
	else if ( GetPowerupType() == UBERCHARGE )
	{
		return "#TF_PVE_UsePowerup_Ubercharge";
	}
	else if ( GetPowerupType() == RECALL )
	{
		return "#TF_PVE_UsePowerup_Recall";
	}
	else if ( GetPowerupType() == REFILL_AMMO )
	{
		return "#TF_PVE_UsePowerup_RefillAmmo";
	}
	else if ( GetPowerupType() == BUILDING_UPGRADE )
	{
		return "#TF_PVE_UsePowerup_BuildinginstaUpgrade";
	}

	return "#TF_PVE_UsePowerup_MinMode";
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const char *C_TFPowerupBottle::GetEffectIconName( void )
{
	if ( GetPowerupType() == CRITBOOST )
	{
		return "../hud/ico_powerup_critboost_red";
	}
	else if ( GetPowerupType() == UBERCHARGE )
	{
		return "../hud/ico_powerup_ubercharge_red";
	}
	else if ( GetPowerupType() == RECALL )
	{
		return "../hud/ico_powerup_recall_red";
	}
	else if ( GetPowerupType() == REFILL_AMMO )
	{
		return "../hud/ico_powerup_refill_ammo_red";
	}
	else if ( GetPowerupType() == BUILDING_UPGRADE )
	{
		return "../hud/ico_powerup_building_upgrade_red";
	}

	return "";
}

#endif

int	CTFPowerupBottle::GetPowerupType( void ) const
{
	int iType = EMPTY;

	int iCritBoost = 0;
	CALL_ATTRIB_HOOK_INT( iCritBoost, critboost );
	if ( iCritBoost )
		iType = CRITBOOST;

	int iUbercharge = 0;
	CALL_ATTRIB_HOOK_INT( iUbercharge, ubercharge );
	if ( iUbercharge )
		iType = UBERCHARGE;

	int iRecall = 0;
	CALL_ATTRIB_HOOK_INT( iRecall, recall );
	if ( iRecall )
		iType = RECALL;

	int iRefill = 0;
	CALL_ATTRIB_HOOK_INT( iRefill, refill_ammo );
	if ( iRefill )
		iType = REFILL_AMMO;

	int iInstantUpgrade = 0;
	CALL_ATTRIB_HOOK_INT( iInstantUpgrade, building_instant_upgrade );
	if ( iInstantUpgrade )
		iType = BUILDING_UPGRADE;

	return iType;
}

int	CTFPowerupBottle::GetNumCharges( void ) const
{
	return m_usNumCharges;
}

int	CTFPowerupBottle::GetMaxNumCharges( void ) const
{
	return 3;
}