#include "ccjson.h"
#include <stdexcept>
#include <cstring>
#include <stack>
#include <cassert>
#include <cmath>
#include "JException.h"
namespace json {

	const int max_depth = 200;

	using std::string;
	using std::move;

	struct Null {
		bool operator==(Null) const { return true; }
		bool operator<(Null) const { return false; }
	};

	// wrapper class
	template <Json::Jtype tag, typename T>
	class Value: public JValue {
	protected:
		explicit Value(const T& value) : m_value(value) {}
		explicit Value(T&& value) : m_value(move(value)) {}

		Json::Jtype get_type() const override {
			return tag;
		}

		bool equals(const JValue* rhs) const override {
			return m_value == static_cast<const Value<tag, T> *>(rhs)->m_value;
		}
		bool less(const JValue* rhs) const override {
			return m_value < static_cast<const Value<tag, T> *>(rhs)->m_value;
		}

		static void dump(const string& v, string& out) {
        	out += '\"';
        	for(size_t i = 0; i < v.length(); ++i) {
            	unsigned char ch = v[i];
            	switch (ch) {
                	case '\"': out += "\\\""; break;
                	case '\\': out += "\\\\"; break;
                	case '\b': out += "\\b";  break;
                	case '\f': out += "\\f";  break;
                	case '\n': out += "\\n";  break;
                	case '\r': out += "\\r";  break;
                	case '\t': out += "\\t";  break;
                	default:
                		if(static_cast<uint8_t>(ch) <= 0x1f) {
                			char buf[8];
                			snprintf(buf, sizeof(buf), "\\u%04x", ch);
                			out += buf;
                		} else if(static_cast<uint8_t>(ch) == 0xe2 && static_cast<uint8_t>(v[i+1]) == 0x80
                				  && static_cast<uint8_t>(v[i+2]) == 0xa8) {
                			out += "\\u2028";
                			i += 2;
                		} else if(static_cast<uint8_t>(ch) == 0xe2 && static_cast<uint8_t>(v[i+1]) == 0x80
                				  && static_cast<uint8_t>(v[i+2]) == 0xa9) {
                			out += "\\u2029";
                			i += 2;
                		} else
                			out += ch;
            	}
        	}
        	out += '\"';
		}

		T m_value;
	};

	class JDouble final: public Value<Json::JNUMBER, double> {
	public:
		explicit JDouble(double value): Value(value) {}
	private:
		double get_number() const override { return m_value; }
		// bool equals(const JValue* rhs) const override {
		// 	return m_value == rhs->get_number();
		// }
		// bool less(const JValue* rhs) const override {
		// 	return m_value < rhs->get_number();
		// }
		static void dump(double value, string& out) {
			if(std::isfinite(value)) {
				char buf[32];
				snprintf(buf, sizeof(buf), "%.17g", value);
				out += buf;
			} else {
				out += "null";
			}
		}

		void dump(std::string& out) const {
			dump(m_value, out);
		}

		void set_value(double v) {
			m_value = v;
		}
	};

	class JBool final: public Value<Json::JBOOL, bool> {
	public:
		explicit JBool(bool value): Value(value) {}
	private:
		bool get_bool() const override { return m_value; }
		static void dump(bool value, string& out) {
			out += value ? "true" : "false";
		}

		void dump(string& out) const {
			dump(m_value, out);
		}

		void set_value(bool v) {
			m_value = v;
		} 
	};

	class JString final: public Value<Json::JSTRING, string> {
	public:
		explicit JString(const string& value): Value(value) {}
		explicit JString(string&& value): Value(move(value)) {}
	private:
		const string& get_string() const override { return m_value; }

		void dump(string& out) const {
			Value::dump(m_value, out); 
		}

		void set_value(const string& v) {
			m_value = v;
		}
		void set_value(string&& v) {
			m_value = move(v);
		}
	};

	class JArray final: public Value<Json::JARRAY, Json::Jarray> {
	public:
		explicit JArray(const Json::Jarray& value): Value(value) {}
		explicit JArray(Json::Jarray&& value): Value(move(value)) {}
	private:
		const Json::Jarray& get_array() const override { return m_value; }
		const Json& operator[](size_t i) const override;
		Json& operator[](size_t i) override;
		static void dump(const Json::Jarray& values, string& out) {
			out += "[";
			for(size_t i = 0; i < values.size(); ++i) {
				if(i > 0)
					out += ",";
				values[i].dump(out);
			}

			out += "]";
		}

