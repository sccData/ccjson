#ifndef JSON_H
#define JSON_H
#include <memory>
#include <string>

namespace json {
	enum lept_type:int {LEPT_NULL, LEPT_BOOL, LEPT_NUMBER, LEPT_STRING, LEPT_ARRAY, LEPT_OBJECT};
	class JValue;
}
namespace pain {
	class Json {
	public:
		void parse(const std::string& content, std::string& status);
		void parse(const std::string& content);

		int get_type() const;
		std::shared_ptr<json::JValue> get_value() { return v; }

		Json();
		~Json();
	// Json(const Json& rhs);
	// Json& operator=(const Json& rhs);
	// Json(Json &&rhs);
	// Json& operator=(Json&& rhs);
	// void swap(Json& rhs);

	// lept_type get_type() const;

	// void set_null();
	// void set_boolean(bool b);
	// void set_number(double d);
	// void set_string(const std::string& str);

	// double get_number() const;
	// const std::string get_string() const;

private:
	std::shared_ptr<JValue> v;
	// friend bool operator==(const Json& lhs, const Json& rhs);
	// friend bool operator!=(const Json& lhs, const Json& rhs);
};

// bool operator==(const Json& lhs, const Json& rhs);
// bool operator!=(const Json& lhs, const Json& rhs);
// void swap(Json& lhs, Json& rhs);
}
#endif