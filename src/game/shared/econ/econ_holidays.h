//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//========================================================================//

#ifndef ECON_HOLIDAYS_H
#define ECON_HOLIDAYS_H

#ifdef _WIN32
#pragma once
#endif

int EconHolidays_GetHolidayForString( const char* pszHolidayName );
const char* EconHolidays_GetActiveHolidayString();
bool EconHolidays_IsHolidayActive( /*EHoliday*/ int holiday, const uint32 curtime );

#endif
