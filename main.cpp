#include "json.h"

int main() {
    std::ifstream ifs("./emxample_data/xxx.json");
    std::stringstream ss;
    ss << ifs.rdbuf();
    std::string s{ss.str()};
    json::Node x = json::parse(s).value();
    std::cout << x << '\n';
    if (x.type() == "Object") {
        json::Object& root = x.as<json::Object>();
        root["Cukoo"] = {json::Array{}};
        root["Cukoo"].as<json::Array>().push_back({json::Value{24}});
        root["Cukoo"].as<json::Array>().push_back({json::Value{"boy"}});
        std::cout << x << '\n';
    }
}
