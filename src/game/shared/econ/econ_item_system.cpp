#include "cbase.h"
#include "econ_item_system.h"
#include "script_parser.h"
#include "activitylist.h"
#include "attribute_types.h"
#ifdef GAME_DLL
#include "sceneentity.h"
#endif

const char *g_TeamVisualSections[TF_TEAM_COUNT] =
{
	"visuals",			// TEAM_UNASSIGNED
	"",					// TEAM_SPECTATOR
	"visuals_red",		// TEAM_RED
	"visuals_blu",		// TEAM_BLUE
	"visuals_grn",		// TEAM_GREEN
	"visuals_ylw",		// TEAM_YELLOW
	//"visuals_mvm_boss"	// ???
};

const char *g_AttributeDescriptionFormats[] =
{
	"value_is_percentage",
	"value_is_inverted_percentage",
	"value_is_additive",
	"value_is_additive_percentage",
	"value_is_or",
	"value_is_date",
	"value_is_account_id",
	"value_is_particle_index",
	"value_is_killstreakeffect_index",
	"value_is_killstreak_idleeffect_index",
	"value_is_item_def",
	"value_is_from_lookup_table"
};

const char *g_EffectTypes[] =
{
	"unusual",
	"strange",
	"neutral",
	"positive",
	"negative"
};

const char *g_szQualityStrings[] =
{
	"normal",
	"rarity1",
	"rarity2",
	"vintage",
	"rarity3",
	"rarity4",
	"unique",
	"community",
	"developer",
	"selfmade",
	"customized",
	"strange",
	"completed",
	"haunted",
	"collectors",
	"paintkitWeapon",
};

const char *g_szQualityColorStrings[] =
{
	"QualityColorNormal",
	"QualityColorrarity1",
	"QualityColorrarity2",
	"QualityColorVintage",
	"QualityColorrarity3",
	"QualityColorrarity4",
	"QualityColorUnique",
	"QualityColorCommunity",
	"QualityColorDeveloper",
	"QualityColorSelfMade",
	"QualityColorSelfMadeCustomized",
	"QualityColorStrange",
	"QualityColorCompleted",
	"QualityColorHaunted",
	"QualityColorCollectors",
	"QualityColorPaintkitWeapon",
};

const char *g_szQualityLocalizationStrings[] =
{
	"#Normal",
	"#rarity1",
	"#rarity2",
	"#vintage",
	"#rarity3",
	"#rarity4",
	"#unique",
	"#community",
	"#developer",
	"#selfmade",
	"#customized",
	"#strange",
	"#completed",
	"#haunted",
	"#collectors",
	"#paintkitWeapon",
};

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
static CEconItemSchema g_EconItemSchema;
CEconItemSchema *GetItemSchema()
{
	return &g_EconItemSchema;
}

