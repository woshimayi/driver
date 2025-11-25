#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm> // 用于 std::replace

/**
 * 使用 std::stringstream 进行字符串切割 (Split)
 *
 * @param input 待切割的原始字符串
 * @param delimiter 用于切割字符串的分隔符
 * @return 包含切割后子字符串的 vector<string>
 */
std::vector<std::string> stringSplit_stringstream(const std::string &input, char delimiter)
{
    // 1. 复制输入字符串
    std::string temp_input = input;

    // 2. 将所有分隔符替换成空格
    // 这是为了让 stringstream 能够正确地使用空格作为默认分隔符进行读取。
    std::replace(temp_input.begin(), temp_input.end(), delimiter, ' ');

    // 3. 使用 stringstream 逐个读取子串
    std::stringstream ss(temp_input);
    std::string segment;
    std::vector<std::string> result;

    // >> 操作符会从流中提取被空格分隔的“单词”
    while (ss >> segment)
    {
        // 检查 segment 是否为空，虽然 ss >> segment 通常会忽略开头的空格，
        // 但如果输入末尾有多个分隔符，这里能更健壮地处理。
        if (!segment.empty())
        {
            result.push_back(segment);
        }
    }

    return result;
}

// =========================================================
// 演示函数
// =========================================================
void run_stringstream_example()
{
    std::cout << "--- 示例 1: 使用 stringstream 进行切割 ---\n";
    // std::string data = "Apple,Banana,Orange,Grape";
    std::string data = "12251|12983|12033|15524|11680|16065|14532|18325|13622";
    char delimiter = '|';

    std::vector<std::string> parts = stringSplit_stringstream(data, delimiter);

    std::cout << "原始字符串: \"" << data << "\"\n";
    std::cout << "分隔符: '" << delimiter << "'\n";
    std::cout << "切割结果:\n";
    for (const std::string &part : parts)
    {
        std::cout << "  [" << part << "]\n";
    }

    // 额外测试: 处理多个连续分隔符（会被忽略）
    std::string data_multi = "A::B:::C";
    delimiter = ':';
    std::vector<std::string> parts_multi = stringSplit_stringstream(data_multi, delimiter);
    std::cout << "\n额外测试 (连续分隔符 A::B:::C):\n";
    for (const std::string &part : parts_multi)
    {
        std::cout << "  [" << part << "]\n";
    }
}

/**
 * 使用 find/substr 进行字符串切割 (Split)
 *
 * @param input 待切割的原始字符串
 * @param delimiter 用于切割字符串的分隔符
 * @return 包含切割后子字符串的 vector<string>
 */
std::vector<std::string> stringSplit_find_substr(const std::string &input, const std::string &delimiter)
{
    std::vector<std::string> tokens;
    size_t start = 0;
    size_t end = 0;

    // 确保分隔符不为空，否则会进入无限循环
    if (delimiter.empty())
        return {input};

    while ((end = input.find(delimiter, start)) != std::string::npos)
    {
        // 提取从 start 到 end-start 长度的子串，并添加到结果中
        tokens.push_back(input.substr(start, end - start));
        // 更新起始位置到分隔符的后面
        start = end + delimiter.length();
    }

    // 循环结束后，将最后一个子串（从 start 到字符串末尾）添加到结果中
    tokens.push_back(input.substr(start));

    return tokens;
}

// =========================================================
// 演示函数
// =========================================================
void run_find_substr_example()
{
    std::cout << "\n--- 示例 2: 使用 find/substr 进行切割 ---\n";
    std::string data = "data1|data2|data3|"; // 注意末尾的分隔符
    std::string delimiter = "|";

    std::vector<std::string> parts = stringSplit_find_substr(data, delimiter);

    std::cout << "原始字符串: \"" << data << "\"\n";
    std::cout << "分隔符: '" << delimiter << "'\n";
    std::cout << "切割结果:\n";
    for (const std::string &part : parts)
    {
        std::cout << "  [" << part << "]\n";
    }

    // 额外测试: 处理连续分隔符（会保留空字符串）
    std::string data_multi = "A,,B,,,C";
    delimiter = ",";
    std::vector<std::string> parts_multi = stringSplit_find_substr(data_multi, delimiter);
    std::cout << "\n额外测试 (连续分隔符 A,,B,,,C):\n";
    for (const std::string &part : parts_multi)
    {
        std::cout << "  [" << part << "]\n";
    }
}

int main()
{
    run_stringstream_example();
    run_find_substr_example();
    return 0;
}