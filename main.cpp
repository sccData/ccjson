#include "ccjson.h"
#include "minunit.h"
#include <limits>
using namespace json;

static void TEST_STRING(std::string expect, std::string json)
{
    auto res = Json::load(json);
    mu_assert_string_eq(expect.c_str(), res.get_string().c_str());
};

static auto TEST_DOUBLE = [](double expect, std::string json) {
    double res = Json::load(json).get_number();
    mu_assert_double_eq(expect, res);
};

static void TEST_NULL(Json::Jtype expect, std::string json) {
    auto res = Json::load(json).get_type();
    mu_assert_int_eq(Json::JNULL, res);
}

static void TEST_BOOL(bool expect, std::string json) {
    auto res = Json::load(json).get_bool();
    mu_check(expect == res);
}

static void TEST_STRINGLING(std::string json) {
    std::string s;
    s = Json::load(json).dump();
    mu_assert_string_eq(json.c_str(), s.c_str());
}

MU_TEST(test_base_null_object)
{
    Json nd;
    mu_assert_int_eq(Json::JNULL, nd.get_type());
    TEST_NULL(Json::JNULL, "null");
}

MU_TEST(test_string_object)
{
    Json nd("Hello");
    std::string tmp;
    tmp = nd.get_string();

    mu_assert_string_eq(tmp.c_str(), "Hello");
    mu_check(nd.get_type() == Json::JSTRING);
}

MU_TEST(test_string_parse)
{
    TEST_STRING("", "\"\"");
    TEST_STRING("Hello", "\"Hello\"");

    TEST_STRING("Hello\nWorld", "\"Hello\\nWorld\"");
    TEST_STRING("\" \\ / \b \f \n \r \t", "\"\\\" \\\\ \\/ \\b \\f \\n \\r \\t\"");

    TEST_STRING("Hello\0World", "\"Hello\\u0000World\"");
    TEST_STRING("\x24", "\"\\u0024\"");         /* Dollar sign U+0024 */
    TEST_STRING("\xC2\xA2", "\"\\u00A2\"");     /* Cents sign U+00A2 */
    TEST_STRING("\xE2\x82\xAC", "\"\\u20AC\""); /* Euro sign U+20AC */
    TEST_STRING("\xF0\x9D\x84\x9E", "\"\\uD834\\uDD1E\"");  /* G clef sign U+1D11E */
    TEST_STRING("\xF0\x9D\x84\x9E", "\"\\ud834\\udd1e\"");  /* G clef sign U+1D11E */
}

MU_TEST(test_double_parse)
{
    TEST_DOUBLE(0.0, "0.0");
    TEST_DOUBLE(0.0, "0");
    TEST_DOUBLE(0.0, "-0");
    TEST_DOUBLE(0.0, "-0.0");
    TEST_DOUBLE(1.0, "1");
    TEST_DOUBLE(-1.0, "-1");
    TEST_DOUBLE(1.5, "1.5");
    TEST_DOUBLE(-1.5, "-1.5");
    TEST_DOUBLE(3.1416, "3.1416");
    TEST_DOUBLE(1E10, "1E10");
    TEST_DOUBLE(1e10, "1e10");
    TEST_DOUBLE(1E+10, "1E+10");
    TEST_DOUBLE(1E-10, "1E-10");
    TEST_DOUBLE(-1E10, "-1E10");
    TEST_DOUBLE(-1e10, "-1e10");
    TEST_DOUBLE(-1E+10, "-1E+10");
    TEST_DOUBLE(-1E-10, "-1E-10");
    TEST_DOUBLE(1.234E+10, "1.234E+10");
    TEST_DOUBLE(1.234E-10, "1.234E-10");
    TEST_DOUBLE(0.0, "1e-10000"); /* must underflow */
    TEST_DOUBLE((std::numeric_limits<double>::max)(),
                std::to_string((std::numeric_limits<double>::max)()));
    TEST_DOUBLE((std::numeric_limits<double>::min)(),
                std::to_string((std::numeric_limits<double>::min)()));
}

MU_TEST(test_bool_parse) {
    TEST_BOOL(true, "true");
    TEST_BOOL(false, "false");
}

