#include "json.h"

namespace json
{
    using namespace std::string_literals;

    namespace
    {
        Node LoadNode(std::istream& input_);

        std::string LoadLiteral(std::istream& input_)
        {
            std::string str;

            while (std::isalpha(input_.peek()))
            {
                str.push_back(static_cast<char>(input_.get()));
            }
            return str;
        }

        Node LoadArray(std::istream& input_)
        {
            std::vector<Node> array;

            for (char ch; input_ >> ch && ch != ']';)
            {
                if (ch != ',')
                {
                    input_.putback(ch);
                }
                array.push_back(LoadNode(input_));
            }
            if (!input_)
            {
                throw ParsingError("unable to parse array"s);
            }
            return Node(array);
        }

        Node LoadNull(std::istream& input_)
        {
            if (std::string literal = LoadLiteral(input_); literal == "null"s)
            {
                return Node(nullptr);
            }
            else
            {
                throw ParsingError("unable to parse '"s + literal + "' as null"s);
            }
        }

        Node LoadBool(std::istream& input_)
        {
            const std::string str = LoadLiteral(input_);

            if (str == "true"s)
            {
                return Node(true);
            }
            else if (str == "false"s)
            {
                return Node(false);
            }
            else
            {
                throw ParsingError("unable to parse '"s + str + "' as bool"s);
            }
        }

        Node LoadNumber(std::istream& input_)
        {
            std::string number;

            auto read_char = [&number, &input_]
                {
                    number += static_cast<char>(input_.get());

                    if (!input_)
                    {
                        throw ParsingError("unable to read number"s);
                    }
                };

            auto read_digits = [&input_, read_char]
                {
                    if (!std::isdigit(input_.peek()))
                    {
                        throw ParsingError("digit expected"s);
                    }
                    else
                    {
                        while (std::isdigit(input_.peek()))
                        {
                            read_char();
                        }
                    }
                };

            if (input_.peek() == '-')
            {
                read_char();
            }
            if (input_.peek() == '0')
            {
                read_char();
            }
            else
            {
                read_digits();
            }

            bool is_int = true;

            if (input_.peek() == '.')
            {
                read_char();
                read_digits();
                is_int = false;
            }
            if (int ch = input_.peek(); ch == 'e' || ch == 'E')
            {
                read_char();

                if (ch = input_.peek(); ch == '+' || ch == '-')
                {
                    read_char();
                }

                read_digits();
                is_int = false;
            }
            try
            {
                if (is_int)
                {
                    try
                    {
                        return Node(std::stoi(number));

                    }
                    catch (...) {}
                }
                return Node(std::stod(number));

            }
            catch (...)
            {
                throw ParsingError("unable to convert "s + number + " to number"s);
            }
        }

        Node LoadString(std::istream& input_)
        {
            auto it = std::istreambuf_iterator<char>(input_);
            auto end = std::istreambuf_iterator<char>();
            std::string str;

            while (true)
            {
                if (it == end)
                {
                    throw ParsingError("unable to parse string"s);
                }

                const char ch = *it;
                if (ch == '"')
                {
                    ++it;
                    break;
                }
                else if (ch == '\\')
                {
                    ++it;
                    if (it == end)
                    {
                        throw ParsingError("unable to parse string"s);
                    }

                    const char esc_ch = *(it);

                    switch (esc_ch)
                    {
                    case 'n':
                        str.push_back('\n');
                        break;
                    case 't':
                        str.push_back('\t');
                        break;
                    case 'r':
                        str.push_back('\r');
                        break;
                    case '"':
                        str.push_back('"');
                        break;
                    case '\\':
                        str.push_back('\\');
                        break;
                    default:
                        throw ParsingError("invalid esc \\"s + esc_ch);
                    }

                }
                else if (ch == '\n' || ch == '\r')
                {
                    throw ParsingError("invalid line end"s);
                }
                else
                {
                    str.push_back(ch);
                }
                ++it;
            }
            return Node(str);
        }

        Node LoadDictionary(std::istream& input_)
        {
            Dict dictionary;

            for (char ch; input_ >> ch && ch != '}';)
            {

                if (ch == '"')
                {
                    std::string key = LoadString(input_).AsString();

                    if (input_ >> ch && ch == ':')
                    {
                        if (dictionary.find(key) != dictionary.end())
                        {
                            throw ParsingError("duplicate key '"s + key + "'found"s);
                        }

                        dictionary.emplace(std::move(key), LoadNode(input_));
                    }
                    else
                    {
                        throw ParsingError(": expected. but '"s + ch + "' found"s);
                    }

                }
                else if (ch != ',')
                {
                    throw ParsingError("',' expected. but '"s + ch + "' found"s);
                }
            }

            if (!input_)
            {
                throw ParsingError("unable to parse dictionary"s);
            }
            else
            {
                return Node(dictionary);
            }
        }

        Node LoadNode(std::istream& input_)
        {
            char ch;

            if (!(input_ >> ch))
            {
                throw ParsingError(""s);
            }
            else
            {
                switch (ch)
                {
                case '[':
                {
                    return LoadArray(input_);
                }
                case '{':
                {
                    return LoadDictionary(input_);
                }
                case '"':
                {
                    return LoadString(input_);
                }
                case 't':  case 'f':
                {
                    input_.putback(ch);
                    return LoadBool(input_);
                }
                case 'n':
                {
                    input_.putback(ch);
                    return LoadNull(input_);
                }
                default:
                {
                    input_.putback(ch);
                    return LoadNumber(input_);
                }
                }
            }
        }

    } // end namespace