		void dump(string& out) const {
			dump(m_value, out);
		}

		void set_value(const Json::Jarray& v) {
			m_value = v;
		}
		void set_value(Json::Jarray&& v) {
			m_value = move(v);
		}
	};

	class JObject final: public Value<Json::JOBJECT, Json::Jobject> {
	public:
		explicit JObject(const Json::Jobject& value): Value(value) {}
		explicit JObject(Json::Jobject&& value): Value(move(value)) {}
	private:
		const Json::Jobject& get_object() const override { return m_value; }
		const Json& operator[](const string& key) const override;
		Json& operator[](const string& key) override;
		static void dump(const Json::Jobject& values, string& out) {
			out += "{";
			// for(size_t i = 0; i < values.size(); ++i) {
			// 	if(i > 0)
			// 		out += ", ";
			// 	dump(values[i].first, out);
			// 	out += ": ";
			// 	values[i].second.dump(out);
			// }
			for(auto iter = values.cbegin(); iter != values.cend(); ++iter) {
				if(iter != values.cbegin())
					out += ",";
				Value<Json::JOBJECT, Json::Jobject>::dump(iter->first, out);
				out += ":";
				iter->second.dump(out);
			}

			out += "}";
		}

		void dump(std::string& out) const {
			dump(m_value, out);
		}

		void set_value(const Json::Jobject& v) {
			m_value = v;
		}
		void set_value(Json::Jobject&& v) {
			m_value = move(v);
		}
	};

	class JNull final: public Value<Json::JNULL, Null> {
	public:
		JNull(): Value(Null()) {}
	private:
		static void dump(Null, string& out) {
			out += "null";
		}

		void dump(std::string& out) const {
			dump(Null(), out);
		}
	};

	// Json
	Json::Json() noexcept                  : m_ptr(std::make_shared<JNull>()) {}
	Json::Json(double value)               : m_ptr(std::make_shared<JDouble>(value)) {}
	Json::Json(bool value)                 : m_ptr(std::make_shared<JBool>(value)) {}
	Json::Json(const string& value)        : m_ptr(std::make_shared<JString>(value)) {}
	Json::Json(string&& value)             : m_ptr(std::make_shared<JString>(move(value))) {}
	Json::Json(const char* value)          : m_ptr(std::make_shared<JString>(string(value))) {}
	Json::Json(const Json::Jarray& value)  : m_ptr(std::make_shared<JArray>(value)) {}
	Json::Json(Json::Jarray&& value)       : m_ptr(std::make_shared<JArray>(move(value))) {}
	Json::Json(const Json::Jobject& value) : m_ptr(std::make_shared<JObject>(value)) {}
	Json::Json(Json::Jobject&& value)      : m_ptr(std::make_shared<JObject>(move(value))) {}

	Json::Jtype Json::get_type() const { return m_ptr->get_type(); }
	double Json::get_number() const { return m_ptr->get_number(); }
	bool Json::get_bool() const { return m_ptr->get_bool(); }
	const string& Json::get_string() const { return m_ptr->get_string(); }
	const Json::Jarray& Json::get_array() const { return m_ptr->get_array(); }
	const Json::Jobject& Json::get_object() const { return m_ptr->get_object(); }

	void Json::set_value() {
		if(!is_null()) {
			m_ptr = std::make_shared<JNull>();
		}
	}
	void Json::set_value(double value) {
		if(is_number()) {
			m_ptr->set_value(value);
		} else {
			m_ptr = std::make_shared<JDouble>(value);
		}
	}
	void Json::set_value(bool value) {
		if(is_bool()) {
			m_ptr->set_value(value);
		} else {
			m_ptr = std::make_shared<JBool>(value);
		}
	}
	void Json::set_value(const string& value) {
		if(is_string()) {
			m_ptr->set_value(value);
		} else {
			m_ptr = std::make_shared<JString>(value);
		}
	}
	void Json::set_value(string&& value) {
		if(is_string()) {
			m_ptr->set_value(value);
		} else {
			m_ptr = std::make_shared<JString>(value);
		}
	}
	void Json::set_value(const char* value) {
		if(is_string()) {
			m_ptr->set_value(string(value));
		} else {
			m_ptr = std::make_shared<JString>(string(value));
		}
	}
	void Json::set_value(const Jarray& value) {
		if(is_array()) {
			m_ptr->set_value(value);
		} else {
			m_ptr = std::make_shared<JArray>(value);
		}
 	}
 	void Json::set_value(Jarray&& value) {
 		if(is_array()) {
 			m_ptr->set_value(value);
 		} else {
 			m_ptr = std::make_shared<JArray>(value);
 		}
 	}
 	void Json::set_value(const Jobject& value) {
 		if(is_object()) {
 			m_ptr->set_value(value);
 		} else {
 			m_ptr = std::make_shared<JObject>(value);
 		}
 	}
 	void Json::set_value(Jobject&& value) {
 		if(is_object()) {
 			m_ptr->set_value(value);
 		} else {
 			m_ptr = std::make_shared<JObject>(value);
 		}
 	}

