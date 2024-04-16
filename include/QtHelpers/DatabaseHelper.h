#pragma once

#include "Configuration.h"
#include <QComboBox>
#include <QString>
#include <QWidget>
#include <cstdint>
#include <vector>

namespace S2Plugin
{
    namespace DB
    {
        size_t populateComparisonCombobox(QComboBox* CompareFieldComboBox, const std::vector<MemoryField>& fields, size_t offset = 0, std::string prefix = "");
        std::pair<QString, QVariant> valueForField(const QVariant& data, uintptr_t addr);
    } // namespace DB
} // namespace S2Plugin
