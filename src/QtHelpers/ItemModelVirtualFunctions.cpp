#include "QtHelpers/ItemModelVirtualFunctions.h"

#include "JsonNameDefinitions.h"
#include "Views/ViewSettings.h"
#include "pluginmain.h"
#include <QIcon>
#include <QString>

S2Plugin::ItemModelVirtualFunctions::ItemModelVirtualFunctions(std::string_view typeName, uintptr_t memoryAddress, QObject* parent)
    : QAbstractItemModel(parent), mTypeName(typeName), mMemoryAddress(memoryAddress)
{
    auto config = Configuration::get();
    if (config->isEntitySubclass(mTypeName))
    {
        std::string currentType = mTypeName;
        const auto& hierarchy = config->entityClassHierarchy();
        std::vector<std::vector<VirtualFunction>*> reverseOrderFunctions;
        while (true)
        {
            if (auto it = config->mVirtualFunctions.find(currentType); it != config->mVirtualFunctions.end())
                reverseOrderFunctions.emplace_back(&it->second);

            if (currentType == JsonName::Entity)
                break;

            if (auto it = hierarchy.find(currentType); it != hierarchy.end())
                currentType = it->second;
            else
            {
                dprintf("unknown type requested in ItemModelVirtualFunctions::ItemModelVirtualFunctions() (%s)\n", currentType.c_str());
                break;
            }
        }
        for (auto it = reverseOrderFunctions.rbegin(); it != reverseOrderFunctions.rend(); ++it)
            for (auto& func : **it)
                mFunctions.emplace_back(func);

        if (!std::is_sorted(mFunctions.begin(), mFunctions.end()))
            std::sort(mFunctions.begin(), mFunctions.end());

        if (!Settings::get()->checkB(Settings::DEVELOPER_MODE))
            return;

        for (size_t index = 0; index < mFunctions.size(); ++index)
            if (mFunctions[index].index > index)
                mFunctions.emplace(mFunctions.begin() + static_cast<int>(index), index, "unknown" + std::to_string(index), "", "void", "", "undocumented");
    }
    else
    {
        auto& functions = config->virtualFunctionsOfType(mTypeName); // guaranteed to be sorted
        if (functions.empty())
            return;

        if (Settings::get()->checkB(Settings::DEVELOPER_MODE))
        {
            mFunctions.reserve(functions.back().index + 1);
            for (size_t funcIdx = 0, it = 0; it < functions.size(); ++it, ++funcIdx)
            {
                while (functions[it].index > funcIdx)
                {
                    mFunctions.emplace_back(funcIdx, "unknown" + std::to_string(funcIdx), "", "void", "", "undocumented");
                    ++funcIdx;
                }
                mFunctions.emplace_back(functions[it]);
            }
        }
        else
            mFunctions = functions;
    }
}

QVariant S2Plugin::ItemModelVirtualFunctions::data(const QModelIndex& index, int role) const
{
    const VirtualFunction& entry = mFunctions[static_cast<size_t>(index.row())];
    switch (role)
    {
        case Qt::DisplayRole:
        {
            switch (index.column())
            {
                case Column::Index:
                    return entry.index;
                case Column::Signature:
                    return QString("%1 %2::<b>%3</b>(%4)")
                        .arg(QString::fromStdString(entry.returnValue))
                        .arg(QString::fromStdString(entry.type))
                        .arg(QString::fromStdString(entry.name))
                        .arg(QString::fromStdString(entry.params));
                case Column::TableAddress:
                    return QString::asprintf("<font color='blue'><u>0x%016llX</u></font>", mMemoryAddress + (entry.index * 8));
                case Column::FunctionAddress:
                    return QString::asprintf("<font color='green'><u>0x%016llX</u></font>", Script::Memory::ReadQword(mMemoryAddress + (entry.index * 8)));
                case Column::Comment:
                    return QString::fromStdString(entry.comment);
                case Column::Offset:
                    return QString::asprintf("0x%04X", entry.index * 8);
            }
            break;
        }
        case gsRoleFunctionIndex:
        {
            return entry.index;
        }
        case gsRoleFunctionTableAddress:
        {
            return mMemoryAddress + (entry.index * 8);
        }
        case gsRoleFunctionFunctionAddress:
        {
            return Script::Memory::ReadQword(mMemoryAddress + (entry.index * 8));
        }
        case Qt::TextAlignmentRole:
        {
            switch (index.column())
            {
                case Column::Index:
                case Column::Signature:
                case Column::Comment:
                    return Qt::AlignLeft;

                case Column::FunctionAddress:
                case Column::Offset:
                case Column::TableAddress:
                    return Qt::AlignCenter;
            }
            break;
        }
        case Qt::ToolTipRole:
        {
            if (Settings::get()->checkB(Settings::COMMENTS_AS_TOOLTIP))
                return QString::fromStdString(entry.comment);

            break;
        }
        case Qt::DecorationRole:
        {
            if (Settings::get()->checkB(Settings::COMMENTS_AS_TOOLTIP) && index.column() == ItemModelVirtualFunctions::Index && !entry.comment.empty())
                return QIcon(":/icons/hint.png");

            break;
        }
    }
    return QVariant();
}

int S2Plugin::ItemModelVirtualFunctions::columnCount([[maybe_unused]] const QModelIndex& parent) const
{
    if (Settings::get()->checkB(Settings::COMMENTS_AS_TOOLTIP))
        return 5;

    return 6;
}

QVariant S2Plugin::ItemModelVirtualFunctions::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Orientation::Horizontal && role == Qt::DisplayRole)
    {
        switch (section)
        {
            case Column::Index:
                return "#";
            case Column::Offset:
                return "Offset";
            case Column::Signature:
                return "Signature";
            case Column::TableAddress:
                return "Table Address";
            case Column::FunctionAddress:
                return "Function Address";
            case Column::Comment:
                return "Comment";
        }
    }
    return QVariant();
}
