#include "Weather.h"

#include <ArduinoJson.h> //https://github.com/bblanchon/ArduinoJson

#include <string>
#include <stdexcept>
#include <sstream>

std::string GetWeatherText(std::string Json, int CacheSize)
{
    DynamicJsonDocument j(CacheSize);

    DeserializationError error = deserializeJson(j, Json);
    if (error)
    {
        throw std::runtime_error("JSON deserialization failed.");
    }

    auto now = j["data"]["now"];
    auto firstDay = j["data"]["daily"][0];

    std::string location = static_cast<const char*>(j["data"]["location"]["name"]);
    double temperature = now["temperature"];
    double low = firstDay["low"];
    double high = firstDay["high"];
    std::string dayText = static_cast<const char*>(firstDay["dayText"]);
    std::string nightText = static_cast<const char*>(firstDay["nightText"]);

    auto description = dayText == nightText ? dayText : (dayText + "转" + nightText);

    std::stringstream ss;
    ss << location << " " << temperature << "℃(" << low << "℃~" << high << "℃) " << description;
    return ss.str();
}
