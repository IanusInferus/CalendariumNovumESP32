#include "Weather.h"

#include <ArduinoJson.h> //https://github.com/bblanchon/ArduinoJson

#include <string>
#include <stdexcept>
#include <sstream>
#include <regex>
#include <limits>

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

typedef std::chrono::duration<int, std::ratio<86400>> days;

WeatherInfo ParseWeatherText(std::string Json, int CacheSize)
{
    DynamicJsonDocument j(CacheSize);

    DeserializationError error = deserializeJson(j, Json);
    if (error)
    {
        throw std::runtime_error("JSON deserialization failed.");
    }

    auto now = j["data"]["now"];
    auto firstDay = j["data"]["daily"][0];

    WeatherInfo i;
    i.Location = static_cast<const char*>(j["data"]["location"]["name"]);
    i.Temperature = now["temperature"];
    i.Low = firstDay["low"];
    i.High = firstDay["high"];
    std::string dayText = static_cast<const char*>(firstDay["dayText"]);
    std::string nightText = static_cast<const char*>(firstDay["nightText"]);
    i.Description = dayText == nightText ? dayText : (dayText + "转" + nightText);

    std::regex rTime(R"(([0-9]{4,})/([0-9]{2})/([0-9]{2}) ([0-9]{2}):([0-9]{2}))");

    std::string text = static_cast<const char*>(j["data"]["lastUpdate"]);
    std::smatch match;
    if (std::regex_match(text, match, rTime))
    {
        auto year = std::stol(match[1].str());
        auto month = std::stol(match[2].str());
        auto day = std::stol(match[3].str());
        auto hour = std::stol(match[4].str());
        auto minute = std::stol(match[5].str());

        auto LastUpdate = static_cast<std::chrono::system_clock::time_point>(static_cast<days>(days_from_civil(year, month, day)));
        LastUpdate += static_cast<std::chrono::hours>(hour);
        LastUpdate += static_cast<std::chrono::minutes>(minute);
        LastUpdate += static_cast<std::chrono::hours>(-8);

        i.LastUpdate = LastUpdate;
    }
    else
    {
        throw std::runtime_error("JSON parsing failed.");
    }

    return i;
}
