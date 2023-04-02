#include "NeoDateTime.h"

#include <limits>
#include <tuple>
#include <cmath>
#include <vector>
#include <string>

// http://howardhinnant.github.io/date_algorithms.html

// Returns number of days since civil 1970-01-01.  Negative values indicate
//    days prior to 1970-01-01.
// Preconditions:  y-m-d represents a date in the civil (Gregorian) calendar
//                 m is in [1, 12]
//                 d is in [1, last_day_of_month(y, m)]
//                 y is "approximately" in
//                   [numeric_limits<Int>::min()/366, numeric_limits<Int>::max()/366]
//                 Exact range of validity is:
//                 [civil_from_days(numeric_limits<Int>::min()),
//                  civil_from_days(numeric_limits<Int>::max()-719468)]
template <class Int>
static Int days_from_civil(Int y, unsigned m, unsigned d)
{
    static_assert(std::numeric_limits<unsigned>::digits >= 18, "This algorithm has not been ported to a 16 bit unsigned integer");
    static_assert(std::numeric_limits<Int>::digits >= 20, "This algorithm has not been ported to a 16 bit signed integer");
    y -= m <= 2;
    const Int era = (y >= 0 ? y : y - 399) / 400;
    const unsigned yoe = static_cast<unsigned>(y - era * 400);      // [0, 399]
    const unsigned doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;  // [0, 365]
    const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;         // [0, 146096]
    return era * 146097 + static_cast<Int>(doe)-719468;
}

// Returns year/month/day triple in civil calendar
// Preconditions:  z is number of days since 1970-01-01 and is in the range:
//                   [numeric_limits<Int>::min(), numeric_limits<Int>::max()-719468].
template <class Int>
static std::tuple<Int, unsigned, unsigned> civil_from_days(Int z)
{
    static_assert(std::numeric_limits<unsigned>::digits >= 18, "This algorithm has not been ported to a 16 bit unsigned integer");
    static_assert(std::numeric_limits<Int>::digits >= 20, "This algorithm has not been ported to a 16 bit signed integer");
    z += 719468;
    const Int era = (z >= 0 ? z : z - 146096) / 146097;
    const unsigned doe = static_cast<unsigned>(z - era * 146097);          // [0, 146096]
    const unsigned yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;  // [0, 399]
    const Int y = static_cast<Int>(yoe)+era * 400;
    const unsigned doy = doe - (365 * yoe + yoe / 4 - yoe / 100);                // [0, 365]
    const unsigned mp = (5 * doy + 2) / 153;                                   // [0, 11]
    const unsigned d = doy - (153 * mp + 2) / 5 + 1;                             // [1, 31]
    const unsigned m = mp + (mp < 10 ? 3 : -9);                            // [1, 12]
    return std::tuple<Int, unsigned, unsigned>(y + (m <= 2), m, d);
}

typedef std::chrono::duration<int, std::ratio<86400>> days;

static std::chrono::system_clock::time_point GetDate(std::chrono::system_clock::time_point Time)
{
    auto DaysFromEpoch = std::chrono::duration_cast<days>(Time.time_since_epoch());
    return std::chrono::system_clock::time_point(DaysFromEpoch);
}

NeoDateTimeUtc::NeoDateTimeUtc(int Year, int Day, int Hour, int Minute, int Second, std::int64_t Nanosecond)
	: Year(Year), Day(Day), Hour(Hour), Minute(Minute), Second(Second), Nanosecond(Nanosecond)
{
}

NeoDateTimeUtc NeoDateTimeUtc::FromTimePoint(std::chrono::system_clock::time_point Time)
{
    auto Epoch = static_cast<std::chrono::system_clock::time_point>(static_cast<days>(days_from_civil(2022, 12, 21))); //2022-12-21T21:48:01Z
    Epoch += static_cast<std::chrono::hours>(21);
    Epoch += static_cast<std::chrono::minutes>(48);
    Epoch += static_cast<std::chrono::seconds>(01);
    const double TerraTropicalPeriod = 365.2421897;

    auto YearDiff = static_cast<int>(std::floor(std::chrono::duration_cast<std::chrono::duration<double>>(Time - Epoch).count() / (24 * 3600) / TerraTropicalPeriod));
    auto YearStart = GetDate(Epoch + std::chrono::duration_cast<std::chrono::system_clock::duration>(std::chrono::duration<double>(YearDiff * TerraTropicalPeriod * 24 * 3600)));
    auto YearStartNext = GetDate(Epoch + std::chrono::duration_cast<std::chrono::system_clock::duration>(std::chrono::duration<double>((YearDiff + 1) * TerraTropicalPeriod * 24 * 3600)));
    if (Time >= YearStartNext)
    {
        YearDiff += 1;
        YearStart = YearStartNext;
    }
    auto Year = 2023 + YearDiff;
    auto Day = static_cast<int>(std::floor(std::chrono::duration_cast<std::chrono::duration<double>>(Time - YearStart).count() / (24 * 3600)));

    auto t = Time.time_since_epoch();
    auto DaysFromEpoch = std::chrono::duration_cast<days>(t);
    t -= DaysFromEpoch;
    auto Hour = std::chrono::duration_cast<std::chrono::hours>(t);
    t -= Hour;
    auto Minute = std::chrono::duration_cast<std::chrono::minutes>(t);
    t -= Minute;
    auto Second = std::chrono::duration_cast<std::chrono::seconds>(t);
    t -= Second;
    auto Nanosecond = std::chrono::duration_cast<std::chrono::nanoseconds>(t);

    return NeoDateTimeUtc(Year, Day, static_cast<int>(Hour.count()), static_cast<int>(Minute.count()), static_cast<int>(Second.count()), static_cast<std::int64_t>(Nanosecond.count()));
}

