//
// Created by scc on 2019/11/5.
//

#ifndef TEST_JEXCEPTION_H
#define TEST_JEXCEPTION_H
#include <string>
#include <stdexcept>

namespace json{

    class Exception final : public std::logic_error{
    public:
        Exception(const std::string &errMsg) : logic_error(errMsg) { }
    };

}
#endif //TEST_JEXCEPTION_H
