#include "json.h"
#include "JParser.h"

namespace pain {
	void Json::parse(const std::string& content, std::string& status) {
		try{
			parse(content);
			status = "PARSE_OK";
		} catch(const json::Exception& msg) {
			status = msg.what();
		} catch(...) {

		}
	}

	void Json::parse(const std::string& content) {
		JParser(v, content);
	}

	int Json::get_type() const {
		return v->get_type();
	}
}