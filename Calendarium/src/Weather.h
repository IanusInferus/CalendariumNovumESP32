#include <string>
#include <chrono>

class WeatherInfo
{
public:
    std::string Location;
    double Temperature;
    double Low;
    double High;
    std::string Description;
    std::chrono::system_clock::time_point LastUpdate;
};

WeatherInfo ParseWeatherText(std::string Json, int CacheSize);
