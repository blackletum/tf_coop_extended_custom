//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//========================================================================//

#include "cbase.h"
#include "econ_holidays.h"
#include "tf_shareddefs.h"
#include "rtime.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


int EconHolidays_GetHolidayForString( const char *pszHolidayName )
{
	for ( int i = 0; i < kHolidayCount; i )
	{
		if ( !Q_strcmp(pszHolidayName, g_pszEconHolidayNames[i]) )
			return (EHoliday)i;
	}

	return kHoliday_None;
}

const char* EconHolidays_GetActiveHolidayString()
{
	for ( int i = 0; i < kHolidayCount; i )
	{
		if ( EconHolidays_IsHolidayActive( i, CRTime::RTime32TimeCur() ) )
		{
			return g_pszEconHolidayNames[i];
		}
	}

	return g_pszEconHolidayNames[kHoliday_None];
}

bool EconHolidays_IsHolidayActive( /*EHoliday*/ int holiday, const uint32 curtime )
{
	time_t ltime = curtime;
	const time_t *ptime = &ltime;
	struct tm *today = localtime( ptime );
	if ( today )
	{
		if ( ( today->tm_mon == 1 ) && today->tm_mday == 14 )
		{
			if ( holiday == kHoliday_ValentinesDay || holiday == kHoliday_HalloweenOrFullMoonOrValentines )
				return true;
		}
		else if ( ( today->tm_mon == 3 ) && today->tm_mday == 1 )
		{
			if ( holiday == kHoliday_AprilFools )
				return true;
		}
		else if ( ( today->tm_mon == 5 ) && today->tm_mday == 27 )
		{
			if ( holiday == kHoliday_MeetThePyro )
				return true;
		}
		else if ( ( today->tm_mon == 7 ) && ( today->tm_mday == 23 || today->tm_mday == 24 ) )
		{
			if ( holiday == kHoliday_TF2Birthday )
				return true;
		}
		else if ( ( today->tm_mon == 7 ) && today->tm_mday == 18 )
		{
			if ( holiday == kHoliday_LFBirthday )
				return true;
		}
		if ( ( ( today->tm_mon == 9 ) && ( ( today->tm_mday >= 23 ) && ( today->tm_mday <= 31 ) ) ) || ( ( today->tm_mon == 10 ) && ( today->tm_mday <= 2 ) ) )
		{
			if ( holiday == kHoliday_Halloween || holiday == kHoliday_HalloweenOrFullMoon || holiday == kHoliday_HalloweenOrFullMoonOrValentines )
				return true;
		}
		else if ( ( today->tm_mon == 11 ) && ( ( today->tm_mday >= 20 ) && ( today->tm_mday <= 30 ) ) )
		{
			if ( holiday == kHoliday_Christmas )
				return true;
		}
		else if ( today->tm_mon == 11 && ( ( today->tm_mday >= 8 ) && ( today->tm_mday <= 17 ) ) )
		{
			if ( holiday == kHoliday_EOTL )
				return true;
		}
		else if ( ( ( today->tm_mon == 11 ) && ( today->tm_mday == 31 ) ) || ( ( today->tm_mon == 0 ) && ( today->tm_mday == 1 ) ) )
		{
			if ( holiday == kHoliday_NewYears )
				return true;
		}
		else if ( today->tm_mon == 2 && ( ( today->tm_mday >= 23 ) && ( today->tm_mday <= 27 ) ) )
		{
			if ( holiday == kHoliday_HLAlyx )
				return true;
		}

		// vintage math check
		{
			// We convert our date to difference in days since 18:14 UTC on 01/6/2000, the first new moon of 2000. (Also referred to as Lunation Number 0/Brown Lunation Number 953)
			// Year calculations are based on the difference since 1900, so offset for that.
			float flDaysSinceMeeusMoon = ( ( ( ( today->tm_year + 1900 ) * 365.25 ) + ( today->tm_yday + ( ( today->tm_hour + ( today->tm_min / 60 ) ) / 24 ) ) ) - ( ( 2000 * 365.25 ) + ( 5 + ( ( 18 + ( 14 / 60 ) ) / 24 ) ) ) );

			// Check how many New Moons there have been, and find our Lunation Number.
			float flMeanMoonDays = 29.530587981; // Mean difference between full moons, in days. Variation is -0.259/+0.302.
			float flMeeusLunationDecimal = flDaysSinceMeeusMoon / flMeanMoonDays; 
			int iMeeusLunationNumber = flDaysSinceMeeusMoon / flMeanMoonDays; // Amount of moon cycles since LN 0/BLN 953.

			// Check how close we are to a new lunar month.
			// A New Moon is at 0.
			float flCurrentMoonPhase = ( flMeeusLunationDecimal - iMeeusLunationNumber );

			// TF2 does their full moons for a 24 hour period, so it should be Full Moon +/- 12 hours.
			// A Full Moon is halfway after a New Moon, so add 0.5 to our tolerances.
			int iFullMoonHours = 12;
			float flFullMoonTolerance = ( ( iFullMoonHours / 24 ) / flMeanMoonDays );
			float iFullMoonPhaseMax = 0.5 + flFullMoonTolerance;
			float iFullMoonPhaseMin = 0.5 - flFullMoonTolerance;

			if ( ( flCurrentMoonPhase >= iFullMoonPhaseMin ) && ( flCurrentMoonPhase <= iFullMoonPhaseMax ) )
			{
				if ( holiday == kHoliday_FullMoon )
					return true;
			}
		}

		//kHoliday_CommunityUpdate?

		if ( holiday == kHoliday_None )
			return true;
	}
	else
	{
		if ( holiday == kHoliday_None )
			return true;
	}

	return false;
}