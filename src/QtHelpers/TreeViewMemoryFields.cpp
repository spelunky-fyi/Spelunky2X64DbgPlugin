#include "QtHelpers/TreeViewMemoryFields.h"

#include "Configuration.h"
#include "Data/CharacterDB.h"
#include "Data/Entity.h"
#include "Data/EntityDB.h"
#include "Data/ParticleDB.h"
#include "Data/State.h"
#include "Data/StdString.h"
#include "Data/StringsTable.h"
#include "Data/TextureDB.h"
#include "QtHelpers/DialogEditSimpleValue.h"
#include "QtHelpers/DialogEditState.h"
#include "QtHelpers/DialogEditString.h"
#include "QtHelpers/StyledItemDelegateHTML.h"
#include "QtPlugin.h"
#include "Spelunky2.h"
#include "Views/ViewCharacterDB.h"
#include "Views/ViewEntity.h"
#include "Views/ViewEntityDB.h"
#include "Views/ViewParticleDB.h"
#include "Views/ViewTextureDB.h"
#include "Views/ViewToolbar.h"
#include "make_unsigned_integer.h"
#include "pluginmain.h"
#include "read_helpers.h"
#include <QDrag>
#include <QDragMoveEvent>
#include <QMimeData>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QString>
#include <QTextCodec>
#include <inttypes.h>
#include <iomanip>
#include <sstream>
#include <string>

S2Plugin::TreeViewMemoryFields::TreeViewMemoryFields(QWidget* parent) : QTreeView(parent)
{
    setItemDelegate(new StyledItemDelegateHTML(this));
    setAlternatingRowColors(true);
    mModel = new QStandardItemModel(this);
    setModel(mModel);

    setDragDropMode(QAbstractItemView::DragDropMode::DragDrop);
    setDragEnabled(true);
    setAcceptDrops(true);

    setStyleSheet("QTreeView::branch:has-siblings:!adjoins-item {\
    border-image: url(:/images/vline.png) 0;\
}\
QTreeView::branch:has-siblings:adjoins-item {\
    border-image: url(:/images/branch-more.png) 0;\
}\
QTreeView::branch:!has-children:!has-siblings:adjoins-item {\
    border-image: url(:/images/branch-end.png) 0;\
}\
QTreeView::branch:has-children:!has-siblings:closed,\
QTreeView::branch:closed:has-children:has-siblings {\
        border-image: none;\
        image: url(:/images/branch-closed.png);\
}\
QTreeView::branch:open:has-children:!has-siblings,\
QTreeView::branch:open:has-children:has-siblings  {\
        border-image: none;\
        image: url(:/images/branch-open.png);\
}");

    QObject::connect(this, &QTreeView::clicked, this, &TreeViewMemoryFields::cellClicked);
}

void S2Plugin::TreeViewMemoryFields::addMemoryFields(const std::vector<MemoryField>& fields, const std::string& mainName, uintptr_t structAddr, size_t initialDelta, uint8_t deltaPrefixCount,
                                                     QStandardItem* parent)
{
    size_t currentOffset = structAddr;
    size_t currentDelta = initialDelta;

    for (auto& field : fields)
    {
        addMemoryField(field, mainName + "." + field.name, currentOffset, currentDelta, deltaPrefixCount, parent);
        auto size = field.get_size();
        currentDelta += size;
        if (structAddr != 0)
            currentOffset += size;
    }
}

QStandardItem* S2Plugin::TreeViewMemoryFields::addMemoryField(const MemoryField& field, const std::string& fieldNameOverride, uintptr_t memoryAddress, size_t delta, uint8_t deltaPrefixCount,
                                                              QStandardItem* parent)
{
    auto createAndInsertItem = [&delta, &deltaPrefixCount](const MemoryField& field, const std::string& fieldNameUID, QStandardItem* itemParent, uintptr_t memAddr,
                                                           bool showDelta = true) -> QStandardItem*
    {
        auto itemFieldName = new QStandardItem();
        itemFieldName->setEditable(false);
        itemFieldName->setData(QString::fromStdString(field.name), Qt::DisplayRole);
        itemFieldName->setData(QString::fromStdString(fieldNameUID), gsRoleUID);
        itemFieldName->setData(QVariant::fromValue(field.type), gsRoleType);
        itemFieldName->setData(field.isPointer, gsRoleIsPointer);
        itemFieldName->setData(memAddr, gsRoleMemoryAddress);

        auto itemFieldValue = new QStandardItem();
        itemFieldValue->setEditable(false);
        if (field.isPointer == false) // if it's pointer, we set it on first update
            itemFieldValue->setData(memAddr, gsRoleMemoryAddress);

        auto itemFieldValueHex = new QStandardItem();
        itemFieldValueHex->setEditable(false);

        auto itemFieldComparisonValue = new QStandardItem();
        itemFieldComparisonValue->setEditable(false);

        auto itemFieldComparisonValueHex = new QStandardItem();
        itemFieldComparisonValueHex->setEditable(false);

        auto itemFieldMemoryOffset = new QStandardItem();
        itemFieldMemoryOffset->setEditable(false);
        if (memAddr != 0)
        {
            itemFieldMemoryOffset->setData(QString::asprintf("<font color='blue'><u>0x%016llX</u></font>", memAddr), Qt::DisplayRole);
            // for click event. we could just use the itemFieldName(gsRoleMemoryAddress), but doing it this way for potential itemComparisonFieldMemoryOffset in the future
            itemFieldMemoryOffset->setData(memAddr, gsRoleRawValue);
        }

        auto itemFieldMemoryOffsetDelta = new QStandardItem();
        itemFieldMemoryOffsetDelta->setEditable(false);
        if (showDelta) // this should only ever be false for flag field
        {
            QString text;
            if (deltaPrefixCount > 0)
            {
                text = QString(deltaPrefixCount - 1, QChar(0x2502)); // '│'
                text += QChar(0x2514);                               // '└'
                text += QChar(0x2192);                               // '→'
            }
            text += QString::asprintf("+0x%llX", delta);
            itemFieldMemoryOffsetDelta->setData(text, Qt::DisplayRole);
            itemFieldMemoryOffsetDelta->setData(delta, gsRoleRawValue);
        }

        auto itemFieldComment = new QStandardItem();
        itemFieldComment->setEditable(false);
        itemFieldComment->setData(QString::fromStdString(field.comment).toHtmlEscaped(), Qt::DisplayRole);

        auto itemFieldType = new QStandardItem();
        itemFieldType->setEditable(false);

        QString typeName = field.isPointer ? "<b>P</b>: " : ""; // add color?
        if (field.type == MemoryFieldType::EntitySubclass || field.type == MemoryFieldType::DefaultStructType)
            typeName += QString::fromStdString(field.jsonName);
        else if (auto str = Configuration::getTypeDisplayName(field.type); !str.empty())
            typeName += QString::fromUtf8(str.data(), static_cast<int>(str.size()));
        else
            typeName += "Unknown field type";

        itemFieldType->setData(typeName, Qt::DisplayRole);

        itemParent->appendRow(QList<QStandardItem*>() << itemFieldName << itemFieldValue << itemFieldValueHex << itemFieldComparisonValue << itemFieldComparisonValueHex << itemFieldMemoryOffset
                                                      << itemFieldMemoryOffsetDelta << itemFieldType << itemFieldComment);

        return itemFieldName;
    };

    if (parent == nullptr)
        parent = mModel->invisibleRootItem();

    uint8_t flags = 0;
    QStandardItem* returnField = nullptr;
    switch (field.type)
    {
        case MemoryFieldType::Skip:
        {
            // TODO: add skip when set in settings
            // save offset with gsRoleSize
            break;
        }
        case MemoryFieldType::CodePointer:
        case MemoryFieldType::DataPointer:
        case MemoryFieldType::Byte:
        case MemoryFieldType::UnsignedByte:
        case MemoryFieldType::Word:
        case MemoryFieldType::UnsignedWord:
        case MemoryFieldType::Dword:
        case MemoryFieldType::UnsignedDword:
        case MemoryFieldType::Qword:
        case MemoryFieldType::UnsignedQword:
        case MemoryFieldType::Float:
        case MemoryFieldType::Double:
        case MemoryFieldType::Bool:
        case MemoryFieldType::StringsTableID:
        case MemoryFieldType::ParticleDBID:
        case MemoryFieldType::EntityDBID:
        case MemoryFieldType::EntityUID:
        case MemoryFieldType::EntityPointer:
        case MemoryFieldType::EntityDBPointer:
        case MemoryFieldType::TextureDBPointer:
        case MemoryFieldType::TextureDBID:
        case MemoryFieldType::CharacterDBID:
        case MemoryFieldType::LevelGenPointer:
        case MemoryFieldType::ParticleDBPointer:
        case MemoryFieldType::ConstCharPointerPointer:
        case MemoryFieldType::ConstCharPointer:
        case MemoryFieldType::LevelGenRoomsPointer:
        case MemoryFieldType::LevelGenRoomsMetaPointer:
        case MemoryFieldType::JournalPagePointer:
        case MemoryFieldType::ThemeInfoPointer:
        case MemoryFieldType::UTF16Char:
        case MemoryFieldType::IPv4Address:
        {
            returnField = createAndInsertItem(field, fieldNameOverride, parent, memoryAddress);
            break;
        }
        case MemoryFieldType::UTF16StringFixedSize:
        case MemoryFieldType::UTF8StringFixedSize:
        {
            returnField = createAndInsertItem(field, fieldNameOverride, parent, memoryAddress);
            returnField->setData(field.get_size(), gsRoleSize);
            break;
        }
        case MemoryFieldType::State8:
        case MemoryFieldType::State16:
        case MemoryFieldType::State32:
        {
            returnField = createAndInsertItem(field, fieldNameOverride, parent, memoryAddress);
            returnField->setData(QVariant::fromValue(field.firstParameterType), gsRoleRefName);
            break;
        }
        case MemoryFieldType::VirtualFunctionTable:
        {
            returnField = createAndInsertItem(field, fieldNameOverride, parent, memoryAddress);
            returnField->setData(QVariant::fromValue(field.firstParameterType), gsRoleRefName);
            break;
        }
        case MemoryFieldType::Flags32:
            flags = 32;
            [[fallthrough]];
        case MemoryFieldType::Flags16:
            if (flags == 0)
                flags = 16;
            [[fallthrough]];
        case MemoryFieldType::Flags8:
        {
            if (flags == 0)
                flags = 8;

            auto flagsParent = createAndInsertItem(field, fieldNameOverride, parent, memoryAddress);
            for (uint8_t x = 1; x <= flags; ++x)
            {
                MemoryField flagField;
                flagField.name = "flag_" + std::to_string(x);
                flagField.type = MemoryFieldType::Flag;
                bool showDelta = false;
                if ((x - 1) % 8 == 0)
                {
                    delta += x == 1 ? 0 : 1;
                    showDelta = true;
                }
                auto flagFieldItem = createAndInsertItem(flagField, fieldNameOverride + "." + flagField.name, flagsParent, 0, showDelta);
                flagFieldItem->setData(x, gsRoleFlagIndex);
                auto flagName = Configuration::get()->flagTitle(field.firstParameterType, x);
                QString realFlagName = QString::fromStdString(flagName.empty() ? Configuration::get()->flagTitle("unknown", x) : flagName); // TODO: don't show unknown unless it was chosen in settings

                flagsParent->child(x - 1, gsColValue)->setData(realFlagName, Qt::DisplayRole);
            }
            returnField = flagsParent;
            break;
        }
        case MemoryFieldType::UndeterminedThemeInfoPointer:
        {
            returnField = createAndInsertItem(field, fieldNameOverride, parent, memoryAddress);
            addMemoryFields(Configuration::get()->typeFieldsOfDefaultStruct("ThemeInfoPointer"), fieldNameOverride, 0, 0, deltaPrefixCount + 1, returnField);
            break;
        }
        case MemoryFieldType::StdVector:
        {
            returnField = createAndInsertItem(field, fieldNameOverride, parent, memoryAddress);
            returnField->setData(QVariant::fromValue(field.firstParameterType), gsRoleStdContainerFirstParameterType);
            if (field.isPointer)
                addMemoryFields(Configuration::get()->typeFields(field.type), fieldNameOverride, 0, 0, deltaPrefixCount + 1, returnField);
            else
                addMemoryFields(Configuration::get()->typeFields(field.type), fieldNameOverride, memoryAddress, delta, deltaPrefixCount, returnField);

            break;
        }
        case MemoryFieldType::StdMap:
        {
            returnField = createAndInsertItem(field, fieldNameOverride, parent, memoryAddress);
            returnField->setData(QVariant::fromValue(field.firstParameterType), gsRoleStdContainerFirstParameterType);
            returnField->setData(QVariant::fromValue(field.secondParameterType), gsRoleStdContainerSecondParameterType);
            if (field.isPointer)
                addMemoryFields(Configuration::get()->typeFields(field.type), fieldNameOverride, 0, 0, deltaPrefixCount + 1, returnField);
            else
                addMemoryFields(Configuration::get()->typeFields(field.type), fieldNameOverride, memoryAddress, delta, deltaPrefixCount, returnField);

            break;
        }
        case MemoryFieldType::EntitySubclass:
        {
            returnField = createAndInsertItem(field, fieldNameOverride, parent, 0);
            returnField->setData(memoryAddress, gsRoleMemoryAddress);
            addMemoryFields(Configuration::get()->typeFieldsOfEntitySubclass(field.jsonName), fieldNameOverride, memoryAddress, delta, deltaPrefixCount, returnField);
            break;
        }
        case MemoryFieldType::DefaultStructType:
        {
            returnField = createAndInsertItem(field, fieldNameOverride, parent, memoryAddress);
            if (field.isPointer)
                addMemoryFields(Configuration::get()->typeFieldsOfDefaultStruct(field.jsonName), fieldNameOverride, 0, 0, deltaPrefixCount + 1, returnField);
            else
                addMemoryFields(Configuration::get()->typeFieldsOfDefaultStruct(field.jsonName), fieldNameOverride, memoryAddress, delta, deltaPrefixCount, returnField);

            break;
        }
        default:
        {
            returnField = createAndInsertItem(field, fieldNameOverride, parent, memoryAddress);
            if (field.isPointer)
                addMemoryFields(Configuration::get()->typeFields(field.type), fieldNameOverride, 0, 0, deltaPrefixCount + 1, returnField);
            else
                addMemoryFields(Configuration::get()->typeFields(field.type), fieldNameOverride, memoryAddress, delta, deltaPrefixCount, returnField);

            break;
        }
    }
    return returnField;
}

