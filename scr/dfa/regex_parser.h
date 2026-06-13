/**
 * @file regex_parser.h
 * @brief 正则表达式解析器
 * @details 将正则表达式转换为后缀形式，供Thompson构造法使用
 *          支持字符类[a-z]、连接、选择、闭包等操作
 * @author PL/0 Compiler Project
 * @date 2026-06-11
 */

#ifndef REGEX_PARSER_H
#define REGEX_PARSER_H

#include <string>

/**
 * @class RegexParser
 * @brief 正则表达式解析类
 * @details 提供正则表达式的预处理和后缀转换功能
 */
class RegexParser {
public:
    /**
     * @brief 插入显式连接符
     * @details 在需要连接的位置插入'.'运算符
     *          例如：ab -> a.b, a[b-c] -> a.[b-c]
     * @param regex 原始正则表达式
     * @return 插入连接符后的正则表达式
     */
    static std::string insertConcat(const std::string& regex);
    
    /**
     * @brief 调度场算法转换为后缀
     * @details 使用调度场算法将中缀正则表达式转换为后缀形式
     *          运算符优先级：*、+ (3) > . (2) > | (1)
     * @param infix 中缀形式的正则表达式
     * @return 后缀形式的正则表达式
     */
    static std::string shuntingYard(const std::string& infix);

    /**
     * @brief 扩展{n}量词
     * @details 简单的正则表达式流程不支持{n}量词
     *          此函数将X{n}展开为X重复n次
     *          支持字符类[a-z]{n}和单字符操作数
     * @param regex 原始正则表达式
     * @return 展开量词后的正则表达式
     */
    static std::string expandQuantifiers(const std::string& regex);

private:
    /**
     * @brief 获取运算符优先级
     * @param op 运算符字符
     * @return 优先级数值（越大越高）
     */
    static int precedence(char op);
};

#endif
