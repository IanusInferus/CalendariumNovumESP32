#include "NeoDateTime.h"

#include <chrono>

int main()
{
	auto UtcNow = std::chrono::system_clock::now();
	auto LocalNowStr = TimePointToLocalShortDateString(UtcNow, 7);

	auto NeoUtcNow = NeoDateTimeUtc::FromTimePoint(UtcNow);
	auto NeoLocalNow = NeoUtcNow.ToLocal(7);
	auto NeoLocalNowShortStr = NeoLocalNow.ToShortString();
	auto NeoLocalNowLongDayStr = NeoLocalNow.ToLongDayString();

	return 0;
}