std::chrono::system_clock::time_point NeoDateTimeUtc::ToTimePoint()
{
    auto YearDiff = Year - 2023;
    auto YearStart = static_cast<std::chrono::system_clock::time_point>(static_cast<days>(days_from_civil(2022 + YearDiff, 12, 21)));
    auto Result = YearStart + days(Day) + std::chrono::hours(Hour) + std::chrono::minutes(Minute) + std::chrono::seconds(Second) + std::chrono::microseconds(Nanosecond / 1000);
    return Result;
}

NeoDateTimeLocal NeoDateTimeUtc::ToLocal(double OffsetAtTime)
{
    auto t = FromTimePoint(ToTimePoint() + std::chrono::seconds(static_cast<int>(std::floor(OffsetAtTime * 3600))));
    return NeoDateTimeLocal(t.Year, t.Day, t.Hour, t.Minute, t.Second, t.Nanosecond);
}

NeoDateTimeLocal::NeoDateTimeLocal(int Year, int Day, int Hour, int Minute, int Second, std::int64_t Nanosecond)
	: Year(Year), Day(Day), Hour(Hour), Minute(Minute), Second(Second), Nanosecond(Nanosecond)
{
}

std::string NeoDateTimeLocal::ToShortString()
{
    auto y = std::to_string(Year);
    auto d = std::to_string(Day);
    if (d.size() < 3)
    {
        d = std::string(3 - d.size(), '0') + d;
    }
    auto h = std::to_string(Hour);
    if (h.size() < 2)
    {
        h = std::string(2 - h.size(), '0') + h;
    }
    auto m = std::to_string(Minute);
    if (m.size() < 2)
    {
        m = std::string(2 - m.size(), '0') + m;
    }
    return y + "-" + d + " " + h + ":" + m;
}

std::string NeoDateTimeLocal::ToShortDateString()
{
    auto y = std::to_string(Year);
    auto d = std::to_string(Day);
    if (d.size() < 3)
    {
        d = std::string(3 - d.size(), '0') + d;
    }
    return y + "-" + d;
}

std::string NeoDateTimeLocal::ToLongDayString()
{
    static std::vector<std::string> DayNames = { "zerodi", "unidi", "duodi", "tridi", "quartidi", "quintidi", "sextidi", "septidi", "octidi", "nonidi" };
    auto d = std::to_string(Day);
    if (d.size() < 3)
    {
        d = std::string(3 - d.size(), '0') + d;
    }
    return "decade " + std::to_string(Day / 10) + " " + DayNames[Day % 10] + ", 第" + std::to_string(Day / 10) + "旬第" + std::to_string(Day % 10) + "天";
}

std::string TimePointToLocalShortDateString(std::chrono::system_clock::time_point Time, double OffsetAtTime)
{
    auto t = (Time + std::chrono::seconds(static_cast<int>(std::floor(OffsetAtTime * 3600)))).time_since_epoch();
    auto DaysFromEpoch = std::chrono::duration_cast<days>(t);
    auto tu = civil_from_days<int>(DaysFromEpoch.count());
    auto Year = std::get<0>(tu);
    auto Month = std::get<1>(tu);
    auto Day = std::get<2>(tu);
    t -= DaysFromEpoch;

    auto y = std::to_string(Year);
    auto m = std::to_string(Month);
    if (m.size() < 2)
    {
        m = std::string(2 - m.size(), '0') + m;
    }
    auto d = std::to_string(Day);
    if (d.size() < 2)
    {
        d = std::string(2 - d.size(), '0') + d;
    }
    return y + "-" + m + "-" + d;
}
