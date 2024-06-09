#pragma once

#include "pluginmain.h"
#include <cstdint>
#include <utility>

namespace S2Plugin
{
    // For the proper use you need to use template to define the types
    // otherwise the functions ::find ::at ::contains ::node::key ::node::value, auto for loop, won't work properly
    // when not using template, you need to define the size of key and alignments with the correct constructor
    // this can also be used as std::set, just set the value alignment to 0
    // it's not the safes implementation but the one that was needed
    namespace
    {
        struct _EmptyType
        {
        };
    } // namespace

    template <class Key = _EmptyType, class Value = _EmptyType>
    struct StdMap
    {
        // only for the template
        template <typename K = Key, typename V = Value>
        StdMap(uintptr_t addr, std::enable_if_t<!std::is_same_v<K, _EmptyType> || !std::is_same_v<V, _EmptyType>, int> = 0)
        {
            uintptr_t data[2];
            Script::Memory::Read(addr, &data, sizeof(data), nullptr);
            head = Node{data[0]};
            mSize = data[1];
            head.set_offsets();
        };

        // value size only needed for value() function
        StdMap(uintptr_t addr, uint8_t keyAlignment, uint8_t valueAlignment, size_t keySize)
        {
            uintptr_t data[2];
            Script::Memory::Read(addr, &data, sizeof(data), nullptr);
            head = Node{data[0]};
            mSize = data[1];
            head.set_offsets(keySize, keyAlignment, valueAlignment);
        };

        struct Node
        {
            Node(const Node& t) : node_ptr(t.node_ptr), key_offset(t.key_offset), value_offset(t.value_offset){};
            Key key() const
            {
                Key tmp{};
                Script::Memory::Read(key_ptr(), &tmp, sizeof(Key), nullptr);
                return tmp;
            }
            Value value() const
            {
                Value tmp{};
                Script::Memory::Read(value_ptr(), &tmp, sizeof(Value), nullptr);
                return tmp;
            }
            uintptr_t key_ptr() const
            {
                return node_ptr + key_offset;
            }
            uintptr_t value_ptr() const
            {
                return node_ptr + value_offset;
            }
            Node left() const
            {
                auto left_addr = Script::Memory::ReadQword(node_ptr);
                Node copy = *this;
                copy.node_ptr = left_addr;
                return copy;
            }
            Node parent() const
            {
                auto parent_addr = Script::Memory::ReadQword(node_ptr + 0x8);
                Node copy = *this;
                copy.node_ptr = parent_addr;
                return copy;
            }
            Node right() const
            {
                auto right_addr = Script::Memory::ReadQword(node_ptr + 0x10);
                Node copy = *this;
                copy.node_ptr = right_addr;
                return copy;
            }
            bool color() const
            {
                return (bool)Script::Memory::ReadByte(node_ptr + 0x18);
            }
            bool is_nil() const
            {
                return (bool)Script::Memory::ReadByte(node_ptr + 0x19);
            }
            // returning value ptr instead of value itself since it's more usefull for us
            std::pair<Key, uintptr_t> operator*()
            {
                return {key(), value_ptr()};
            }
            Node operator++()
            {
                if (is_nil())
                {
                    // should probably throw
                    return *this;
                }
                // get right node and check if it's valid
                Node r = right();
                if (!r.is_nil())
                {
                    // get the most left node from here
                    Node l = r.left();
                    while (!l.is_nil())
                    {
                        r = l;
                        l = r.left();
                    }

                    *this = r;
                    return *this;
                }
                // save the node for faster comparison
                // this only makes sense here as we can just compare the address
                // instead of using Read function for the nil flag
                // normally you would just check the nil flag
                Node _end = r;
                // if the right node is nil, we go up
                // if the parrent node it's nil, then that's the end
                Node p = parent();
                Node c = *this;
                while (p != _end && p.right() == c)
                {
                    // if we came from the right side, we need to go up again
                    c = p;
                    p = p.parent();
                }
                *this = p;
                return *this;
            }
            Node operator++(int)
            {
                Node old = *this;
                operator++();
                return old;
            }
            // not doing the -- operator for now as it's not really needed

            bool operator==(Node other) const noexcept
            {
                return other.node_ptr == node_ptr;
            }
            bool operator!=(Node other) const noexcept
            {
                return other.node_ptr != node_ptr;
            }

          private:
            Node() = default;
            Node(size_t addr) : node_ptr(addr){};
            void set_offsets(size_t keytype_size = sizeof(Key), uint8_t key_alignment = alignof(Key), uint8_t value_alignment = alignof(Value)) noexcept
            {
                // key and value in map are treated as std::pair
                // we need to figure out if it's placed right after the bucket flags
                // or if there is a padding added for alignment
                // the issue is, if key or value are a structs, we need to know their alignments, not just their size

                uint8_t alignment = std::max(key_alignment, value_alignment);

                switch (alignment)
                {
                    case 0:
                    case 1:
                    case 2:
                        key_offset = 0x1A; // 3 pointers and 2 bool field
                        break;
                    case 3:
                    case 4:
                        key_offset = 0x1C;
                        break;
                    case 5:
                    case 6:
                    case 7:
                    case 8:
                    default:
                        key_offset = 0x20;
                        break;
                }
                size_t offset = key_offset + keytype_size;
                // dealing with the padding between key and value
                switch (value_alignment)
                {
                    case 0:
                    case 1:
                        value_offset = offset;
                        break;
                    case 2:
                        value_offset = (offset + 1) & ~1;
                        break;
                    case 3:
                    case 4:
                        value_offset = (offset + 3) & ~3;
                        break;
                    case 5:
                    case 6:
                    case 7:
                    case 8:
                    default:
                        value_offset = (offset + 7) & ~7;
                        break;
                }
            }
            uintptr_t node_ptr{0};
            size_t key_offset{0};
            size_t value_offset{0};
            friend struct StdMap;
        };

        size_t size() const noexcept
        {
            return mSize;
        }
        size_t at(Key v) const
        {
            Node f = find(v);
            if (f == end())
            {
                // throw std::out_of_range
            }
            return f.value();
        }
        Node begin() const
        {
            return head.left();
        }
        Node end() const noexcept
        {
            return head;
        }
        Node find(Key k) const
        {
            Node _end = end();
            Node cur = root();
            while (true)
            {
                if (cur == _end)
                    return _end;

                Key node_key = cur.key();

                if (node_key == k)
                    return cur;
                else if (node_key > k)
                    cur = cur.left();
                else
                    cur = cur.right();
            }
            return _end;
        }
        bool contains(Key k) const
        {
            Node f = find(v);
            if (f == end())
                return false;
            else
                return true;
        }

      private:
        Node root() const
        {
            return end().parent();
        }
        Node last() const
        {
            return end().right();
        }
        Node first() const
        {
            return end().left();
        }

        Node head;
        size_t mSize;
    };
} // namespace S2Plugin