MU_TEST(test_array_parse)
{
    {
        std::string ins("[ null , false ,true, 123.0, \"hello\\nworld\"]");
        auto res = Json::load(ins);

        mu_check(res[0].get_type() == Json::JNULL);
        mu_check(true == res[2].get_bool());
        mu_check(false == res[1].get_bool());
        mu_check(123.0 == res[3].get_number());
        mu_check("hello\nworld" == res[4].get_string());
    }

    {
        std::string ins("[true,[true,false],true]");
        auto res = Json::load(ins);
        mu_check(true == res[0].get_bool());
        mu_check(true == res[2].get_bool());
        mu_check(true == res[1][0].get_bool());
        mu_check(false == res[1][1].get_bool());

        // [t,[t,t],t];
        res[1][1].set_value(true);
        mu_check(true == res[1][1].get_bool());
    }

    {
    
        std::string ins("[\"Hello\",\"Wo\\trld\" ,0.9,true]");
        auto res = Json::load(ins);
        res.set_value(3.0);
        mu_check(3.0 == res.get_number());
    }
}
#include <chrono>
#include <iostream>
MU_TEST(test_object_parse2_long_string)
{
	{
		using namespace std::chrono;
		system_clock::time_point today = system_clock::now();

		int length = 10000*10000;
		std::string jsonStr(length, '0');
		auto jsonStr1 = ("\"") + jsonStr + "\"}";
		std::string ins("{\"a\":1.0, \"b\":");
		ins.append(jsonStr1);
		auto res = Json::load(ins);
		auto b1 = res["b"].get_string();
		if (b1[0] == '0')
		{
			//ok
		}
		else
		{
			std::cout << "error" << std::endl;
		}
		system_clock::time_point today2 = system_clock::now();
		auto dur = today2 - today;
		typedef std::chrono::duration<int> seconds_type;
		typedef std::chrono::duration<int, std::micro> microseconds_type;
		auto durMicr = std::chrono::duration_cast<microseconds_type>(dur);
		auto durSecond = std::chrono::duration_cast<seconds_type>(dur);
		std::cout << "test_object_parse2_long_string eslaped(second) :" << durSecond.count() << "\n";
		std::cout << "test_object_parse2_long_string eslaped(microsecond) :" << durMicr.count() << "\n";
	}
}
MU_TEST(test_object_parse1_member)
{
	{
		std::string ins("{\"a\":1.0, \"b\":1.1}");
		auto res = Json::load(ins);
		auto b1 = res["a"].get_number();
		auto b2 = res["b"].get_number();
		mu_assert_double_eq(1.0, b1);
		mu_assert_double_eq(1.1, b2);
	}
}
MU_TEST(test_object_parse)
{
    {
        std::string ins("{\"a\":1.0, \"b\":1.0}");
        auto res = Json::load(ins);
        mu_assert_double_eq(1.0, res["b"].get_number());
    }
    {
        std::string ins("{"
                              "\"a\":1.2,"
                              "\"null\":null ,"
                              "\"array\":[true,false],"
                              "\"str\":\"string\"}");
        auto res = Json::load(ins);
        mu_assert_double_eq(1.2, res["a"].get_number());
        mu_check(res["array"][0].get_bool());
        mu_check(!res["array"][1].get_bool());
    }
}


MU_TEST(test_stringly) {
    TEST_STRINGLING("0");
    TEST_STRINGLING("-0");
    TEST_STRINGLING("1");
    TEST_STRINGLING("-1");
    TEST_STRINGLING("1.5");
    TEST_STRINGLING("-1.5");
    TEST_STRINGLING("3.25");
    TEST_STRINGLING("1e+20");
    TEST_STRINGLING("1.234e+20");
    TEST_STRINGLING("1.234e-20");

    TEST_STRINGLING("1.0000000000000002"); /* the smallest number > 1 */
    TEST_STRINGLING("4.9406564584124654e-324"); /* minimum denormal */
    TEST_STRINGLING("-4.9406564584124654e-324");
    TEST_STRINGLING("2.2250738585072009e-308");  /* Max subnormal double */
    TEST_STRINGLING("-2.2250738585072009e-308");
    TEST_STRINGLING("2.2250738585072014e-308");  /* Min normal positive double */
    TEST_STRINGLING("-2.2250738585072014e-308");
    TEST_STRINGLING("1.7976931348623157e+308");  /* Max double */
    TEST_STRINGLING("-1.7976931348623157e+308");

    TEST_STRINGLING("\"\"");
    TEST_STRINGLING("\"Hello\"");
    TEST_STRINGLING("\"Hello\\nWorld\"");
    TEST_STRINGLING("\"\\\" \\\\ / \\b \\f \\n \\r \\t\"");
    TEST_STRINGLING("\"Hello\\u0000World\"");

    TEST_STRINGLING("[]");
    TEST_STRINGLING("[null,false,true,123,\"abc\",[1,2,3]]");

    TEST_STRINGLING("{}");

    TEST_STRINGLING("null");
    TEST_STRINGLING("false");
    TEST_STRINGLING("true");
}

MU_TEST_SUITE(parser_suit) {
    MU_RUN_TEST(test_double_parse);
    MU_RUN_TEST(test_string_parse);
    MU_RUN_TEST(test_string_object);
    MU_RUN_TEST(test_base_null_object);
    MU_RUN_TEST(test_bool_parse);
    MU_RUN_TEST(test_array_parse);
	MU_RUN_TEST(test_object_parse); 
	MU_RUN_TEST(test_object_parse1_member);
	MU_RUN_TEST(test_object_parse2_long_string);
	
    MU_RUN_TEST(test_stringly);
}

int main() {
    MU_RUN_TEST(parser_suit);
    MU_REPORT();
    return minunit_status;
}