void S2Plugin::TreeViewMemoryFields::updateTableHeader(bool restoreColumnWidths)
{
    mModel->setHorizontalHeaderLabels({"Field", "Value", "Value (hex)", "Comparison value", "Comparison value (hex)", "Memory offset", "Δ", "Type", "Comment"});

    setColumnHidden(gsColField, !activeColumns.test(gsColField));
    setColumnHidden(gsColValue, !activeColumns.test(gsColValue));
    setColumnHidden(gsColValueHex, !activeColumns.test(gsColValueHex));
    setColumnHidden(gsColComparisonValue, !activeColumns.test(gsColComparisonValue));
    setColumnHidden(gsColComparisonValueHex, !activeColumns.test(gsColComparisonValueHex));
    setColumnHidden(gsColMemoryAddress, !activeColumns.test(gsColMemoryAddress));
    setColumnHidden(gsColMemoryAddressDelta, !activeColumns.test(gsColMemoryAddressDelta));
    setColumnHidden(gsColType, !activeColumns.test(gsColType));
    setColumnHidden(gsColComment, !activeColumns.test(gsColComment));

    if (restoreColumnWidths)
    {
        if (mSavedColumnWidths[gsColField] != 0)
        {
            setColumnWidth(gsColField, mSavedColumnWidths[gsColField]);
        }
        if (mSavedColumnWidths[gsColValue] != 0)
        {
            setColumnWidth(gsColValue, mSavedColumnWidths[gsColValue]);
        }
        if (mSavedColumnWidths[gsColValueHex] != 0)
        {
            setColumnWidth(gsColValueHex, mSavedColumnWidths[gsColValueHex]);
        }
        if (mSavedColumnWidths[gsColComparisonValue] != 0)
        {
            setColumnWidth(gsColComparisonValue, mSavedColumnWidths[gsColComparisonValue]);
        }
        if (mSavedColumnWidths[gsColComparisonValueHex] != 0)
        {
            setColumnWidth(gsColComparisonValueHex, mSavedColumnWidths[gsColComparisonValueHex]);
        }
        if (mSavedColumnWidths[gsColMemoryAddress] != 0)
        {
            setColumnWidth(gsColMemoryAddress, mSavedColumnWidths[gsColMemoryAddress]);
        }
        if (mSavedColumnWidths[gsColMemoryAddressDelta] != 0)
        {
            setColumnWidth(gsColMemoryAddressDelta, mSavedColumnWidths[gsColMemoryAddressDelta]);
        }
        if (mSavedColumnWidths[gsColType] != 0)
        {
            setColumnWidth(gsColType, mSavedColumnWidths[gsColType]);
        }
        if (mSavedColumnWidths[gsColComment] != 0)
        {
            setColumnWidth(gsColComment, mSavedColumnWidths[gsColComment]);
        }
    }
}

void S2Plugin::TreeViewMemoryFields::updateTree(uintptr_t newAddr, uintptr_t newComparisonAddr, bool initial)
{
    for (int row = 0; row < mModel->invisibleRootItem()->rowCount(); ++row)
    {
        updateRow(row, newAddr == 0 ? std::nullopt : std::optional<uintptr_t>(newAddr), newComparisonAddr == 0 ? std::nullopt : std::optional<uintptr_t>(newComparisonAddr), nullptr, initial);
    }
}

// this would be much better as lambda function, but lamba with templates is C++20 thing
// hope that the compiler can inline and optimise all of this 🙏
template <typename T>
inline std::optional<T> updateField(QStandardItem* itemField, uintptr_t memoryAddress, QStandardItem* itemValue, const char* valueFormat, QStandardItem* itemValueHex, bool isPointer,
                                    const char* hexFormat, bool updateBackground, bool resetBackgroundToTransparent, const QColor& background)
{
    std::optional<T> value;
    if (memoryAddress == 0)
    {
        itemValue->setData({}, Qt::DisplayRole);
        if (!isPointer)
            itemValueHex->setData({}, Qt::DisplayRole);

        itemValue->setData({}, S2Plugin::gsRoleRawValue);
        itemField->setBackground(Qt::transparent);
    }
    else
    {
        T valueTmp = S2Plugin::Read<T>(memoryAddress);
        value = valueTmp;
        auto data = itemValue->data(S2Plugin::gsRoleRawValue);
        T valueOld = data.value<T>();
        if (!data.isValid() || value.value() != valueOld)
        {
            if (updateBackground)
                itemField->setBackground(background);

            if (valueFormat != nullptr)
            {
                itemValue->setData(QString::asprintf(valueFormat, value.value()), Qt::DisplayRole);
            }
            if (!isPointer)
            {
                using unsignedT = make_uint<T>;
                auto newHexValue = QString::asprintf(hexFormat, reinterpret_cast<unsignedT&>(value.value()));
                itemValueHex->setData(newHexValue, Qt::DisplayRole);
            }
            itemValue->setData(value.value(), S2Plugin::gsRoleRawValue);
        }
        else if (updateBackground && resetBackgroundToTransparent)
            itemField->setBackground(Qt::transparent);
    }
    return value;
}

