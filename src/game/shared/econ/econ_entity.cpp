//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//===========================================================================//

#include "cbase.h"
#include "econ_entity.h"
#include "eventlist.h"
#include "tf_gamerules.h"
#include "datacache/imdlcache.h"

#ifdef GAME_DLL
#include "tf_player.h"
#else
#include "c_tf_player.h"
#include "model_types.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_NETWORKCLASS_ALIASED( EconEntity, DT_EconEntity )

BEGIN_NETWORK_TABLE( CEconEntity, DT_EconEntity )
#ifdef CLIENT_DLL
	RecvPropDataTable( RECVINFO_DT( m_AttributeManager ), 0, &REFERENCE_RECV_TABLE( DT_AttributeContainer ) ),
	RecvPropString( RECVINFO( m_ParticleName ) ),
#else
	SendPropDataTable( SENDINFO_DT( m_AttributeManager ), &REFERENCE_SEND_TABLE( DT_AttributeContainer ) ),
	SendPropString( SENDINFO( m_ParticleName ) ),
#endif
END_NETWORK_TABLE()

CEconEntity::CEconEntity()
{
	m_pAttributes = this;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CEconEntity::Precache( void )
{
	BaseClass::Precache();

	CEconItemDefinition *pItem = GetItem()->GetStaticData();
	if ( pItem == nullptr )
		return;

	static CSchemaFieldHandle<CEconAttributeDefinition> pAttribDef_CustomProjectile( "custom projectile model" );
	static CSchemaFieldHandle<CEconAttributeDefinition> pAttribDef_CosmeticTauntSound( "cosmetic taunt sound" );
	static CSchemaFieldHandle<CEconAttributeDefinition> pAttribDef_ProjectileEntityName( "projectile entity name" );
	static CSchemaFieldHandle<CEconAttributeDefinition> pAttribDef_ProjectileParticleName( "projectile particle name" );
	static CSchemaFieldHandle<CEconAttributeDefinition> pAttribDef_AttachedParticleName( "attach particle effect name" );

	// Precache models.
	if ( pItem->model_world[0] != '\0' )
		CBaseEntity::PrecacheModel( pItem->model_world );

	if ( pItem->model_player[0] != '\0' )
		CBaseEntity::PrecacheModel( pItem->model_player );

	if ( pItem->extra_wearable[0] != '\0' )
		CBaseEntity::PrecacheModel( pItem->extra_wearable );

	if ( pItem->model_vision_filtered[0] != '\0' )
		CBaseEntity::PrecacheModel( pItem->model_vision_filtered );

	for ( int iClass = 0; iClass < TF_CLASS_COUNT_ALL; iClass++ )
	{
		const char *pszModel = pItem->model_player_per_class[iClass];
		if ( pszModel[0] != '\0' )
			CBaseEntity::PrecacheModel( pszModel );
	}

	// Precache visuals.
	for ( int i = 0; i < TF_TEAM_COUNT; i++ )
	{
		if ( i == TEAM_SPECTATOR )
			continue;

		PerTeamVisuals_t *pVisuals = &pItem->visual[i];

		// Precache sounds.
		for ( int i = 0; i < NUM_SHOOT_SOUND_TYPES; i++ )
		{
			if ( pVisuals->aWeaponSounds[i][0] != '\0' )
				CBaseEntity::PrecacheScriptSound( pVisuals->aWeaponSounds[i] );
		}

		// Precache attachments.
		for ( int i = 0; i < pVisuals->attached_models.Count(); i++ )
		{
			const char *pszModel = pVisuals->attached_models[i].model;
			if ( pszModel != '\0' )
				CBaseEntity::PrecacheModel( pszModel );
		}

		// Precache custom particles
		const char *pszCustomParticle = pVisuals->custom_particlesystem;
		if ( pszCustomParticle[0] != '\0' )
		{
			PrecacheParticleSystem( pszCustomParticle );
		}

		const char *pszParticleTwo = pVisuals->custom_particlesystem2;
		if ( pszParticleTwo[0] != '\0' )
		{
			PrecacheParticleSystem( pszParticleTwo );
		}

		// Precache particles
		const char *pszParticle = pVisuals->particle_effect;
		if ( pszParticle[0] != '\0' )
		{
			PrecacheParticleSystem( pszParticle );
		}

		// Precache custom muzzle flash
		const char *pszMuzzleFlash = pVisuals->muzzle_flash;
		if ( pszMuzzleFlash[0] != '\0' )
			PrecacheParticleSystem( pszMuzzleFlash );

		// Precache custom tracer effect
		const char *pszTracerEffect = pVisuals->tracer_effect;
		if ( pszTracerEffect[0] != '\0' )
			PrecacheParticleSystem( pszTracerEffect );
	}

	// Cache all attrbute names.
	for ( static_attrib_t const &attrib : pItem->attributes )
	{
		const CEconAttributeDefinition *pAttribute = attrib.GetStaticData();

		// Special case for string attribute.
		if ( pAttribute == pAttribDef_CustomProjectile )
			CBaseEntity::PrecacheModel( attrib.value.sVal->Get() );

		if ( pAttribute == pAttribDef_CosmeticTauntSound )
			CBaseEntity::PrecacheScriptSound( attrib.value.sVal->Get() );

		if ( pAttribute == pAttribDef_ProjectileParticleName ||
			 pAttribute == pAttribDef_AttachedParticleName )
			PrecacheParticleSystem( attrib.value.sVal->Get() );

		if ( pAttribute == pAttribDef_ProjectileEntityName )
			UTIL_PrecacheOther( attrib.value.sVal->Get() );
	}
}

#ifdef CLIENT_DLL

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CEconEntity::OnPreDataChanged( DataUpdateType_t updateType )
{
	BaseClass::OnPreDataChanged( updateType );

	m_AttributeManager.OnPreDataChanged( updateType );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CEconEntity::OnDataChanged( DataUpdateType_t updateType )
{
	BaseClass::OnDataChanged( updateType );

	if ( updateType == DATA_UPDATE_CREATED )
	{
		BaseClass::UpdateVisibility();
	}

	if ( updateType == DATA_UPDATE_DATATABLE_CHANGED )
	{
		UpdateParticleSystems();

		/*CEconItemDefinition *pStatic = GetItem()->GetStaticData();
		if ( pStatic )
		{
			PerTeamVisuals_t *pVisuals =	pStatic->GetVisuals();
			if ( pVisuals )
			{
				const char *pszMat = pVisuals->material_override;
				if ( Q_strlen( pszMat ) > 0 )
				{
					MaterialOverride( pszMat );

					//C_ViewmodelAttachmentModel *pAttach = GetViewmodelAddon();
					//if ( pAttach )	
					//	pAttach->MaterialOverride( pszMat );
				}
			}
		}*/

	}

	m_AttributeManager.OnDataChanged( updateType );

	UpdateAttachmentModels();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CEconEntity::FireEvent( const Vector& origin, const QAngle& angles, int event, const char *options )
{
	if ( event == AE_CL_BODYGROUP_SET_VALUE_CMODEL_WPN )
	{
		C_ViewmodelAttachmentModel *pAttach = GetViewmodelAddon();
		if ( pAttach)
		{
			pAttach->FireEvent( origin, angles, AE_CL_BODYGROUP_SET_VALUE, options );
		}
	}

	BaseClass::FireEvent( origin, angles, event, options );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CEconEntity::OnFireEvent( C_BaseViewModel *pViewModel, const Vector& origin, const QAngle& angles, int event, const char *options )
{
	if ( event == AE_CL_BODYGROUP_SET_VALUE_CMODEL_WPN )
	{
		C_ViewmodelAttachmentModel *pAttach = GetViewmodelAddon();
		if ( pAttach)
		{
			pAttach->FireEvent( origin, angles, AE_CL_BODYGROUP_SET_VALUE, options );
			return true;
		}
	}
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CEconEntity::IsOverridingViewmodel( void ) const
{
	if ( GetMaterialOverride( GetTeamNumber() ) )
		return true;

	if ( !m_hAttachmentParent )
		return false;

	CEconItemDefinition *pStatic = GetItem()->GetStaticData();
	if ( pStatic == nullptr )
		return false;

	PerTeamVisuals_t *pVisuals = pStatic->GetVisuals( GetTeamNumber() );
	if ( !pVisuals->attached_models.IsEmpty() )
		return true;

	return false;
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
int CEconEntity::InternalDrawModel( int flags )
{
	if ( GetMaterialOverride( GetTeamNumber() ) == nullptr || !( flags & STUDIO_RENDER ) )
		return BaseClass::InternalDrawModel( flags );

	modelrender->ForcedMaterialOverride( m_aMaterials[ GetTeamNumber() ] );
	int result = BaseClass::InternalDrawModel( flags );
	modelrender->ForcedMaterialOverride( NULL );

	return result;
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void CEconEntity::ViewModelAttachmentBlending( CStudioHdr *hdr, Vector pos[], Quaternion q[], float currentTime, int boneMask )
{
	// NUB
}

bool CEconEntity::OnInternalDrawModel( ClientModelRenderInfo_t *pInfo )
{
	if ( BaseClass::OnInternalDrawModel( pInfo ) )
	{
		DrawEconEntityAttachedModels( this, this, pInfo, AM_WORLDMODEL );
		return true;
	}
 	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CEconEntity::DrawOverriddenViewModel( C_BaseViewModel *pViewModel, int flags )
{
	// Temporary, just want to see if anything happens before figuring out logic
	pViewModel->DrawOverriddenViewmodel( flags );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CEconEntity::UpdateAttachmentModels( void )
{
	m_aAttachments.RemoveAll();

	if ( GetItem()->GetStaticData() )
	{
		CEconItemDefinition *pItem = GetItem()->GetStaticData();
		if ( !pItem )
			return;

		if ( AttachmentModelsShouldBeVisible() )
		{
			PerTeamVisuals_t *pVisuals = pItem->GetVisuals( GetTeamNumber() );
			if ( pVisuals )
			{
				for ( int i=0; i<pVisuals->attached_models.Count(); ++i )
				{
					AttachedModel_t attachment = pVisuals->attached_models[i];
					int iMdlIndex = modelinfo->GetModelIndex( attachment.model );
					if ( iMdlIndex >= 0 )
					{
						AttachedModelData_t attachmentData;
						attachmentData.model = modelinfo->GetModel( iMdlIndex );
						attachmentData.modeltype = attachment.model_display_flags;
						m_aAttachments.AddToTail( attachmentData );
					}
				}
			}

			if ( pItem->attach_to_hands || pItem->attach_to_hands_vm_only )
			{
				C_BasePlayer *pPlayer = ToBasePlayer( GetOwnerEntity() );
				if ( pPlayer && pPlayer->IsAlive() && !pPlayer->ShouldDrawThisPlayer() )
				{
					if ( !m_hAttachmentParent || m_hAttachmentParent != GetMoveParent() )
					{
						// Some validation or something
						return;
					}

					CBaseViewModel *pViewmodel = pPlayer->GetViewModel();
					if ( !pViewmodel )
					{
						// Same thing as above
						return;
					}

					/*C_ViewmodelAttachmentModel *pAddon = new C_ViewmodelAttachmentModel;
					if ( !pAddon )
						return;

					if ( pAddon->InitializeAsClientEntity( GetItem()->GetPlayerDisplayModel(), RENDER_GROUP_VIEW_MODEL_OPAQUE ) )
					{
						pAddon->SetOwnerEntity( this );
						pAddon->SetParent( pViewmodel );
						pAddon->SetLocalOrigin( vec3_origin );
						pAddon->UpdatePartitionListEntry();
						pAddon->CollisionProp()->UpdatePartition();
						pAddon->UpdateVisibility();

						m_hAttachmentParent = pAddon;
					}*/
				}
				else
				{
					if ( m_hAttachmentParent )
						m_hAttachmentParent->Release();
				}
			}
		}
	}
	else
	{
		if ( m_hAttachmentParent )
			m_hAttachmentParent->Release();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool C_EconEntity::GetAttachment( int iAttachment, Vector &absOrigin )
{
	if ( m_hAttachmentParent.Get() )
		return m_hAttachmentParent->GetAttachment( iAttachment, absOrigin );

	return BaseClass::GetAttachment( iAttachment, absOrigin );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool C_EconEntity::GetAttachment( int iAttachment, Vector &absOrigin, QAngle &absAngles )
{
	if ( m_hAttachmentParent.Get() )
		return m_hAttachmentParent->GetAttachment( iAttachment, absOrigin, absAngles );

	return BaseClass::GetAttachment( iAttachment, absOrigin, absAngles );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool C_EconEntity::GetAttachment( int iAttachment, matrix3x4_t &matrix )
{
	if ( m_hAttachmentParent.Get() )
		return m_hAttachmentParent->GetAttachment( iAttachment, matrix );

	return BaseClass::GetAttachment( iAttachment, matrix );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_EconEntity::SetMaterialOverride( int iTeam, const char *pszMaterial )
{
	if ( iTeam < 4 )
		m_aMaterials[iTeam].Init( pszMaterial, "ClientEffect textures", true );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_EconEntity::SetMaterialOverride( int iTeam, CMaterialReference &material )
{
	if ( iTeam < 4 )
		m_aMaterials[iTeam].Init( material );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CEconEntity::UpdateParticleSystems( void )
{
	if ( Q_stricmp( m_ParticleName, "" ) && !m_pUnusualParticle )
	{
		if ( ShouldDrawParticleSystems() )
			m_pUnusualParticle = ParticleProp()->Create( m_ParticleName, PATTACH_ABSORIGIN_FOLLOW );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CEconEntity::ShouldDraw( void )
{
	return ShouldHideForVisionFilterFlags();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CEconEntity::ShouldDrawParticleSystems( void )
{
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CEconEntity::ShouldHideForVisionFilterFlags( void )
{
	CEconItemDefinition *pStatic = GetItem()->GetStaticData();
	if ( pStatic )
	{
		if ( TFGameRules() )
		{
			int iFlags = pStatic->vision_filter_flags;
			if ( ( iFlags == TF_VISION_FILTER_PYRO ) && !IsLocalPlayerUsingVisionFilterFlags( TF_VISION_FILTER_PYRO ) ) 
				return true;

			if ( ( iFlags == TF_VISION_FILTER_HALLOWEEN ) && !IsLocalPlayerUsingVisionFilterFlags( TF_VISION_FILTER_HALLOWEEN ) ) 
				return true;

			if ( ( iFlags == TF_VISION_FILTER_ROME ) && !IsLocalPlayerUsingVisionFilterFlags( TF_VISION_FILTER_ROME ) ) 
				return true;
		}
	}

	return BaseClass::ShouldDraw();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const Vector &CEconEntity::GetItemTintColor( void )
{
	vec_t vecRGB[3] = { 0.0f, 0.0f, 0.0f };

	string_t strRGB = NULL_STRING;
	CALL_ATTRIB_HOOK_STRING( strRGB, set_item_tint_rgb );
	if ( strRGB != NULL_STRING )
	{
		UTIL_StringToVector( vecRGB, strRGB );
	}

	static Vector vecColor( vecRGB[0], vecRGB[1], vecRGB[2] );
	return vecColor;
}

#endif

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CEconEntity::SetItem( CEconItemView const &pItem )
{
#if defined( GAME_DLL )
	m_AttributeManager.m_Item.CopyFrom( pItem );
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CEconEntity::HasItemDefinition( void ) const
{
	return ( GetItem()->GetItemDefIndex() >= 0 );
}

//-----------------------------------------------------------------------------
// Purpose: Shortcut to get item ID.
//-----------------------------------------------------------------------------
int CEconEntity::GetItemID( void ) const
{
	return GetItem()->GetItemDefIndex();
}

//-----------------------------------------------------------------------------
// Purpose: Derived classes need to override this.
//-----------------------------------------------------------------------------
void CEconEntity::GiveTo( CBaseEntity *pEntity )
{
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CEconEntity::SetParticle( const char* name )
{
#ifdef CLIENT_DLL
	if ( m_pUnusualParticle )
	{
		ParticleProp()->StopEmission( m_pUnusualParticle );
		m_pUnusualParticle = NULL;
	}
#endif

#ifdef GAME_DLL
	Q_snprintf( m_ParticleName.GetForModify(), PARTICLE_MODIFY_STRING_SIZE, name );
#else
	Q_snprintf( m_ParticleName, PARTICLE_MODIFY_STRING_SIZE, name );
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Add or remove this from owner's attribute providers list.
//-----------------------------------------------------------------------------
void CEconEntity::ReapplyProvision( void )
{
	CBaseEntity *pOwner = GetOwnerEntity();
	CBaseEntity *pOldOwner = m_hOldOwner.Get();

	if ( pOwner != pOldOwner )
	{
		if ( pOldOwner )
		{
			m_AttributeManager.StopProvidingTo( pOldOwner );
		}

		if ( pOwner )
		{
			m_AttributeManager.ProvideTo( pOwner );
			m_hOldOwner = pOwner;
		}
		else
		{
			m_hOldOwner = NULL;
		}
	}
}

void CEconEntity::InitializeAttributes( void )
{
	m_AttributeManager.InitializeAttributes( this );
}

//-----------------------------------------------------------------------------
// Purpose: Update visible bodygroups
//-----------------------------------------------------------------------------
void CEconEntity::UpdatePlayerBodygroups( void )
{
	CTFPlayer *pPlayer = dynamic_cast < CTFPlayer * >( GetOwnerEntity() );
	if ( !pPlayer )
		return;

	// bodygroup enabling/disabling
	CEconItemDefinition *pStatic = GetItem()->GetStaticData();
	if ( pStatic )
	{
		PerTeamVisuals_t *pVisuals = pStatic->GetVisuals();
		if ( pVisuals )
		{
			for ( int i = 0; i < pPlayer->GetNumBodyGroups(); i++ )
			{
				unsigned int index = pVisuals->player_bodygroups.Find( pPlayer->GetBodygroupName( i ) );
				if ( pVisuals->player_bodygroups.IsValidIndex( index ) )
				{
					bool bTrue = pVisuals->player_bodygroups.Element( index );
					if ( bTrue )
					{
						pPlayer->SetBodygroup( i , 1 );
					}
					else
					{
						pPlayer->SetBodygroup( i , 0 );
					}
				}
			}

			if ( pVisuals->wm_bodygroup_override > 0 )
				pPlayer->SetBodygroup( pVisuals->wm_bodygroup_override, pVisuals->wm_bodygroup_state_override );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CEconEntity::UpdateOnRemove( void )
{
	SetOwnerEntity( NULL );
	ReapplyProvision();
	BaseClass::UpdateOnRemove();
}

#ifdef CLIENT_DLL
void DrawEconEntityAttachedModels( C_BaseAnimating *pAnimating, C_EconEntity *pEconEntity, ClientModelRenderInfo_t const *pInfo, int iModelType )
{
	if ( pAnimating && pEconEntity && pInfo )
	{
		for ( int i=0; i<pEconEntity->m_aAttachments.Count(); ++i )
		{
			if ( pEconEntity->m_aAttachments[i].model && ( pEconEntity->m_aAttachments[i].modeltype & iModelType ) )
			{
				ClientModelRenderInfo_t newInfo;
				V_memcpy( &newInfo, pInfo, sizeof( ClientModelRenderInfo_t ) );
				newInfo.pRenderable = (IClientRenderable *)pAnimating;
				newInfo.instance = MODEL_INSTANCE_INVALID;
				newInfo.entity_index = pAnimating->entindex();
				newInfo.pModel = pEconEntity->m_aAttachments[i].model;
				newInfo.pModelToWorld = &newInfo.modelToWorld;

				// Turns the origin + angles into a matrix
				AngleMatrix( newInfo.angles, newInfo.origin, newInfo.modelToWorld );

				DrawModelState_t state;
				matrix3x4_t *pBoneToWorld = NULL;
				bool bMarkAsDrawn = modelrender->DrawModelSetup( newInfo, &state, NULL, &pBoneToWorld );
				pAnimating->DoInternalDrawModel( &newInfo, ( bMarkAsDrawn && ( newInfo.flags & STUDIO_RENDER ) ) ? &state : NULL, pBoneToWorld );
			}
		}
	}
}
#endif
