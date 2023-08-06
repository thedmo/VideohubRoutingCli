#include <catch2/catch.hpp>
#include <ErrorTrace/ErrorTrace.hpp>


TEST_CASE("Test capabilities of errortracing") {
	int result = 0;
	
	result = ET::Collector::Add("Error 1");
	REQUIRE(result == 1);

	result = ET::Collector::Add("Error 2");
	REQUIRE(result == 1);

	result = ET::Collector::Add("Error 3");
	REQUIRE(result == 1);

	result = ET::Collector::Add("Error 4");
	REQUIRE(result == 1);

	result = ET::Logger::LogTraceToFile("ErrorFile1");
	REQUIRE(result == 0);

	result = ET::Printer::PrintTrace();
	REQUIRE(result == 0);
}