void S2Plugin::TreeViewMemoryFields::updateRow(int row, std::optional<uintptr_t> newAddr, std::optional<uintptr_t> newAddrComparison, QStandardItem* parent, bool disableChangeHighlighting)
{
    if (parent == nullptr)
        parent = mModel->invisibleRootItem();

    QStandardItem* itemField = parent->child(row, gsColField);
    QStandardItem* itemValue = parent->child(row, gsColValue);
    QStandardItem* itemValueHex = parent->child(row, gsColValueHex);
    QStandardItem* itemComparisonValue = parent->child(row, gsColComparisonValue);
    QStandardItem* itemComparisonValueHex = parent->child(row, gsColComparisonValueHex);

    if (itemField == nullptr)
    {
        dprintf("ERROR: tried to updateRow(%d) but did not find itemField in treeview\n", row);
        return;
    }
    else if (itemValue == nullptr || itemValueHex == nullptr || itemComparisonValue == nullptr || itemComparisonValueHex == nullptr)
    {
        dprintf("ERROR: tried to updateRow(%d), field '%s', but did not find items in treeview\n", row, itemField->data(gsRoleUID).toString().toStdString().c_str());
        return;
    }

    MemoryFieldType fieldType = itemField->data(gsRoleType).value<MemoryFieldType>();
    if (fieldType == MemoryFieldType::Skip) // TODO: change when setting for it is available
        return;

    if (fieldType == MemoryFieldType::None)
    {
        dprintf("ERROR: unknown type in updateRow('%s' row: %d)\n", itemField->data(gsRoleUID).toString().toStdString().c_str(), row);
        return;
    }

    bool isPointer = itemField->data(gsRoleIsPointer).toBool();
    uintptr_t memoryOffset = 0;
    uintptr_t comparisonMemoryOffset = 0;
    const auto comparisonDifferenceColor = QColor::fromRgb(255, 221, 184);
    QColor highlightColor = (mEnableChangeHighlighting && !disableChangeHighlighting) ? QColor::fromRgb(255, 184, 184) : Qt::transparent;
    // updating memory offset
    if (newAddr.has_value() && fieldType != MemoryFieldType::Flag) // if (fieldType != MemoryFieldType::EntitySubclass) // there should never be a situation when they get the memoryoffset updated
    {
        QStandardItem* itemMemoryOffset = parent->child(row, gsColMemoryAddress);
        QStandardItem* itemMemoryOffsetDelta = parent->child(row, gsColMemoryAddressDelta);
        auto deltaData = itemMemoryOffsetDelta->data(gsRoleRawValue);
        memoryOffset = newAddr.value() == 0 ? 0 : newAddr.value() + deltaData.toULongLong();
        if (!deltaData.isNull())
        {
            itemMemoryOffset->setData(QString::asprintf("<font color='blue'><u>0x%016llX</u></font>", memoryOffset), Qt::DisplayRole);
            itemMemoryOffset->setData(memoryOffset, gsRoleRawValue);
        }
        itemField->setData(memoryOffset, gsRoleMemoryAddress);
        if (isPointer == false)
            itemValue->setData(memoryOffset, gsRoleMemoryAddress);
    }
    else
        memoryOffset = itemField->data(gsRoleMemoryAddress).toULongLong();

    // updating memory offset for comparison
    if (newAddrComparison.has_value())
    {
        QStandardItem* itemMemoryOffsetDelta = parent->child(row, gsColMemoryAddressDelta);
        comparisonMemoryOffset = newAddrComparison.value() == 0 ? 0 : newAddrComparison.value() + itemMemoryOffsetDelta->data(gsRoleRawValue).toULongLong();
        itemField->setData(comparisonMemoryOffset, gsRoleComparisonMemoryAddress);
        if (isPointer == false)
            itemComparisonValue->setData(comparisonMemoryOffset, gsRoleMemoryAddress);
    }
    else
        comparisonMemoryOffset = itemField->data(gsRoleComparisonMemoryAddress).toULongLong();

    bool pointerUpdate = false;
    bool comparisonPointerUpdate = false;
    bool comparisonActive = activeColumns.test(gsColComparisonValue) || activeColumns.test(gsColComparisonValueHex);
    uintptr_t valueMemoryOffset = memoryOffset;                     // 0, memory offset or pointer value (no bad values)
    uintptr_t valueComparisonMemoryOffset = comparisonMemoryOffset; // 0, memory offset or pointer value (no bad values)

    if (isPointer)
    {
        uintptr_t newPointer = 0;
        uintptr_t newComparisonPointer = 0;
        bool comparisonPointerDifference = false;
        // dealing with itemValueHex and itemComparisonValueHex for all pointers and check if the pointer changed
        auto checkAndUpdatePointer = [fieldType](uintptr_t& pointerValue, QStandardItem* valueHexField) -> bool
        {
            auto oldData = valueHexField->data(gsRoleRawValue);
            uintptr_t oldPointer = oldData.toULongLong();
            auto pointertmp = pointerValue;
            if (oldData.isNull() || oldPointer != pointerValue)
            {
                QString newHexValue;
                if (pointerValue == 0)
                    newHexValue = "<font color='#aaa'>nullptr</font>";
                else if (!Script::Memory::IsValidPtr(pointerValue))
                {
                    newHexValue = "<font color='#aaa'>bad ptr</font>";
                    pointerValue = 0;
                }
                else
                {
                    if (fieldType == MemoryFieldType::CodePointer)
                        newHexValue = QString::asprintf("<font color='green'><u>0x%016llX</u></font>", pointerValue);
                    else
                        newHexValue = QString::asprintf("<font color='blue'><u>0x%016llX</u></font>", pointerValue);
                }

                valueHexField->setData(newHexValue, Qt::DisplayRole);
                valueHexField->setData(pointertmp, gsRoleRawValue);
                return true;
            }
            return false;
        };
        newPointer = Script::Memory::ReadQword(memoryOffset);
        valueMemoryOffset = newPointer;
        pointerUpdate = checkAndUpdatePointer(valueMemoryOffset, itemValueHex);
        itemValue->setData(valueMemoryOffset, gsRoleMemoryAddress);

        if (pointerUpdate)
            itemField->setBackground(highlightColor);
        else
            itemField->setBackground(Qt::transparent); // can be updated later if needed

        if (comparisonActive)
        {
            newComparisonPointer = Script::Memory::ReadQword(comparisonMemoryOffset);
            valueComparisonMemoryOffset = newComparisonPointer;
            comparisonPointerUpdate = checkAndUpdatePointer(valueComparisonMemoryOffset, itemComparisonValueHex);
            itemComparisonValue->setData(valueComparisonMemoryOffset, gsRoleMemoryAddress);

            comparisonPointerDifference = newPointer != newComparisonPointer;
            itemComparisonValueHex->setBackground(comparisonPointerDifference ? comparisonDifferenceColor : Qt::transparent);
        }
    }

    auto shouldUpdateChildren = false;
    if (auto modelIndex = mModel->indexFromItem(itemField); modelIndex.isValid())
    {
        if (itemField->hasChildren())
        {
            shouldUpdateChildren = isExpanded(modelIndex);

            // always update if memory offset was changed
            if (pointerUpdate || comparisonPointerUpdate || newAddr.has_value() || newAddrComparison.has_value())
                shouldUpdateChildren = true;
        }
    }

    auto flagsString = [](uint32_t value, uint8_t size)
    {
        std::stringstream ss;
        uint8_t counter = 0;
        for (uint8_t x = size - 1;; --x)
        {
            if (counter % 4 == 0)
            {
                ss << (x + 1) << ": ";
            }
            if ((value & (1u << x)) == (1u << x))
            {
                ss << "<font color='green'>Y</font> ";
            }
            else
            {
                ss << "<font color='red'>N</font> ";
            }
            counter++;
            if (x == 0)
                break;
        }
        return ss;
    };

    switch (fieldType)
    {
        case MemoryFieldType::CodePointer:
        case MemoryFieldType::DataPointer:
        {
            if (pointerUpdate)
                itemValue->setData(itemValueHex->data(Qt::DisplayRole), Qt::DisplayRole);

            if (comparisonActive)
            {
                if (comparisonPointerUpdate)
                    itemComparisonValue->setData(itemComparisonValueHex->data(Qt::DisplayRole), Qt::DisplayRole);

                itemComparisonValue->setBackground(itemComparisonValueHex->background());
            }
            break;
        }
        case MemoryFieldType::Byte:
        {
            std::optional<int8_t> value;
            value = updateField<int8_t>(itemField, valueMemoryOffset, itemValue, "%d", itemValueHex, isPointer, "0x%02X", true, !pointerUpdate, highlightColor);

            if (comparisonActive)
            {
                std::optional<int8_t> comparisonValue;
                comparisonValue = updateField<int8_t>(itemField, valueComparisonMemoryOffset, itemComparisonValue, "%d", itemComparisonValueHex, isPointer, "0x%02X", false, false, highlightColor);

                itemComparisonValue->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
                if (isPointer == false)
                    itemComparisonValueHex->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            }
            break;
        }
        case MemoryFieldType::UnsignedByte:
        {
            std::optional<uint8_t> value;
            value = updateField<uint8_t>(itemField, valueMemoryOffset, itemValue, "%u", itemValueHex, isPointer, "0x%02X", true, !pointerUpdate, highlightColor);

            if (comparisonActive)
            {
                std::optional<uint8_t> comparisonValue;
                comparisonValue = updateField<uint8_t>(itemField, valueComparisonMemoryOffset, itemComparisonValue, "%u", itemComparisonValueHex, isPointer, "0x%02X", false, false, highlightColor);

                itemComparisonValue->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
                if (isPointer == false)
                    itemComparisonValueHex->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            }
            break;
        }
        case MemoryFieldType::Word:
        {
            std::optional<int16_t> value;
            value = updateField<int16_t>(itemField, valueMemoryOffset, itemValue, "%d", itemValueHex, isPointer, "0x%04X", true, !pointerUpdate, highlightColor);

            if (comparisonActive)
            {
                std::optional<int16_t> comparisonValue;
                comparisonValue = updateField<int16_t>(itemField, valueComparisonMemoryOffset, itemComparisonValue, "%d", itemComparisonValueHex, isPointer, "0x%04X", false, false, highlightColor);

                itemComparisonValue->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
                if (isPointer == false)
                    itemComparisonValueHex->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            }
            break;
        }
        case MemoryFieldType::UnsignedWord:
        {
            std::optional<uint16_t> value;
            value = updateField<uint16_t>(itemField, valueMemoryOffset, itemValue, "%u", itemValueHex, isPointer, "0x%04X", true, !pointerUpdate, highlightColor);

            if (comparisonActive)
            {
                std::optional<uint16_t> comparisonValue;
                comparisonValue = updateField<uint16_t>(itemField, valueComparisonMemoryOffset, itemComparisonValue, "%u", itemComparisonValueHex, isPointer, "0x%04X", false, false, highlightColor);

                itemComparisonValue->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
                if (isPointer == false)
                    itemComparisonValueHex->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            }
            break;
        }
        case MemoryFieldType::Dword:
        {
            std::optional<int32_t> value;
            value = updateField<int32_t>(itemField, valueMemoryOffset, itemValue, "%ld", itemValueHex, isPointer, "0x%08X", true, !pointerUpdate, highlightColor);

            if (comparisonActive)
            {
                std::optional<int32_t> comparisonValue;
                comparisonValue = updateField<int32_t>(itemField, valueComparisonMemoryOffset, itemComparisonValue, "%ld", itemComparisonValueHex, isPointer, "0x%08X", false, false, highlightColor);

                itemComparisonValue->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
                if (isPointer == false)
                    itemComparisonValueHex->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            }
            break;
        }
        case MemoryFieldType::UnsignedDword:
        {
            std::optional<uint32_t> value;
            value = updateField<uint32_t>(itemField, valueMemoryOffset, itemValue, "%lu", itemValueHex, isPointer, "0x%08X", true, !pointerUpdate, highlightColor);

            if (comparisonActive)
            {
                std::optional<uint32_t> comparisonValue;
                comparisonValue = updateField<uint32_t>(itemField, valueComparisonMemoryOffset, itemComparisonValue, "%lu", itemComparisonValueHex, isPointer, "0x%08X", false, false, highlightColor);

                itemComparisonValue->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
                if (isPointer == false)
                    itemComparisonValueHex->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            }
            break;
        }
        case MemoryFieldType::Qword:
        {
            std::optional<int64_t> value;
            value = updateField<int64_t>(itemField, valueMemoryOffset, itemValue, "%lld", itemValueHex, isPointer, "0x%016llX", true, !pointerUpdate, highlightColor);

            if (comparisonActive)
            {
                std::optional<int64_t> comparisonValue;
                comparisonValue =
                    updateField<int64_t>(itemField, valueComparisonMemoryOffset, itemComparisonValue, "%lld", itemComparisonValueHex, isPointer, "0x%016llX", false, false, highlightColor);

                itemComparisonValue->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
                if (isPointer == false)
                    itemComparisonValueHex->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            }
            break;
        }
        case MemoryFieldType::UnsignedQword:
        {
            std::optional<uint64_t> value;
            value = updateField<uint64_t>(itemField, valueMemoryOffset, itemValue, "%llu", itemValueHex, isPointer, "0x%016llX", true, !pointerUpdate, highlightColor);

            if (comparisonActive)
            {
                std::optional<uint64_t> comparisonValue;
                comparisonValue =
                    updateField<uint64_t>(itemField, valueComparisonMemoryOffset, itemComparisonValue, "%llu", itemComparisonValueHex, isPointer, "0x%016llX", false, false, highlightColor);

                itemComparisonValue->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
                if (isPointer == false)
                    itemComparisonValueHex->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            }
            break;
        }
        case MemoryFieldType::Float:
        {
            std::optional<uint32_t> value;
            value = updateField<uint32_t>(itemField, valueMemoryOffset, itemValue, nullptr, itemValueHex, isPointer, "0x%08X", true, !pointerUpdate, highlightColor);
            if (value.has_value())
                itemValue->setData(QString::asprintf("%f", reinterpret_cast<float&>(value.value())), Qt::DisplayRole);

            if (comparisonActive)
            {
                std::optional<uint32_t> comparisonValue;
                comparisonValue =
                    updateField<uint32_t>(itemField, valueComparisonMemoryOffset, itemComparisonValue, nullptr, itemComparisonValueHex, isPointer, "0x%08X", false, false, highlightColor);
                if (comparisonValue.has_value())
                    itemComparisonValue->setData(QString::asprintf("%f", reinterpret_cast<float&>(comparisonValue.value())), Qt::DisplayRole);

                itemComparisonValue->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
                if (isPointer == false)
                    itemComparisonValueHex->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            }
            break;
        }
        case MemoryFieldType::Double:
        {
            std::optional<size_t> value;
            value = updateField<size_t>(itemField, valueMemoryOffset, itemValue, nullptr, itemValueHex, isPointer, "0x%016llX", true, !pointerUpdate, highlightColor);
            if (value.has_value())
                itemValue->setData(QString::asprintf("%lf", reinterpret_cast<double&>(value.value())), Qt::DisplayRole);

            if (comparisonActive)
            {
                std::optional<size_t> comparisonValue;
                comparisonValue =
                    updateField<size_t>(itemField, valueComparisonMemoryOffset, itemComparisonValue, nullptr, itemComparisonValueHex, isPointer, "0x%016llX", false, false, highlightColor);
                if (comparisonValue.has_value())
                    itemComparisonValue->setData(QString::asprintf("%lf", reinterpret_cast<double&>(comparisonValue.value())), Qt::DisplayRole);

                itemComparisonValue->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
                if (isPointer == false)
                    itemComparisonValueHex->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            }
            break;
        }
        case MemoryFieldType::Bool:
        {
            std::optional<bool> value;
            value = updateField<bool>(itemField, valueMemoryOffset, itemValue, nullptr, itemValueHex, isPointer, "0x%02X", true, !pointerUpdate, highlightColor);
            if (value.has_value())
                itemValue->setData(value.value() ? "<font color='green'>True</font>" : "<font color='red'>False</font>", Qt::DisplayRole);

            if (comparisonActive)
            {
                std::optional<bool> comparisonValue;
                comparisonValue = updateField<bool>(itemField, valueComparisonMemoryOffset, itemComparisonValue, nullptr, itemComparisonValueHex, isPointer, "0x%02X", false, false, highlightColor);
                if (comparisonValue.has_value())
                    itemComparisonValue->setData(comparisonValue.value() ? "<font color='green'>True</font>" : "<font color='red'>False</font>", Qt::DisplayRole);

                itemComparisonValue->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
                if (isPointer == false)
                    itemComparisonValueHex->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            }
            break;
        }
        case MemoryFieldType::Flags32:
        {
            std::optional<uint32_t> value;
            value = updateField<uint32_t>(itemField, valueMemoryOffset, itemValue, nullptr, itemValueHex, isPointer, "0x%08X", true, !pointerUpdate, highlightColor);
            if (value.has_value())
                itemValue->setData(QString::fromStdString(flagsString(value.value(), 32).str()), Qt::DisplayRole);

            if (comparisonActive)
            {
                std::optional<uint32_t> comparisonValue;
                comparisonValue =
                    updateField<uint32_t>(itemField, valueComparisonMemoryOffset, itemComparisonValue, nullptr, itemComparisonValueHex, isPointer, "0x%08X", false, false, highlightColor);
                if (comparisonValue.has_value())
                    itemComparisonValue->setData(QString::fromStdString(flagsString(comparisonValue.value(), 32).str()), Qt::DisplayRole);

                itemComparisonValue->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
                if (isPointer == false)
                    itemComparisonValueHex->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            }
            if (shouldUpdateChildren)
            {
                for (uint8_t x = 0; x < itemField->rowCount(); ++x)
                {
                    updateRow(x, std::nullopt, std::nullopt, itemField, disableChangeHighlighting);
                }
            }
            break;
        }
        case MemoryFieldType::Flags16:
        {
            std::optional<uint16_t> value;
            value = updateField<uint16_t>(itemField, valueMemoryOffset, itemValue, nullptr, itemValueHex, isPointer, "0x%04X", true, !pointerUpdate, highlightColor);
            if (value.has_value())
                itemValue->setData(QString::fromStdString(flagsString(value.value(), 16).str()), Qt::DisplayRole);

            if (comparisonActive)
            {
                std::optional<uint16_t> comparisonValue;
                comparisonValue =
                    updateField<uint16_t>(itemField, valueComparisonMemoryOffset, itemComparisonValue, nullptr, itemComparisonValueHex, isPointer, "0x%04X", false, false, highlightColor);
                if (comparisonValue.has_value())
                    itemComparisonValue->setData(QString::fromStdString(flagsString(comparisonValue.value(), 16).str()), Qt::DisplayRole);

                itemComparisonValue->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
                if (isPointer == false)
                    itemComparisonValueHex->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            }
            if (shouldUpdateChildren)
            {
                for (uint8_t x = 0; x < itemField->rowCount(); ++x)
                {
                    updateRow(x, 0, 0, itemField, disableChangeHighlighting);
                }
            }
            break;
        }
        case MemoryFieldType::Flags8:
        {
            std::optional<uint8_t> value;
            value = updateField<uint8_t>(itemField, valueMemoryOffset, itemValue, nullptr, itemValueHex, isPointer, "0x%02X", true, !pointerUpdate, highlightColor);
            if (value.has_value())
                itemValue->setData(QString::fromStdString(flagsString(value.value(), 8).str()), Qt::DisplayRole);

            if (comparisonActive)
            {
                std::optional<uint8_t> comparisonValue;
                comparisonValue = updateField<uint8_t>(itemField, valueComparisonMemoryOffset, itemComparisonValue, nullptr, itemComparisonValueHex, isPointer, "0x%02X", false, false, highlightColor);
                if (comparisonValue.has_value())
                    itemComparisonValue->setData(QString::fromStdString(flagsString(comparisonValue.value(), 8).str()), Qt::DisplayRole);

                itemComparisonValue->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
                if (isPointer == false)
                    itemComparisonValueHex->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            }
            if (shouldUpdateChildren)
            {
                for (uint8_t x = 0; x < itemField->rowCount(); ++x)
                {
                    updateRow(x, std::nullopt, std::nullopt, itemField, disableChangeHighlighting);
                }
            }
            break;
        }
        case MemoryFieldType::Flag: // can't be pointer, always have parent
        {
            constexpr auto getDataFrom = [](const QModelIndex& idx, int col, int role) { return idx.sibling(idx.row(), col).data(role); };

            // [Known Issue]: null memory address is not handled
            const auto flagIndex = itemField->data(gsRoleFlagIndex).toUInt();
            const uint mask = (1 << (flagIndex - 1));

            auto value = getDataFrom(itemField->parent()->index(), gsColValue, gsRoleRawValue).toUInt();
            bool flagSet = (value & mask) == mask;
            if (itemValue->data(gsRoleRawValue).toBool() != flagSet)
            {
                itemValue->setData(flagSet, gsRoleRawValue);
                itemField->setBackground(highlightColor);
            }
            else
                itemField->setBackground(Qt::transparent);

            itemValue->setData(flagSet ? QColor("green") : QColor("red"), Qt::TextColorRole);

            if (comparisonActive)
            {
                auto comparisonValue = getDataFrom(itemField->parent()->index(), gsColComparisonValue, gsRoleRawValue).toUInt();
                bool comparisonFlagSet = (comparisonValue & mask) == mask;
                itemComparisonValue->setData(comparisonFlagSet ? QColor("green") : QColor("red"), Qt::TextColorRole);

                itemComparisonValue->setBackground(flagSet != comparisonFlagSet ? comparisonDifferenceColor : Qt::transparent);
                itemComparisonValueHex->setBackground(flagSet != comparisonFlagSet ? comparisonDifferenceColor : Qt::transparent);
            }
            break;
        }
        case MemoryFieldType::State8:
        {
            std::string stateRef = itemField->data(gsRoleRefName).value<std::string>();
            auto config = Configuration::get();

            std::optional<int8_t> value;
            value = updateField<int8_t>(itemField, valueMemoryOffset, itemValue, nullptr, itemValueHex, isPointer, "0x%02X", true, !pointerUpdate, highlightColor);
            if (value.has_value())
            {
                auto stateTitle = QString::fromStdString(std::to_string(value.value()) + ": " + config->stateTitle(stateRef, value.value()));
                itemValue->setData(stateTitle, Qt::DisplayRole);
            }

            if (comparisonActive)
            {
                std::optional<int8_t> comparisonValue;
                comparisonValue = updateField<int8_t>(itemField, valueComparisonMemoryOffset, itemComparisonValue, nullptr, itemComparisonValueHex, isPointer, "0x%02X", false, false, highlightColor);
                if (comparisonValue.has_value())
                {
                    auto stateTitle = QString::fromStdString(std::to_string(comparisonValue.value()) + ": " + config->stateTitle(stateRef, comparisonValue.value()));
                    itemComparisonValue->setData(stateTitle, Qt::DisplayRole);
                }

                itemComparisonValue->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
                if (isPointer == false)
                    itemComparisonValueHex->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            }
            break;
        }
        case MemoryFieldType::State16:
        {
            std::string stateRef = itemField->data(gsRoleRefName).value<std::string>();
            auto config = Configuration::get();

            std::optional<int16_t> value;
            value = updateField<int16_t>(itemField, valueMemoryOffset, itemValue, nullptr, itemValueHex, isPointer, "0x%04X", true, !pointerUpdate, highlightColor);
            if (value.has_value())
            {
                auto stateTitle = QString::fromStdString(std::to_string(value.value()) + ": " + config->stateTitle(stateRef, value.value()));
                itemValue->setData(stateTitle, Qt::DisplayRole);
            }

            if (comparisonActive)
            {
                std::optional<int16_t> comparisonValue;
                comparisonValue = updateField<int16_t>(itemField, valueComparisonMemoryOffset, itemComparisonValue, nullptr, itemComparisonValueHex, isPointer, "0x%04X", false, false, highlightColor);
                if (comparisonValue.has_value())
                {
                    auto stateTitle = QString::fromStdString(std::to_string(comparisonValue.value()) + ": " + config->stateTitle(stateRef, comparisonValue.value()));
                    itemComparisonValue->setData(stateTitle, Qt::DisplayRole);
                }

                itemComparisonValue->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
                if (isPointer == false)
                    itemComparisonValueHex->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            }
            break;
        }
        case MemoryFieldType::State32:
        {
            std::string stateRef = itemField->data(gsRoleRefName).value<std::string>();
            auto config = Configuration::get();

            std::optional<int32_t> value;
            value = updateField<int32_t>(itemField, valueMemoryOffset, itemValue, nullptr, itemValueHex, isPointer, "0x%08X", true, !pointerUpdate, highlightColor);
            if (value.has_value())
            {
                auto stateTitle = QString::fromStdString(std::to_string(value.value()) + ": " + config->stateTitle(stateRef, value.value()));
                itemValue->setData(stateTitle, Qt::DisplayRole);
            }

            if (comparisonActive)
            {
                std::optional<int32_t> comparisonValue;
                comparisonValue = updateField<int32_t>(itemField, valueComparisonMemoryOffset, itemComparisonValue, nullptr, itemComparisonValueHex, isPointer, "0x%08X", false, false, highlightColor);
                if (comparisonValue.has_value())
                {
                    auto stateTitle = QString::fromStdString(std::to_string(comparisonValue.value()) + ": " + config->stateTitle(stateRef, comparisonValue.value()));
                    itemComparisonValue->setData(stateTitle, Qt::DisplayRole);
                }

                itemComparisonValue->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
                if (isPointer == false)
                    itemComparisonValueHex->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            }
            break;
        }
        case MemoryFieldType::UTF16Char:
        {
            std::optional<uint16_t> value;
            value = updateField<uint16_t>(itemField, valueMemoryOffset, itemValue, nullptr, itemValueHex, isPointer, "0x%04X", true, !pointerUpdate, highlightColor);
            if (value.has_value())
                itemValue->setData(QString("'<b>%1</b>' (%2)").arg(QString(QChar(value.value())).toHtmlEscaped()).arg(value.value()), Qt::DisplayRole);

            if (comparisonActive)
            {
                std::optional<uint16_t> comparisonValue;
                comparisonValue =
                    updateField<uint16_t>(itemField, valueComparisonMemoryOffset, itemComparisonValue, nullptr, itemComparisonValueHex, isPointer, "0x%04X", false, false, highlightColor);
                if (comparisonValue.has_value())
                    itemComparisonValue->setData(QString("'<b>%1</b>' (%2)").arg(QString(QChar(comparisonValue.value())).toHtmlEscaped()).arg(comparisonValue.value()), Qt::DisplayRole);

                itemComparisonValue->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
                if (isPointer == false)
                    itemComparisonValueHex->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            }
            break;
        }
        case MemoryFieldType::UTF16StringFixedSize:
        {
            // size in bytes
            auto size = itemField->data(gsRoleSize).toULongLong();
            auto lenght = size / 2;
            std::optional<std::wstring> value;
            if (valueMemoryOffset == 0)
            {
                itemField->setBackground(Qt::transparent);
                itemValue->setData({}, Qt::DisplayRole);
                if (!isPointer)
                    itemValueHex->setData({}, Qt::DisplayRole);
            }
            else
            {
                value = std::wstring();
                value->resize(lenght);
                Script::Memory::Read(valueMemoryOffset, value->data(), size, nullptr);
                auto buffer_w = reinterpret_cast<const ushort*>(value->c_str());
                auto valueString = ('\"' + QString::fromUtf16(buffer_w) + '\"').toHtmlEscaped();

                auto valueOld = itemValue->data(Qt::DisplayRole); // no need for gsRoleRawValue
                if (valueOld.isNull() || valueString != valueOld.toString())
                {
                    itemField->setBackground(highlightColor);
                    itemValue->setData(valueString, Qt::DisplayRole);
                    if (!isPointer)
                    {
                        std::stringstream ss;
                        ss << "0x" << std::hex << std::setfill('0');
                        for (int i = 0; i < std::min(lenght, 10ull); ++i)
                            ss << std::setw(4) << reinterpret_cast<const uint16_t&>(buffer_w[i]);

                        itemValueHex->setData(QString::fromStdString(ss.str()), Qt::DisplayRole);
                    }
                }
                else if (!pointerUpdate)
                    itemField->setBackground(Qt::transparent);
            }
            if (comparisonActive)
            {
                std::optional<std::wstring> comparisonValue;
                if (valueComparisonMemoryOffset == 0)
                {
                    itemComparisonValue->setData({}, Qt::DisplayRole);
                    if (!isPointer)
                        itemComparisonValueHex->setData({}, Qt::DisplayRole);
                }
                else
                {
                    comparisonValue = std::wstring();
                    comparisonValue->resize(lenght);
                    Script::Memory::Read(valueComparisonMemoryOffset, comparisonValue->data(), size, nullptr);
                    auto buffer_w = reinterpret_cast<const ushort*>(comparisonValue->c_str());
                    auto valueString = ('\"' + QString::fromUtf16(buffer_w) + '\"').toHtmlEscaped();

                    auto valueOld = itemComparisonValue->data(Qt::DisplayRole); // no need for gsRoleRawValue
                    if (valueOld.isNull() || valueString != valueOld.toString())
                    {
                        itemComparisonValue->setData(valueString, Qt::DisplayRole);
                        if (!isPointer)
                        {
                            std::stringstream ss;
                            ss << "0x" << std::hex << std::setfill('0');
                            for (int i = 0; i < std::min(lenght, 10ull) && buffer_w[i] != 0; ++i)
                                ss << std::setw(4) << reinterpret_cast<const uint16_t&>(buffer_w[i]);

                            itemComparisonValueHex->setData(QString::fromStdString(ss.str()), Qt::DisplayRole);
                        }
                    }
                }
                itemComparisonValue->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
                if (isPointer == false)
                    itemComparisonValueHex->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            }
            break;
        }
        case MemoryFieldType::UTF8StringFixedSize:
        {
            auto size = itemField->data(gsRoleSize).toULongLong();
            std::optional<std::string> value;
            if (valueMemoryOffset == 0)
            {
                itemField->setBackground(Qt::transparent);
                itemValue->setData({}, Qt::DisplayRole);
                if (!isPointer)
                    itemValueHex->setData({}, Qt::DisplayRole);
            }
            else
            {
                value = std::string();
                value->resize(size);
                Script::Memory::Read(valueMemoryOffset, value->data(), size, nullptr);
                auto valueString = ('\"' + QString::fromUtf8(value->c_str()) + '\"').toHtmlEscaped(); // using c_str and not fromStdString to ignore characters after null terminator

                auto valueOld = itemValue->data(Qt::DisplayRole); // no need for gsRoleRawValue
                if (valueOld.isNull() || valueString != valueOld.toString())
                {
                    itemField->setBackground(highlightColor);
                    itemValue->setData(valueString, Qt::DisplayRole);
                    if (!isPointer)
                    {
                        std::stringstream ss;
                        ss << "0x" << std::hex << std::setfill('0');
                        for (int i = 0; i < std::min(size, 10ull); ++i)
                            ss << std::setw(2) << reinterpret_cast<uint8_t&>(value->data()[i]);

                        itemValueHex->setData(QString::fromStdString(ss.str()), Qt::DisplayRole);
                    }
                }
                else if (!pointerUpdate)
                    itemField->setBackground(Qt::transparent);
            }
            if (comparisonActive)
            {
                std::optional<std::string> comparisonValue;
                if (valueComparisonMemoryOffset == 0)
                {
                    itemComparisonValue->setData({}, Qt::DisplayRole);
                    if (!isPointer)
                        itemComparisonValueHex->setData({}, Qt::DisplayRole);
                }
                else
                {
                    comparisonValue = std::string();
                    comparisonValue->resize(size);
                    Script::Memory::Read(valueComparisonMemoryOffset, comparisonValue->data(), size, nullptr);
                    auto valueString = ('\"' + QString::fromUtf8(comparisonValue->c_str()) + '\"').toHtmlEscaped();

                    auto valueOld = itemComparisonValue->data(Qt::DisplayRole); // no need for gsRoleRawValue
                    if (valueOld.isNull() || valueString != valueOld.toString())
                    {
                        itemComparisonValue->setData(valueString, Qt::DisplayRole);
                        if (!isPointer)
                        {
                            std::stringstream ss;
                            ss << "0x" << std::hex << std::setfill('0');
                            for (int i = 0; i < std::min(size, 10ull); ++i)
                                ss << std::setw(2) << reinterpret_cast<uint8_t&>(comparisonValue->data()[i]);

                            itemComparisonValueHex->setData(QString::fromStdString(ss.str()), Qt::DisplayRole);
                        }
                    }
                }
                itemComparisonValue->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
                if (isPointer == false)
                    itemComparisonValueHex->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            }
            break;
        }
        case MemoryFieldType::EntityDBID:
        {
            std::optional<uint32_t> value;
            value = updateField<uint32_t>(itemField, valueMemoryOffset, itemValue, nullptr, itemValueHex, isPointer, "0x%08X", true, !pointerUpdate, highlightColor);
            if (value.has_value())
            {
                auto entityName = Configuration::get()->entityList().nameForID(value.value());
                if (entityName._Starts_with("UNKNOWN ID: "))
                    itemValue->setData(QString::asprintf("%u (%s)", value, entityName.c_str()), Qt::DisplayRole);
                else
                    itemValue->setData(QString::asprintf("<font color='blue'><u>%u (%s)</u></font>", value, entityName.c_str()), Qt::DisplayRole);
            }

            if (comparisonActive)
            {
                std::optional<uint32_t> comparisonValue;
                comparisonValue =
                    updateField<uint32_t>(itemField, valueComparisonMemoryOffset, itemComparisonValue, nullptr, itemComparisonValueHex, isPointer, "0x%08X", false, false, highlightColor);
                if (comparisonValue.has_value())
                {
                    auto entityName = Configuration::get()->entityList().nameForID(comparisonValue.value());
                    if (entityName._Starts_with("UNKNOWN ID: "))
                        itemComparisonValue->setData(QString::asprintf("%u (%s)", comparisonValue, entityName.c_str()), Qt::DisplayRole);
                    else
                        itemComparisonValue->setData(QString::asprintf("<font color='blue'><u>%u (%s)</u></font>", comparisonValue, entityName.c_str()), Qt::DisplayRole);
                }

                itemComparisonValue->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
                if (isPointer == false)
                    itemComparisonValueHex->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            }
            break;
        }
        case MemoryFieldType::TextureDBID:
        {
            std::optional<int32_t> value;
            value = updateField<int32_t>(itemField, valueMemoryOffset, itemValue, nullptr, itemValueHex, isPointer, "0x%08X", true, !pointerUpdate, highlightColor);
            if (value.has_value())
            {
                if (value.value() < 0)
                {
                    // TODO: write better explanation
                    itemValue->setData(QString::asprintf("<font color='blue'><u>%d (dynamically applied in ThemeInfo->get_dynamic_floor_texture_id())</u></font>", value.value()), Qt::DisplayRole);
                }
                else
                {
                    itemValue->setData(QString::asprintf("<font color='blue'><u>%d (%s)</u></font>", value.value(), Spelunky2::get()->get_TextureDB().nameForID(value.value()).c_str()),
                                       Qt::DisplayRole);
                }
            }
            if (comparisonActive)
            {
                std::optional<int32_t> comparisonValue;
                comparisonValue = updateField<int32_t>(itemField, valueComparisonMemoryOffset, itemComparisonValue, nullptr, itemComparisonValueHex, isPointer, "0x%08X", false, false, highlightColor);
                if (comparisonValue.has_value())
                {
                    if (comparisonValue.value() < 0)
                    {
                        itemComparisonValue->setData(
                            QString::asprintf("<font color='blue'><u>%d (dynamically applied in ThemeInfo->get_dynamic_floor_texture_id())</u></font>", comparisonValue.value()), Qt::DisplayRole);
                    }
                    else
                    {
                        itemComparisonValue->setData(
                            QString::asprintf("<font color='blue'><u>%d (%s)</u></font>", comparisonValue.value(), Spelunky2::get()->get_TextureDB().nameForID(comparisonValue.value()).c_str()),
                            Qt::DisplayRole);
                    }
                }

                itemComparisonValue->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
                if (isPointer == false)
                    itemComparisonValueHex->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            }
            break;
        }
        case MemoryFieldType::StringsTableID:
        {
            std::optional<uint32_t> value;
            value = updateField<uint32_t>(itemField, valueMemoryOffset, itemValue, nullptr, itemValueHex, isPointer, "0x%08X", true, !pointerUpdate, highlightColor);
            if (value.has_value())
                itemValue->setData(QString("%1: %2").arg(value.value()).arg(Spelunky2::get()->get_StringsTable().stringForIndex(value.value())).toHtmlEscaped(), Qt::DisplayRole);

            if (comparisonActive)
            {
                std::optional<uint32_t> comparisonValue;
                comparisonValue =
                    updateField<uint32_t>(itemField, valueComparisonMemoryOffset, itemComparisonValue, nullptr, itemComparisonValueHex, isPointer, "0x%08X", false, false, highlightColor);
                if (comparisonValue.has_value())
                {
                    itemComparisonValue->setData(QString("%1: %2").arg(comparisonValue.value()).arg(Spelunky2::get()->get_StringsTable().stringForIndex(comparisonValue.value())).toHtmlEscaped(),
                                                 Qt::DisplayRole);
                }

                itemComparisonValue->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
                if (isPointer == false)
                    itemComparisonValueHex->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            }
            break;
        }
        case MemoryFieldType::ParticleDBID:
        {
            std::optional<uint32_t> value;
            value = updateField<uint32_t>(itemField, valueMemoryOffset, itemValue, nullptr, itemValueHex, isPointer, "0x%08X", true, !pointerUpdate, highlightColor);
            if (value.has_value())
            {
                std::string particleName = Configuration::get()->particleEmittersList().nameForID(value.value());
                itemValue->setData(QString::asprintf("<font color='blue'><u>%u (%s)</u></font>", value.value(), particleName.c_str()), Qt::DisplayRole);
            }

            if (comparisonActive)
            {
                std::optional<uint32_t> comparisonValue;
                comparisonValue =
                    updateField<uint32_t>(itemField, valueComparisonMemoryOffset, itemComparisonValue, nullptr, itemComparisonValueHex, isPointer, "0x%08X", false, false, highlightColor);
                if (comparisonValue.has_value())
                {
                    std::string particleName = Configuration::get()->particleEmittersList().nameForID(comparisonValue.value());
                    itemComparisonValue->setData(QString::asprintf("<font color='blue'><u>%u (%s)</u></font>", comparisonValue.value(), particleName.c_str()), Qt::DisplayRole);
                }

                itemComparisonValue->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
                if (isPointer == false)
                    itemComparisonValueHex->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            }
            break;
        }
        case MemoryFieldType::EntityUID:
        {
            std::optional<int32_t> value;
            value = updateField<int32_t>(itemField, valueMemoryOffset, itemValue, nullptr, itemValueHex, isPointer, "0x%08X", true, !pointerUpdate, highlightColor);
            if (value.has_value())
            {
                if (value.value() < 0)
                {
                    itemValue->setData(QString::asprintf("(%d) Nothing", value.value()), Qt::DisplayRole);
                    itemValue->setData(0, gsRoleEntityAddress);
                }
                else
                {
                    uintptr_t entityOffset = S2Plugin::State{Spelunky2::get()->get_StatePtr()}.findEntitybyUID(value.value());
                    if (entityOffset != 0)
                    {
                        auto entityName = Configuration::get()->getEntityName(Entity{entityOffset}.entityTypeID());
                        itemValue->setData(QString::asprintf("<font color='blue'><u>UID %u (%s)</u></font>", value.value(), entityName.c_str()), Qt::DisplayRole);
                        itemValue->setData(entityOffset, gsRoleEntityAddress);
                    }
                    else
                    {
                        itemValue->setData(QString::asprintf("(%d) UNKNOWN ENTITY", value.value()), Qt::DisplayRole);
                        itemValue->setData(0, gsRoleEntityAddress);
                    }
                }
            }

            if (comparisonActive)
            {
                std::optional<int32_t> comparisonValue;
                comparisonValue = updateField<int32_t>(itemField, valueComparisonMemoryOffset, itemComparisonValue, nullptr, itemComparisonValueHex, isPointer, "0x%08X", false, false, highlightColor);
                if (comparisonValue.has_value())
                {
                    if (comparisonValue.value() < 0)
                    {
                        itemComparisonValue->setData(QString::asprintf("(%d) Nothing", comparisonValue.value()), Qt::DisplayRole);
                        itemComparisonValue->setData(0, gsRoleEntityAddress);
                    }
                    else
                    {
                        uintptr_t comparisonEntityOffset = S2Plugin::State{Spelunky2::get()->get_StatePtr()}.findEntitybyUID(comparisonValue.value());
                        if (comparisonEntityOffset != 0)
                        {
                            auto entityName = Configuration::get()->getEntityName(Entity{comparisonEntityOffset}.entityTypeID());
                            itemComparisonValue->setData(QString::asprintf("<font color='blue'><u>UID %u (%s)</u></font>", comparisonValue.value(), entityName.c_str()), Qt::DisplayRole);
                            itemComparisonValue->setData(comparisonEntityOffset, gsRoleEntityAddress);
                        }
                        else
                        {
                            itemComparisonValue->setData(QString::asprintf("(%d) UNKNOWN ENTITY", comparisonValue.value()), Qt::DisplayRole);
                            itemComparisonValue->setData(0, gsRoleEntityAddress);
                        }
                    }
                }

                itemComparisonValue->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
                if (isPointer == false)
                    itemComparisonValueHex->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            }
            break;
        }
        case MemoryFieldType::EntityPointer:
        {
            if (valueMemoryOffset == 0) // nullptr or bad ptr
                itemValue->setData(itemValueHex->data(Qt::DisplayRole), Qt::DisplayRole);
            else
            {
                // TODO: unknown entity?
                auto entityName = Configuration::get()->getEntityName(Entity{valueMemoryOffset}.entityTypeID());
                itemValue->setData(QString::asprintf("<font color='blue'><u>%s</u></font>", entityName.c_str()), Qt::DisplayRole);
                itemValue->setData(valueMemoryOffset, gsRoleRawValue); // set to 0/clear when unknown entity?
            }
            if (comparisonActive)
            {
                if (valueComparisonMemoryOffset == 0)
                    itemComparisonValue->setData(itemComparisonValueHex->data(Qt::DisplayRole));
                else
                {
                    auto comparisonEntityName = Configuration::get()->getEntityName(Entity{valueComparisonMemoryOffset}.entityTypeID());
                    itemComparisonValue->setData(QString::asprintf("<font color='blue'><u>%s</u></font>", comparisonEntityName.c_str()), Qt::DisplayRole);
                    itemComparisonValue->setData(valueComparisonMemoryOffset, gsRoleRawValue); // set to 0/clear when unknown entity?
                }
                itemComparisonValue->setBackground(itemComparisonValueHex->background());
            }
            break;
        }
        case MemoryFieldType::EntityDBPointer:
        {
            if (valueMemoryOffset == 0) // nullptr or bad ptr
                itemValue->setData(itemValueHex->data(Qt::DisplayRole), Qt::DisplayRole);
            else
            {
                auto id = Script::Memory::ReadDword(valueMemoryOffset + 0x14);
                auto entityName = Configuration::get()->entityList().nameForID(id);
                itemValue->setData(QString::asprintf("<font color='blue'><u>EntityDB %d %s</u></font>", id, entityName.c_str()), Qt::DisplayRole);
            }
            if (comparisonActive)
            {
                if (valueComparisonMemoryOffset == 0)
                    itemComparisonValue->setData(itemComparisonValueHex->data(Qt::DisplayRole));
                else
                {
                    auto comparisonID = Script::Memory::ReadDword(valueComparisonMemoryOffset + 20);
                    auto comparisonEntityName = Configuration::get()->entityList().nameForID(comparisonID);
                    itemComparisonValue->setData(QString::asprintf("<font color='blue'><u>EntityDB %d %s</u></font>", comparisonID, comparisonEntityName.c_str()), Qt::DisplayRole);
                }
                itemComparisonValue->setBackground(itemComparisonValueHex->background());
            }
            break;
        }
        case MemoryFieldType::TextureDBPointer:
        {
            if (valueMemoryOffset == 0) // nullptr or bad ptr
                itemValue->setData(itemValueHex->data(Qt::DisplayRole), Qt::DisplayRole);
            else
            {
                auto id = Script::Memory::ReadQword(valueMemoryOffset);
                auto& textureName = Spelunky2::get()->get_TextureDB().nameForID(id);
                itemValue->setData(QString::asprintf("<font color='blue'><u>TextureDB %d %s</u></font>", id, textureName.c_str()), Qt::DisplayRole);
            }
            if (comparisonActive)
            {
                if (valueComparisonMemoryOffset == 0)
                    itemComparisonValue->setData(itemComparisonValueHex->data(Qt::DisplayRole));
                else
                {
                    auto comparisonID = Script::Memory::ReadQword(valueComparisonMemoryOffset);
                    auto& comparisonTextureName = Spelunky2::get()->get_TextureDB().nameForID(comparisonID);
                    itemComparisonValue->setData(QString::asprintf("<font color='blue'><u>TextureDB %d %s</u></font>", comparisonID, comparisonTextureName.c_str()), Qt::DisplayRole);
                }
                itemComparisonValue->setBackground(itemComparisonValueHex->background());
            }
            break;
        }
        case MemoryFieldType::LevelGenPointer:
        {
            if (valueMemoryOffset == 0) // nullptr or bad ptr
                itemValue->setData(itemValueHex->data(Qt::DisplayRole), Qt::DisplayRole);
            else
                itemValue->setData("<font color='blue'><u>Show level gen</u></font>", Qt::DisplayRole);

            if (comparisonActive)
            {
                if (valueComparisonMemoryOffset == 0)
                    itemComparisonValue->setData(itemComparisonValueHex->data(Qt::DisplayRole));
                else
                    itemComparisonValue->setData("<font color='blue'><u>Show level gen</u></font>", Qt::DisplayRole);

                itemComparisonValue->setBackground(itemComparisonValueHex->background());
            }
            break;
        }
        case MemoryFieldType::ParticleDBPointer:
        {
            if (valueMemoryOffset == 0) // nullptr or bad ptr
                itemValue->setData(itemValueHex->data(Qt::DisplayRole), Qt::DisplayRole);
            else
            {
                auto id = Script::Memory::ReadDword(valueMemoryOffset);
                auto particleName = Configuration::get()->particleEmittersList().nameForID(id);
                itemValue->setData(QString::asprintf("<font color='blue'><u>ParticleDB %d %s</u></font>", id, particleName.c_str()), Qt::DisplayRole);
            }
            if (comparisonActive)
            {
                if (valueComparisonMemoryOffset == 0)
                    itemComparisonValue->setData(itemComparisonValueHex->data(Qt::DisplayRole));
                else
                {
                    auto comparisonID = Script::Memory::ReadQword(valueComparisonMemoryOffset);
                    auto comparisonParticleName = Configuration::get()->particleEmittersList().nameForID(comparisonID);
                    itemComparisonValue->setData(QString::asprintf("<font color='blue'><u>ParticleDB %d %s</u></font>", comparisonID, comparisonParticleName.c_str()), Qt::DisplayRole);
                }
                itemComparisonValue->setBackground(itemComparisonValueHex->background());
            }
            break;
        }
        case MemoryFieldType::VirtualFunctionTable:
        {
            if (valueMemoryOffset == 0) // nullptr or bad ptr
                itemValue->setData(itemValueHex->data(Qt::DisplayRole), Qt::DisplayRole);
            else
                itemValue->setData("<font color='blue'><u>Show functions</u></font>", Qt::DisplayRole);

            if (comparisonActive)
            {
                if (valueComparisonMemoryOffset == 0)
                    itemComparisonValue->setData(itemComparisonValueHex->data(Qt::DisplayRole));
                else
                    itemComparisonValue->setData("<font color='blue'><u>Show functions</u></font>", Qt::DisplayRole);

                itemComparisonValue->setBackground(itemComparisonValueHex->background());
            }
            break;
        }
        case MemoryFieldType::CharacterDBID:
        {
            std::optional<uint32_t> value;
            value = updateField<uint32_t>(itemField, valueMemoryOffset, itemValue, nullptr, itemValueHex, isPointer, "0x%08X", true, !pointerUpdate, highlightColor);
            if (value.has_value())
            {
                auto& characterDB = Spelunky2::get()->get_CharacterDB();
                bool isValidCharacter = value.value() < characterDB.charactersCount();
                auto& characterName = isValidCharacter ? Spelunky2::get()->get_CharacterDB().characterNamesStringList().at(value.value()) : "";
                itemValue->setData(QString("<font color='blue'><u>%1 (%2)</u></font>").arg(value.value()).arg(characterName), Qt::DisplayRole);
            }

            if (comparisonActive)
            {
                std::optional<uint32_t> comparisonValue;
                comparisonValue =
                    updateField<uint32_t>(itemField, valueComparisonMemoryOffset, itemComparisonValue, nullptr, itemComparisonValueHex, isPointer, "0x%08X", false, false, highlightColor);
                if (comparisonValue.has_value())
                {
                    auto& characterDB = Spelunky2::get()->get_CharacterDB();
                    bool isValidCharacter = comparisonValue.value() < characterDB.charactersCount();
                    auto& characterName = isValidCharacter ? Spelunky2::get()->get_CharacterDB().characterNamesStringList().at(comparisonValue.value()) : "";
                    itemComparisonValue->setData(QString("<font color='blue'><u>%1 (%2)</u></font>").arg(comparisonValue.value()).arg(characterName), Qt::DisplayRole);
                }

                itemComparisonValue->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
                if (isPointer == false)
                    itemComparisonValueHex->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            }
            break;
        }
        case MemoryFieldType::ConstCharPointerPointer:
        {
            // TODO: probably delete? it's actually a struct not just a pointer?
            if (valueMemoryOffset != 0)
                valueMemoryOffset = Script::Memory::ReadQword(valueMemoryOffset);

            if (valueComparisonMemoryOffset != 0)
                valueComparisonMemoryOffset = Script::Memory::ReadQword(valueComparisonMemoryOffset);

            [[fallthrough]];
        }
        case MemoryFieldType::ConstCharPointer:
        {
            if (valueMemoryOffset == 0)
            {
                itemValue->setData(itemValueHex->data(Qt::DisplayRole), Qt::DisplayRole);
            }
            else
            {
                std::string str = '\"' + ReadConstString(valueMemoryOffset) + '\"';
                itemValue->setData(QString::fromStdString(str).toHtmlEscaped(), Qt::DisplayRole);
            }

            if (comparisonActive)
            {
                if (valueComparisonMemoryOffset == 0)
                {
                    itemComparisonValue->setData(itemComparisonValueHex->data(Qt::DisplayRole), Qt::DisplayRole);
                }
                else
                {
                    std::string comparisonStr = '\"' + ReadConstString(valueComparisonMemoryOffset) + '\"';
                    itemComparisonValue->setData(QString::fromStdString(comparisonStr).toHtmlEscaped(), Qt::DisplayRole);
                }
                // pointer compare
                itemComparisonValue->setBackground(itemComparisonValueHex->background());
            }
            break;
        }
        case MemoryFieldType::StdString:
        {
            std::optional<std::string> stringValue;
            if (valueMemoryOffset == 0)
            {
                itemField->setBackground(Qt::transparent);
                itemValue->setData({}, Qt::DisplayRole);
                if (!isPointer)
                    itemValueHex->setData({}, Qt::DisplayRole);
            }
            else
            {
                StdString string{valueMemoryOffset};
                // [Known Issue]: i don't think we will have pointer to std::string, but note just in case: this would override the pointer value in hex
                auto ptr = string.string_ptr();
                itemValueHex->setData(QString::asprintf("<font color='blue'><u>0x%016llX</u></font>", ptr), Qt::DisplayRole);
                itemValueHex->setData(ptr, gsRoleRawValue);

                stringValue = '\"' + string.get_string() + '\"';
                auto displayValue = QString::fromStdString(stringValue.value()).toHtmlEscaped();
                itemField->setBackground(itemValue->data(Qt::DisplayRole).toString() == displayValue ? Qt::transparent : highlightColor);
                itemValue->setData(displayValue, Qt::DisplayRole);
            }

            if (comparisonActive)
            {
                std::optional<std::string> comparisonStringValue;
                if (valueComparisonMemoryOffset == 0)
                {
                    itemComparisonValue->setData({}, Qt::DisplayRole);
                    if (!isPointer)
                        itemComparisonValueHex->setData({}, Qt::DisplayRole);
                }
                else
                {
                    StdString comparisonString{valueComparisonMemoryOffset};
                    comparisonStringValue = '\"' + comparisonString.get_string() + '\"';
                    itemComparisonValue->setData(QString::fromStdString(comparisonStringValue.value()).toHtmlEscaped(), Qt::DisplayRole);

                    auto ptr = comparisonString.string_ptr();
                    itemComparisonValueHex->setData(QString::asprintf("<font color='blue'><u>0x%016llX</u></font>", ptr), Qt::DisplayRole);
                    itemComparisonValueHex->setData(ptr, gsRoleRawValue);
                }
                bool compare = stringValue != comparisonStringValue;
                itemComparisonValue->setBackground(compare ? comparisonDifferenceColor : Qt::transparent);
                // for the hex should probably compare the actual value displayed, but those will never be the same unless it's the exact same string object
                // if(!isPointer)
                itemComparisonValueHex->setBackground(compare ? comparisonDifferenceColor : Qt::transparent);
            }
            if (shouldUpdateChildren)
            {
                std::optional<uintptr_t> addr = pointerUpdate ? valueMemoryOffset : (isPointer ? std::nullopt : newAddr);
                std::optional<uintptr_t> comparisonAddr = comparisonPointerUpdate ? valueComparisonMemoryOffset : (isPointer ? std::nullopt : newAddrComparison);
                for (uint8_t x = 0; x < itemField->rowCount(); ++x)
                    updateRow(x, addr, comparisonAddr, itemField);
            }
            break;
        }
        case MemoryFieldType::StdWstring:
        {
            constexpr ushort quotationMark = static_cast<ushort>(u'\"');
            std::optional<std::basic_string<ushort>> stringValue;
            std::optional<std::basic_string<ushort>> comparisonStringValue;
            if (valueMemoryOffset == 0)
            {
                itemField->setBackground(Qt::transparent);
                itemValue->setData({}, Qt::DisplayRole);
                if (!isPointer)
                    itemValueHex->setData({}, Qt::DisplayRole);
            }
            else
            {
                StdWstring string{valueMemoryOffset};
                // i don't think we will have pointer to std::string, but note just in case: this would override the pointer value in hex
                auto ptr = string.string_ptr();
                itemValueHex->setData(QString::asprintf("<font color='blue'><u>0x%016llX</u></font>", ptr), Qt::DisplayRole);
                itemValueHex->setData(ptr, gsRoleRawValue);

                stringValue = quotationMark + string.get_string() + quotationMark;
                auto displayValue = QString::fromUtf16(stringValue->c_str(), static_cast<int>(stringValue->size())).toHtmlEscaped();
                itemField->setBackground(itemValue->data(Qt::DisplayRole).toString() == displayValue ? Qt::transparent : highlightColor);
                itemValue->setData(displayValue, Qt::DisplayRole);
            }

            if (comparisonActive)
            {
                if (valueComparisonMemoryOffset == 0)
                {
                    itemComparisonValue->setData({}, Qt::DisplayRole);
                    if (!isPointer)
                        itemComparisonValueHex->setData({}, Qt::DisplayRole);
                }
                else
                {
                    StdWstring comparisonString{valueComparisonMemoryOffset};
                    comparisonStringValue = quotationMark + comparisonString.get_string() + quotationMark;
                    auto displayValue = QString::fromUtf16(comparisonStringValue->data(), static_cast<int>(comparisonStringValue->size())).toHtmlEscaped();
                    itemComparisonValue->setData(displayValue, Qt::DisplayRole);

                    auto ptr = comparisonString.string_ptr();
                    itemComparisonValueHex->setData(QString::asprintf("<font color='blue'><u>0x%016llX</u></font>", ptr), Qt::DisplayRole);
                    itemComparisonValueHex->setData(ptr, gsRoleRawValue);
                }
                bool compare = stringValue != comparisonStringValue;
                itemComparisonValue->setBackground(compare ? comparisonDifferenceColor : Qt::transparent);
                // for the hex should probably compare the actual value displayed, but those will never be the same unless it's the exact same string object
                itemComparisonValueHex->setBackground(compare ? comparisonDifferenceColor : Qt::transparent);
            }
            if (shouldUpdateChildren)
            {
                std::optional<uintptr_t> addr = pointerUpdate ? valueMemoryOffset : (isPointer ? std::nullopt : newAddr);
                std::optional<uintptr_t> comparisonAddr = comparisonPointerUpdate ? valueComparisonMemoryOffset : (isPointer ? std::nullopt : newAddrComparison);
                for (uint8_t x = 0; x < itemField->rowCount(); ++x)
                    updateRow(x, addr, comparisonAddr, itemField);
            }
            break;
        }
        case MemoryFieldType::ThemeInfoPointer:
        case MemoryFieldType::UndeterminedThemeInfoPointer:
        {
            if (valueMemoryOffset == 0)
                itemValue->setData("n/a", Qt::DisplayRole);
            else
                itemValue->setData(Spelunky2::get()->themeNameOfOffset(valueMemoryOffset), Qt::DisplayRole);

            if (comparisonActive)
            {
                if (valueComparisonMemoryOffset == 0)
                    itemComparisonValue->setData("n/a", Qt::DisplayRole);
                else
                    itemComparisonValue->setData(Spelunky2::get()->themeNameOfOffset(valueMemoryOffset), Qt::DisplayRole);

                itemComparisonValue->setBackground(itemComparisonValueHex->background());
            }

            if (shouldUpdateChildren && fieldType == MemoryFieldType::UndeterminedThemeInfoPointer)
            {
                std::optional<uintptr_t> addr = pointerUpdate ? valueMemoryOffset : (isPointer ? std::nullopt : newAddr);
                std::optional<uintptr_t> comparisonAddr = comparisonPointerUpdate ? valueComparisonMemoryOffset : (isPointer ? std::nullopt : newAddrComparison);
                for (uint8_t x = 0; x < itemField->rowCount(); ++x)
                    updateRow(x, addr, comparisonAddr, itemField);
            }
            break;
        }
        case MemoryFieldType::LevelGenRoomsPointer:
        case MemoryFieldType::LevelGenRoomsMetaPointer:
        {
            if (valueMemoryOffset == 0)
                itemValue->setData(itemValueHex->data(Qt::DisplayRole), Qt::DisplayRole);
            else
                itemValue->setData("<font color='blue'><u>Show rooms</u></font>", Qt::DisplayRole);

            if (comparisonActive)
            {
                // we can't show comparison version since it's a tab, not a new window
                // maybe add comparison in the show rooms tab?
                itemComparisonValue->setData({}, Qt::DisplayRole);
                itemComparisonValue->setData(0, gsRoleMemoryAddress);

                itemComparisonValue->setBackground(itemComparisonValueHex->background());
            }
            break;
        }
        case MemoryFieldType::JournalPagePointer:
        {
            if (valueMemoryOffset == 0)
                itemValue->setData(itemValueHex->data(Qt::DisplayRole), Qt::DisplayRole);
            else
                itemValue->setData("<font color='blue'><u>Show journal page</u></font>", Qt::DisplayRole);

            // just for completness, there probably won't be comparison with this
            if (comparisonActive)
            {
                if (valueComparisonMemoryOffset == 0)
                    itemComparisonValue->setData(itemComparisonValueHex->data(Qt::DisplayRole), Qt::DisplayRole);
                else
                    itemComparisonValue->setData("<font color='blue'><u>Show journal page</u></font>", Qt::DisplayRole);

                itemComparisonValue->setBackground(itemComparisonValueHex->background());
            }
            break;
        }
        case MemoryFieldType::IPv4Address:
        {
            std::optional<uint32_t> value;
            value = updateField<uint32_t>(itemField, valueMemoryOffset, itemValue, nullptr, itemValueHex, isPointer, "0x%08X", true, !pointerUpdate, highlightColor);
            if (value.has_value())
            {
                uint32_t ipaddr = value.value();
                QString ipaddrString = QString("%1.%2.%3.%4")
                                           .arg((unsigned char)(ipaddr & 0xFF))
                                           .arg((unsigned char)(ipaddr >> 8 & 0xFF))
                                           .arg((unsigned char)(ipaddr >> 16 & 0xFF))
                                           .arg((unsigned char)(ipaddr >> 24 & 0xFF));

                itemValue->setData(ipaddrString, Qt::DisplayRole);
            }

            if (comparisonActive)
            {
                std::optional<uint32_t> comparisonValue;
                comparisonValue =
                    updateField<uint32_t>(itemField, valueComparisonMemoryOffset, itemComparisonValue, nullptr, itemComparisonValueHex, isPointer, "0x%08X", false, false, highlightColor);

                if (comparisonValue.has_value())
                {
                    uint32_t ipaddr = comparisonValue.value();
                    QString ipaddrString = QString("%1.%2.%3.%4")
                                               .arg((unsigned char)(ipaddr & 0xFF))
                                               .arg((unsigned char)(ipaddr >> 8 & 0xFF))
                                               .arg((unsigned char)(ipaddr >> 16 & 0xFF))
                                               .arg((unsigned char)(ipaddr >> 24 & 0xFF));

                    itemComparisonValue->setData(ipaddrString, Qt::DisplayRole);
                }

                itemComparisonValue->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
                if (isPointer == false)
                    itemComparisonValueHex->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            }
            break;
        }
        case MemoryFieldType::StdVector:
        {
            std::optional<uintptr_t> value;
            // we use the end pointer to check if it was changed
            value = updateField<uintptr_t>(itemField, valueMemoryOffset == 0 ? 0 : valueMemoryOffset + 0x8, itemValue, nullptr, nullptr, true, nullptr, true, !pointerUpdate, highlightColor);
            if (value.has_value())
            {
                itemValue->setData("<font color='blue'><u>Show contents</u></font>", Qt::DisplayRole);
                // maybe show hex as the begin pointer ?
            }

            if (comparisonActive)
            {
                std::optional<uintptr_t> comparisonValue;
                auto addr = valueComparisonMemoryOffset == 0 ? 0 : valueComparisonMemoryOffset + 0x8;
                comparisonValue = updateField<uintptr_t>(itemField, addr, itemComparisonValue, nullptr, nullptr, true, nullptr, true, !pointerUpdate, highlightColor);
                if (comparisonValue.has_value())
                {
                    itemComparisonValue->setData("<font color='blue'><u>Show contents</u></font>", Qt::DisplayRole);
                }

                itemComparisonValue->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
                if (isPointer == false)
                    itemComparisonValueHex->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            }
            if (shouldUpdateChildren)
            {
                std::optional<uintptr_t> addr = pointerUpdate ? valueMemoryOffset : (isPointer ? std::nullopt : newAddr);
                std::optional<uintptr_t> comparisonAddr = comparisonPointerUpdate ? valueComparisonMemoryOffset : (isPointer ? std::nullopt : newAddrComparison);
                for (uint8_t x = 0; x < itemField->rowCount(); ++x)
                    updateRow(x, addr, comparisonAddr, itemField);
            }
            break;
        }
        case MemoryFieldType::StdMap:
        {
            std::optional<size_t> value;
            // we use the size to check if it was changed
            value = updateField<size_t>(itemField, valueMemoryOffset == 0 ? 0 : valueMemoryOffset + 0x8, itemValue, nullptr, nullptr, true, nullptr, true, !pointerUpdate, highlightColor);
            if (value.has_value())
            {
                itemValue->setData("<font color='blue'><u>Show contents</u></font>", Qt::DisplayRole);
                // maybe show hex as the pointer ?
            }

            if (comparisonActive)
            {
                std::optional<size_t> comparisonValue;
                auto addr = valueComparisonMemoryOffset == 0 ? 0 : valueComparisonMemoryOffset + 0x8;
                comparisonValue = updateField<size_t>(itemField, addr, itemComparisonValue, nullptr, nullptr, true, nullptr, true, !pointerUpdate, highlightColor);
                if (comparisonValue.has_value())
                {
                    itemComparisonValue->setData("<font color='blue'><u>Show contents</u></font>", Qt::DisplayRole);
                }
                // maybe it should be based on the pointer not size?
                itemComparisonValue->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
                if (isPointer == false)
                    itemComparisonValueHex->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            }
            if (shouldUpdateChildren)
            {
                std::optional<uintptr_t> addr = pointerUpdate ? valueMemoryOffset : (isPointer ? std::nullopt : newAddr);
                std::optional<uintptr_t> comparisonAddr = comparisonPointerUpdate ? valueComparisonMemoryOffset : (isPointer ? std::nullopt : newAddrComparison);
                for (uint8_t x = 0; x < itemField->rowCount(); ++x)
                    updateRow(x, addr, comparisonAddr, itemField);
            }
            break;
        }
        case MemoryFieldType::Skip:
        {
            // TODO when setting for skip is done
            break;
        }
        case MemoryFieldType::EntitySubclass:
        {
            // can't be a pointer
            if (isExpanded(itemField->index()))
                itemValue->setData("<font color='darkMagenta'><u>[Collapse]</u></font>", Qt::DisplayRole);
            else
                itemValue->setData("<font color='#aaa'><u>[Expand]</u></font>", Qt::DisplayRole);

            if (comparisonActive)
                itemComparisonValue->setData(itemValue->data(Qt::DisplayRole), Qt::DisplayRole);

            if (shouldUpdateChildren)
            {
                for (int x = 0; x < itemField->rowCount(); ++x)
                    updateRow(x, newAddr, newAddrComparison, itemField);
            }
            break;
        }
        case MemoryFieldType::DefaultStructType:
        {
            if (isExpanded(itemField->index()))
                itemValue->setData("<font color='darkMagenta'><u>[Collapse]</u></font>", Qt::DisplayRole);
            else
                itemValue->setData("<font color='#aaa'><u>[Expand]</u></font>", Qt::DisplayRole);

            if (comparisonActive)
                itemComparisonValue->setData(itemValue->data(Qt::DisplayRole), Qt::DisplayRole);

            if (shouldUpdateChildren)
            {
                std::optional<uintptr_t> addr = pointerUpdate ? valueMemoryOffset : (isPointer ? std::nullopt : newAddr);
                std::optional<uintptr_t> comparisonAddr = comparisonPointerUpdate ? valueComparisonMemoryOffset : (isPointer ? std::nullopt : newAddrComparison);
                for (int x = 0; x < itemField->rowCount(); ++x)
                    updateRow(x, addr, comparisonAddr, itemField);
            }
            break;
        }
        case MemoryFieldType::Dummy:
        {
            if (isExpanded(itemField->index()))
                itemValue->setData("<font color='darkMagenta'><u>[Collapse]</u></font>", Qt::DisplayRole);
            else
                itemValue->setData("<font color='#aaa'><u>[Expand]</u></font>", Qt::DisplayRole);

            // if (comparisonActive)
            //     itemComparisonValue->setData(itemValue->data(Qt::DisplayRole), Qt::DisplayRole);
            //  probably won't be involved in comparisons
            if (shouldUpdateChildren)
            {
                for (int x = 0; x < itemField->rowCount(); ++x)
                    updateRow(x, newAddr, newAddrComparison, itemField);
            }
            break;
        }
        default:
        {
            // just for debug
            dprintf("WARNING: type %d not handled in TreeViewMemoryFields::updateRow ('%s' row: %d)\n", fieldType, itemField->data(gsRoleUID).toString().toStdString().c_str(), row);
            if (shouldUpdateChildren)
            {
                for (int x = 0; x < itemField->rowCount(); ++x)
                {
                    std::optional<uintptr_t> addr = pointerUpdate ? valueMemoryOffset : (isPointer ? std::nullopt : newAddr);
                    std::optional<uintptr_t> comparisonAddr = comparisonPointerUpdate ? valueComparisonMemoryOffset : (isPointer ? std::nullopt : newAddrComparison);
                    updateRow(x, addr, comparisonAddr, itemField);
                }
            }
            break;
        }
    }
}