 	const Json& Json::operator[](size_t i) const {
 		return (*m_ptr)[i];
 	}
 	const Json& Json::operator[](const string& key) const {
 		return (*m_ptr)[key];
 	}
 	Json& Json::operator[](size_t i) {
 		return (*m_ptr)[i];
 	}
 	Json& Json::operator[](const string& key) {
 		return (*m_ptr)[key];
 	}

 	// JValue
 	double JValue::get_number()                       const { throw std::runtime_error("NOT_NUMBER"); }
 	bool JValue::get_bool()                           const { throw std::runtime_error("NOT_BOOL"); }
 	const string& JValue::get_string()                const { throw std::runtime_error("NOT_STRING"); }
 	const Json::Jarray& JValue::get_array()           const { throw std::runtime_error("NOT_ARRAY"); }
 	const Json& JValue::operator[](size_t i)          const { throw std::runtime_error("NOT_ARRAY"); }
 	Json& JValue::operator[](size_t i)                      { throw std::runtime_error("NOT_ARRAY"); }
 	const Json::Jobject& JValue::get_object()         const { throw std::runtime_error("NOT_OBJECT"); }
 	const Json& JValue::operator[](const string& key) const { throw std::runtime_error("NOT_OBJECT"); }
 	Json& JValue::operator[](const string& key)             { throw std::runtime_error("NOT_OBJECT"); }
 	void JValue::set_value(double value)                    { throw std::runtime_error("NOT_NUMBER"); }
	void JValue::set_value(bool value)                      { throw std::runtime_error("NOT_BOOL"); }
	void JValue::set_value(const std::string& value)        { throw std::runtime_error("NOT_STRING"); }
	void JValue::set_value(std::string&& value)             { throw std::runtime_error("NOT_STRING"); }
	void JValue::set_value(const Json::Jarray& value)       { throw std::runtime_error("NOT_ARRAY"); }
	void JValue::set_value(Json::Jarray&& value)            { throw std::runtime_error("NOT_ARRAY"); }
	void JValue::set_value(const Json::Jobject& value)      { throw std::runtime_error("NOT_OBJECT"); }
    void JValue::set_value(Json::Jobject&& value)           { throw std::runtime_error("NOT_OBJECT"); }

    // Json
 	Json& Json::operator=(const Json& rhs) {
 		std::shared_ptr<JValue> tmp(rhs.m_ptr);
 		m_ptr = tmp;
 		return *this;
 	}
 	Json& Json::operator=(Json&& rhs) {
 		std::shared_ptr<JValue> tmp(rhs.m_ptr);
 		m_ptr = move(tmp);
 		return *this;
 	}

 	bool Json::operator==(const Json& rhs) const {
 		if(m_ptr == rhs.m_ptr)
 			return true;
 		if(m_ptr->get_type() != rhs.m_ptr->get_type())
 			return false;

 		return m_ptr->equals(rhs.m_ptr.get());
 	}
 	bool Json::operator< (const Json& rhs) const {
 		if(m_ptr == rhs.m_ptr)
 			return false;
 		if(m_ptr->get_type() != rhs.m_ptr->get_type())
 			return false;
 		return m_ptr->less(rhs.m_ptr.get());
 	}

 	const Json& JArray::operator[](size_t i) const {
 		return m_value[i];
 	}
 	Json& JArray::operator[](size_t i) {
 		return m_value[i];
 	}

 	const Json& JObject::operator[](const string& key) const {
 		return m_value.at(key);
 	}
 	Json& JObject::operator[](const string& key) {
 		return m_value[key];
 	}

 	namespace {
 		struct JParser final {
 			const char* cur;

