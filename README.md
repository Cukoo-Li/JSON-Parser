# JSON-Parser

本项目为使用 C++17 实现的 JSON 解析器，十分简易、轻量。

## 技术与功能
- 使用 `std::variant` 管理 JSON 的基本数据类型。
- 使用 `std::string_view` 明确只读语义，避免拷贝字符串对象
- 使用 `std::optional` 实现非侵入式的异常处理
- 通过递归下降对 JSON 字符串进行解析
- 支持解析后动态修改，并重新输出为 JSON 字符串

## 使用示例

