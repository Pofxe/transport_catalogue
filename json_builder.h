#pragma once

#include "json.h"

#include <string>
#include <memory>

namespace json
{
    class KeyContext;
    class DictionaryContext;
    class ArrayContext;

    class Builder
    {
    public:

        Node MakeNode(const Node::Value& value_);
        void AddNode(const Node& node_);

        KeyContext Key(const std::string& key_);
        Builder& Value(const Node::Value& value_);

        DictionaryContext StartDict();
        Builder& EndDict();

        ArrayContext StartArray();
        Builder& EndArray();

        Node Build();

    private:

        Node root{ nullptr };
        std::vector<std::unique_ptr<Node>> nodes_stack;

    };

    class BaseContext
    {
    public:

        BaseContext(Builder& builder_);

        KeyContext Key(const std::string& key_);
        Builder& Value(const Node::Value& value_);

        DictionaryContext StartDict();
        Builder& EndDict();

        ArrayContext StartArray();
        Builder& EndArray();

    protected:

        Builder& builder;
    };

    class KeyContext : public BaseContext
    {
    public:

        KeyContext(Builder& builder_);

        KeyContext Key(const std::string& key_) = delete;
        DictionaryContext Value(const Node::Value& value_);

        BaseContext EndDict() = delete;

        BaseContext EndArray() = delete;
    };

    class DictionaryContext : public BaseContext
    {
    public:

        DictionaryContext(Builder& builder_);

        Builder& Value(const Node::Value& value_) = delete;

        DictionaryContext StartDict() = delete;

        ArrayContext StartArray() = delete;
        Builder& EndArray() = delete;
    };

    class ArrayContext : public BaseContext
    {
    public:

        ArrayContext(Builder& builder_);

        KeyContext Key(const std::string& key_) = delete;
        ArrayContext Value(const Node::Value& value_);

        Builder& EndDict() = delete;
    };

} // end namespace json
