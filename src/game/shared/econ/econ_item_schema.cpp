#include "cbase.h"
#include "econ_item_schema.h"
#include "econ_item_system.h"
#include "attribute_types.h"
#include "tier3/tier3.h"
#include "vgui/ILocalize.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

const char *EconQuality_GetColorString( EEconItemQuality quality )
{
	if ( quality >= 0 && quality < QUALITY_COUNT )
	{
		return g_szQualityColorStrings[quality];
	}

	return NULL;
}

const char *EconQuality_GetLocalizationString( EEconItemQuality quality )
{
	if ( quality >= 0 && quality < QUALITY_COUNT )
	{
		return g_szQualityLocalizationStrings[quality];
	}

	return NULL;
}

BEGIN_NETWORK_TABLE_NOBASE( CEconItemAttribute, DT_EconItemAttribute )
#ifdef CLIENT_DLL
	RecvPropInt( RECVINFO( m_iAttributeDefinitionIndex ) ),
	RecvPropInt( RECVINFO( m_iRawValue32 ) ),
#else
	SendPropInt( SENDINFO( m_iAttributeDefinitionIndex ) ),
	SendPropInt( SENDINFO( m_iRawValue32 ), -1, SPROP_UNSIGNED ),
#endif
END_NETWORK_TABLE()

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CEconItemAttribute::CEconItemAttribute( CEconItemAttribute const &src )
{
	m_iAttributeDefinitionIndex = src.m_iAttributeDefinitionIndex;
	m_iRawValue32 = src.m_iRawValue32;
	m_iAttributeClass = src.m_iAttributeClass;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CEconItemAttribute::Init( int iIndex, float flValue, const char *pszAttributeClass /*= NULL*/ )
{
	m_iAttributeDefinitionIndex = iIndex;

	m_iRawValue32 = FloatBits( flValue );

	if ( pszAttributeClass )
	{
		m_iAttributeClass = AllocPooledString_StaticConstantStringPointer( pszAttributeClass );
	}
	else
	{
		CEconAttributeDefinition *pAttribDef = GetStaticData();
		if ( pAttribDef )
		{
			m_iAttributeClass = AllocPooledString_StaticConstantStringPointer( pAttribDef->attribute_class );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CEconItemAttribute::Init( int iIndex, const char *pszValue, const char *pszAttributeClass /*= NULL*/ )
{
	m_iAttributeDefinitionIndex = iIndex;

	m_iRawValue32 = *(unsigned int *)STRING( AllocPooledString( pszValue ) );

	if ( pszAttributeClass )
	{
		m_iAttributeClass = AllocPooledString_StaticConstantStringPointer( pszAttributeClass );
	}
	else
	{
		CEconAttributeDefinition *pAttribDef = GetStaticData();
		if ( pAttribDef )
		{
			m_iAttributeClass = AllocPooledString_StaticConstantStringPointer( pAttribDef->attribute_class );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CEconItemAttribute &CEconItemAttribute::operator=( CEconItemAttribute const &src )
{
	m_iAttributeDefinitionIndex = src.m_iAttributeDefinitionIndex;
	m_iRawValue32 = src.m_iRawValue32;
	m_iAttributeClass = src.m_iAttributeClass;

	return *this;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CEconAttributeDefinition *CEconItemAttribute::GetStaticData( void )
{
	return GetItemSchema()->GetAttributeDefinition( m_iAttributeDefinitionIndex );
}


//=============================================================================
// CEconItemDefinition
//=============================================================================

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CEconItemDefinition::~CEconItemDefinition()
{
	FOR_EACH_VEC( attributes, i )
	{
		CEconAttributeDefinition const *pDefinition = attributes[i].GetStaticData();
		pDefinition->type->UnloadEconAttributeValue( &attributes[i].value );
	}
	attributes.RemoveAll();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
PerTeamVisuals_t *CEconItemDefinition::GetVisuals( int iTeamNum /*= TEAM_UNASSIGNED*/ )
{
	if ( iTeamNum > LAST_SHARED_TEAM && iTeamNum < TF_TEAM_COUNT )
	{
		return &visual[iTeamNum];
	}

	return &visual[TEAM_UNASSIGNED];
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CEconItemDefinition::GetLoadoutSlot( int iClass /*= TF_CLASS_UNDEFINED*/ )
{
	if ( iClass && item_slot_per_class[iClass] != -1 )
	{
		return item_slot_per_class[iClass];
	}

	return item_slot;
}

//-----------------------------------------------------------------------------
// Purpose: Generate item name to show in UI with prefixes, qualities, etc...
//-----------------------------------------------------------------------------
const wchar_t *CEconItemDefinition::GenerateLocalizedFullItemName( void )
{
	static wchar_t wszFullName[256];

	wchar_t wszQuality[128] = { '\0' };

	if ( item_quality == QUALITY_UNIQUE )
	{
		// Attach "the" if necessary to unique items.
		if ( propername )
		{
			const wchar_t *pszPrepend = g_pVGuiLocalize->Find( "#TF_Unique_Prepend_Proper" );

			if ( pszPrepend )
			{
				V_wcsncpy( wszQuality, pszPrepend, sizeof( wszQuality ) );
			}
		}
	}
	else if ( item_quality != QUALITY_NORMAL )
	{
		// Live TF2 apparently allows multiple qualities per item but eh, we don't need that for now.
		const char *pszLocale = EconQuality_GetLocalizationString( item_quality );

		if ( pszLocale )
		{	
			const wchar_t *pszQuality = g_pVGuiLocalize->Find( pszLocale );

			if ( pszQuality )
			{
				V_wcsncpy( wszQuality, pszQuality, sizeof( wszQuality ) );
				// Add a space at the end.
				V_wcsncat( wszQuality, L" ", sizeof( wszQuality ) >> 2 );
			}
		}	
	}

	// Get base item name.
	wchar_t wszItemName[128];

	const wchar_t *pszLocalizedName = g_pVGuiLocalize->Find( item_name );
	if ( pszLocalizedName )
	{
		V_wcsncpy( wszItemName, pszLocalizedName, sizeof( wszItemName ) );
	}
	else
	{
		g_pVGuiLocalize->ConvertANSIToUnicode( item_name, wszItemName, sizeof( wszItemName ) );
	}

	// Oh boy.
	wchar_t wszCraftNumber[128] = { '\0' };
	wchar_t wszCraftSeries[128] = { '\0' };
	wchar_t wszToolTarget[128] = { '\0' };
	wchar_t wszRecipeComponent[128] = { '\0' };

	CAttribute_String strCustomName;
	static CSchemaFieldHandle<CEconAttributeDefinition> pAttrDef_CustomName( "custom name attr" );
	if ( pAttrDef_CustomName )
	{
		CAttributeIterator_GetSpecificAttribute<CAttribute_String> func( pAttrDef_CustomName, &strCustomName );
		IterateAttributes( &func );
	}

	if ( strCustomName && *strCustomName )
		g_pVGuiLocalize->ConvertANSIToUnicode( strCustomName, wszItemName, sizeof( wszItemName ) );

	const wchar_t *pszFormat = g_pVGuiLocalize->Find( "#ItemNameFormat" );

	if ( pszFormat )
	{
		g_pVGuiLocalize->ConstructString( wszFullName, sizeof( wszFullName ), pszFormat, 6,
			wszQuality, wszItemName, wszCraftNumber, wszCraftSeries, wszToolTarget, wszRecipeComponent );
	}
	else
	{
		V_wcsncpy( wszFullName, L"Unlocalized", sizeof( wszFullName ) );
	}

	return wszFullName;
}

//-----------------------------------------------------------------------------
// Purpose: Find an attribute with the specified class.
//-----------------------------------------------------------------------------
void CEconItemDefinition::IterateAttributes( IEconAttributeIterator *iter )
{
	FOR_EACH_VEC( attributes, i )
	{
		CEconAttributeDefinition const *pDefinition = attributes[i].GetStaticData();
		if ( !pDefinition->type->OnIterateAttributeValue( iter, pDefinition, attributes[ i ].value ) )
			return;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool static_attrib_t::BInitFromKV_SingleLine( KeyValues *const kv )
{
	CEconAttributeDefinition *pAttrib = GetItemSchema()->GetAttributeDefinitionByName( kv->GetName() );
	if( pAttrib == nullptr || pAttrib->index == -1 || pAttrib->type == nullptr )
		return false;

	iAttribIndex = pAttrib->index;

	pAttrib->type->InitializeNewEconAttributeValue( &value );

	if ( !pAttrib->type->BConvertStringToEconAttributeValue( pAttrib, kv->GetString(), &value ) )
		return false;

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool static_attrib_t::BInitFromKV_MultiLine( KeyValues *const kv )
{
	CEconAttributeDefinition *pAttrib = GetItemSchema()->GetAttributeDefinitionByName( kv->GetName() );
	if( pAttrib == nullptr || pAttrib->index == -1 || pAttrib->type == nullptr )
		return false;

	iAttribIndex = pAttrib->index;

	pAttrib->type->InitializeNewEconAttributeValue( &value );

	if ( !pAttrib->type->BConvertStringToEconAttributeValue( pAttrib, kv->GetString( "value" ), &value ) )
		return false;

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Construction
//-----------------------------------------------------------------------------
CAttribute_String::CAttribute_String()
{
	m_pString = &_default_value_;
	m_nLength = 0;
	m_bInitialized = false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CAttribute_String::CAttribute_String( CAttribute_String const &src )
{
	CAttribute_String();

	*this = src;
}

//-----------------------------------------------------------------------------
// Purpose: Destruction
//-----------------------------------------------------------------------------
CAttribute_String::~CAttribute_String()
{
	if ( m_pString != &_default_value_ && m_pString != nullptr )
		delete m_pString;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAttribute_String::Clear( void )
{
	if ( m_bInitialized && m_pString != &_default_value_ )
	{
		m_pString->Clear();
	}
	m_bInitialized = false;
}

//-----------------------------------------------------------------------------
// Purpose: Copy assignment
//-----------------------------------------------------------------------------
CAttribute_String &CAttribute_String::operator=( CAttribute_String const &src )
{
	Assert( this != &src );

	if ( src.m_bInitialized )
	{
		Initialize();
		m_pString->Set( src.Get() );
	}

	return *this;
}

//-----------------------------------------------------------------------------
// Purpose: String assignment
//-----------------------------------------------------------------------------
CAttribute_String &CAttribute_String::operator=( char const *src )
{
	Clear();
	Initialize();

	m_pString->Set( src );

	return *this;
}

CUtlConstString CAttribute_String::_default_value_;