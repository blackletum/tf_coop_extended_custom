//=============================================================================//
//
// Purpose: change the difficulty on the fly
//
//=============================================================================//

#include "cbase.h"
#include "tf_gamerules.h"

class CLogicDifficulty : public CLogicalEntity
{
public:
	DECLARE_CLASS( CLogicDifficulty , CLogicalEntity );
	DECLARE_DATADESC();

	CLogicDifficulty ( void ) {}

	void InputSetDifficulty( inputdata_t &inputData );

};

LINK_ENTITY_TO_CLASS( logic_difficulty, CLogicDifficulty  );

BEGIN_DATADESC( CLogicDifficulty  )

	DEFINE_INPUTFUNC( FIELD_INTEGER, "SetDifficulty", InputSetDifficulty ),

END_DATADESC()

void CLogicDifficulty::InputSetDifficulty( inputdata_t &inputdata )
{
	TFGameRules()->SetSkillLevel( inputdata.value.Int() );
}