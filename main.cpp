#include "JValue.h"
#include "minunit.h"
#include <limits>

static void TEST_STRING(std::string expect, std::string json)
{
    auto res = json::JValue::parse_json(json);
    mu_assert_string_eq(expect.c_str(), res->get_string().c_str());
};

static auto TEST_DOUBLE = [](double expect, std::string json) {
    double res = json::JValue::parse_json(json)->get_number();
    mu_assert_double_eq(expect, res);
};

static void TEST_NULL(json::lept_type expect, std::string json) {
    auto res = json::JValue::parse_json(json)->get_type();
    mu_assert_int_eq(json::LEPT_NULL, res);
}

static void TEST_BOOL(bool expect, std::string json) {
    auto res = json::JValue::parse_json(json)->get_bool();
    mu_check(expect == res);
}

static void TEST_STRINGLING(std::string json) {
    std::string s;
    json::JValue::parse_json(json)->stringify_value(s);
    mu_assert_string_eq(json.c_str(), s.c_str());
}

MU_TEST(test_base_null_object)
{
    json::JValue nd;
    mu_assert_int_eq(json::LEPT_NULL, nd.get_type());
    TEST_NULL(json::LEPT_NULL, "null");
}

MU_TEST(test_string_object)
{
    json::JString nd("Hello");
    std::string tmp;
    tmp = nd.get_string();

    mu_assert_string_eq(tmp.c_str(), "Hello");
    mu_check(nd.get_type() == json::LEPT_STRING);
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
        auto res = json::JValue::parse_json(ins);

        mu_check(res->get_array().at(0)->get_type() == json::LEPT_NULL);
        mu_check(true == res->get_array().at(2)->get_bool());
        mu_check(false == res->get_array().at(1)->get_bool());
        mu_check(123.0 == res->get_array().at(3)->get_number());
        mu_check("hello\nworld" == res->get_array().at(4)->get_string());
    }

    {
        std::string ins("[true,[true,false],true]");
        auto res = json::JValue::parse_json(ins);
        mu_check(true == res->get_array().at(0)->get_bool());
        mu_check(true == res->get_array().at(2)->get_bool());
        mu_check(true == res->get_array().at(1)->get_array().at(0)->get_bool());
        mu_check(false == res->get_array().at(1)->get_array().at(1)->get_bool());

        // [t,[t,t],t];
        res->get_array().at(1)->get_array().at(1)->get_bool() = true;
        std::cout << std::endl;
        res->write(std::cout, 0);
    }

    {
        std::string ins("[\"Hello\",\"Wo\\trld\" ,0.9,true]");
        auto res = json::JValue::parse_json(ins);
        std::cout << std::endl;
        res->write(std::cout, 0);
    }
}

MU_TEST(test_object_parse)
{
    {
        std::string ins("{\"a\":1.0}");
        auto res = json::JValue::parse_json(ins);
        mu_assert_double_eq(1.0, res->get_object().find("a")->second->get_number());
        std::cout << '\n';
        res->write(std::cout, 0);
    }
    {
        std::string ins("{"
                              "\"a\":1.2,"
                              "\"null\":null ,"
                              "\"array\":[true,false],"
                              "\"str\":\"string\"}");
        auto res = json::JValue::parse_json(ins);
        mu_assert_double_eq(1.2, res->get_object().find("a")->second->get_number());
        mu_assert_string_eq("array", res->get_object().find("array")->first.c_str());
        mu_check(res->get_object().find("array")->second->get_array().at(0)->get_bool());
        mu_check(!res->get_object().find("array")->second->get_array().at(1)->get_bool());
        std::cout << '\n';
        res->write(std::cout, 0);
    }
}

MU_TEST(test_new_json)
{
    json::JObject jobj;

    jobj["name"] = std::make_shared<json::JString>("bob");
    jobj["number"] = std::make_shared<json::JNumber>(1e+9);
    jobj["Is_Boy"] = std::make_shared<json::JBool>(true);
    jobj["property"] = std::make_shared<json::JArray>();

    auto &property_array = jobj["property"]->get_array();
    property_array.push_back(std::make_shared<json::JValue>() //null
    );

    auto another_obj = std::make_shared<json::JObject>();
    //another way to insert key-value
    another_obj->get_object()["nested"] = std::make_shared<json::JBool>(false);

    //third way to insert key-value
    another_obj->insert<json::JNumber>("weight", 170.6);

    property_array.push_back(another_obj);

    std::ofstream of;
    of.open("test.json", std::ios::app | std::ios::out);
    std::cout << std::endl;
    if (!of.is_open()) {
        jobj.write(std::cout, 0);
    } else {
        jobj.write(of, 0);
    }

    //another way to write-to-file
    jobj.write_to_file("test1.json");
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
    MU_RUN_TEST(test_new_json);
    MU_RUN_TEST(test_stringly);
}

int main() {
    MU_RUN_TEST(parser_suit);

	std::string s;
	auto json = "{\"id\":123}";
	auto value = json::JValue::parse_json(json);
	auto number = value->get_number();
	std::cout << "number=" << number << std::endl;

    MU_REPORT();
    return minunit_status;
}