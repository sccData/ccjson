// #pragma once
#ifndef CCJSON_H
#define CCJSON_H

#include <vector>
#include <map>
#include <string>
#include <memory>

namespace json {

	class JValue;

	class Json final {
	public:
		enum Jtype:int
		{
			JNULL, JBOOL, JNUMBER, JSTRING, JARRAY, JOBJECT
		};

		typedef std::vector<Json> Jarray;
		typedef std::map<std::string, Json> Jobject;

		Json() noexcept;                      // JNULL  
		Json(double value);                   // JNUMBER
		Json(bool value);                     // JBOOL
		Json(const std::string& value);       // JSTRING
		Json(std::string&& value);            // JSTRING
		Json(const char* value);              // JSTRING
		Json(const Jarray& value);             // JARRAY
		Json(Jarray&& value);                  // JARRAY
		Json(const Jobject& value);            // JOBJECT
		Json(Jobject&& value);                 // JOBJECT

		Json(const Json& t):m_ptr(t.m_ptr) {}

		Jtype get_type() const;

		bool is_null()   const { return get_type() == JNULL; }
		bool is_number() const { return get_type() == JNUMBER; }
		bool is_bool()   const { return get_type() == JBOOL; }
		bool is_string() const { return get_type() == JSTRING; }
		bool is_array()  const { return get_type() == JARRAY; }
		bool is_object() const { return get_type() == JOBJECT; }

		double get_number() const;
		bool get_bool() const;
		const std::string& get_string() const;
		const Jarray& get_array() const;
		const Jobject& get_object() const;

		void set_value();
		void set_value(double value);
		void set_value(bool value);
		void set_value(const std::string& value);
		void set_value(std::string&& value);
		void set_value(const char* value);
		void set_value(const Jarray& value);
		void set_value(Jarray&& value);
		void set_value(const Jobject& value);
		void set_value(Jobject&& value);

		const Json& operator[](size_t i) const;
		const Json& operator[](const std::string& key) const;
		Json& operator[](size_t i);
		Json& operator[](const std::string& key);

		void dump(std::string& out) const;
		std::string dump() const {
			std::string out;
			dump(out);
			return out;
		}

		static Json load(const std::string& in, std::string& err);
		static Json load(const char* in, std::string& err) {
			if(in) {
				return load(std::string(in), err);
			} else {
				err = "null input";
				return nullptr;
			}
		}

		Json& operator= (const Json& rhs);
		Json& operator= (Json&& rhs);
		bool operator== (const Json& rhs) const;
		bool operator<  (const Json& rhs) const;
		bool operator!= (const Json& rhs) const { return !(*this == rhs); }
		bool operator<= (const Json& rhs) const { return !(rhs > *this); }
		bool operator>  (const Json& rhs) const { return (rhs < *this); }
		bool operator>= (const Json& rhs) const { return !(*this < rhs); }

	private:
		std::shared_ptr<JValue> m_ptr;

	};

	class JValue {
		friend class Json;
		// friend class JDouble;
	protected:
		virtual Json::Jtype get_type() const = 0;
		virtual bool equals(const JValue* rhs) const = 0;
		virtual bool less(const JValue* rhs) const = 0;
		virtual void dump(std::string& out) const = 0;
		virtual double get_number() const;
		virtual bool get_bool() const;
		virtual const std::string& get_string() const;
		virtual const Json::Jarray& get_array() const;
		virtual const Json& operator[](size_t i) const;
		virtual Json& operator[](size_t i);
		virtual const Json::Jobject& get_object() const;;
		virtual const Json& operator[](const std::string& key) const;
		virtual Json& operator[](const std::string& key);

		virtual void set_value(double value);
		virtual void set_value(bool value);
		virtual void set_value(const std::string& value);
		virtual void set_value(std::string&& value);
		virtual void set_value(const Json::Jarray& value);
		virtual void set_value(Json::Jarray&& value);
		virtual void set_value(const Json::Jobject& value);
		virtual void set_value(Json::Jobject&& value);

		virtual ~JValue() {}
	};
}

#endif