 			void expect(const char* &c, char ch) {
        		assert(*c == ch);
        		++c;
    		}
    		bool isdigit09(char ch) {
        		return (ch >= '1' && ch <= '9');
    		}
 			void parse_whitespace() {
        		while(*cur == ' ' || *cur == '\t' || *cur == '\n' || *cur == '\r')
            		++cur;
    		}
    		Json parse_json() {
    			parse_whitespace();
    			Json result = parse_value(0);
    			parse_whitespace();
    			if(*cur != '\0') {
    				throw(Exception("PARSE_ROOT_NOT_SINGULAR"));
    			}
    			return result;
    		}
    		Json parse_value(int depth) {
    			if(depth > max_depth)
    				throw(Exception("EXCEEDED_MAXIMUM_NESTING_DEPTH"));
        		switch(*cur) {
            		case 'n': return parse_literal("null", Json::JNULL);
            		case 't': return parse_literal("true", Json::JBOOL);
            		case 'f': return parse_literal("false", Json::JBOOL);
            		case '\"': return parse_string();
            		case '[': return parse_array(depth);
            		case '{': return parse_object(depth);
            		default:  return parse_number();
            		case '\0': throw(Exception("PARSE_EXPECT_VALUE"));
        		}
    		}
    		Json parse_literal(const char* literal, Json::Jtype t) {
    			expect(cur, literal[0]);
        		size_t i;
        		for(i=0; literal[i+1]; ++i) {
            		if(cur[i] != literal[i+1])
                		throw(Exception("PARSE_INVALID_VALUE"));
        		}
        		cur += i;
        		if(strcmp(literal, "true") == 0)
            		return true;
        		else if(strcmp(literal, "false") == 0)
            		return false;
            	else if(strcmp(literal, "null") == 0)
            		return Json();
    		}
    		Json parse_number() {
        		const char* p = cur;
        		if(*p == '-') ++p;
        		if(*p == '0') {
        			++p;
        			if(isdigit(*p))
        				throw (Exception("PARSE_INVALID_VALUE"));
        		} else {
            		if(!isdigit09(*p))
                		throw (Exception("PARSE_INVALID_VALUE"));
            		for(++p; isdigit(*p); ++p)
                		;
        		}

        		if(*p == '.') {
            		if(!isdigit(*++p))
                		throw (Exception("PARSE_INVALID_VALUE"));
            		for(++p; isdigit(*p); ++p)
                		;
        		}

        		if(*p == 'e' || *p == 'E') {
            		++p;
            		if(*p == '+' || *p == '-')
                		++p;
            		if(!isdigit(*p))
                		throw (Exception("PARSE_INVALID_VALUE"));
            		for(++p; isdigit(*p); ++p)
                		;
        		}
        		errno = 0;
        		double v = strtod(cur, nullptr);
        		if(errno == ERANGE && (v == HUGE_VAL || v == -HUGE_VAL))
            		throw (Exception("PARSE_NUMBER_TOO_BIG"));
        		cur = p;
        		return v;
    		}

    		string parse_string() {
    			expect(cur, '\"');
        		const char* p = cur;
        		string tmp;
        		unsigned u = 0, u2 = 0;
        		for(;;) {
            		char ch = *p++;
		            if(ch == '\"')
		                break;
		            else if(ch == '\0')
		                throw(Exception("PARSE_MISS_QUOTATION_MARK"));
		            else if(ch == '\\' && p) {
		                switch(*p++) {
		                    case '\"': tmp += '\"'; break;
		                    case '\\': tmp += '\\'; break;
		                    case '/':  tmp += '/';  break;
		                    case 'b':  tmp += '\b'; break;
		                    case 'f':  tmp += '\f'; break;
		                    case 'n':  tmp += '\n'; break;
		                    case 'r':  tmp += '\r'; break;
		                    case 't':  tmp += '\t'; break;
		                    case 'u':
		                        parse_hex4(p, u);
		                        if(u >= 0xD800 && u <= 0xDBFF) {
		                            if(*p++ != '\\')
		                                throw(Exception("PARSE_INVALID_UNICODE_SURROGATE"));
		                            if(*p++ != 'u')
		                                throw(Exception("PARSE_INVALID_UNICODE_SURROGATE"));
		                            parse_hex4(p, u2);
		                            if(u2 < 0xDC00 || u2 > 0xDFFF)
		                                throw(Exception("PARSE_INVALID_UNICODE_SURROGATE"));
		                            u = (((u - 0xD800) << 10) | (u2 - 0xDC00)) + 0x10000;
		                        }
		                        parse_encode_utf8(tmp, u);
		                        break;
		                    default:
		                        throw (Exception("PARSE_INVALID_STRING_ESCAPE"));
		                }
		            } else if((unsigned char)ch < 0x20) {
		                throw (Exception("PARSE_INVALID_STRING_CHAR"));
		            } else
		                tmp += ch;
		        }
		        cur = p;
		        return tmp;
    		}