class CEconSchemaParser : public CScriptParser
{
public:
	DECLARE_CLASS_GAMEROOT( CEconSchemaParser, CScriptParser );

#define GET_STRING(copyto, from, name)													\
		if (from->GetString(#name, NULL))												\
			V_strncpy(copyto->name, from->GetString(#name), sizeof(copyto->name))

#define GET_STRING_DEFAULT(copyto, from, name, defaultstring)		\
		V_strncpy(copyto->name, from->GetString(#name, #defaultstring), sizeof(copyto->name))

#define GET_BOOL(copyto, from, name)													\
		copyto->name = from->GetBool(#name, copyto->name)

#define GET_FLOAT(copyto, from, name)													\
		copyto->name = from->GetFloat(#name, copyto->name)

#define GET_INT(copyto, from, name)														\
		copyto->name = from->GetInt(#name, copyto->name)

#define GET_STRING_CONVERT(copyto, from, name)											\
		if (from->GetString(#name, NULL))

#define FIND_ELEMENT(map, key, val)						\
		unsigned int index = map.Find(key);				\
		if (index != map.InvalidIndex())						\
			val = map.Element(index)				

#define FIND_ELEMENT_STRING(map, key, val)						\
		unsigned int index = map.Find(key);						\
		if (index != map.InvalidIndex())								\
			Q_snprintf(val, sizeof(val), map.Element(index))

#define IF_ELEMENT_FOUND(map, key)						\
		unsigned int index = map.Find(key);				\
		if (index != map.InvalidIndex())			

#define GET_VALUES_FAST_BOOL(dict, keys)\
		for (KeyValues *pKeyData = keys->GetFirstSubKey(); pKeyData != NULL; pKeyData = pKeyData->GetNextKey())\
		{													\
			IF_ELEMENT_FOUND(dict, pKeyData->GetName())		\
			{												\
				dict.Element(index) = pKeyData->GetBool();	\
			}												\
			else											\
			{												\
				dict.Insert(pKeyData->GetName(), pKeyData->GetBool());\
			}												\
		}


#define GET_VALUES_FAST_STRING(dict, keys)\
		for (KeyValues *pKeyData = keys->GetFirstSubKey(); pKeyData != NULL; pKeyData = pKeyData->GetNextKey())	\
		{													\
			IF_ELEMENT_FOUND(dict, pKeyData->GetName())		\
			{												\
				Q_snprintf((char*)dict.Element(index), sizeof(dict.Element(index)), pKeyData->GetString());		\
			}												\
			else											\
			{												\
				dict.Insert(pKeyData->GetName(), strdup(pKeyData->GetString()));\
			}												\
		}	

	void Parse( KeyValues *pKeyValuesData, const char *szFileWithoutEXT )
	{
		GetItemSchema()->m_pSchema = pKeyValuesData->MakeCopy();

		KeyValues *pPrefabs = pKeyValuesData->FindKey( "prefabs" );
		if ( pPrefabs )
		{
			ParsePrefabs( pPrefabs );
		}

		KeyValues *pGameInfo = pKeyValuesData->FindKey( "game_info" );
		if ( pGameInfo )
		{
			ParseGameInfo( pGameInfo );
		}

		KeyValues *pQualities = pKeyValuesData->FindKey( "qualities" );
		if ( pQualities )
		{
			ParseQualities( pQualities );
		}

		KeyValues *pColors = pKeyValuesData->FindKey( "colors" );
		if ( pColors )
		{
			ParseColors( pColors );
		}

		KeyValues *pAttributes = pKeyValuesData->FindKey( "attributes" );
		if ( pAttributes )
		{
			ParseAttributes( pAttributes );
		}

		KeyValues *pItems = pKeyValuesData->FindKey( "items" );
		if ( pItems )
		{
			ParseItems( pItems );
		}
	};

	void ParseGameInfo( KeyValues *pKeyValuesData )
	{
		for ( KeyValues *pSubData = pKeyValuesData->GetFirstSubKey(); pSubData != NULL; pSubData = pSubData->GetNextKey() )
		{
			GetItemSchema()->m_GameInfo.Insert( pSubData->GetName(), pSubData->GetFloat() );
		}
	};

	void ParseQualities( KeyValues *pKeyValuesData )
	{
		for ( KeyValues *pSubData = pKeyValuesData->GetFirstSubKey(); pSubData != NULL; pSubData = pSubData->GetNextKey() )
		{
			EconQuality Quality;
			GET_INT( ( &Quality ), pSubData, value );
			GetItemSchema()->m_Qualities.Insert( pSubData->GetName(), Quality );
		}
	};

	void ParseColors( KeyValues *pKeyValuesData )
	{
		for ( KeyValues *pSubData = pKeyValuesData->GetFirstSubKey(); pSubData != NULL; pSubData = pSubData->GetNextKey() )
		{
			EconColor ColorDesc;
			GET_STRING( ( &ColorDesc ), pSubData, color_name );
			GetItemSchema()->m_Colors.Insert( pSubData->GetName(), ColorDesc );
		}
	};

	void ParsePrefabs( KeyValues *pKeyValuesData )
	{
		for ( KeyValues *pSubData = pKeyValuesData->GetFirstTrueSubKey(); pSubData != NULL; pSubData = pSubData->GetNextTrueSubKey() )
		{
			if ( GetItemSchema()->m_PrefabsValues.IsValidIndex( GetItemSchema()->m_PrefabsValues.Find( pSubData->GetName() ) ) )
			{
				Error( "Duplicate prefab name (%s)\n", pSubData->GetName() );
				continue;
			}

			KeyValues *Values = pSubData->MakeCopy();
			GetItemSchema()->m_PrefabsValues.Insert( pSubData->GetName(), Values );
		}
	};

	void ParseItems( KeyValues *pKeyValuesData )
	{
		for ( KeyValues *pData = pKeyValuesData->GetFirstTrueSubKey(); pData != NULL; pData = pData->GetNextTrueSubKey() )
		{
			// Skip over default item, not sure why it's there.
			if ( V_stricmp( pData->GetName(), "default" ) == 0 )
				continue;

			CEconItemDefinition *pItem = new CEconItemDefinition;
			int index = atoi( pData->GetName() );
			pItem->index = index;

			KeyValues *pDefinition = new KeyValues( pData->GetName() );
			MergeDefinitionPrefabs( pDefinition, pData );
			pItem->m_pDefinition = pDefinition;

			GET_STRING( pItem, pDefinition, name );
			GET_BOOL( pItem, pDefinition, enabled );
			GET_BOOL( pItem, pDefinition, show_in_armory );

			GET_STRING( pItem, pDefinition, item_class );
			GET_STRING( pItem, pDefinition, item_name );
			GET_STRING( pItem, pDefinition, item_description );
			GET_STRING( pItem, pDefinition, item_type_name );

			const char *pszQuality = pDefinition->GetString( "item_quality" );
			if ( pszQuality[0] )
			{
				int iQuality = UTIL_StringFieldToInt( pszQuality, g_szQualityStrings, ARRAYSIZE( g_szQualityStrings ) );
				if ( iQuality != -1 )
				{
					pItem->item_quality = (EEconItemQuality)iQuality;
				}
			}

			GET_STRING( pItem, pDefinition, item_logname );
			GET_STRING( pItem, pDefinition, item_iconname );

			const char *pszLoadoutSlot = pDefinition->GetString( "item_slot" );

			if ( pszLoadoutSlot[0] )
			{
				pItem->item_slot = UTIL_StringFieldToInt( pszLoadoutSlot, g_LoadoutSlots, LOADOUT_POSITION_COUNT );
			}

			const char *pszAnimSlot = pDefinition->GetString( "anim_slot" );
			if ( pszAnimSlot[0] )
			{
				if ( V_strcmp( pszAnimSlot, "FORCE_NOT_USED" ) != 0 )
				{
					pItem->anim_slot = UTIL_StringFieldToInt( pszAnimSlot, g_AnimSlots, TF_WPN_TYPE_COUNT );
				}
				else
				{
					pItem->anim_slot = -2;
				}
			}

			GET_BOOL( pItem, pDefinition, baseitem );
			GET_INT( pItem, pDefinition, min_ilevel );
			GET_INT( pItem, pDefinition, max_ilevel );

			GET_STRING( pItem, pDefinition, image_inventory );
			GET_INT( pItem, pDefinition, image_inventory_size_w );
			GET_INT( pItem, pDefinition, image_inventory_size_h );

			GET_STRING( pItem, pDefinition, model_player );
			GET_STRING(pItem, pDefinition, response_criteria);
			GET_STRING( pItem, pDefinition, model_vision_filtered );
			GET_STRING( pItem, pDefinition, model_world );
			GET_STRING( pItem, pDefinition, extra_wearable );
			GET_INT(pItem, pDefinition, extra_econ);

			GET_INT( pItem, pDefinition, attach_to_hands );
			GET_INT( pItem, pDefinition, attach_to_hands_vm_only );
			GET_BOOL( pItem, pDefinition, act_as_wearable );
			GET_INT( pItem, pDefinition, hide_bodygroups_deployed_only );

			GET_STRING( pItem, pDefinition, holiday_restriction );

			GET_STRING( pItem, pDefinition, mouse_pressed_sound );
			GET_STRING( pItem, pDefinition, drop_sound );

			GET_BOOL( pItem, pDefinition, flip_viewmodel );

			GET_INT( pItem, pDefinition, vision_filter_flags );

			for ( KeyValues *pSubData = pDefinition->GetFirstSubKey(); pSubData != NULL; pSubData = pSubData->GetNextKey() )
			{
				if ( !V_stricmp( pSubData->GetName(), "capabilities" ) )
				{
					GET_VALUES_FAST_BOOL( pItem->capabilities, pSubData );
				}
				else if ( !V_stricmp( pSubData->GetName(), "tags" ) )
				{
					GET_VALUES_FAST_BOOL( pItem->tags, pSubData );
				}
				else if ( !V_stricmp( pSubData->GetName(), "model_player_per_class" ) )
				{
					for ( KeyValues *pClassData = pSubData->GetFirstSubKey(); pClassData != NULL; pClassData = pClassData->GetNextKey() )
					{
						const char *pszClass = pClassData->GetName();
						int iClass = UTIL_StringFieldToInt( pszClass, g_aPlayerClassNames_NonLocalized, TF_CLASS_COUNT_ALL );

						if ( iClass != -1 )
						{
							V_strncpy( pItem->model_player_per_class[iClass], pClassData->GetString(), 128 );
						}
					}
				}
				else if ( !V_stricmp( pSubData->GetName(), "used_by_classes" ) )
				{
					for ( KeyValues *pClassData = pSubData->GetFirstSubKey(); pClassData != NULL; pClassData = pClassData->GetNextKey() )
					{
						const char *pszClass = pClassData->GetName();
						int iClass = UTIL_StringFieldToInt( pszClass, g_aPlayerClassNames_NonLocalized, TF_CLASS_COUNT_ALL );

						if ( iClass != -1 )
						{
							pItem->used_by_classes |= ( 1 << iClass );
							const char *pszSlotname = pClassData->GetString();

							if ( pszSlotname[0] != '1' )
							{
								int iSlot = UTIL_StringFieldToInt( pszSlotname, g_LoadoutSlots, LOADOUT_POSITION_COUNT );

								if ( iSlot != -1 )
									pItem->item_slot_per_class[iClass] = iSlot;
							}
						}
					}
				}
				else if ( !V_stricmp( pSubData->GetName(), "attributes" ) )
				{
					for ( KeyValues *pAttribData = pSubData->GetFirstTrueSubKey(); pAttribData != NULL; pAttribData = pAttribData->GetNextTrueSubKey() )
					{
						static_attrib_t attribute;

#if defined( _LINUX )
						// you might be wondering why i've manually inlined a function
						// the answer is: g++ is the worst thing ever created
						// i have tried everything i can possibly think of to get it to link
						// g++ is just awful

						CEconAttributeDefinition *pAttrib = GetItemSchema()->GetAttributeDefinitionByName( pAttribData->GetName() );
						if( pAttrib == nullptr || pAttrib->index == -1 || pAttrib->type == nullptr )
							continue;

						attribute.iAttribIndex = pAttrib->index;

						pAttrib->type->InitializeNewEconAttributeValue( &attribute.value );

						if ( !pAttrib->type->BConvertStringToEconAttributeValue( pAttrib, pAttribData->GetString( "value" ), &attribute.value ) )
							continue;
#else
						if ( !attribute.BInitFromKV_MultiLine( pAttribData ) )
							continue;
#endif

						pItem->attributes.AddToTail( attribute );
					}
				}
				else if ( !V_stricmp( pSubData->GetName(), "static_attrs" ) )
				{
					for ( KeyValues *pAttribData = pSubData->GetFirstSubKey(); pAttribData != NULL; pAttribData = pAttribData->GetNextKey() )
					{
						static_attrib_t attribute;

#if defined( _LINUX )
						CEconAttributeDefinition *pAttrib = GetItemSchema()->GetAttributeDefinitionByName( pAttribData->GetName() );
						if( pAttrib == nullptr || pAttrib->index == -1 || pAttrib->type == nullptr )
							continue;

						attribute.iAttribIndex = pAttrib->index;

						pAttrib->type->InitializeNewEconAttributeValue( &attribute.value );

						if ( !pAttrib->type->BConvertStringToEconAttributeValue( pAttrib, pAttribData->GetString(), &attribute.value ) )
							continue;
#else
						if ( !attribute.BInitFromKV_SingleLine( pAttribData ) )
							continue;
#endif

						pItem->attributes.AddToTail( attribute );
					}
				}
				else if ( !V_stricmp( pSubData->GetName(), "visuals_mvm_boss" ) )
				{
					// Deliberately skipping this.
				}
				else if ( !V_strnicmp( pSubData->GetName(), "visuals", 7 ) )
				{
					// Figure out what team is this meant for.
					int iVisuals = UTIL_StringFieldToInt( pSubData->GetName(), g_TeamVisualSections, TF_TEAM_COUNT );

					if ( iVisuals != -1 )
					{
						if ( iVisuals == TEAM_UNASSIGNED )
						{
							// Hacky: for standard visuals block, assign it to all teams at once.
							for ( int i = 0; i < TF_TEAM_COUNT; i++ )
							{
								if ( i == TEAM_SPECTATOR )
									continue;

								ParseVisuals( pSubData, pItem, i );
							}
						}
						else
						{
							ParseVisuals( pSubData, pItem, iVisuals );
						}
					}
				}
				else if ( !V_stricmp( pSubData->GetName(), "taunt" ) )
				{
					for ( KeyValues *pTauntData = pSubData->GetFirstSubKey(); pTauntData != NULL; pTauntData = pTauntData->GetNextKey() )
					{
						GET_BOOL( pItem, pTauntData, is_partner_taunt );
						GET_BOOL( pItem, pTauntData, stop_taunt_if_moved );
						GET_FLOAT( pItem, pTauntData, taunt_separation_forward_distance );
						GET_FLOAT( pItem, pTauntData, taunt_separation_right_distance );
						GET_FLOAT( pItem, pTauntData, camera_dist_up );
						GET_FLOAT( pItem, pTauntData, min_taunt_time );
						GET_STRING( pItem, pTauntData, particle_attachment );

						if ( !V_stricmp( pTauntData->GetName(), "custom_taunt_scene_per_class" ) )
						{
							for ( KeyValues *pScenesData = pTauntData->GetFirstSubKey(); pScenesData != NULL; pScenesData = pScenesData->GetNextKey() )
							{
								const char *pszClass = pScenesData->GetName();
								int iClass = UTIL_StringFieldToInt( pszClass, g_aPlayerClassNames_NonLocalized, TF_CLASS_COUNT_ALL );

								if ( iClass != -1 )
								{
									V_strncpy( pItem->custom_taunt_scene_per_class[iClass], pScenesData->GetString(), 128 );
								}
							}
						}
						else if ( !V_stricmp( pTauntData->GetName(), "custom_taunt_outro_scene_per_class" ) )
						{
							for ( KeyValues *pScenesData = pTauntData->GetFirstSubKey(); pScenesData != NULL; pScenesData = pScenesData->GetNextKey() )
							{
								const char *pszClass = pScenesData->GetName();
								int iClass = UTIL_StringFieldToInt( pszClass, g_aPlayerClassNames_NonLocalized, TF_CLASS_COUNT_ALL );

								if ( iClass != -1 )
								{
									V_strncpy( pItem->custom_taunt_outro_scene_per_class[iClass], pScenesData->GetString(), 128 );
								}
							}
						}
						else if ( !V_stricmp( pTauntData->GetName(), "custom_taunt_prop_per_class" ) )
						{
							for ( KeyValues *pScenesData = pTauntData->GetFirstSubKey(); pScenesData != NULL; pScenesData = pScenesData->GetNextKey() )
							{
								const char *pszClass = pScenesData->GetName();
								int iClass = UTIL_StringFieldToInt( pszClass, g_aPlayerClassNames_NonLocalized, TF_CLASS_COUNT_ALL );

								if ( iClass != -1 )
								{
									V_strncpy( pItem->custom_taunt_prop_per_class[iClass], pScenesData->GetString(), 128 );
								}
							}
						}
						else if ( !V_stricmp( pTauntData->GetName(), "custom_taunt_prop_scene_per_class" ) )
						{
							for ( KeyValues *pScenesData = pTauntData->GetFirstSubKey(); pScenesData != NULL; pScenesData = pScenesData->GetNextKey() )
							{
								const char *pszClass = pScenesData->GetName();
								int iClass = UTIL_StringFieldToInt( pszClass, g_aPlayerClassNames_NonLocalized, TF_CLASS_COUNT_ALL );

								if ( iClass != -1 )
								{
									V_strncpy( pItem->custom_taunt_prop_scene_per_class[iClass], pScenesData->GetString(), 128 );
								}
							}
						}
						else if ( !V_stricmp( pTauntData->GetName(), "custom_taunt_prop_outro_scene_per_class" ) )
						{
							for ( KeyValues *pScenesData = pTauntData->GetFirstSubKey(); pScenesData != NULL; pScenesData = pScenesData->GetNextKey() )
							{
								const char *pszClass = pScenesData->GetName();
								int iClass = UTIL_StringFieldToInt( pszClass, g_aPlayerClassNames_NonLocalized, TF_CLASS_COUNT_ALL );

								if ( iClass != -1 )
								{
									V_strncpy( pItem->custom_taunt_prop_outro_scene_per_class[iClass], pScenesData->GetString(), 128 );
								}
							}
						}
						else if ( !V_stricmp( pTauntData->GetName(), "custom_partner_taunt_per_class" ) )
						{
							for ( KeyValues *pScenesData = pTauntData->GetFirstSubKey(); pScenesData != NULL; pScenesData = pScenesData->GetNextKey() )
							{
								const char *pszClass = pScenesData->GetName();
								int iClass = UTIL_StringFieldToInt( pszClass, g_aPlayerClassNames_NonLocalized, TF_CLASS_COUNT_ALL );

								if ( iClass != -1 )
								{
									V_strncpy( pItem->custom_partner_taunt_per_class[iClass], pScenesData->GetString(), 128 );
								}
							}
						}
						else if ( !V_stricmp( pTauntData->GetName(), "custom_partner_taunt_initiator_per_class" ) )
						{
							for ( KeyValues *pScenesData = pTauntData->GetFirstSubKey(); pScenesData != NULL; pScenesData = pScenesData->GetNextKey() )
							{
								const char *pszClass = pScenesData->GetName();
								int iClass = UTIL_StringFieldToInt( pszClass, g_aPlayerClassNames_NonLocalized, TF_CLASS_COUNT_ALL );

								if ( iClass != -1 )
								{
									V_strncpy( pItem->custom_partner_taunt_initiator_per_class[iClass], pScenesData->GetString(), 128 );
								}
							}
						}
						else if ( !V_stricmp( pTauntData->GetName(), "custom_partner_taunt_receiver_per_class" ) )
						{
							for ( KeyValues *pScenesData = pTauntData->GetFirstSubKey(); pScenesData != NULL; pScenesData = pScenesData->GetNextKey() )
							{
								const char *pszClass = pScenesData->GetName();
								int iClass = UTIL_StringFieldToInt( pszClass, g_aPlayerClassNames_NonLocalized, TF_CLASS_COUNT_ALL );

								if ( iClass != -1 )
								{
									V_strncpy( pItem->custom_partner_taunt_receiver_per_class[iClass], pScenesData->GetString(), 128 );
								}
							}
						}
						else if ( !V_stricmp( pTauntData->GetName(), "custom_taunt_input_remap" ) )
						{
							/*for ( KeyValues *pButtonData = pTauntData->GetFirstSubKey(); pButtonData != NULL; pButtonData = pButtonData->GetNextKey() )
							{
								if ( !V_stricmp( pButtonData->GetName(), "IN_ATTACK" ) )
								{
									for ( KeyValues *pStateData = pTauntData->GetFirstSubKey(); pStateData != NULL; pStateData = pStateData->GetNextKey() )
									{
										if ( !V_stricmp( pStateData->GetName(), "pressed" ) )
										{
											for ( KeyValues *pScenesData = pTauntData->GetFirstSubKey(); pScenesData != NULL; pScenesData = pScenesData->GetNextKey() )
											{
												const char *pszClass = pScenesData->GetName();
												int iClass = UTIL_StringFieldToInt( pszClass, g_aPlayerClassNames_NonLocalized, TF_CLASS_COUNT_ALL );
												if ( iClass != -1 )
													V_strncpy( pItem->custom_partner_taunt_receiver_per_class[iClass], pScenesData->GetString(), 128 );
											}
										}
									}
								}
							}*/
						}
					}
				}
			}

			GetItemSchema()->m_Items.Insert( index, pItem );
		}
	};

	void ParseAttributes( KeyValues *pKeyValuesData )
	{
		for ( KeyValues *pSubData = pKeyValuesData->GetFirstTrueSubKey(); pSubData != NULL; pSubData = pSubData->GetNextTrueSubKey() )
		{
			CEconAttributeDefinition *pAttribute = new CEconAttributeDefinition;
			pAttribute->index = V_atoi( pSubData->GetName() );

			GET_STRING_DEFAULT( pAttribute, pSubData, name, ( unnamed ) );
			GET_STRING( pAttribute, pSubData, attribute_class );
			GET_STRING( pAttribute, pSubData, description_string );
			pAttribute->string_attribute = ( V_stricmp( pSubData->GetString( "attribute_type" ), "string" ) == 0 );

			const char *pszFormat = pSubData->GetString( "description_format" );
			pAttribute->description_format = UTIL_StringFieldToInt( pszFormat, g_AttributeDescriptionFormats, ARRAYSIZE( g_AttributeDescriptionFormats ) );

			const char *pszEffect = pSubData->GetString( "effect_type" );
			pAttribute->effect_type = UTIL_StringFieldToInt( pszEffect, g_EffectTypes, ARRAYSIZE( g_EffectTypes ) );

			const char *pszType = pSubData->GetString( "attribute_type" );
			pAttribute->type = GetItemSchema()->GetAttributeType( pszType );

			GET_BOOL( pAttribute, pSubData, hidden );
			GET_BOOL( pAttribute, pSubData, stored_as_integer );

			GetItemSchema()->m_Attributes.Insert( pAttribute->index, pAttribute );
		}
	};

	bool ParseVisuals( KeyValues *pData, CEconItemDefinition* pItem, int iIndex )
	{
		EconPerTeamVisuals *pVisuals = &pItem->visual[iIndex];

		for ( KeyValues *pVisualData = pData->GetFirstSubKey(); pVisualData != NULL; pVisualData = pVisualData->GetNextKey() )
		{
			if ( !V_stricmp( pVisualData->GetName(), "player_bodygroups" ) )
			{
				GET_VALUES_FAST_BOOL( pVisuals->player_bodygroups, pVisualData );
			}
			else if ( !V_stricmp( pVisualData->GetName(), "player_poseparam" ) )
			{
				GET_VALUES_FAST_BOOL( pVisuals->player_poseparam, pVisualData );
			}
			else if ( !V_stricmp( pVisualData->GetName(), "attached_models" ) )
			{
				for (KeyValues *pAttachment = pVisualData->GetFirstSubKey(); pAttachment != NULL; pAttachment = pAttachment->GetNextKey())
				{
					AttachedModel_t attached_model;
					attached_model.model_display_flags = pAttachment->GetInt( "model_display_flags", AM_VIEWMODEL|AM_WORLDMODEL );
					V_strncpy( attached_model.model, pAttachment->GetString( "model" ), sizeof( attached_model.model ) );

					pVisuals->attached_models.AddToTail( attached_model );
				}
			}
			else if ( !V_stricmp( pVisualData->GetName(), "custom_particlesystem" ) )
			{
				V_strncpy( pVisuals->custom_particlesystem, pVisualData->GetString( "system" ), sizeof( pVisuals->custom_particlesystem ) );
			}
			else if ( !V_stricmp( pVisualData->GetName(), "custom_particlesystem2" ) )
			{
				V_strncpy( pVisuals->custom_particlesystem2, pVisualData->GetString( "system" ), sizeof( pVisuals->custom_particlesystem2 ) );
			}
			else if ( !V_stricmp( pVisualData->GetName(), "animation_replacement" ) )
			{
				for ( KeyValues *pKeyData = pVisualData->GetFirstSubKey(); pKeyData != NULL; pKeyData = pKeyData->GetNextKey() )
				{
					int key = ActivityList_IndexForName( pKeyData->GetName() );
					int value = ActivityList_IndexForName( pKeyData->GetString() );

					if ( key != kActivityLookup_Missing && value != kActivityLookup_Missing )
					{
						pVisuals->animation_replacement.Insert( key, value );
					}

					/*if ( !V_stricmp( pKeyData->GetName(), "taunt_concept" ) )
					{
						pVisuals->animation_replacement.Insert( pKeyData->GetName(), pKeyData->GetString() );
					}*/
				}
			}
			else if ( !V_stricmp( pVisualData->GetName(), "playback_activity" ) )
			{
				GET_VALUES_FAST_STRING( pVisuals->playback_activity, pVisualData );
			}
			else if ( !V_strnicmp( pVisualData->GetName(), "sound_", 6 ) )
			{
				// Fetching this similar to weapon script file parsing.
				// Advancing pointer past sound_ prefix... why couldn't they just make a subsection for sounds?
				int iSound = GetWeaponSoundFromString( pVisualData->GetName() + 6 );

				if ( iSound != -1 )
				{
					V_strncpy( pVisuals->aWeaponSounds[iSound], pVisualData->GetString(), MAX_WEAPON_STRING );
				}
			}
			else if ( !V_stricmp( pVisualData->GetName(), "styles" ) )
			{
				for ( KeyValues *pStyleData = pVisualData->GetFirstSubKey(); pStyleData != NULL; pStyleData = pStyleData->GetNextKey() )
				{
					EconItemStyle *style;
					IF_ELEMENT_FOUND( pVisuals->styles, pStyleData->GetName() )
					{
						style = pVisuals->styles.Element( index );
					}
					else
					{
						style = new EconItemStyle;
						pVisuals->styles.Insert( pStyleData->GetName(), style );
					}

					GET_STRING( style, pStyleData, name );
					GET_STRING( style, pStyleData, model_player );
					GET_STRING(style, pStyleData, model_world);
					GET_STRING( style, pStyleData, image_inventory );
					GET_BOOL( style, pStyleData, selectable );
					GET_INT( style, pStyleData, skin_red );
					GET_INT( style, pStyleData, skin_blu );

					for ( KeyValues *pStyleModelData = pStyleData->GetFirstSubKey(); pStyleModelData != NULL; pStyleModelData = pStyleModelData->GetNextKey() )
					{
						if ( !V_stricmp( pStyleModelData->GetName(), "model_player_per_class" ) )
						{
							GET_VALUES_FAST_STRING( style->model_player_per_class, pStyleModelData );
						}
					}
				}
			}
			else if ( !V_stricmp( pVisualData->GetName(), "skin" ) )
			{
				pVisuals->skin = pVisualData->GetInt();
			}
			else if ( !V_stricmp( pVisualData->GetName(), "use_per_class_bodygroups" ) )
			{
				pVisuals->use_per_class_bodygroups = pVisualData->GetInt();
			}
			else if ( !V_stricmp( pVisualData->GetName(), "wm_bodygroup_override" ) )
			{
				pVisuals->wm_bodygroup_override = pVisualData->GetInt();
			}
			else if ( !V_stricmp( pVisualData->GetName(), "vm_bodygroup_override" ) )
			{
				pVisuals->vm_bodygroup_override = pVisualData->GetInt();
			}
			else if ( !V_stricmp( pVisualData->GetName(), "wm_bodygroup_state_override" ) )
			{
				pVisuals->wm_bodygroup_state_override = pVisualData->GetInt();
			}
			else if ( !V_stricmp( pVisualData->GetName(), "vm_bodygroup_state_override" ) )
			{
				pVisuals->vm_bodygroup_state_override = pVisualData->GetInt();
			}
			else if ( !V_stricmp( pVisualData->GetName(), "muzzle_flash" ) )
			{
				V_strncpy( pVisuals->muzzle_flash, pVisualData->GetString(), sizeof( pVisuals->tracer_effect ) );
			}
			else if ( !V_stricmp( pVisualData->GetName(), "tracer_effect" ) )
			{
				V_strncpy( pVisuals->tracer_effect, pVisualData->GetString(), sizeof( pVisuals->tracer_effect ) );
			}
			else if ( !V_stricmp( pVisualData->GetName(), "material_override" ) )
			{
				V_strncpy( pVisuals->material_override, pVisualData->GetString(), sizeof( pVisuals->material_override ) );
			}
			else if ( !V_stricmp( pVisualData->GetName(), "particle_effect" ) )
			{
				V_strncpy( pVisuals->particle_effect, pVisualData->GetString(), sizeof( pVisuals->particle_effect ) );
			}
			else if ( !V_stricmp( pVisualData->GetName(), "custom_sound0" ) )
			{
				V_strncpy( pVisuals->custom_sound0, pVisualData->GetString(), sizeof( pVisuals->custom_sound0 ) );
			}
			else if ( !V_stricmp( pVisualData->GetName(), "custom_sound2" ) )
			{
				V_strncpy( pVisuals->custom_sound2, pVisualData->GetString(), sizeof( pVisuals->custom_sound2 ) );
			}
			else
			{
				GET_VALUES_FAST_STRING( pVisuals->misc_info, pVisualData );
			}
		}

		return true;
	}

protected:
	void MergeDefinitionPrefabs( KeyValues *pDefinition, KeyValues *pSchemeData )
	{
		char prefab[64];
		Q_snprintf( prefab, sizeof( prefab ), pSchemeData->GetString( "prefab" ) );

		if ( prefab[0] != '\0' )
		{
			//check if there's prefab for prefab.. PREFABSEPTION
			CUtlStringList strings;
			V_SplitString( prefab, " ", strings );

			FOR_EACH_VEC_BACK( strings, i )
			{
				KeyValues *pPrefabValues = NULL;
				FIND_ELEMENT( GetItemSchema()->m_PrefabsValues, strings[i], pPrefabValues );
				if ( pPrefabValues )
				{
					MergeDefinitionPrefabs( pDefinition, pPrefabValues );
				}
			}
		}

		InheritKVRec( pSchemeData, pDefinition );
	}

	void InheritKVRec( KeyValues *pFrom, KeyValues *pTo )
	{
		for ( KeyValues *pSubData = pFrom->GetFirstSubKey(); pSubData != NULL; pSubData = pSubData->GetNextKey() )
		{
			switch ( pSubData->GetDataType() )
			{
				// Identifies the start of a subsection
				case KeyValues::TYPE_NONE:
				{
					KeyValues *pKey = pTo->FindKey( pSubData->GetName() );
					if ( pKey == NULL )
					{
						pKey = pTo->CreateNewKey();
						pKey->SetName( pSubData->GetName() );
					}

					InheritKVRec( pSubData, pKey );
					break;
				}
				// Actual types
				case KeyValues::TYPE_STRING:
				{
					pTo->SetString( pSubData->GetName(), pSubData->GetString() );
					break;
				}
				case KeyValues::TYPE_INT:
				{
					pTo->SetInt( pSubData->GetName(), pSubData->GetInt() );
					break;
				}
				case KeyValues::TYPE_FLOAT:
				{
					pTo->SetFloat( pSubData->GetName(), pSubData->GetFloat() );
					break;
				}
				case KeyValues::TYPE_WSTRING:
				{
					pTo->SetWString( pSubData->GetName(), pSubData->GetWString() );
					break;
				}
				case KeyValues::TYPE_COLOR:
				{
					pTo->SetColor( pSubData->GetName(), pSubData->GetColor() );
					break;
				}
				case KeyValues::TYPE_UINT64:
				{
					pTo->SetUint64( pSubData->GetName(), pSubData->GetUint64() );
					break;
				}
				default:
					break;
			}
		}
	}
};
CEconSchemaParser g_EconSchemaParser;

#if defined( _LINUX )
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
template<> CSchemaFieldHandle<CEconAttributeDefinition>::CSchemaFieldHandle( char const *name )
        : m_pName( name ), m_pSchema( GetItemSchema()->GetSchemaKeyValues() )
{
        m_pHandle = GetItemSchema()->GetAttributeDefinitionByName( name );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
template<> CSchemaFieldHandle<CEconAttributeDefinition>::operator const CEconAttributeDefinition *( )
{
	return m_pHandle;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
template<> CSchemaFieldHandle<CEconItemDefinition>::CSchemaFieldHandle( char const *name )
        : m_pName( name ), m_pSchema( GetItemSchema()->GetSchemaKeyValues() )
{
        m_pHandle = GetItemSchema()->GetItemDefinitionByName( name );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
template<> CSchemaFieldHandle<CEconItemDefinition>::operator const CEconItemDefinition *( )
{
        return m_pHandle;
}
#endif

//-----------------------------------------------------------------------------
// Purpose: constructor
//-----------------------------------------------------------------------------
CEconItemSchema::CEconItemSchema()
{
	SetDefLessFunc( m_Items );
	SetDefLessFunc( m_Attributes );

	m_bInited = false;
}

CEconItemSchema::~CEconItemSchema()
{
	FOR_EACH_DICT_FAST( m_PrefabsValues, i )
	{
		m_PrefabsValues[i]->deleteThis();
	}
	m_PrefabsValues.RemoveAll();

	m_Items.PurgeAndDeleteElements();
	m_Attributes.PurgeAndDeleteElements();

	for ( attr_type_t const &atype : m_AttributeTypes )
		delete atype.pType;
}

//-----------------------------------------------------------------------------
// Purpose: Initializer
//-----------------------------------------------------------------------------
bool CEconItemSchema::Init( void )
{
	//if ( !m_bInited )
	//{
		// Must register activities early so we can parse animation replacements.
		ActivityList_Free();
		ActivityList_RegisterSharedActivities();

		InitAttributeTypes();

		float flStartTime = engine->Time();
		g_EconSchemaParser.InitParser( "scripts/items/items_game.txt", true, false );
		float flEndTime = engine->Time();
		ConColorMsg( Color( 77, 116, 85, 255 ), "[%s] Processing item schema took %.02fms. Parsed %d items and %d attributes.\n",
			CBaseEntity::IsServer() ? "SERVER" : "CLIENT",
			( flEndTime - flStartTime ) * 1000.0f,
			m_Items.Count(),
			m_Attributes.Count() );

		m_bInited = true;
	//}

	return true;
}

void CEconItemSchema::InitAttributeTypes( void )
{
	attr_type_t defaultType;
	defaultType.szName = NULL;
	defaultType.pType = new CSchemaAttributeType_Default;
	m_AttributeTypes[ m_AttributeTypes.AddToTail() ] = defaultType;

	attr_type_t longType;
	longType.szName = "uint64";
	longType.pType = new CSchemaAttributeType_UInt64;
	m_AttributeTypes[ m_AttributeTypes.AddToTail() ] = longType;

	attr_type_t floatType;
	floatType.szName = "float";
	floatType.pType = new CSchemaAttributeType_Float;
	m_AttributeTypes[ m_AttributeTypes.AddToTail() ] = floatType;

	attr_type_t stringType;
	stringType.szName = "string";
	stringType.pType = new CSchemaAttributeType_String;
	m_AttributeTypes[ m_AttributeTypes.AddToTail() ] = stringType;
}

//-----------------------------------------------------------------------------
// Purpose: Runs on level start, precaches models and sounds from schema.
//-----------------------------------------------------------------------------
void CEconItemSchema::Precache( void )
{
	static CSchemaFieldHandle<CEconAttributeDefinition> pAttribDef_CosmeticTauntSound( "cosmetic taunt sound" );
	static CSchemaFieldHandle<CEconAttributeDefinition> pAttribDef_TauntSuccessSound( "taunt success sound" );
	static CSchemaFieldHandle<CEconAttributeDefinition> pAttribDef_TauntSuccessSoundLoop( "taunt success sound loop" );

	// Precache everything from schema.
	FOR_EACH_MAP( m_Items, i )
	{
		CEconItemDefinition *pItem = m_Items[i];

		for ( int iClass = 0; iClass < TF_CLASS_COUNT_ALL; iClass++ )
		{
			const char *pszTauntProp = pItem->custom_taunt_prop_per_class[iClass];
			if ( pszTauntProp[0] != '\0' )
				CBaseEntity::PrecacheModel( pszTauntProp );

#ifdef GAME_DLL
			const char *pszTauntScene = pItem->custom_taunt_scene_per_class[iClass];
			if ( pszTauntScene[0] != '\0' )
				PrecacheInstancedScene( pszTauntScene );

			const char *pszTauntOutroScene = pItem->custom_taunt_outro_scene_per_class[iClass];
			if ( pszTauntOutroScene[0] != '\0' )
				PrecacheInstancedScene( pszTauntOutroScene );

			const char *pszTauntPropScene = pItem->custom_taunt_prop_scene_per_class[iClass];
			if ( pszTauntPropScene[0] != '\0' )
				PrecacheInstancedScene( pszTauntPropScene );

			const char *pszTauntPropOutroScene = pItem->custom_taunt_prop_outro_scene_per_class[iClass];
			if ( pszTauntPropOutroScene[0] != '\0' )
				PrecacheInstancedScene( pszTauntPropOutroScene );

			const char *pszTauntPartnerScene = pItem->custom_partner_taunt_per_class[iClass];
			if ( pszTauntPartnerScene[0] != '\0' )
				PrecacheInstancedScene( pszTauntPartnerScene );

			const char *pszTauntPartnerInitiatorScene = pItem->custom_partner_taunt_initiator_per_class[iClass];
			if ( pszTauntPartnerInitiatorScene[0] != '\0' )
				PrecacheInstancedScene( pszTauntPartnerInitiatorScene );

			const char *pszTauntPartnerReceiverScene = pItem->custom_partner_taunt_receiver_per_class[iClass];
			if ( pszTauntPartnerReceiverScene[0] != '\0' )
				PrecacheInstancedScene( pszTauntPartnerReceiverScene );
#endif
		}

		// Precache visuals.
		for ( int i = 0; i < TF_TEAM_COUNT; i++ )
		{
			if ( i == TEAM_SPECTATOR )
				continue;

			PerTeamVisuals_t *pVisuals = &pItem->visual[i];

			// Precache sounds.
			const char *pszCustomSound = pVisuals->custom_sound0;
			if ( pszCustomSound[0] != '\0' )
			{
				CBaseEntity::PrecacheScriptSound( pszCustomSound );
			}

			const char *pszCustomSoundTwo = pVisuals->custom_sound2;
			if ( pszCustomSoundTwo[0] != '\0' )
			{
				CBaseEntity::PrecacheScriptSound( pszCustomSoundTwo );
			}
		}

		// Cache all attrbute names.
		for ( static_attrib_t const &attrib : pItem->attributes )
		{
			const CEconAttributeDefinition *pAttribute = attrib.GetStaticData();

			// Special case for string attribute.
			if ( pAttribute == pAttribDef_TauntSuccessSound ||
				 pAttribute == pAttribDef_TauntSuccessSoundLoop )
				CBaseEntity::PrecacheScriptSound( attrib.value.sVal->Get() );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CEconItemDefinition* CEconItemSchema::GetItemDefinition( int id )
{
	if ( id < 0 )
		return NULL;

	CEconItemDefinition *itemdef = NULL;
	FIND_ELEMENT( m_Items, id, itemdef );
	return itemdef;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CEconItemDefinition *CEconItemSchema::GetItemDefinitionByName( const char *name )
{
	FOR_EACH_MAP_FAST( m_Items, i )
	{
		if ( m_Items[i]->index > -1 && !V_stricmp( m_Items[i]->name, name ) )
		{
			return m_Items[i];
		}
	}

	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CEconAttributeDefinition *CEconItemSchema::GetAttributeDefinition( int id )
{
	if ( id < 0 )
		return NULL;

	CEconAttributeDefinition *attribdef = NULL;
	FIND_ELEMENT( m_Attributes, id, attribdef );
	return attribdef;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CEconAttributeDefinition *CEconItemSchema::GetAttributeDefinitionByName( const char *name )
{
	FOR_EACH_MAP_FAST( m_Attributes, i )
	{
		if ( !V_stricmp( m_Attributes[i]->name, name ) )
		{
			return m_Attributes[i];
		}
	}

	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CEconAttributeDefinition *CEconItemSchema::GetAttributeDefinitionByClass( const char *classname )
{
	FOR_EACH_MAP_FAST( m_Attributes, i )
	{
		if ( !V_stricmp( m_Attributes[i]->attribute_class, classname ) )
		{
			return m_Attributes[i];
		}
	}

	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CEconItemSchema::GetAttributeIndex( const char *name )
{
	if ( !name )
		return -1;

	FOR_EACH_MAP_FAST( m_Attributes, i )
	{
		if ( !V_stricmp( m_Attributes[i]->name, name ) )
		{
			return m_Attributes.Key( i );
		}
	}

	return -1;
}

ISchemaAttributeType *CEconItemSchema::GetAttributeType( const char *name ) const
{
	for ( attr_type_t const &type : m_AttributeTypes )
	{
		if ( type.szName == name )
			return type.pType;
	}

	return nullptr;
}