void S2Plugin::TreeViewMemoryFields::cellClicked(const QModelIndex& index)
{
    auto clickedItem = mModel->itemFromIndex(index);
    constexpr auto getDataFrom = [](const QModelIndex& idx, int col, int role) { return idx.sibling(idx.row(), col).data(role); };

    switch (index.column())
    {
        case gsColMemoryAddress:
        {
            GuiDumpAt(clickedItem->data(gsRoleRawValue).toULongLong());
            GuiShowCpu();
            break;
        }
        case gsColValueHex:
        case gsColComparisonValueHex:
        {
            // only pointers have gsRoleRawValue in Hex field, no value will result in 0
            auto addr = clickedItem->data(gsRoleRawValue).toULongLong();
            if (Script::Memory::IsValidPtr(addr))
            {
                // exception since there is no point in showing code address in memory dump
                if (getDataFrom(index, gsColField, gsRoleType).value<MemoryFieldType>() == MemoryFieldType::CodePointer)
                {
                    GuiDisasmAt(addr, GetContextData(UE_CIP));
                    GuiShowCpu();
                    break;
                }
                GuiDumpAt(addr);
                GuiShowCpu();
            }
            break;
        }
        case gsColValue:
        case gsColComparisonValue:
        {
            auto dataType = getDataFrom(index, gsColField, gsRoleType).value<MemoryFieldType>();
            switch (dataType)
            {
                case MemoryFieldType::CodePointer:
                {
                    auto rawValue = clickedItem->data(gsRoleMemoryAddress).toULongLong();
                    GuiDisasmAt(rawValue, GetContextData(UE_CIP));
                    GuiShowCpu();
                    break;
                }
                case MemoryFieldType::DataPointer:
                {
                    auto rawValue = clickedItem->data(gsRoleMemoryAddress).toULongLong();
                    if (rawValue != 0)
                    {
                        GuiDumpAt(rawValue);
                        GuiShowCpu();
                    }
                    break;
                }
                case MemoryFieldType::EntityPointer:
                {
                    auto rawValue = clickedItem->data(gsRoleRawValue).toULongLong();
                    if (Script::Memory::IsValidPtr(rawValue))
                    {
                        getToolbar()->showEntity(rawValue);
                    }
                    break;
                }
                case MemoryFieldType::EntityUID:
                {
                    auto offset = clickedItem->data(gsRoleEntityAddress).toULongLong();
                    if (offset != 0)
                    {
                        getToolbar()->showEntity(offset);
                    }
                    break;
                }
                case MemoryFieldType::EntityDBID:
                {
                    auto id = clickedItem->data(gsRoleRawValue).toUInt();
                    if (id != 0)
                    {
                        auto view = getToolbar()->showEntityDB();
                        if (view != nullptr)
                        {
                            view->showID(id);
                        }
                    }
                    break;
                }
                case MemoryFieldType::CharacterDBID:
                {
                    auto id = clickedItem->data(gsRoleRawValue);
                    if (!id.isNull())
                    {
                        auto view = getToolbar()->showCharacterDB();
                        if (view != nullptr)
                        {
                            view->showID(id.toUInt());
                        }
                    }
                    break;
                }
                case MemoryFieldType::TextureDBID:
                {
                    auto id = clickedItem->data(gsRoleRawValue);
                    if (!id.isNull())
                    {
                        auto view = getToolbar()->showTextureDB();
                        if (view != nullptr)
                        {
                            view->showID(id.toUInt());
                        }
                    }
                    break;
                }
                case MemoryFieldType::ParticleDBID:
                {
                    auto id = clickedItem->data(gsRoleRawValue);
                    if (!id.isNull() && id.toUInt() != -1)
                    {
                        auto view = getToolbar()->showParticleDB();
                        if (view != nullptr)
                        {
                            view->showID(id.toUInt());
                        }
                    }
                    break;
                }
                case MemoryFieldType::EntityDBPointer:
                {
                    auto rawValue = clickedItem->data(gsRoleMemoryAddress).toULongLong();
                    if (rawValue != 0)
                    {
                        auto view = getToolbar()->showEntityDB();
                        if (view != nullptr)
                        {
                            view->showRAW(rawValue);
                        }
                    }
                    break;
                }
                case MemoryFieldType::TextureDBPointer:
                {
                    auto rawValue = clickedItem->data(gsRoleMemoryAddress).toULongLong();
                    if (rawValue != 0)
                    {
                        auto view = getToolbar()->showTextureDB();
                        if (view != nullptr)
                        {
                            view->showRAW(rawValue);
                        }
                    }
                    break;
                }
                case MemoryFieldType::State8:
                case MemoryFieldType::State16:
                case MemoryFieldType::State32:
                {
                    auto addr = clickedItem->data(gsRoleMemoryAddress).toULongLong();
                    if (addr != 0)
                    {
                        auto name = getDataFrom(index, gsColField, gsRoleUID).toString();
                        auto refName = qvariant_cast<std::string>(getDataFrom(index, gsColField, gsRoleRefName));
                        auto dialog = new DialogEditState(name, refName, addr, dataType, this);
                        dialog->exec();
                    }
                    break;
                }
                case MemoryFieldType::LevelGenPointer:
                {
                    auto rawValue = clickedItem->data(gsRoleMemoryAddress).toULongLong();
                    if (rawValue != 0)
                    {
                        getToolbar()->showLevelGen(rawValue);
                    }
                    break;
                }
                case MemoryFieldType::StdVector:
                {
                    auto addr = clickedItem->data(gsRoleMemoryAddress).toULongLong();
                    auto fieldType = qvariant_cast<std::string>(getDataFrom(index, gsColField, gsRoleStdContainerFirstParameterType));
                    if (addr != 0)
                    {
                        getToolbar()->showStdVector(addr, fieldType);
                    }
                    break;
                }
                case MemoryFieldType::StdMap:
                {
                    auto addr = clickedItem->data(gsRoleMemoryAddress).toULongLong();
                    auto fieldkeyType = qvariant_cast<std::string>(getDataFrom(index, gsColField, gsRoleStdContainerFirstParameterType));
                    auto fieldvalueType = qvariant_cast<std::string>(getDataFrom(index, gsColField, gsRoleStdContainerSecondParameterType));
                    if (addr != 0)
                    {
                        getToolbar()->showStdMap(addr, fieldkeyType, fieldvalueType);
                    }
                    break;
                }
                case MemoryFieldType::ParticleDBPointer:
                {
                    auto rawValue = clickedItem->data(gsRoleMemoryAddress).toULongLong();
                    if (rawValue != 0)
                    {
                        auto view = getToolbar()->showParticleDB();
                        if (view != nullptr)
                        {
                            view->showRAW(rawValue);
                        }
                    }
                    break;
                }
                case MemoryFieldType::VirtualFunctionTable:
                {
                    auto rawValue = clickedItem->data(gsRoleMemoryAddress).toULongLong();
                    if (rawValue != 0)
                    {
                        // maybe use the "entity interpret as" for vtable? (it's doable)
                        auto vftType = qvariant_cast<std::string>(getDataFrom(index, gsColField, gsRoleRefName));
                        if (vftType == "Entity") // in case of Entity, we have to see what the entity is interpreted as, and show those functions
                        {
                            // rare case, we need the address not the pointer value to get entity
                            uintptr_t ent;
                            if (index.column() == gsColComparisonValue)
                                ent = getDataFrom(index, gsColField, gsRoleComparisonMemoryAddress).toULongLong();
                            else
                                ent = getDataFrom(index, gsColField, gsRoleMemoryAddress).toULongLong();

                            getToolbar()->showVirtualFunctions(rawValue, Entity{ent}.entityClassName());
                        }
                        else
                        {
                            getToolbar()->showVirtualFunctions(rawValue, vftType);
                        }
                    }
                    break;
                }
                case MemoryFieldType::Bool:
                {
                    auto offset = clickedItem->data(gsRoleMemoryAddress).toULongLong();
                    if (offset != 0)
                    {
                        auto currentValue = clickedItem->data(gsRoleRawValue).toBool();
                        Script::Memory::WriteByte(offset, !currentValue);
                    }
                    break;
                }
                case MemoryFieldType::Flag:
                {
                    auto flagIndex = getDataFrom(index, gsColField, gsRoleFlagIndex).toUInt();
                    auto offset = clickedItem->parent()->data(gsRoleMemoryAddress).toULongLong();
                    if (offset != 0)
                    {
                        auto currentValue = Script::Memory::ReadDword(offset);
                        Script::Memory::WriteDword(offset, currentValue ^ (1U << (flagIndex - 1)));
                    }
                    break;
                }
                case MemoryFieldType::Byte:
                case MemoryFieldType::UnsignedByte:
                case MemoryFieldType::Word:
                case MemoryFieldType::UnsignedWord:
                case MemoryFieldType::Dword:
                case MemoryFieldType::UnsignedDword:
                case MemoryFieldType::Qword:
                case MemoryFieldType::UnsignedQword:
                case MemoryFieldType::Float:
                case MemoryFieldType::Double:
                case MemoryFieldType::StringsTableID:
                {
                    auto offset = clickedItem->data(gsRoleMemoryAddress).toULongLong();
                    if (offset != 0)
                    {
                        auto fieldName = getDataFrom(index, gsColField, gsRoleUID).toString();
                        auto dialog = new DialogEditSimpleValue(fieldName, offset, dataType, this);
                        dialog->exec();
                    }
                    break;
                }
                case MemoryFieldType::LevelGenRoomsPointer:
                case MemoryFieldType::LevelGenRoomsMetaPointer:
                {
                    // available only in the level gen
                    auto rawValue = clickedItem->data(gsRoleMemoryAddress).toULongLong();
                    if (rawValue != 0)
                    {
                        emit levelGenRoomsPointerClicked();
                    }
                    break;
                }
                case MemoryFieldType::JournalPagePointer:
                {
                    auto rawValue = clickedItem->data(gsRoleMemoryAddress).toULongLong();
                    if (rawValue != 0)
                    {
                        getToolbar()->showJournalPage(rawValue);
                    }
                    break;
                }
                case MemoryFieldType::DefaultStructType:
                case MemoryFieldType::EntitySubclass:
                case MemoryFieldType::Dummy:
                {
                    auto fieldIndex = index.sibling(index.row(), gsColField);
                    if (isExpanded(fieldIndex))
                        collapse(fieldIndex);
                    else
                        expand(index.sibling(index.row(), gsColField));

                    break;
                }
                case MemoryFieldType::UTF16Char:
                {
                    auto offset = clickedItem->data(gsRoleMemoryAddress).toULongLong();
                    if (offset != 0)
                    {
                        auto fieldName = getDataFrom(index, gsColField, gsRoleUID).toString();
                        QChar c = static_cast<ushort>(clickedItem->data(gsRoleRawValue).toUInt());
                        auto dialog = new DialogEditString(fieldName, c, offset, 1, dataType, this);
                        dialog->exec();
                    }
                    break;
                }
                case MemoryFieldType::UTF16StringFixedSize:
                {
                    auto offset = clickedItem->data(gsRoleMemoryAddress).toULongLong();
                    if (offset != 0)
                    {
                        int size = getDataFrom(index, gsColField, gsRoleSize).toInt();
                        auto fieldName = getDataFrom(index, gsColField, gsRoleUID).toString();
                        auto stringData = std::make_unique<ushort[]>(size);
                        Script::Memory::Read(offset, stringData.get(), size, nullptr);
                        auto s = QString::fromUtf16(stringData.get());
                        auto dialog = new DialogEditString(fieldName, s, offset, size / 2 - 1, dataType, this);
                        dialog->exec();
                    }
                    break;
                }
                case MemoryFieldType::ConstCharPointer:
                {
                    auto offset = clickedItem->data(gsRoleMemoryAddress).toULongLong();
                    if (offset != 0)
                    {
                        auto fieldName = getDataFrom(index, gsColField, gsRoleUID).toString();
                        auto s = QString::fromStdString(ReadConstString(offset));
                        // [Known Issue]: Now way to safely determinate allowed lenght, so we just allow as much characters as there is already
                        auto dialog = new DialogEditString(fieldName, s, offset, s.length(), dataType, this);
                        dialog->exec();
                    }
                    break;
                }
                case MemoryFieldType::UTF8StringFixedSize:
                {
                    auto offset = clickedItem->data(gsRoleMemoryAddress).toULongLong();
                    if (offset != 0)
                    {
                        int size = getDataFrom(index, gsColField, gsRoleSize).toInt();
                        auto fieldName = getDataFrom(index, gsColField, gsRoleUID).toString();
                        auto stringData = std::make_unique<char[]>(size);
                        Script::Memory::Read(offset, stringData.get(), size, nullptr);
                        auto s = QString::fromUtf8(stringData.get());
                        auto dialog = new DialogEditString(fieldName, s, offset, size - 1, dataType, this);
                        dialog->exec();
                    }
                    break;
                }
            }
            emit memoryFieldValueUpdated(index.row(), clickedItem->parent());
        }
    }
}

