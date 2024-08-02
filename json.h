#pragma once

#include <cstdint>
#include <format>
#include <fstream>
#include <iostream>
#include <map>
#include <optional>
#include <sstream>
#include <string>
#include <variant>
#include <vector>

namespace json {

struct Node;

// JSON 的基本数据类型
using Null = std::monostate;
using Bool = bool;
using Int = int64_t;
using Float = double;
using String = std::string;
using Array = std::vector<Node>;
using Object = std::map<std::string, Node>;

using Value = std::variant<Null, Bool, Int, Float, String, Array, Object>;

// Value 所保有的类型可以是基本数据类型中的任意一种
// Node 是 Value 的包装器，在其基础上提供一些强化功能：
// 1. 可以使用 type 返回当前所保有的类型
// 2. 可以使用 as<> 返回当前所保有的对象的引用
struct Node {
    Value value;

    std::string_view type() {
        switch (value.index()) {
            case 0:
                return "Null";
            case 1:
                return "Bool";
            case 2:
                return "Int";
            case 3:
                return "Float";
            case 4:
                return "String";
            case 5:
                return "Array";
            case 6:
                return "Object";
            default:
                return "valueless_by_exception";
        }
    }

    template <typename T>
    T& as() {
        T* p = std::get_if<T>(&value);
        if (!p) {
            throw std::runtime_error(
                std::format("Type error, expected {}", type()));
        }
        return *p;
    }
};

// JsonParser 描述了一个 Json 解析器
// 当解析失败时不抛出异常，而是返回 std::nullopt
class JsonParser {
   public:
    explicit JsonParser(std::string_view json_str) : json_str_(json_str) {}

    ~JsonParser() = default;

    auto parse() -> std::optional<Node> {
        auto value = parse_value();
        if (!value) {
            return std::nullopt;
        }
        return Node{*value};
    }

   private:
    void parse_whitespace() {
        while (pos_ < json_str_.size() && std::isspace(json_str_[pos_])) {
            ++pos_;
        }
    }

    auto parse_null() -> std::optional<Value> {
        if (json_str_.substr(pos_, 4) == "null") {
            pos_ += 4;
            return Null{};
        }
        return std::nullopt;
    }

    auto parse_true() -> std::optional<Value> {
        if (json_str_.substr(pos_, 4) == "true") {
            pos_ += 4;
            return true;
        }
        return std::nullopt;
    }

    auto parse_false() -> std::optional<Value> {
        if (json_str_.substr(pos_, 5) == "false") {
            pos_ += 5;
            return false;
        }
        return std::nullopt;
    }

    auto parse_number() -> std::optional<Value> {
        size_t endpos = pos_;
        while (endpos < json_str_.size() &&
               (std::isdigit(json_str_[endpos]) || json_str_[endpos] == 'e' ||
                json_str_[endpos] == '.')) {
            ++endpos;
        }
        std::string number = std::string{json_str_.substr(pos_, endpos - pos_)};
        pos_ = endpos;
        static auto is_float = [](std::string& number) {
            return number.find('.') != std::string::npos ||
                   number.find('.') != std::string::npos;
        };
        try {
            if (is_float(number)) {
                Float x = std::stod(number);
                return x;
            } else {
                Int x = std::stoi(number);
                return x;
            }
        } catch (...) {
            return std::nullopt;
        }
    }

    auto parse_string() -> std::optional<Value> {
        ++pos_;  // "
        size_t endpos = pos_;
        while (endpos < json_str_.size() && json_str_[endpos] != '"') {
            ++endpos;
        }
        std::string str = std::string{json_str_.substr(pos_, endpos - pos_)};
        pos_ = endpos + 1;  // "
        return str;
    }

    // 递归下降解析 Array
    auto parse_array() -> std::optional<Value> {
        ++pos_;  // [
        Array arr;
        while (pos_ < json_str_.size() && json_str_[pos_] != ']') {
            auto val = parse_value();
            if (!val) {
                arr.push_back({});
            } else {
                arr.push_back({val.value()});
            }
            parse_whitespace();
            if (pos_ < json_str_.size() && json_str_[pos_] == ',') {
                ++pos_;
            }
        }
        ++pos_;
        return arr;
    }

    // 递归下降解析 Object
    auto parse_object() -> std::optional<Value> {
        ++pos_;  // {
        Object obj;
        while (pos_ < json_str_.size() && json_str_[pos_] != '}') {
            // key 必须是一个 String，否则返回 Null
            // 解析出错也返回 Null
            auto key = parse_value();
            if (!key || !std::holds_alternative<String>(key.value())) {
                return std::nullopt;
            }

            parse_whitespace();
            if (pos_ < json_str_.size() && json_str_[pos_] == ':') {
                ++pos_;
            }

            auto val = parse_value();
            if (!val) {
                return std::nullopt;
            }
            obj[std::get<String>(key.value())] = {val.value()};

            parse_whitespace();
            if (pos_ < json_str_.size() && json_str_[pos_] == ',') {
                ++pos_;
            }
        }
        parse_whitespace();
        ++pos_;  // }
        return obj;
    }

    auto parse_value() -> std::optional<Value> {
        parse_whitespace();
        switch (json_str_[pos_]) {
            case 'n':
                return parse_null();
            case 't':
                return parse_true();
            case 'f':
                return parse_false();
            case '"':
                return parse_string();
            case '[':
                return parse_array();
            case '{':
                return parse_object();
            default:
                return parse_number();
        }
    }

   private:
    std::string_view json_str_;
    size_t pos_{};
};

inline auto parse(std::string_view json_str) -> std::optional<Node> {
    return JsonParser{json_str}.parse();
}

struct JsonGenerator {
    static auto generate_node(const Node& node) -> std::string {
        static auto f = [](auto&& arg) -> std::string {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, Null>) {
                return "null";
            } else if constexpr (std::is_same_v<T, Bool>) {
                return arg ? "true" : "false";
            } else if constexpr (std::is_same_v<T, Int>) {
                return std::to_string(arg);
            } else if constexpr (std::is_same_v<T, Float>) {
                return std::to_string(arg);
            } else if constexpr (std::is_same_v<T, String>) {
                return generate_string(arg);
            } else if constexpr (std::is_same_v<T, Array>) {
                return generate_array(arg);
            } else if constexpr (std::is_same_v<T, Object>) {
                return generate_object(arg);
            }
        };
        // f 的入参是 variant 当前所保有的成员，而不是 variant
        return std::visit(f, node.value);
    }

    static auto generate_string(const String& str) -> std::string {
        std::string json_str = "\"";
        json_str.append(str);
        json_str.append("\"");
        return json_str;
    }

    static auto generate_array(const Array& array) -> std::string {
        std::string json_str = "[";
        for (const auto& node : array) {
            json_str.append(generate_node(node));
            json_str.append(",");
        }
        if (!array.empty()) {
            json_str.pop_back();
        }
        json_str.append("]");
        return json_str;
    }
    
    static auto generate_object(const Object& object) -> std::string {
        std::string json_str = "{";
        for (const auto& [key, node] : object) {
            json_str.append(generate_string(key));
            json_str.append(":");
            json_str.append(generate_node(node));
            json_str.append(",");
        }
        if (!object.empty()) {
            json_str.pop_back();
        }
        json_str.append("}");
        return json_str;
    }
};

inline auto generate(const Node& node) -> std::string {
    return JsonGenerator::generate_node(node);
}

inline auto operator<<(std::ostream& out, const Node& node) -> std::ostream& {
    return out << JsonGenerator::generate_node(node);
}

}  // namespace json
