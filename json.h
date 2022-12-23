#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>

namespace json {

class Node;
// Сохраните объявления Dict и Array без изменения
using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;

// Эта ошибка должна выбрасываться при ошибках парсинга JSON
class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

class Node {
public:
   /* Реализуйте Node, используя std::variant */
    using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;

    const Value& GetValue() const { return value_;}

    Node() = default;
    Node(Array array);
    Node(Dict map);
    Node(int value);
    Node(std::string value);
    Node(bool value);
    Node(std::nullptr_t value);
    Node(double value);
    
    bool IsInt() const;
    bool IsDouble() const;
    bool IsPureDouble() const;
    bool IsBool() const;
    bool IsString() const;
    bool IsNull() const;
    bool IsArray() const;
    bool IsMap() const;
    
    int AsInt() const;
    bool AsBool() const;
    double AsDouble() const;
    const std::string& AsString() const;
    const Array& AsArray() const;
    const Dict& AsMap() const;
    
private:
    Value value_;
};


class Document {
public:
    explicit Document(Node root);

    const Node& GetRoot() const;

private:
    Node root_;
};

Document Load(std::istream& input);

void Print(const Document& doc, std::ostream& output);
void PrintNode(const Node& node, std::ostream& out);

void PrintValue(std::nullptr_t, std::ostream& out);
void PrintValue(bool bool_value, std::ostream& out);
void PrintValue(std::string word, std::ostream& out);
void PrintValue(int value, std::ostream& out);
void PrintValue(double value, std::ostream& out);
void PrintValue(const Array& array, std::ostream& out);
void PrintValue(const Dict& dict, std::ostream& out);

bool operator==(const Node& lhs, const Node& rhs);
bool operator!=(const Node& lhs, const Node& rhs);
bool operator==(const Document &lhs, const Document &rhs);
bool operator!=(const Document &lhs, const Document &rhs);

}  // namespace json
