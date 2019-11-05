//
// Created by scc on 2019/11/5.
//

#ifndef TEST_JPARSER_H
#define TEST_JPARSER_H

#include "JValue.h"
namespace json {
    class JParser {
    public:
        JParser(std::shared_ptr<JValue>& val, const std::string& content);
    private:
        void parse_whitespace();
        void parse_value();
        void parse_literal(const char* literal, json::lept_type t);
        void parse_number();
        void parse_string();
        void parse_string(std::string& tmp);
        void parse_hex4(const char* &p, unsigned& u);
        void parse_encode_utf8(std::string& s, unsigned u) const;
        void parse_array();
        void parse_object();
        bool isdigit09(char ch);
        void expect(const char* &c, char ch);
        std::shared_ptr<JValue>& val_;
        const char* cur_;
    };

}
#endif //TEST_JPARSER_H