    		void parse_hex4(const char* &p, unsigned &u) {
		        u = 0;
		        for(int i=0; i<4; ++i) {
		            char ch = *p++;
		            u <<= 4;
		            if(isdigit(ch))
		                u |= ch - '0';
		            else if(ch >= 'A' && ch <= 'F')
		                u |= ch - ('A' - 10);
		            else if(ch >= 'a' && ch <= 'f')
		                u |= ch - ('a' - 10);
		            else
		                throw(Exception("PARSE_INVALID_UNICODE_HEX"));
		        }
    		}

    		void parse_encode_utf8(std::string& str, unsigned u) const {
        		if(u <= 0x7F)
            		str += static_cast<char>(u & 0xFF);
        		else if(u <= 0x7FF) {
            		str += static_cast<char>(0xC0 | ((u >> 6) & 0xFF));
            		str += static_cast<char>(0x80 | ( u       & 0x3F));
        		} else if(u <= 0xFFFF) {
            		str += static_cast<char>(0xE0 | ((u >> 12) & 0xFF));
            		str += static_cast<char>(0x80 | ((u >>  6) & 0x3F));
            		str += static_cast<char>(0x80 | ( u        & 0x3F));
        		} else {
            		assert(u <= 0x10FFFF);
            		str += static_cast<char>(0xF0 | ((u >> 18) & 0xFF));
            		str += static_cast<char>(0x80 | ((u >> 12) & 0x3F));
            		str += static_cast<char>(0x80 | ((u >>  6) & 0x3F));
            		str += static_cast<char>(0x80 | ( u        & 0x3F));
        		}
    		}

    		Json::Jarray parse_array(int depth) {
    			expect(cur, '[');
    			parse_whitespace();
    			Json::Jarray tmp;
    			if(*cur == ']') {
    				++cur;
    				return tmp;
    			}
    			for(;;) {
    				Json tmpVal;
    				try{
    					tmpVal = parse_value(depth+1);
    				} catch(Exception) {
    					throw;
    				}
    				tmp.push_back(tmpVal);
    				parse_whitespace();
    				if(*cur == ',') {
    					++cur;
    					parse_whitespace();
    				} else if(*cur == ']') {
    					++cur;
    					return tmp;
    				} else {
    					throw(Exception("PARSE_MISS_COMMA_OR_SQUARE_BRACKET"));
    				}
    			}
    		}

    		Json::Jobject parse_object(int depth) {
    			expect(cur, '{');
    			parse_whitespace();
    			Json::Jobject tmp;
    			string key;
    			if(*cur == '}') {
    				++cur;
    				return tmp;
    			}

    			for(;;) {
    				if(*cur != '\"')
    					throw(Exception("PARSE_MISS_KEY"));
    				try{
    					key = parse_string();
    				} catch(Exception) {
    					throw(Exception("PARSE_MISS_KEY"));
    				}

    				parse_whitespace();
    				if(*cur++ != ':')
    					throw(Exception("PARSE_MISS_COLON"));
    				parse_whitespace();
    				Json tmpVal;
    				try {
    					tmpVal = parse_value(depth+1);
    				} catch(Exception) {
    					throw;
    				}

    				tmp[key] = tmpVal;
    				key.clear();
    				parse_whitespace();
    				if(*cur == ',') {
    					++cur;
    					parse_whitespace();
    				} else if(*cur == '}') {
    					++cur;
    					return tmp;
    				} else {
    					throw(Exception("PARSE_MISS_COMMA_OR_CURLY_BRACKET"));
    				}
    			}
    		}

 		};
 	}

 	void Json::dump(std::string& out) const {
 		m_ptr->dump(out);
 	}

 	Json Json::load(const string& in, string& err) {
    	JParser parser { in.c_str() };
    	Json result;
    	try {
    		result = parser.parse_json();
    		err = "PARSE_OK";
    	} catch(const Exception& msg) {
    		err = msg.what();
    	} catch(...) {

    	}

    	return result;
    } 
}