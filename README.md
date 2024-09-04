# JSON-Parser

这是一个使用 C++17 实现的 JSON 解析器，非常简单、轻量、易用。

## 技术与功能
- 使用 `std::variant` 管理 JSON 的基本数据类型
- 使用 `std::string_view` 明确只读语义，避免拷贝字符串对象
- 使用 `std::optional` 作为错误处理方案
- 通过递归下降对 JSON 字符串进行解析
- 支持对解析后的对象进行动态修改，并重新输出为 JSON 字符串
