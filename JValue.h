#ifndef JLEPT_H
#define JLEPT_H
#include <string>
#include <iostream>
#include <stdexcept>
#include <memory>
#include <unordered_map>
#include <fstream>
#include <vector>

namespace json {
class JValue {
public:
	JValue() { this->type_ = json::LEPT_NULL; }
	virtual int get_type() const { return type_; }

	virtual void write(std::ostream& out, int indent = 0) { out << "null"; }
	virtual double& get_number() { throw std::runtime_error("NOT_NUMBER"); }
	virtual bool& get_bool() { throw std::runtime_error("NOT_BOOL"); }
	virtual std::string& get_string() { throw std::runtime_error("NOT_STRING"); }
	virtual std::vector<std::shared_ptr<JValue>>& get_array() {
		throw std::runtime_error("NOT_ARRAY");
	}

	virtual std::unordered_map<std::string, std::shared_ptr<JValue>>& get_object() {
		throw std::runtime_error("NOT_OBJECT");
	}

	void write_to_file(const std::string& filename) {
		std::ofstream out(filename);
		if(!out.good())
			throw(std::runtime_error("Can't open" + filename));
		write(out, 0);
	}

	virtual ~JValue() = default;

protected:
	json::lept_type type_;
	static void indent(std::ostream& out, int depth) {
		for(int i=0; i<depth; ++i) 
			out << '\t';
	}
};

class JString:public JValue {
public:
	JString() { this->type_ = json::LEPT_STRING; }
	JString(const std::string& str) : str_(str) {
		this->type_ = json::LEPT_STRING;
	}

	virtual int get_type() const final { return type_; }
	virtual void write(std::ostream& out, int indent = 0) final {
		out << str_;
	}
	virtual std::string& get_string() final { return str_; }
private:
	std::string str_;
};

class JNumber:public JValue {
public:
	JNumber() { this->type_ = json::LEPT_NUMBER; }
	JNumber(double value) : value_(value) {
		this->type_ = json::LEPT_NUMBER;
	}

	virtual int get_type() final { return type_; }
	virtual void write(std::ostream& out, int indent = 0) final { out << value_; }
	virtual double& get_number() final { return value_; }
private:
	double value_;
};

class JBool:public JValue {
public:
	JBool() { this->type_ = json::LEPT_BOOL; }
	JBool(bool value) : value_(value) { this->type_ = json::LEPT_BOOL; }
	virtual int get_type() const final { return type_; }
	virtual void write(std::ostream& out, int indent = 0) final {
		out << (value_ ? "true" : "false");
	}

	virtual bool& get_bool() final { return value_; }
private:
	bool value_;
};

class JArray:public JValue {
public:
	JArray() { this->type_ = json::LEPT_ARRAY; }
	virtual int get_type() const final { return type_; }
	virtual std::vector<std::shared_ptr<JValue>>& get_array() {
		return vec_;
	}
	std::shared_ptr<JValue>& operator[](size_t index) { return vec_[index]; }
	virtual void write(std::ostream& out, int idt = 0) final;
private:
	std::vector<std::shared_ptr<JValue>> vec_;
};

class JObject:public JValue {
public:
	JObject() { this->type_ = json::LEPT_OBJECT; }
	virtual int get_type() const final { return type_; }
	virtual std::unordered_map<std::string, std::shared_ptr<JValue>>& get_object() {
		return obj_;
	}
	std::shared_ptr<JValue>& operator[](const std::string& key) { return obj_[key]; }
	virtual void write(std::ostream& out, int idt = 0) final;

	template<typename T>
	void insert(const std:: string& key, T value) {
		obj_[key] = std::make_shared<T>(value);
	}
private:
	std::unordered_map<std::string, std::shared_ptr<JValue>> obj_;
};

#endif