    Node::Node(std::nullptr_t) : Node() {}
    Node::Node(Array array_) : value(std::move(array_)) {}
    Node::Node(Dict dict_) : value(std::move(dict_)) {}
    Node::Node(bool value_) : value(value_) {}
    Node::Node(int value_) : value(value_) {}
    Node::Node(double value_) : value(value_) {}
    Node::Node(std::string value_) : value(std::move(value_)) {}

    const Array& Node::AsArray() const
    {
        if (!IsArray())
        {
            throw std::logic_error("value is not an array"s);
        }
        else
        {
            return std::get<Array>(value);
        }
    }

    const Dict& Node::AsDict() const
    {
        if (!IsDict())
        {
            throw std::logic_error("value is not a dictionary"s);
        }
        else
        {
            return std::get<Dict>(value);
        }
    }

    const std::string& Node::AsString() const
    {
        if (!IsString())
        {
            throw std::logic_error("value is not a string"s);
        }
        else
        {
            return std::get<std::string>(value);
        }
    }

    int Node::AsInt() const
    {
        if (!IsInt())
        {
            throw std::logic_error("value is not an int"s);
        }
        else
        {
            return std::get<int>(value);
        }
    }

    double Node::AsDouble() const
    {
        if (!IsDouble())
        {
            throw std::logic_error("value is not a double"s);
        }
        else if (IsPureDouble())
        {
            return std::get<double>(value);
        }
        else
        {
            return AsInt();
        }
    }

    bool Node::AsBool() const
    {
        if (!IsBool())
        {
            throw std::logic_error("value is not a bool"s);
        }
        else
        {
            return std::get<bool>(value);
        }
    }

    bool Node::IsNull() const
    {
        return std::holds_alternative<std::nullptr_t>(value);
    }
    bool Node::IsInt() const
    {
        return std::holds_alternative<int>(value);
    }
    bool Node::IsDouble() const
    {
        return IsPureDouble() || IsInt();
    }
    bool Node::IsPureDouble() const
    {
        return std::holds_alternative<double>(value);
    }
    bool Node::IsBool() const
    {
        return std::holds_alternative<bool>(value);
    }
    bool Node::IsString() const
    {
        return std::holds_alternative<std::string>(value);
    }
    bool Node::IsArray() const
    {
        return std::holds_alternative<Array>(value);
    }
    bool Node::IsDict() const
    {
        return std::holds_alternative<Dict>(value);
    }

    const Node::Value& Node::GetValue() const
    {
        return value;
    }

    Document::Document(Node root) : root(std::move(root)) {}
    const Node& Document::GetRoot() const
    {
        return root;
    }

    Document Load(std::istream& input_)
    {
        return Document(LoadNode(input_));
    }

    namespace
    {
        struct PrintInfoContext
        {
            std::ostream& out;
            int indent_step = 4;
            int indent = 0;

            void PrintIndent() const
            {
                for (int i = 0; i < indent; ++i)
                {
                    out.put(' ');
                }
            }

            PrintInfoContext Indented() const
            {
                return { out, indent_step, indent_step + indent };
            }
        };

        void PrintNode(const Node& node_, const PrintInfoContext& context_);

        void PrintString(const std::string& value_, std::ostream& out_)
        {
            out_.put('"');

            for (const char ch : value_)
            {
                switch (ch)
                {
                case '\r':
                {
                    out_ << R"(\r)";
                    break;
                }
                case '\n':
                {
                    out_ << R"(\n)";
                    break;
                }
                case '"':
                {
                    out_ << R"(\")";
                    break;
                }
                case '\\':
                {
                    out_ << R"(\\)";
                    break;
                }
                default:
                {
                    out_.put(ch);
                    break;
                }
                }
            }
            out_.put('"');
        }

        template <typename Value>
        void PrintValue(const Value& value_, const PrintInfoContext& context_)
        {
            context_.out << value_;
        }

        template <>
        void PrintValue<std::string>(const std::string& value_, const PrintInfoContext& context_)
        {
            PrintString(value_, context_.out);
        }

        void PrintValue(const std::nullptr_t&, const PrintInfoContext& context_)
        {
            context_.out << "null"s;
        }

        void PrintValue(bool value_, const PrintInfoContext& context_)
        {
            context_.out << std::boolalpha << value_;
        }

        [[maybe_unused]] void PrintValue(Array nodes_, const PrintInfoContext& context_)
        {
            std::ostream& out = context_.out;
            out << "[\n"s;
            bool first = true;
            auto inner_context = context_.Indented();

            for (const Node& node : nodes_)
            {
                if (first)
                {
                    first = false;
                }
                else
                {
                    out << ",\n"s;
                }
                inner_context.PrintIndent();
                PrintNode(node, inner_context);
            }
            out.put('\n');
            context_.PrintIndent();
            out.put(']');
        }

        void PrintValue(Dict nodes_, const PrintInfoContext& context_)
        {
            std::ostream& out = context_.out;
            out << "{\n"s;
            bool first = true;
            auto inner_context = context_.Indented();

            for (const auto& [key, node] : nodes_)
            {
                if (first)
                {
                    first = false;
                }
                else
                {
                    out << ",\n"s;
                }
                inner_context.PrintIndent();
                PrintString(key, context_.out);
                out << ": "s;
                PrintNode(node, inner_context);
            }
            out.put('\n');
            context_.PrintIndent();
            out.put('}');
        }

        void PrintNode(const Node& node_, const PrintInfoContext& context_)
        {
            std::visit([&context_](const auto& value) { PrintValue(value, context_); }, node_.GetValue());
        }
    } // end namespace

    void PrintDocument(const Document& document_, std::ostream& output_)
    {
        PrintNode(document_.GetRoot(), PrintInfoContext{ output_ });
    }

} // end namespace json