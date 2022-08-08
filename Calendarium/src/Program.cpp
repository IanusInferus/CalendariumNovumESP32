#include "Weather.h"
#include "Graphics.h"
#include "NeoDateTime.h"

#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <HTTPClient.h>

#include <chrono>
#include <sstream>
#include <iomanip>

RTC_DATA_ATTR bool s_WeatherInfoInitialized = false;
RTC_DATA_ATTR WeatherInfo s_WeatherInfo = {};

void printLocalTime()
{
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo, 10000))
    {
        Serial.println("Failed to obtain time");
        return;
    }
    Serial.println(&timeinfo, "%F %T %A"); // 格式化输出
}

void update()
{
    Graphics::Init();

    WiFi.mode(WIFI_STA);

    WiFiManager wm;

    // reset settings - wipe stored credentials for testing
    // these are stored by the esp library
    //wm.resetSettings();

    const char* APName = "CalendariumNovum";
    wm.setAPCallback([=](WiFiManager* pwm)
    {
        Graphics::Clear();
        Graphics::DrawText(10, 10, "Calendarium Novum");
        Graphics::DrawText(10, 30, "WiFi无法连接");
        Graphics::DrawText(10, 50, "请连接WiFi CalendariumNovum进行配置");
        Graphics::DrawText(10, 70, "配置网页http://192.168.4.1");
        Graphics::Flush();
    });

    if (!wm.autoConnect(APName))
    {
		Serial.println("WiFi connection failed.");
        return;
    }

    // Connection is complete
    Serial.println("IP: " + WiFi.localIP().toString());
    Serial.println("Gateway: " + WiFi.gatewayIP().toString());
    Serial.println("Subnet: " + WiFi.subnetMask().toString());
    Serial.println("DNS: " + WiFi.dnsIP().toString());

    configTime(8 * 3600, 0, "ntp.tencent.com"); //pool.ntp.org
    printLocalTime();

    Serial.println();
    Serial.println("Calendarium Novum");
    Graphics::Clear();

	auto UtcNow = std::chrono::system_clock::now();
	auto LocalNowStr = TimePointToLocalShortDateString(UtcNow, 7);
	auto NeoUtcNow = NeoDateTimeUtc::FromTimePoint(UtcNow);
	auto NeoLocalNow = NeoUtcNow.ToLocal(7);
    auto NeoLocalNowShortDateStr = NeoLocalNow.ToShortDateString();
	auto NeoLocalNowLongDayStr = NeoLocalNow.ToLongDayString();
    Serial.println(NeoLocalNowShortDateStr.c_str());
    Serial.println(("(Calendarium Gregorianum " + LocalNowStr + ")").c_str());
    Graphics::DrawText(180, 10, "Calendarium Novum");
    Graphics::DrawText(120, 26, NeoLocalNowShortDateStr.c_str(), 4);
    Graphics::DrawText(128, 90, NeoLocalNowLongDayStr.c_str());
    Graphics::DrawText(128, 110, ("(Cal. Gregorianum " + LocalNowStr + ")").c_str());

    HTTPClient http;
    http.begin("http://weather.cma.cn/api/weather/view");
    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK)
    {
        String payload = http.getString();
        std::string WeatherText;
        std::string Temperature;

        s_WeatherInfo = ParseWeatherText(payload.c_str(), 4096);
        s_WeatherInfoInitialized = true;
    }
    else
    {
        Serial.println("Weather fetch failed");
    }
    http.end();

    if (s_WeatherInfoInitialized)
    {
        auto i = &s_WeatherInfo;
        auto LastUpdateText = NeoDateTimeUtc::FromTimePoint(i->LastUpdate).ToLocal(7).ToShortString();
        std::stringstream sTemperature;
        sTemperature << std::fixed << std::showpoint << std::setprecision(1) << i->Temperature;
        auto Temperature = sTemperature.str();
        std::stringstream sWeatherText1;
        sWeatherText1 << std::fixed << std::showpoint << std::setprecision(1) << "" << i->Low << "℃～" << i->High << "℃ " << i->Description;
        auto WeatherText1 = sWeatherText1.str();
        std::stringstream sWeatherText2;
        sWeatherText2 << LastUpdateText << " " << i->Location;
        auto WeatherText2 = sWeatherText2.str();
        Graphics::DrawText(120, 130, Temperature.c_str(), 8);
        Graphics::DrawText(160, 250, WeatherText1.c_str());
        Graphics::DrawText(160, 270, WeatherText2.c_str());
    }

    static std::vector<std::string> DayNames = { "zerodi", "unidi", "duodi", "tridi", "quartidi", "quintidi", "sextidi", "septidi", "octidi", "nonidi" };
    for (int k = 0; k < 10; k += 1)
    {
        if (k == NeoLocalNow.Day % 10)
        {
            Graphics::DrawText(10, 16 + k * 28, (DayNames[k] + " ←").c_str());
        }
        else
        {
            Graphics::DrawText(10, 16 + k * 28, DayNames[k].c_str());
        }
    }

    Serial.println();
    Serial.println();

    Graphics::Flush();

    Serial.println("Ok!");
}

void setup() 
{
    Serial.begin(115200);
    delay(10);

    update();

    esp_sleep_enable_timer_wakeup(3600UL * 1E+6); //每小时刷新一次
    esp_deep_sleep_start();
}

void loop() 
{
}