void S2Plugin::TreeViewMemoryFields::clear()
{
    mSavedColumnWidths[gsColField] = columnWidth(gsColField);
    mSavedColumnWidths[gsColValue] = columnWidth(gsColValue);
    mSavedColumnWidths[gsColValueHex] = columnWidth(gsColValueHex);
    mSavedColumnWidths[gsColMemoryAddress] = columnWidth(gsColMemoryAddress);
    mSavedColumnWidths[gsColMemoryAddressDelta] = columnWidth(gsColMemoryAddressDelta);
    mSavedColumnWidths[gsColType] = columnWidth(gsColType);
    mSavedColumnWidths[gsColComment] = columnWidth(gsColComment);
    mModel->clear();
}

void S2Plugin::TreeViewMemoryFields::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->property(gsDragDropMemoryField_Address).isValid())
    {
        if (qobject_cast<TreeViewMemoryFields*>(event->source()) == this)
            return;

        event->acceptProposedAction();
    }
}

void S2Plugin::TreeViewMemoryFields::dragMoveEvent(QDragMoveEvent* event)
{
    if (event->mimeData()->property(gsDragDropMemoryField_Address).isValid())
    {
        event->accept();
    }
}

void S2Plugin::TreeViewMemoryFields::dropEvent(QDropEvent* event)
{
    if (qobject_cast<TreeViewMemoryFields*>(event->source()) == this)
        return;

    auto dropData = event->mimeData()->property(gsDragDropMemoryField_Address);
    if (auto addr = dropData.toULongLong(); addr != 0)
    {
        uintptr_t dataAddr = addr; // just for convenience
        if (event->mimeData()->property(gsDragDropMemoryField_IsPointer).toBool())
        {
            dataAddr = Script::Memory::ReadQword(addr);
            if (!Script::Memory::IsValidPtr(dataAddr))
                return;
        }
        auto rootItem = mModel->invisibleRootItem();
        auto getThisTypeName = [rootItem]() -> QString
        {
            auto parrent = rootItem;
            for (int idx = 0; idx < parrent->rowCount(); ++idx)
            {
                auto item = parrent->child(idx, gsColField);
                if (item == nullptr)
                    break;

                auto data = item->data(gsRoleUID);
                if (data.isValid())
                {
                    auto str = data.toString();
                    auto dot = str.indexOf('.');
                    if (dot != -1)
                        return str.left(dot);
                }
                if (item->hasChildren())
                {
                    parrent = item;
                    idx = 0;
                }
            }
            return {};
        };

        switch (event->mimeData()->property(gsDragDropMemoryField_Type).value<MemoryFieldType>())
        {
            case MemoryFieldType::DataPointer:
            {
                // allow all
                addr = dataAddr;
                break;
            }
            case MemoryFieldType::EntityPointer:
            {
                if (getThisTypeName() != "Entity")
                    return;

                addr = dataAddr;
                break;
            }
            case MemoryFieldType::EntityUID:
            {
                if (getThisTypeName() != "Entity")
                    return;

                auto uid = Script::Memory::ReadDword(dataAddr);
                auto entityPtr = Spelunky2::get()->get_State().findEntitybyUID(uid);
                if (entityPtr == 0)
                    return;

                addr = entityPtr;
                break;
            }
            case MemoryFieldType::EntitySubclass:
            {
                if (getThisTypeName() != "Entity")
                    return;

                break;
            }
            case MemoryFieldType::EntityDBPointer:
            {
                if (getThisTypeName() != "EntityDB")
                    return;

                addr = dataAddr;
                break;
            }
            case MemoryFieldType::EntityDBID:
            {
                if (getThisTypeName() != "EntityDB")
                    return;

                auto id = Script::Memory::ReadDword(dataAddr);
                addr = Spelunky2::get()->get_EntityDB().addressOfIndex(id);
                break;
            }
            case MemoryFieldType::ParticleDBPointer:
            {
                if (getThisTypeName() != "ParticleDB")
                    return;

                addr = dataAddr;
                break;
            }
            case MemoryFieldType::ParticleDBID:
            {
                if (getThisTypeName() != "ParticleDB")
                    return;

                auto id = Script::Memory::ReadDword(dataAddr);
                addr = Spelunky2::get()->get_ParticleDB().addressOfIndex(id);
                break;
            }
            case MemoryFieldType::TextureDBPointer:
            {
                if (getThisTypeName() != "TextureDB")
                    return;

                addr = dataAddr;
                break;
            }
            case MemoryFieldType::TextureDBID:
            {
                if (getThisTypeName() != "TextureDB")
                    return;

                auto id = Script::Memory::ReadDword(dataAddr);
                addr = Spelunky2::get()->get_TextureDB().addressOfID(id);
                break;
            }
            case MemoryFieldType::CharacterDBID:
            {
                if (getThisTypeName() != "CharacterDB")
                    return;

                auto id = Script::Memory::ReadByte(dataAddr);
                addr = Spelunky2::get()->get_CharacterDB().addressOfIndex(id);
                break;
            }
            case MemoryFieldType::VirtualFunctionTable:
            {
                // since it's always the first item in struct, we allow using it like an anchor to the whole struct
                auto refName = event->mimeData()->property(gsDragDropMemoryField_RefName).value<std::string>();
                if (getThisTypeName() != QString::fromStdString(refName))
                    return;
                break;
            }
            case MemoryFieldType::DefaultStructType:
            {
                if (getThisTypeName() != event->mimeData()->property(gsDragDropMemoryField_RefName).toString())
                    return;

                addr = dataAddr;
                break;
            }
            case MemoryFieldType::JournalPagePointer:
            {
                if (getThisTypeName() != "JournalPage")
                    return;

                addr = dataAddr;
                break;
            }
            case MemoryFieldType::LevelGenPointer:
            {
                if (getThisTypeName() != "LevelGen")
                    return;

                addr = dataAddr;
                break;
            }
            case MemoryFieldType::LevelGenRoomsMetaPointer:
            case MemoryFieldType::LevelGenRoomsPointer:
            {
                if (getThisTypeName() != "LevelGen")
                    return;

                break;
            }
            case MemoryFieldType::ThemeInfoPointer:
            case MemoryFieldType::UndeterminedThemeInfoPointer:
            {
                if (getThisTypeName() != "ThemeInfo")
                    return;

                addr = dataAddr;
                break;
            }
            default:
                return;
        }
        if (addr == 0) // just in case some bad id's
            return;

        activeColumns.enable(gsColComparisonValue).enable(gsColComparisonValueHex);
        setColumnHidden(gsColComparisonValue, false);
        setColumnHidden(gsColComparisonValueHex, false);
        updateTree(0, addr);
        emit offsetDropped(addr);
        event->acceptProposedAction();
    }
}

