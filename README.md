# ccjson

#### Introduction

本项目为c++11编写的轻量级json序列化与序列化库, 不依赖第三方库. 仅支持UTF-8 json文本.

| C++         | JSON   |
| ----------- | ------ |
| string      | 字符串 |
| double      | 数字   |
| map         | 对象   |
| bool        | 布尔   |
| vector      | 数组   |
| struct Null | Null   |

#### Envoirment

* OS: 跨平台
* Complier: g++

#### Usage

在项目中包含ccjson.h和ccjson.cpp即可

#### Technical points

* 在特定场景下使用特定设计模式(解释器模式);
* 在恰当时机使用移动语义来提高效率;
* 使用RALL机制, 将内存问题规避, 同时也利用智能指针的特性来避免复制, 提高了效率, 提高了代码的可读性和简洁程度;
* 使用STL的基础设施完成业务代码, 保证了程序的稳定和效率, 不盲目造过于底层的轮子;

#### Test

使用开源的minunit测试框架, 测试用例详见main.cpp

#### Performance

测试用例:

```c++
int length = 10000*10000;
std::string jsonStr(length, '0');
auto jsonStr1 = ("\"") + jsonStr + "\"}";
std::string ins("{\"a\":1.0, \"b\":");
ins.append(jsonStr1);
```

X64 Release Windows 8GB 内存 用时：

```c++
test_object_parse2_long_string eslaped(second) :0
test_object_parse2_long_string eslaped(microsecond) :551868
```

同样的json串，rapidjson的用时：

```
test_object_parse2_long_string eslaped(second) :0
test_object_parse2_long_string eslaped(microsecond) :538021
```

从上可以看出, 在解析轻量级json时, 达到了与json开源库Rapidjson相近的性能(测试用例单一,  复杂json字符串的解析性能有待验证).

#### Superiority

* 提供了简洁明了的接口, 减低了使用者的学习成本;
* 具有较强的可扩展性.