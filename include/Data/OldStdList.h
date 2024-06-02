#pragma once

#include "pluginmain.h"
#include <list>

namespace S2Plugin
{
    // this is the pre C++ 11 version standard
    class OldStdList
    {
      public:
        OldStdList(uintptr_t address) : _end(address){};

        struct Node
        {
            Node(uintptr_t address) : nodeAddress(address){};

            Node prev() const
            {
                return Script::Memory::ReadQword(nodeAddress);
            }
            Node next() const
            {
                return Script::Memory::ReadQword(nodeAddress + sizeof(uintptr_t));
            }
            uintptr_t value_ptr() const
            {
                return nodeAddress + 2 * sizeof(uintptr_t);
            }
            uintptr_t operator*() const
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
            bool operator==(const Node& other) const
            {
                return nodeAddress == other.nodeAddress;
            }
            bool operator!=(const Node& other) const
            {
                return nodeAddress != other.nodeAddress;
            }

          private:
            uintptr_t nodeAddress;
        };

        Node begin() const
        {
            return _end.next();
        }
        Node end() const
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
        Node cend() const
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

      private:
        Node _end;
    };
}; // namespace S2Plugin
