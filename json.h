#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>
#include <fstream>

namespace json
{
    using namespace std::string_literals;

    class Node;

    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;

    class ParsingError : public std::runtime_error
    {
    public:

        using runtime_error::runtime_error;
    };

    class Node final : private std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>
    {
    public:

        using variant::variant;
        using Value = variant;

        Node() = default;
        Node(std::nullptr_t);
        Node(Array array_);
        Node(Dict dict_);
        Node(bool value_);
        Node(int value_);
        Node(double value_);
        Node(std::string value_);

        const Array& AsArray() const;
        const Dict& AsDict() const;
        int AsInt() const;
        double AsDouble() const;
        bool AsBool() const;
        const std::string& AsString() const;

        bool IsNull() const;
        bool IsInt() const;
        bool IsDouble() const;
        bool IsPureDouble() const;
        bool IsBool() const;
        bool IsString() const;
        bool IsArray() const;
        bool IsDict() const;

        const Value& GetValue() const;

    private:

        Value value;
    };

    inline bool operator==(const Node& lhs_, const Node& rhs_)
    {
        return lhs_.GetValue() == rhs_.GetValue();
    }
    inline bool operator!=(const Node& lhs_, const Node& rhs_)
    {
        return !(lhs_ == rhs_);
    }

    class Document
    {
    public:

        Document() = default;
        explicit Document(Node root_);

        const Node& GetRoot() const;

    private:

        Node root;
    };

    inline bool operator==(const Document& lhs_, const Document& rhs_)
    {
        return lhs_.GetRoot() == rhs_.GetRoot();
    }
    inline bool operator!=(const Document& lhs_, const Document& rhs_)
    {
        return !(lhs_ == rhs_);
    }

    Document Load(std::istream& input_);
    void PrintDocument(const Document& doc_, std::ostream& output_);

} // end namespace json