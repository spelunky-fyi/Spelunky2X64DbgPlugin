#pragma once

#include "pluginmain.h"
#include <cstdint>

namespace S2Plugin
{
    // this is the pre C++ 11 version standard
    class OldStdList
    {
      public:
        explicit OldStdList(uintptr_t address) : _end(address){};

        struct Node
        {
            explicit Node(uintptr_t address) : nodeAddress(address){};

            Node prev() const
            {
                return Node{Script::Memory::ReadQword(nodeAddress)};
            }
            Node next() const
            {
                return Node{Script::Memory::ReadQword(nodeAddress + sizeof(uintptr_t))};
            }
            uintptr_t value_ptr() const noexcept
            {
                return nodeAddress + 2 * sizeof(uintptr_t);
            }
            uintptr_t operator*() const noexcept
            {
                return value_ptr();
            }
            Node operator++()
            {
                nodeAddress = next().nodeAddress;
                return *this;
            }
            Node operator--(int)
            {
                auto tmp = *this;
                nodeAddress = prev().nodeAddress;
                return tmp;
            }
            Node operator++(int)
            {
                auto tmp = *this;
                nodeAddress = next().nodeAddress;
                return tmp;
            }
            Node operator--()
            {
                nodeAddress = prev().nodeAddress;
                return *this;
            }
            bool operator==(const Node& other) const noexcept
            {
                return nodeAddress == other.nodeAddress;
            }
            bool operator!=(const Node& other) const noexcept
            {
                return nodeAddress != other.nodeAddress;
            }
            uintptr_t address() const noexcept
            {
                return nodeAddress;
            }

          protected:
            uintptr_t nodeAddress;
        };

        Node begin() const
        {
            return _end.next();
        }
        Node end() const noexcept
        {
            return _end;
        }
        bool empty() const
        {
            return begin() == end();
        }
        Node cbegin() const
        {
            return begin();
        }
        Node cend() const noexcept
        {
            return end();
        }
        Node back() const
        {
            return _end.prev();
        }
        Node front() const
        {
            return _end.next();
        }
        bool isValid() const
        {
            return Script::Memory::IsValidPtr(_end.next().address()) && Script::Memory::IsValidPtr(_end.prev().address());
        }

      private:
        Node _end;
    };

    // the current standard
    class StdList
    {
      public:
        using Node = OldStdList::Node;

        StdList(uintptr_t address)
        {
            Script::Memory::Read(address, this, sizeof(StdList), nullptr);
        }
        Node begin() const
        {
            return mHead.next();
        }
        Node end() const noexcept
        {
            return mHead;
        }
        size_t size() const noexcept
        {
            return mSize;
        }
        bool empty() const noexcept
        {
            return mSize == 0;
        }
        Node cbegin() const
        {
            return begin();
        }
        Node cend() const noexcept
        {
            return end();
        }
        Node back() const
        {
            return mHead.prev();
        }
        Node front() const
        {
            return mHead.next();
        }

      private:
        Node mHead{0};
        size_t mSize;
    };
}; // namespace S2Plugin
