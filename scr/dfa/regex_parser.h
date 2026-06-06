#ifndef REGEX_PARSER_H
#define REGEX_PARSER_H

#include <string>

class RegexParser {
public:
    static std::string insertConcat(const std::string& regex);
    static std::string shuntingYard(const std::string& infix);

private:
    static int precedence(char op);
};

#endif
