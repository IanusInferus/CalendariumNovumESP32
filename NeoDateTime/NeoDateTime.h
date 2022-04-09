#pragma once

#include <cstdint>
#include <chrono>
#include <string>

class NeoDateTimeLocal;

/// <summary>
/// DateTime in Neo Calendar
/// 
/// Neo Calendar:
/// Second and day are the same as the ordinary definition. Leap seconds are possible and shall happen at the end of a year.
/// A year start from the UTC day containing the winter solstice. A winter solstice can offset one day in a local time zone. For reference, the start day of 2023 is at Gregorian 2022-12-21T21:48:01Z.
/// A year has 365 or 366 days. Whether a year is a leap year or not is purely determined by the time of winter solstice, that is, a leap year per 4 or 5 years.
/// Day start from 0. A year has day 0 to day 354 or day 355.
/// A decade(旬) consists of 10 days. Each day in a decade is called zerodi(旬〇), unidi(旬一), duodi(旬二), tridi(旬三), quartidi(旬四), quintidi(旬五), sextidi(旬六), septidi(旬七), octidi(旬八), nonidi(旬九).
/// A year consists of 37 decades, decade 0 to decade 36. Decade 36 is incomplete, only has 5 or 6 days.
/// </summary>
class NeoDateTimeUtc
{
public:
	const int Year;
	const int Day;
	const int Hour;
	const int Minute;
	const int Second;
	const std::int64_t Nanosecond;

	NeoDateTimeUtc(int Year, int Day, int Hour, int Minute, int Second, std::int64_t Nanosecond);
	static NeoDateTimeUtc FromTimePoint(std::chrono::system_clock::time_point Time);
	std::chrono::system_clock::time_point ToTimePoint();
	NeoDateTimeLocal ToLocal(double OffsetAtTime);
};

class NeoDateTimeLocal
{
public:
	const int Year;
	const int Day;
	const int Hour;
	const int Minute;
	const int Second;
	const std::int64_t Nanosecond;

	NeoDateTimeLocal(int Year, int Day, int Hour, int Minute, int Second, std::int64_t Nanosecond);
	std::string ToLongString();
};

std::string TimePointToLocalShortDateString(std::chrono::system_clock::time_point Time, double OffsetAtTime);