void S2Plugin::TreeViewMemoryFields::startDrag(Qt::DropActions)
{
    QModelIndex index = currentIndex();
    if (!index.isValid())
        return;

    if (index.column() != gsColField)
        index = index.sibling(index.row(), gsColField);

    auto mimeData = new QMimeData();
    mimeData->setProperty(gsDragDropMemoryField_UID, index.data(gsRoleUID));
    mimeData->setProperty(gsDragDropMemoryField_Address, index.data(gsRoleMemoryAddress));

    if (currentIndex().column() == gsColMemoryAddress) // if you grab memory address treat it like data pointer
    {
        mimeData->setProperty(gsDragDropMemoryField_Type, QVariant::fromValue(MemoryFieldType::DataPointer));
        // set as not pointer, so it does not read the memory first
        mimeData->setProperty(gsDragDropMemoryField_IsPointer, false);
    }
    else
    {
        mimeData->setProperty(gsDragDropMemoryField_Type, index.data(gsRoleType));
        mimeData->setProperty(gsDragDropMemoryField_IsPointer, index.data(gsRoleIsPointer));

        switch (index.data(gsRoleType).value<MemoryFieldType>())
        {
            case MemoryFieldType::VirtualFunctionTable:
                mimeData->setProperty(gsDragDropMemoryField_RefName, index.data(gsRoleRefName));
                break;

            case MemoryFieldType::LevelGenRoomsMetaPointer:
            case MemoryFieldType::LevelGenRoomsPointer:
            {
                auto indexDelta = index.sibling(index.row(), gsColMemoryAddressDelta);
                if (!indexDelta.isValid())
                    return;

                // note that we set the address to be of whole level gen, not the pointer
                // if in future we need this pointer, will have to use different solution (adding delta value property instead)
                mimeData->setProperty(gsDragDropMemoryField_Address, index.data(gsRoleMemoryAddress).toULongLong() - indexDelta.data(gsRoleRawValue).toULongLong());
                mimeData->setProperty(gsDragDropMemoryField_IsPointer, false);
                break;
            }
            case MemoryFieldType::DefaultStructType:
            {
                auto indexType = index.sibling(index.row(), gsColType);
                if (!indexType.isValid())
                    return;

                auto typeName = indexType.data(Qt::DisplayRole).toString();
                auto colon = typeName.indexOf(':'); // to get rid of the "<b>P</b>: "
                if (colon == -1)
                    mimeData->setProperty(gsDragDropMemoryField_RefName, typeName);
                else
                    mimeData->setProperty(gsDragDropMemoryField_RefName, typeName.mid(colon + 2));

                break;
            }
            case MemoryFieldType::EntitySubclass:
            {
                auto indexType = index.sibling(index.row(), gsColType);
                if (!indexType.isValid())
                    return;

                auto typeName = indexType.data(Qt::DisplayRole).toString();
                if (typeName != "Entity")
                {
                    auto indexDelta = index.sibling(index.row(), gsColMemoryAddressDelta);
                    mimeData->setProperty(gsDragDropMemoryField_Address, index.data(gsRoleMemoryAddress).toULongLong() - indexDelta.data(gsRoleRawValue).toULongLong());
                }

                break;
            }
            case MemoryFieldType::None:
            case MemoryFieldType::Dummy:
                return;
        }
    }
    QDrag* drag = new QDrag(this);
    drag->setMimeData(mimeData);
    drag->exec();
}

