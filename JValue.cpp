#include "assert.h"
#include "JValue.h"


namespace json {
	inline
	void JArray::write(std::ostream& out, int idt = 0) final {
		out << '[';
		if(vec_.empty()) {
			out << (']');
			return;
		}

		bool first_element = true;
		for(auto it = vec_.cbegin(); i != vec_.cend(); ++i) {
			if(first_element) {
				first_element = false;
			} else {
				out << ',';
			}

			out << '\n';
			indent(out, idt);
			(*it)->write(out, idt+1);
		}
		out << '\n';
		indent(out, idt);
		out << ']';
	}

	inline
	void JObject::write(std::ostream& out, int idt = 0) final {
		out << '{';
		if(obj_.empty()) {
			out << ('}');
			return;
		}

		bool first_element = true;
		for(auto it = obj_.cbegin(); it != obj_.cend(); ++it) {
			if(first_element) {
				first_element = false;
			} else {
				out << ',';
			}
			out << '\n';
			indent(out, idt);
			out << it->first;
			out << " : ";
			it->second->write(out, idt+1);
		}

		out << '\n';
		indent(out, idt);
		out << '}';
	}
}