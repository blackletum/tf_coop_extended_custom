#include "cbase.h"
#include "attribute_types.h"

#if defined( _LINUX )
template<> void CSchemaAttributeTypeBase<unsigned int>::InitializeNewEconAttributeValue( attrib_data_union_t *pValue ) const
{
	pValue->iVal = 0;
}

template<> void CSchemaAttributeTypeBase<unsigned int>::UnloadEconAttributeValue( attrib_data_union_t *pValue ) const
{
}

template<> bool CSchemaAttributeTypeBase<unsigned int>::OnIterateAttributeValue( IEconAttributeIterator *pIterator, const CEconAttributeDefinition *pAttrDef, const attrib_data_union_t &value ) const
{
	return pIterator->OnIterateAttributeValue( pAttrDef, value.iVal );
}

template<> void CSchemaAttributeTypeBase<unsigned long long>::InitializeNewEconAttributeValue( attrib_data_union_t *pValue ) const
{
	pValue->lVal = new unsigned long long;
}

template<> void CSchemaAttributeTypeBase<unsigned long long>::UnloadEconAttributeValue( attrib_data_union_t *pValue ) const
{
	if ( pValue->lVal )
		delete pValue->lVal;
}

template<> bool CSchemaAttributeTypeBase<unsigned long long>::OnIterateAttributeValue( IEconAttributeIterator *pIterator, const CEconAttributeDefinition *pAttrDef, const attrib_data_union_t &value ) const
{
	return pIterator->OnIterateAttributeValue( pAttrDef, *(value.lVal) );
}

template<> void CSchemaAttributeTypeBase<float>::InitializeNewEconAttributeValue( attrib_data_union_t *pValue ) const
{
	pValue->iVal = 0;
}

template<> void CSchemaAttributeTypeBase<float>::UnloadEconAttributeValue( attrib_data_union_t *pValue ) const
{
}

template<> bool CSchemaAttributeTypeBase<float>::OnIterateAttributeValue( IEconAttributeIterator *pIterator, const CEconAttributeDefinition *pAttrDef, const attrib_data_union_t &value ) const
{
	return pIterator->OnIterateAttributeValue( pAttrDef, value.flVal );
}

template<> void CSchemaAttributeTypeBase<CAttribute_String>::InitializeNewEconAttributeValue( attrib_data_union_t *pValue ) const
{
	pValue->sVal = new CAttribute_String;
}

template<> void CSchemaAttributeTypeBase<CAttribute_String>::UnloadEconAttributeValue( attrib_data_union_t *pValue ) const
{
	if ( pValue->sVal )
		delete pValue->sVal;
}

template<> bool CSchemaAttributeTypeBase<CAttribute_String>::OnIterateAttributeValue( IEconAttributeIterator *pIterator, const CEconAttributeDefinition *pAttrDef, const attrib_data_union_t &value ) const
{
	return pIterator->OnIterateAttributeValue( pAttrDef, *(value.sVal) );
}
#endif