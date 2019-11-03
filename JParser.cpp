#include "JParser.h"
#include <errno.h>
#include <cctype>
#include <cstdlib>

namespace json {

	inline void JParser::expect(const char* &c, char ch) {
		assert(*c == ch);
		++c;
	}

	inline bool JParser::isdigit09(char ch) {
		return (ch >= '1' && ch <= '9');
	}

	JParser::JParser(shared_ptr<JValue>& val, const std::string& content)
			 :val_(val), cur_(content.c_str())
	{
		// val_.set_type(json::LEPT_NULL);
		parse_whitespace();
		parse_value();
		parse_whitespace();
		if(*cur_ != '\0') {
			// val_.set_type(json::LEPT_NULL);
			val_.reset(new JValue());
			throw(Exception("PARSE_ROOT_NOT_SINGULAR"));
		}
	}

	void JParser::parse_whitespace() {
		while(*cur_ == ' ' || *cur_ == '\t' || *cur_ == '\n' || *cur_ == '\r')
			++cur_;
	}

	void JParser::parse_value() {
		switch(*cur_) {
			case 'n': parse_literal("null", json::LEPT_NULL);   return;
			case 't': parse_literal("true", json::LEPT_TRUE);   return;
			case 'f': parse_literal("false", json::LEPT_FALSE); return;
			case '\"': parse_string();  return;
			case '[': parse_array();    return;
			case '{': parse_object();   return;
			default:  parse_number();   return;
			case '\0': throw(Exception("PARSE_EXPECT_VALUE"));
		}
	}

	void JParser::parse_literal(const char* literal, json::lept_type t) {
		expect(cur_, literal[0]);
		size_t i;
		for(i=0; literal[i+1]; ++i) {
			if(cur_[i] != literal[i+1])
				throw(Exception("PARSE_INVALID_VALUE"));
		}

		cur_ += i;
		if(t == json::LEPT_TRUE)
			val_ = std::make_shared<JBool>(true);
		else if(t == json::LEPT_FALSE)
			val_ = std::make_shared<JBool>(false);
	}

	void JParser::parse_number() {
		const char* p = cur_;
		if(*p == '-') ++p;
		if(*p == '0') ++p;
		else {
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
		double v = strtod(cur_, nullptr);
		if(errno == ERANGE && (v == HUGE_VAL || v == -HUGE_VAL))
			throw (Exception("PARSE_NUMBER_TOO_BIG"));
		val_ = std::make_shared<JNumber>(v);
		cur_ = p;
	}

	void JParser::parse_string() {
		std::string s;
		parse_string(s);
		val_ = std::make_shared<JString>(s);
	}

	void JParser::parse_string(std::string& tmp) {
		expect(cur_, '\"');
		const char* p = cur_;
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
		cur_ = p;
	}

	void JParser::parse_hex4(const char* &p, unsigned &u) {
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

	void JParser::parse_encode_utf8(std::string& str, unsigned u) const {
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

	void JParser::parse_array() {
		expect(cur_, '[');
		parse_whitespace();
		std::vector<std::shared_ptr<JValue>> tmp;
		if(*cur_ == ']') {
			++cur_;
			val_ = std::make_shared<JArray>();
			val_->vec_ = tmp;
			return;
		}
		for(;;) {
			try{
				parse_value();
			} catch(Exception) {
				val_.reset(new JValue());
				throw;
			}

			tmp.push_back(val_);
			parse_whitespace();
			if(*cur_ == ',') {
				++cur_;
				parse_whitespace();
			} else if(*cur_ == ']') {
				++cur_;
				val_.reset(new JArray());
				val_->vec_ = tmp;
				return;
			} else {
				// val_.set_type(json::LEPT_NULL);
				val_.reset(new JValue());
				throw(Exception("PARSE_MISS_COMMA_OR_SQUARE_BRACKET"));
			}
		}
	}

	void JParser::parse_object() {
		expect(cur_, '{');
		parse_whitespace();
		std::vector<std::pair<std::string, JValue>> tmp;
		std::string key;
		if(*cur_ == '}') {
			++cur_;
			// val_.set_object(tmp);
			val_ = make_shared<JObject>();
			val_->obj_ = tmp;
			return;
		}

		for(;;) {
			if(*cur_ != '\"')
				throw(Exception("PARSE_MISS_KEY"));
			try{
				parse_string(key);
			} catch(Exception) {
				throw(Exception("PARSE_MISS_KEY"));
			}

			parse_whitespace();
			if(*cur_++ != ':')
				throw(Exception("PARSE_MISS_COLON"));
			parse_whitespace();
			try{
				parse_value();
			} catch(Exception) {
				// val_.set_type(json::LEPT_NULL);
				val_.reset(new JValue());
				throw;
			}
			tmp.push_back(make_pair(key, val_));
			// val_.set_type(json::LEPT_NULL);
			val_.reset(new JValue());
			key.clear();
			parse_whitespace();
			if(*cur_ == ',') {
				++cur_;
				parse_whitespace();
			} else if(*cur_ == '}') {
				++cur_;
				//val_.set_object(tmp);
				val_ = std::make_shared<JObject>();
				val_->obj_ = tmp;
				return;
			} else {
				//val_.set_type(json::LEPT_NULL);
				val_.reset(new JValue());
				throw(Exception("PARSE_MISS_COMMA_OR_CURLY_BRACKET"));
			}
		}
	}

}
