#include "QtHelpers/ItemModelVirtualFunctions.h"
#include "Configuration.h"
#include "pluginmain.h"

QVariant S2Plugin::ItemModelVirtualFunctions::data(const QModelIndex& index, int role) const
{
    const VirtualFunction entry = Configuration::get()->virtualFunctionsOfType(mTypeName).at(index.row());
    switch (role)
    {
        case Qt::DisplayRole:
        {
            switch (index.column())
            {
                case gsColFunctionIndex:
                    return entry.index;
                case gsColFunctionSignature:
                    return QString("%1 %2::<b>%3</b>(%4)")
                        .arg(QString::fromStdString(entry.returnValue))
                        .arg(QString::fromStdString(entry.type))
                        .arg(QString::fromStdString(entry.name))
                        .arg(QString::fromStdString(entry.params));
                case gsColFunctionTableAddress:
                    return QString::asprintf("<font color='blue'><u>0x%016llX</u></font>", mMemoryAddress + (entry.index * 8));
                case gsColFunctionFunctionAddress:
                    return QString::asprintf("<font color='green'><u>0x%016llX</u></font>", Script::Memory::ReadQword(mMemoryAddress + (entry.index * 8)));
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
    }
    return QVariant();
}

int S2Plugin::ItemModelVirtualFunctions::rowCount(const QModelIndex&) const
{
    return static_cast<int>(Configuration::get()->virtualFunctionsOfType(mTypeName).size());
}

QVariant S2Plugin::ItemModelVirtualFunctions::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Orientation::Horizontal && role == Qt::DisplayRole)
    {
        switch (section)
        {
            case gsColFunctionIndex:
                return "Index";
            case gsColFunctionSignature:
                return "Signature";
            case gsColFunctionTableAddress:
                return "Table Address";
            case gsColFunctionFunctionAddress:
                return "Function Address";
        }
    }
    return QVariant();
}
