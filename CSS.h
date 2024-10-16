// Author: Jake Rieger
// Created: 10/15/2024.
//

#pragma once

#include <utility>
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include <ranges>
#include <iostream>

namespace CSS {
    using PropertyTable = std::unordered_map<std::string, std::string>;
    using Stylesheet    = std::unordered_map<std::string, PropertyTable>;

    struct ParseError {
        ParseError() = default;

        explicit ParseError(std::string msg, std::string line)
            : errMsg(std::move(msg)), errLine(std::move(line)) {}

        void print() const {
            fprintf(stderr, "ParseError:\n\n> %s\n\nError: %s\n", errLine.c_str(), errMsg.c_str());
        }

        std::string errMsg;
        std::string errLine;
    };

    static void PrintStylesheet(Stylesheet& stylesheet) {
        for (const auto& selector : stylesheet | std::views::keys) {
            std::cout << selector << '\n';
            for (const auto& [property, value] : stylesheet[selector]) {
                std::cout << "  " << property << ':' << " " << value << '\n';
            }
        }
    }

    class Parser {
    public:
        explicit Parser(const std::string& css) : position(0) {
            lexer  = std::make_unique<Lexer>(css);
            tokens = lexer->tokenize();
        }

        void parse() noexcept {
            while (!isAtEnd() && !hadError) {
                parseRule();
            }
        }

        [[nodiscard]] Stylesheet getStylesheet() const {
            return stylesheet;
        }

        bool hadError        = false;
        ParseError lastError = {};

    private:
        void makeError(std::string msg) {
            ParseError error;
            error.errMsg  = std::move(msg);
            error.errLine = lexer->input;

            this->lastError = error;
            this->hadError  = true;
        }

        enum class TokenType {
            Identifier,
            Number,
            String,
            Colon,
            Semicolon,
            BraceOpen,
            BraceClose,
            HexColor,
            Unknown,
            EndOfFile,
        };

        struct Token {
            TokenType type;
            std::string value;
            Token(TokenType type, std::string value) : type(type), value(std::move(value)) {}
        };

        class Lexer {
        public:
            explicit Lexer(std::string css) : input(std::move(css)), position(0) {}

            std::vector<Token> tokenize() {
                std::vector<Token> tokens;

                while (position < input.size()) {
                    const char currentChar = input[position];

                    if (std::isspace(currentChar)) {
                        size_t start = position;
                        while (position < input.size() && std::isspace(input[position])) {
                            position++;
                        }
                    } else if (std::isalpha(currentChar) || currentChar == '-') {
                        tokens.push_back(lexIdentifier());
                    } else if (std::isdigit(currentChar)) {
                        tokens.push_back(lexNumber());
                    } else if (currentChar == '"') {
                        tokens.push_back(lexString());
                    } else if (currentChar == ':') {
                        tokens.emplace_back(TokenType::Colon, ":");
                        position++;
                    } else if (currentChar == ';') {
                        tokens.emplace_back(TokenType::Semicolon, ";");
                        position++;
                    } else if (currentChar == '{') {
                        tokens.emplace_back(TokenType::BraceOpen, "{");
                        position++;
                    } else if (currentChar == '}') {
                        tokens.emplace_back(TokenType::BraceClose, "}");
                        position++;
                    } else if (currentChar == '#') {
                        tokens.push_back(lexHexColor());
                    } else if (currentChar == '/' && peek() == '*') {
                        position += 2;  // Skip opening '/*'
                        size_t start = position;

                        while (position < input.size() &&
                               !(input[position] == '*' && peek() == '/')) {
                            position++;
                        }

                        if (position < input.size()) {
                            position += 2;  // Skip closing '*/'
                        }
                    } else {
                        tokens.emplace_back(TokenType::Unknown, std::string(1, currentChar));
                        position++;
                    }
                }

                tokens.emplace_back(TokenType::EndOfFile, "");
                return tokens;
            }

            std::string input;

        private:
            std::size_t position;

            [[nodiscard]] char peek(int offset = 1) const {
                return (position + offset < input.size()) ? input[position + offset] : '\0';
            }

            Token lexIdentifier() {
                const size_t start = position;
                while (position < input.size() &&
                       (std::isalnum(input[position]) || input[position] == '-')) {
                    position++;
                }
                return {TokenType::Identifier, input.substr(start, position - start)};
            }

            Token lexNumber() {
                const size_t start = position;
                while (position < input.size() && std::isdigit(input[position])) {
                    position++;
                }
                return {TokenType::Number, input.substr(start, position - start)};
            }

            Token lexHexColor() {
                position++;  // Skip pound sign
                const size_t start = position;

                while (position < input.size() && input[position] != ';') {
                    position++;
                }

                if ((position - start) != 6) { return {TokenType::Unknown, "<InvalidColor>"}; }

                return {TokenType::HexColor, input.substr(start, position - start)};
            }

            Token lexString() {
                position++;  // Skip opening quote
                const size_t start = position;

                while (position < input.size() && input[position] != '"') {
                    position++;
                }

                if (position < input.size()) {
                    position++;  // Skip closing quote
                }

                return {TokenType::String, input.substr(start, position - start - 1)};
            }
        };

    private:
        std::unique_ptr<Lexer> lexer;
        std::vector<Token> tokens;
        Stylesheet stylesheet;

    private:
        std::size_t position;

        [[nodiscard]] bool isAtEnd() const noexcept {
            return (position >= tokens.size()) or
                   (tokens.at(position).type == TokenType::EndOfFile);
        }

        Token advance() noexcept {
            if (!isAtEnd()) position++;
            return tokens.at(position - 1);
        }

        [[nodiscard]] Token peek() const noexcept {
            return tokens.at(position);
        }

        bool match(TokenType type) noexcept {
            if (isAtEnd() or peek().type != type) return false;
            advance();
            return true;
        }

        void parseRule() noexcept {
            const auto selector = parseSelector();
            if (!match(TokenType::BraceOpen)) { makeError("Expected '{' after selector."); }

            parseDeclarationBlock(selector);

            if (!match(TokenType::BraceClose)) {
                makeError("Expected '}' after declaration block.");
            }
        }

        std::string parseSelector() noexcept {
            std::string selector;
            if (match(TokenType::Identifier)) { selector = tokens.at(position - 1).value; }
            return selector;
        }

        void parseDeclarationBlock(const std::string& selector) noexcept {
            while (peek().type != TokenType::BraceClose and !isAtEnd()) {
                parseDeclaration(selector);
            }
        }

        void parseDeclaration(const std::string& selector) noexcept {
            if (!match(TokenType::Identifier)) { makeError("Expected property name."); }
            const std::string property = tokens.at(position - 1).value;

            if (!match(TokenType::Colon)) { makeError("Expected ':' after property name."); }
            const std::string value = parseValue();

            if (!match(TokenType::Semicolon)) { makeError("Expected ';' after property value."); }

            // Store result in stylesheet
            stylesheet[selector][property] = value;
        }

        std::string parseValue() noexcept {
            std::string value;

            if (match(TokenType::Number) or match(TokenType::String) or
                match(TokenType::Identifier) or match(TokenType::HexColor)) {
                value = tokens.at(position - 1).value;  // Skip the value for now
            } else {
                makeError("Expected a value after '<property>:'.");
            }

            return value;
        }
    };
}  // namespace CSS