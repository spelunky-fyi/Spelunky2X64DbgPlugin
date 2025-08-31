#pragma once

#include <QString>
#include <string_view>

inline QString QStrFromStringView(std::string_view sv)
{
    return QString::fromUtf8(sv.data(), static_cast<int>(sv.size()));
}
