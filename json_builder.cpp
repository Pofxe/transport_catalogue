#include "json_builder.h"

namespace json
{
    //----------------------------------------- BaseContext --------------------------------------------------------------

    BaseContext::BaseContext(Builder& builder_) : builder(builder_) {}

    KeyContext BaseContext::Key(const std::string& key_)
    {
        return builder.Key(key_);
    }

    Builder& BaseContext::Value(const Node::Value& value_)
    {
        return builder.Value(value_);
    }

    DictionaryContext BaseContext::StartDict()
    {
        return DictionaryContext(builder.StartDict());
    }

    Builder& BaseContext::EndDict()
    {
        return builder.EndDict();
    }

    ArrayContext BaseContext::StartArray()
    {
        return ArrayContext(builder.StartArray());
    }

    Builder& BaseContext::EndArray()
    {
        return builder.EndArray();
    }

    //------------------------------------------- KeyContext ---------------------------------------------------------

    KeyContext::KeyContext(Builder& builder_) : BaseContext(builder_) {}

    DictionaryContext KeyContext::Value(const Node::Value& value_)
    {
        return BaseContext::Value(value_);
    }

    //----------------------------------------- DictionaryContext --------------------------------------------------------

    DictionaryContext::DictionaryContext(Builder& builder_) : BaseContext(builder_) {}

    //--------------------------------------------- ArrayContext ---------------------------------------------------------

    ArrayContext::ArrayContext(Builder& builder) : BaseContext(builder) {}

    ArrayContext ArrayContext::Value(const Node::Value& value_) { return BaseContext::Value(value_); }

    //----------------------------------------------- Builder ------------------------------------------------------------

    Node Builder::MakeNode(const Node::Value& value_)
    {
        Node node;

        if (std::holds_alternative<bool>(value_))
        {
            bool bol = std::get<bool>(value_);
            node = Node(bol);
        }
        else if (std::holds_alternative<int>(value_))
        {
            int intt = std::get<int>(value_);
            node = Node(intt);
        }
        else if (std::holds_alternative<double>(value_))
        {
            double doble = std::get<double>(value_);
            node = Node(doble);
        }
        else if (std::holds_alternative<std::string>(value_))
        {
            std::string str = std::get<std::string>(value_);
            node = Node(std::move(str));
        }
        else if (std::holds_alternative<Array>(value_))
        {
            Array arr = std::get<Array>(value_);
            node = Node(std::move(arr));
        }
        else if (std::holds_alternative<Dict>(value_))
        {
            Dict dictionary = std::get<Dict>(value_);
            node = Node(std::move(dictionary));
        }
        else
        {
            node = Node();
        }
        return node;
    }

    void Builder::AddNode(const Node& node_)
    {
        if (nodes_stack.empty())
        {
            if (!root.IsNull())
            {
                throw std::logic_error("root has been added"s);
            }
            root = node_;
            return;
        }
        else
        {
            if (!nodes_stack.back()->IsArray() && !nodes_stack.back()->IsString())
            {
                throw std::logic_error("unable to create node"s);
            }

            if (nodes_stack.back()->IsArray())
            {
                Array arr = nodes_stack.back()->AsArray();
                arr.emplace_back(node_);

                nodes_stack.pop_back();
                std::unique_ptr<json::Node> arr_ptr = std::make_unique<Node>(arr);
                nodes_stack.emplace_back(std::move(arr_ptr));

                return;
            }

            if (nodes_stack.back()->IsString())
            {
                std::string str = nodes_stack.back()->AsString();
                nodes_stack.pop_back();

                if (nodes_stack.back()->IsDict())
                {
                    Dict dictionary = nodes_stack.back()->AsDict();
                    dictionary.emplace(std::move(str), node_);

                    nodes_stack.pop_back();
                    std::unique_ptr<json::Node> dictionary_ptr = std::make_unique<Node>(dictionary);
                    nodes_stack.emplace_back(std::move(dictionary_ptr));
                }
                return;
            }
        }
    }

    KeyContext Builder::Key(const std::string& key_)
    {
        if (nodes_stack.empty())
        {
            throw std::logic_error("unable to create key"s);
        }

        std::unique_ptr<json::Node> key_ptr = std::make_unique<Node>(key_);

        if (nodes_stack.back()->IsDict())
        {
            nodes_stack.emplace_back(std::move(key_ptr));
        }
        return KeyContext(*this);
    }

    Builder& Builder::Value(const Node::Value& value_)
    {
        AddNode(MakeNode(value_));

        return *this;
    }

    DictionaryContext Builder::StartDict()
    {
        nodes_stack.emplace_back(std::move(std::make_unique<Node>(Dict())));

        return DictionaryContext(*this);
    }

    Builder& Builder::EndDict()
    {
        if (nodes_stack.empty())
        {
            throw std::logic_error("unable to close as without opening"s);
        }

        Node node = *nodes_stack.back();

        if (!node.IsDict())
        {
            throw std::logic_error("object isn't dictionary"s);
        }

        nodes_stack.pop_back();
        AddNode(node);

        return *this;
    }

    ArrayContext Builder::StartArray()
    {
        nodes_stack.emplace_back(std::move(std::make_unique<Node>(Array())));

        return ArrayContext(*this);
    }

    Builder& Builder::EndArray()
    {
        if (nodes_stack.empty())
        {
            throw std::logic_error("unable to close without opening"s);
        }

        Node node = *nodes_stack.back();

        if (!node.IsArray())
        {
            throw std::logic_error("object isn't array"s);
        }

        nodes_stack.pop_back();
        AddNode(node);

        return *this;
    }

    Node Builder::Build()
    {
        if (root.IsNull())
        {
            throw std::logic_error("empty json"s);
        }
        if (!nodes_stack.empty())
        {
            throw std::logic_error("invalid json"s);
        }
        return root;
    }
} // end namespace json