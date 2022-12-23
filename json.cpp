#include "json.h"

using namespace std;

namespace json {

namespace {

Node LoadNode(istream& input);

Node LoadArray(istream& input) {
    Array result;
    char c = 0;
    for (; input >> c && c != ']';) {
        if (c != ',') {
            input.putback(c);
        }
        result.push_back(LoadNode(input));
    }
    if (c != ']') {
        throw ParsingError("Failed to load array");
    }

    return Node(move(result));
}

Node LoadNumber(istream& input) {
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
                return Node(std::stoi(parsed_num));
            } catch (...) {
                // В случае неудачи, например, при переполнении,
                // код ниже попробует преобразовать строку в double
            }
        }
        return Node(std::stod(parsed_num));
    } catch (...) {
        throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
    }
}

Node LoadString(istream& input) {
    string s;
    auto it = std::istreambuf_iterator<char>(input);
    auto end = std::istreambuf_iterator<char>();
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

    return Node(move(s));
}

Node LoadDict(istream& input) {
    Dict result;
    char c = 0;
    for (; input >> c && c != '}';) {
        if (c == ',') {
            input >> c;
        }

        string key = LoadString(input).AsString();
        input >> c;
        result.insert({move(key), LoadNode(input)});
    }
    if(c != '}') {
        throw ParsingError("Failed to load dict");
    }

    return Node(move(result));
}

Node LoadNull(istream& input) {
    const string nullptr_parsed = "null";
    string result;
    char c;
    for (size_t i = 0; input >> c && i < nullptr_parsed.size(); ++i) {
        result += c;
    }
    if (result == "null") {
        return Node(nullptr);
    } else {
        throw ParsingError("failed to load null");
    }
}

Node LoadBool(istream& input) {
    if (bool value; input >> std::boolalpha >> value) {
        return Node(value);
    }

    throw ParsingError("Fail to read bool from stream"s);
    
}

Node LoadNode(istream& input) {
    char c;
    input >> c;

    if (c == '[') {
        return LoadArray(input);
    } else if (c == '{') {
        return LoadDict(input);
    } else if (c == '"') {
        return LoadString(input);
    } else if (c == 'n') {
        input.putback(c);
        return LoadNull(input);
    } else if (c == 't' || c == 'f') {
        input.putback(c);
        return LoadBool(input);
    } else {
        input.putback(c);
        return LoadNumber(input);
    }
}

}  // namespace

bool Node::IsInt() const {
    if(std::holds_alternative<int>(value_)) {
        return true;
    }
    return false;
}

bool Node::IsDouble() const {
    if(std::holds_alternative<int>(value_) || std::holds_alternative<double>(value_)) {
        return true;
    }
    return false;
}

bool Node::IsPureDouble() const {
    if(std::holds_alternative<double>(value_)) {
        return true;
    }
    return false;
}

bool Node::IsBool() const {
    if(std::holds_alternative<bool>(value_)) {
        return true;
    }
    return false;
}

bool Node::IsString() const {
    if(std::holds_alternative<std::string>(value_)) {
        return true;
    }
    return false;
}

bool Node::IsNull() const {
    if(std::holds_alternative<std::nullptr_t>(value_)) {
        return true;
    }
    return false;
}

bool Node::IsArray() const {
    if(std::holds_alternative<json::Array>(value_)) {
        return true;
    }
    return false;
}

bool Node::IsMap() const {
    if(std::holds_alternative<json::Dict>(value_)) {
        return true;
    }
    return false;
}

int Node::AsInt() const {
    if(!IsInt()) {
        throw std::logic_error("wrong type");
    }
    return std::get<int>(value_);
}

bool Node::AsBool() const {
    if(!IsBool()) {
        throw std::logic_error("wrong type");
    }
    return std::get<bool>(value_);
}

double Node::AsDouble() const {
    if(IsInt()) {
        double ans = std::get<int>(value_);
        return ans;
    } else if (IsPureDouble()) {
        return std::get<double>(value_);
    } else {
        throw std::logic_error("wrong type");
    }
}

const string& Node::AsString() const {
    if(!IsString()) {
        throw std::logic_error("wrong type");
    }
    return std::get<std::string>(value_);
}

const Array& Node::AsArray() const {
    if(!IsArray()) {
        throw std::logic_error("wrong type");
    }
    return std::get<json::Array>(value_);
}

const Dict& Node::AsMap() const {
    if(!IsMap()) {
        throw std::logic_error("wrong type");
    }
    return std::get<json::Dict>(value_);
}

// Node::Node()
//     : value_(nullptr) {}

Node::Node(Array array)
    : value_(move(array)) {}

Node::Node(Dict map)
    : value_(move(map)) {}

Node::Node(int value)
    : value_(value) {}

Node::Node(string value)
    : value_(move(value)) {}

Node::Node(double value)
    : value_(value) {}

Node::Node(bool value)
    : value_(value) {}

Node::Node(std::nullptr_t value)
    : value_(value) {}

Document::Document(Node root)
    : root_(move(root)) {
}

const Node& Document::GetRoot() const {
    return root_;
}

Document Load(istream& input) {
    return Document{LoadNode(input)};
}

void Print(const Document& doc, std::ostream& output) {
    auto node = doc.GetRoot();
    PrintNode(node, output);

    // Реализуйте функцию самостоятельно
}

void PrintNode(const Node& node, std::ostream& out) {
    std::visit(
        [&out](const auto& value){ PrintValue(value, out); },
        node.GetValue());
}
// Шаблон, подходящий для вывода double и int
void PrintValue(int value, std::ostream &out) {
    out << value;
}

void PrintValue(double value, std::ostream &out) {
    out << value;
}

// Перегрузка функции PrintValue для вывода значений null
void PrintValue(std::nullptr_t, std::ostream& out) {
    out << "null"sv;
}

void PrintValue(std::string word, std::ostream& out) {
    out << "\"";
    for(auto ch: word) {
        switch (ch)
      {
        
        case '\'':
            out << "\\'";
            break;

        case '\"':
            out << "\\\"";
            break;
        
        case '\n':
            out << "\\n";
            break;
        
        case '\r':
            out << "\\r";
            break;

        case '\\':
            out << "\\\\";
            break;

        default:
            out << ch;
      }
   }
   out << "\"";
}


void PrintValue(bool bool_value, std::ostream& out) {
    if(bool_value) {
        out << "true";
    } else {
        out << "false";
    }
}
// Другие перегрузки функции PrintValue пишутся аналогично



void PrintValue(const json::Array& array, std::ostream& out) {
    out << '[';
    bool flag = false;
    for(auto element: array) {
        if(flag) { out << ',';}
        PrintNode(element, out);
        flag = true;
    }
    out << ']';
}

void PrintValue(const json::Dict& dict, std::ostream& out) {
    out << '{';
    bool flag = false;
    for (const auto& kv: dict)
    {
        if(flag) { out << ',';}
        out << '\"';
        out << kv.first << "\": ";
        PrintNode(kv.second, out);
        flag = true;
    }
    out << '}';
}

bool operator==(const Node& lhs, const Node& rhs) {
    return lhs.GetValue() == rhs.GetValue();
}

bool operator!=(const Node& lhs, const Node& rhs) {
    return lhs.GetValue() != rhs.GetValue();
}

bool operator==(const Document &lhs, const Document &rhs) {
    return lhs.GetRoot() == rhs.GetRoot();
}
bool operator!=(const Document &lhs, const Document &rhs) {
    return lhs.GetRoot() != rhs.GetRoot();
}

}  // namespace json
