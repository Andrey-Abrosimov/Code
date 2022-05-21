#include "json.h"

namespace json {
    Node LoadArray(std::istream& input) {
        Array result;

        for (char c; input >> c;) {
            if(c == ']')
            {
                return Node(std::move(result));
            }
            if (c != ',') {
                input.putback(c);
            }
            result.push_back(LoadNode(input));
        }

        throw ParsingError("LoadArray Error");
    }
    
    Number LoadNumber(std::istream& input) {
        using namespace std::literals;

        std::string parsed_num;

        // Считывает в parsed_num очередной символ из input
        auto read_char = [&parsed_num, &input] {
            parsed_num += static_cast<char>(input.get());
            if (!input) {
                throw ParsingError("Failed to read number from stream"s);
            }
        };

        // Считывает одну или более цифр в parsed_num из input
        auto read_digits = [&input, read_char] {
            if (!std::isdigit(input.peek())) {
                throw ParsingError("A digit is expected"s);
            }
            while (std::isdigit(input.peek())) {
                read_char();
            }
        };

        if (input.peek() == '-') {
            read_char();
        }
        // Парсим целую часть числа
        if (input.peek() == '0') {
            read_char();
            // После 0 в JSON не могут идти другие цифры
        } else {
            read_digits();
        }

        bool is_int = true;
        // Парсим дробную часть числа
        if (input.peek() == '.') {
            read_char();
            read_digits();
            is_int = false;
        }

        // Парсим экспоненциальную часть числа
        if (int ch = input.peek(); ch == 'e' || ch == 'E') {
            read_char();
            if (ch = input.peek(); ch == '+' || ch == '-') {
                read_char();
            }
            read_digits();
            is_int = false;
        }

        try {
            if (is_int) {
                // Сначала пробуем преобразовать строку в int
                try {
                    return std::stoi(parsed_num);
                } catch (...) {
                    // В случае неудачи, например, при переполнении,
                    // код ниже попробует преобразовать строку в double
                }
            }
            return std::stod(parsed_num);
        } catch (...) {
            throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
        }
    }
    
    Node LoadString(std::istream& input) {
        using namespace std::literals;

        auto it = std::istreambuf_iterator<char>(input);
        auto end = std::istreambuf_iterator<char>();
        std::string s;
        while (true) {
            if (it == end) {
                // Поток закончился до того, как встретили закрывающую кавычку?
                throw ParsingError("String parsing error");
            }
            const char ch = *it;
            if (ch == '"') {
                // Встретили закрывающую кавычку
                ++it;
                break;
            } else if (ch == '\\') {
                // Встретили начало escape-последовательности
                ++it;
                if (it == end) {
                    // Поток завершился сразу после символа обратной косой черты
                    throw ParsingError("String parsing error");
                }
                const char escaped_char = *(it);
                // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
                switch (escaped_char) {
                    case 'n':
                        s.push_back('\n');
                        break;
                    case 't':
                        s.push_back('\t');
                        break;
                    case 'r':
                        s.push_back('\r');
                        break;
                    case '"':
                        s.push_back('"');
                        break;
                    case '\\':
                        s.push_back('\\');
                        break;
                    default:
                        // Встретили неизвестную escape-последовательность
                        throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
                }
            } else if (ch == '\n' || ch == '\r') {
                // Строковый литерал внутри- JSON не может прерываться символами \r или \n
                throw ParsingError("Unexpected end of line"s);
            } else {
                // Просто считываем очередной символ и помещаем его в результирующую строку
                s.push_back(ch);
            }
            ++it;
        }

        return Node(std::move(s));
    }
    
    Node LoadBool(std::istream& input) {
        std::string result(5, '0');

        for(size_t i = 0; i < 4; i++)
        {
            input >> result[i];
        }

        if(result == "true0")
        {
            return Node(true);
        }
        else if(input >> result[4]; result == "false")
        {
            return Node(false);
        }

        throw ParsingError("LoadNull Error");
    }
    
    Node LoadDict(std::istream& input) {
        Dict result;

        for (char c; input >> c && c != '}';) {
            if (c == ',') {
                input >> c;
            }

            std::string key = LoadString(input).AsString();
            input >> c;
            result.insert({move(key), LoadNode(input)});
        }

        return Node(move(result));
    }

    Node LoadNull(std::istream& input) {
        std::string value(4, '0');

        for(size_t i = 0; i < 4; i++)
        {
            input >> value[i];
        }

        if(value == "null")
        {
            return Node(nullptr);
        }

        throw ParsingError("LoadNull Error");
    }

    Node LoadNode(std::istream& input) {
        char c;
        input >> c;

        if (c == '[') {
            return LoadArray(input);
        } else if (c == '{') {
            return LoadDict(input);
        } else if (c == '"') {
            return LoadString(input);
        } else if(c == 'n') {
            input.putback(c);
            return LoadNull(input);
        } else if(c == 'f' || c == 't') {
            input.putback(c);
            return LoadBool(input);
        } else {
            input.putback(c);
            Number n = LoadNumber(input);
            return (std::holds_alternative<int>(n) ? Node(std::get<int>(n)) : Node(std::get<double>(n)));
        }
    }
    
    Document Load(std::istream& input) {
        return Document{LoadNode(input)};
    }
}
