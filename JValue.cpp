#include "assert.h"
#include "JValue.h"
#include "JParser.h"
#include "JException.h"

namespace json {
    std::shared_ptr<JValue> JValue::parse_json(const std::string& content) {
        std::shared_ptr<JValue> tmp;
        JParser(tmp, content);
        return tmp;
    }
    std::shared_ptr<JValue> JValue::parse_json(const std::string& content, std::string& status) {
        std::shared_ptr<JValue> tmp;
        try{
            tmp = parse_json(content);
            status = "PARSE_OK";
        } catch(const json::Exception& msg) {
            status = msg.what();
        } catch(...) {

        }

        return tmp;
    }
    void JArray::write(std::ostream& out, int idt) {
        out << '[';
        if(vec_.empty()) {
            out << (']');
            return;
        }

        bool first_element = true;
        for(auto it = vec_.cbegin(); it != vec_.cend(); ++it) {
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


    void JObject::write(std::ostream& out, int idt) {
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
            out << '"';
            out << it->first;
            out << '"';
            out << " : ";
            it->second->write(out, idt+1);
        }

        out << '\n';
        indent(out, idt);
        out << '}';
    }

    void JValue::stringify_string(std::string& v, const std::string& str) {
        v += '\"';
        for(auto it = str.begin(); it < str.end(); ++it) {
            unsigned char ch = *it;
            switch (ch) {
                case '\"': v += "\\\""; break;
                case '\\': v += "\\\\"; break;
                case '\b': v += "\\b";  break;
                case '\f': v += "\\f";  break;
                case '\n': v += "\\n";  break;
                case '\r': v += "\\r";  break;
                case '\t': v += "\\t";  break;
                default:
                    if (ch < 0x20) {
                        char buffer[7] = {0};
                        sprintf(buffer, "\\u%04X", ch);
                        v += buffer;
                    }
                    else
                        v += *it;
            }
        }
        v += '\"';
    }

    void JArray::stringify_value(std::string& v) {
        v += '[';
        for (int i = 0; i < vec_.size(); ++i) {
            if (i > 0) v += ',';
            vec_[i]->stringify_value(v);
        }
        v += ']';
    }

    void JObject::stringify_value(std::string &v) {
        v += '{';
        bool flag = false;
        std::string tmp;
        for(auto i:obj_) {
            if(flag)
                v += ',';
            else
                flag = true;
            tmp = i.first;
            stringify_string(v, tmp);
            v += ':';
            (i.second)->stringify_value(v);
        }

        v += '}';
    }

}