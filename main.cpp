#include "json.h"

int main() {
    using namespace json;
    std::ifstream ifs("./example_data/xxx.json");
    std::stringstream ss;
    ss << ifs.rdbuf();
    std::string s{ss.str()};
    Node x = parse(s).value_or(Node{});
    std::cout << x << '\n';
    if (x.type() == "Object") {
        Object& root = x.as<Object>();
        root["Cukoo"] = {Array{}};
        root["Cukoo"].as<Array>().push_back({Value{24}});
        root["Cukoo"].as<Array>().push_back({Value{"boy"}});
        std::cout << x << '\n';
    }
}