static void labelChildren(QStandardItem* parrent, std::string_view prefix)
{
    auto config = S2Plugin::Configuration::get();

    auto pointerCheck = [parrent](int idx)
    {
        auto hex_field = parrent->child(idx, S2Plugin::gsColValueHex);
        auto pointer_value = hex_field->data(S2Plugin::gsRoleRawValue).toULongLong();
        return Script::Memory::IsValidPtr(pointer_value);
    };

    for (int idx = 0; idx < parrent->rowCount(); ++idx)
    {
        auto field = parrent->child(idx, S2Plugin::gsColField);
        bool isPointer = field->data(S2Plugin::gsRoleIsPointer).toBool();
        std::string name;
        auto qstr_name = field->data(S2Plugin::gsRoleUID).toString();
        if (prefix.empty())
        {
            name = qstr_name.toStdString();
        }
        else
        {
            name.reserve(prefix.length() + 1u + qstr_name.length());
            name.append(prefix);
            name += '.';
            name.append(qstr_name.toStdString());
        }

        S2Plugin::MemoryFieldType type = field->data(S2Plugin::gsRoleType).value<S2Plugin::MemoryFieldType>();
        if (type == S2Plugin::MemoryFieldType::DefaultStructType || type == S2Plugin::MemoryFieldType::EntitySubclass || !config->typeFields(type).empty())
        {
            if (isPointer)
            {
                // label children only if it's valid pointer
                if (pointerCheck(idx))
                    labelChildren(field, prefix);
            }
            else
            {
                labelChildren(field, prefix);
                // if it's inline struct we can't label the struct itself since the offset will be the same as the first element in the struct
                continue;
            }
        }
        else if (isPointer) // label address behind poinders
        {
            auto hex_field = parrent->child(idx, S2Plugin::gsColValueHex);
            auto pointer_value = hex_field->data(S2Plugin::gsRoleRawValue).toULongLong();
            if (Script::Memory::IsValidPtr(pointer_value))
            {
                auto typeName = config->getTypeDisplayName(type);
                std::string valueName;
                valueName.reserve(name.length() + 1u + typeName.length());
                valueName += name;
                valueName += '.';
                valueName.append(config->getTypeDisplayName(type));
                if (!DbgSetAutoLabelAt(pointer_value, valueName.c_str()))
                {
                    dprintf("Failed to label value behind pointer: (%s)\n", name.c_str());
                    continue;
                }
            }
        }
        uintptr_t address = field->data(S2Plugin::gsRoleMemoryAddress).toULongLong();
        if (!Script::Memory::IsValidPtr(address))
        {
            dprintf("Failed to label (%s)\n", name.c_str());
            continue;
        }

        if (!DbgSetAutoLabelAt(address, name.c_str()))
        {
            dprintf("Failed to label (%s)\n", name.c_str());
            continue;
        }

        // label each byte in flags field since often the instructions will only read the byte of interest
        if (type == S2Plugin::MemoryFieldType::Flags16)
        {
            DbgSetAutoLabelAt(address + 1, (name + "+0x1").c_str());
        }
        else if (type == S2Plugin::MemoryFieldType::Flags32)
        {
            DbgSetAutoLabelAt(address + 1, (name + "+0x1").c_str());
            DbgSetAutoLabelAt(address + 2, (name + "+0x2").c_str());
            DbgSetAutoLabelAt(address + 3, (name + "+0x3").c_str());
        }
    }
}

void S2Plugin::TreeViewMemoryFields::labelAll(std::string_view prefix)
{
    auto parrent = mModel->invisibleRootItem();
    labelChildren(parrent, prefix);
}

void S2Plugin::TreeViewMemoryFields::expandLast()
{
    auto mod = model();
    auto rows = mod->rowCount();
    if (rows != 0)
        expand(mod->index(mod->rowCount() - 1, 0));
}
