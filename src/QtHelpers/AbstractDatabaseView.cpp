#include "QtHelpers/AbstractDatabaseView.h"

#include "Configuration.h"
#include "QtHelpers/StyledItemDelegateHTML.h"
#include "QtHelpers/TableWidgetItemNumeric.h"
#include "QtHelpers/TreeViewMemoryFields.h"
#include "QtHelpers/TreeWidgetItemNumeric.h"
#include "QtPlugin.h"
#include "Spelunky2.h"
#include "pluginmain.h"
#include <QCheckBox>
#include <QCompleter>
#include <QHash>
#include <QHeaderView>
#include <QPushButton>
#include <QTreeWidgetItem>
#include <QVBoxLayout>

namespace std
{
    template <> // to be able to use QString as a key in unordered map
    struct hash<QString>
    {
        std::size_t operator()(const QString& s) const noexcept
        {
            return qHash(s);
        }
    };
} // namespace std

struct ComparisonField
{
    S2Plugin::MemoryFieldType type{S2Plugin::MemoryFieldType::None};
    size_t offset{0};
    std::string refName; // for flags
    uint8_t flagIndex;
};
Q_DECLARE_METATYPE(ComparisonField)

S2Plugin::AbstractDatabaseView::AbstractDatabaseView(MemoryFieldType type, QWidget* parent) : QWidget(parent)
{
    setWindowIcon(getCavemanIcon());
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setMargin(5);

    mMainTabWidget = new QTabWidget();
    mMainTabWidget->setDocumentMode(false);
    mainLayout->addWidget(mMainTabWidget);

    auto tabLookup = new QWidget();
    auto tabCompare = new QWidget();
    tabLookup->setLayout(new QVBoxLayout());
    tabLookup->layout()->setMargin(10);
    tabLookup->setObjectName("lookupWidget");
    tabLookup->setStyleSheet("QWidget#lookupWidget {border: 1px solid #999;}");
    tabCompare->setLayout(new QVBoxLayout());
    tabCompare->layout()->setMargin(10);
    tabCompare->setObjectName("compareWidget");
    tabCompare->setStyleSheet("QWidget#compareWidget {border: 1px solid #999;}");

    mMainTabWidget->addTab(tabLookup, "Lookup");
    mMainTabWidget->addTab(tabCompare, "Compare");
    auto config = Configuration::get();
    // LOOKUP
    {
        auto topLayout = new QHBoxLayout();

        mSearchLineEdit = new LineEditEx();
        mSearchLineEdit->setPlaceholderText("Search");
        topLayout->addWidget(mSearchLineEdit);
        QObject::connect(mSearchLineEdit, &QLineEdit::returnPressed, this, &AbstractDatabaseView::searchFieldReturnPressed);

        auto labelButton = new QPushButton("Label", this);
        QObject::connect(labelButton, &QPushButton::clicked, this, &AbstractDatabaseView::label);
        topLayout->addWidget(labelButton);

        qobject_cast<QVBoxLayout*>(tabLookup->layout())->addLayout(topLayout);

        mMainTreeView = new TreeViewMemoryFields(this);
        tabLookup->layout()->addWidget(mMainTreeView);
        mMainTreeView->activeColumns.disable(gsColComparisonValue).disable(gsColComparisonValueHex);
        mMainTreeView->updateTableHeader(false);
        mMainTreeView->setEnableChangeHighlighting(false);
        mMainTreeView->setColumnWidth(gsColField, 125);
        mMainTreeView->setColumnWidth(gsColValue, 250);
        mMainTreeView->setColumnWidth(gsColValueHex, 125);
        mMainTreeView->setColumnWidth(gsColMemoryAddress, 120);
        mMainTreeView->setColumnWidth(gsColMemoryAddressDelta, 75);
        mMainTreeView->setColumnWidth(gsColType, 100);

        mMainTreeView->addMemoryFields(config->typeFields(type), std::string(config->getTypeDisplayName(type)), 0);
        QObject::connect(mMainTreeView, &TreeViewMemoryFields::memoryFieldValueUpdated, this, &AbstractDatabaseView::fieldUpdated);
        QObject::connect(mMainTreeView, &TreeViewMemoryFields::expanded, this, &AbstractDatabaseView::fieldExpanded);
    }

    // COMPARE
    {
        auto topLayout = new QHBoxLayout();
        mCompareFieldComboBox = new QComboBox();
        mCompareFieldComboBox->addItem(QString::fromStdString(""));
        populateComparisonComboBox(config->typeFields(type));

        QObject::connect(mCompareFieldComboBox, &QComboBox::currentTextChanged, this, &AbstractDatabaseView::comparisonFieldChosen);
        topLayout->addWidget(mCompareFieldComboBox);

        mCompareFlagComboBox = new QComboBox();
        QObject::connect(mCompareFlagComboBox, &QComboBox::currentTextChanged, this, &AbstractDatabaseView::comparisonFlagChosen);
        mCompareFlagComboBox->hide();
        topLayout->addWidget(mCompareFlagComboBox);

        auto groupCheckbox = new QCheckBox("Group by value", this);
        QObject::connect(groupCheckbox, &QCheckBox::stateChanged, this, &AbstractDatabaseView::compareGroupByCheckBoxClicked);
        topLayout->addWidget(groupCheckbox);

        qobject_cast<QVBoxLayout*>(tabCompare->layout())->addLayout(topLayout);

        mCompareTableWidget = new QTableWidget(0, 3, this); // need to set the row count in subclass
        mCompareTableWidget->setAlternatingRowColors(true);
        mCompareTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
        mCompareTableWidget->setHorizontalHeaderLabels(QStringList() << "ID"
                                                                     << "Name"
                                                                     << "Value");
        mCompareTableWidget->verticalHeader()->setVisible(false);
        mCompareTableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
        mCompareTableWidget->verticalHeader()->setDefaultSectionSize(20);
        mCompareTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
        mCompareTableWidget->setColumnWidth(0, 40);
        mCompareTableWidget->setColumnWidth(1, 325);
        mCompareTableWidget->setColumnWidth(2, 150);
        auto HTMLDelegate = new StyledItemDelegateHTML(this);
        mCompareTableWidget->setItemDelegate(HTMLDelegate);
        QObject::connect(mCompareTableWidget, &QTableWidget::cellClicked, this, &AbstractDatabaseView::comparisonCellClicked);

        mCompareTreeWidget = new QTreeWidget(this);
        mCompareTreeWidget->setAlternatingRowColors(true);
        mCompareTreeWidget->headerItem()->setHidden(true);
        mCompareTreeWidget->setHidden(true);
        mCompareTreeWidget->setItemDelegate(HTMLDelegate);
        QObject::connect(mCompareTreeWidget, &QTreeWidget::itemClicked, this, &AbstractDatabaseView::groupedComparisonItemClicked);

        tabCompare->layout()->addWidget(mCompareTableWidget);
        tabCompare->layout()->addWidget(mCompareTreeWidget);
    }

    mSearchLineEdit->setFocus();
}

QSize S2Plugin::AbstractDatabaseView::minimumSizeHint() const
{
    return QSize(150, 150);
}

void S2Plugin::AbstractDatabaseView::searchFieldReturnPressed()
{
    auto text = mSearchLineEdit->text();
    bool isNumeric = false;
    auto enteredID = text.toUInt(&isNumeric);
    if (isNumeric)
    {
        if (enteredID > highestRecordID()) // exceptions can be dealt in showID
            return;

        showID(enteredID);
    }
    else // maybe should not do this else part at all and just handle it in searchFieldCompleterActivated
    {
        auto cID = getIDForName(std::move(text));
        if (cID.has_value())
            showID(cID.value());
    }
}

void S2Plugin::AbstractDatabaseView::fieldUpdated(int row, QStandardItem* parent)
{
    if (parent != nullptr) // special case: for flag field need to update it's parent, not the flag field
    {
        auto model = qobject_cast<QStandardItemModel*>(mMainTreeView->model());
        auto parentIndex = parent->index();
        auto index = model->index(row, gsColField, parentIndex);
        if (model->data(index, gsRoleType).value<MemoryFieldType>() == MemoryFieldType::Flag)
        {
            mMainTreeView->updateRow(parentIndex.row(), std::nullopt, std::nullopt, parent->parent(), true);
            return;
        }
    }
    mMainTreeView->updateRow(row, std::nullopt, std::nullopt, parent, true);
}

void S2Plugin::AbstractDatabaseView::fieldExpanded(const QModelIndex& index)
{
    auto model = qobject_cast<QStandardItemModel*>(mMainTreeView->model());
    mMainTreeView->updateRow(index.row(), std::nullopt, std::nullopt, model->itemFromIndex(index.parent()), true);
}

void S2Plugin::AbstractDatabaseView::compareGroupByCheckBoxClicked(int state)
{
    mCompareTableWidget->setHidden(state == Qt::Checked);
    mCompareTreeWidget->setHidden(state == Qt::Unchecked);
}

void S2Plugin::AbstractDatabaseView::comparisonFieldChosen()
{
    mFieldChosen = true;
    mCompareTableWidget->clearContents();
    mCompareTreeWidget->clear();

    auto comboIndex = mCompareFieldComboBox->currentIndex();
    if (comboIndex == 0)
        return;

    auto comboBoxData = mCompareFieldComboBox->currentData();
    if (!comboBoxData.isValid())
        return;

    auto type = comboBoxData.value<ComparisonField>().type;
    uint8_t flagsCount = 0;
    if (type == MemoryFieldType::Flags8)
        flagsCount = 8;
    else if (type == MemoryFieldType::Flags16)
        flagsCount = 16;
    else if (type == MemoryFieldType::Flags32)
        flagsCount = 32;

    if (flagsCount != 0)
    {
        mCompareFlagComboBox->clear();
        mCompareFlagComboBox->addItem("");
        std::string refName = comboBoxData.value<ComparisonField>().refName;
        for (uint8_t x = 1; x <= flagsCount; ++x)
        {
            auto flagName = Configuration::get()->flagTitle(refName, x);
            // TODO: don't show unknown unless it was chosen in settings
            QString realFlagName = QString("%1: ").arg(x) + QString::fromStdString(flagName.empty() ? Configuration::get()->flagTitle("unknown", x) : flagName);
            mCompareFlagComboBox->addItem(realFlagName, x - 1);
        }
        mCompareFlagComboBox->show();
    }
    else
    {
        mCompareFlagComboBox->clear();
        mCompareFlagComboBox->hide();
    }

    populateComparisonTableWidget(comboBoxData);
    populateComparisonTreeWidget(comboBoxData);
    mFieldChosen = false;
}

void S2Plugin::AbstractDatabaseView::comparisonFlagChosen(const QString& text)
{
    // protect against infinite loops since this slot is called when adding first element to combo box
    if (mFieldChosen)
        return;

    mCompareTableWidget->clearContents();
    mCompareTreeWidget->clear();

    if (text.isEmpty())
    {
        auto comboBoxData = mCompareFieldComboBox->currentData();
        if (!comboBoxData.isValid())
            return;

        populateComparisonTableWidget(comboBoxData);
        populateComparisonTreeWidget(comboBoxData);

        return;
    }

    auto fieldData = mCompareFieldComboBox->currentData();
    if (!fieldData.isValid())
        return;
    auto flagData = mCompareFlagComboBox->currentData();
    if (!flagData.isValid())
        return;

    ComparisonField comparisonData = fieldData.value<ComparisonField>();
    comparisonData.type = MemoryFieldType::Flag;
    comparisonData.flagIndex = flagData.value<uint8_t>();
    auto newData = QVariant::fromValue(comparisonData);

    populateComparisonTableWidget(newData);
    populateComparisonTreeWidget(newData);
}

void S2Plugin::AbstractDatabaseView::populateComparisonTableWidget(const QVariant& fieldData)
{
    mCompareTableWidget->setSortingEnabled(false);

    int row = 0;
    for (ID_type x = 0; x <= highestRecordID(); ++x)
    {
        if (!isValidRecordID(x))
            continue;

        auto item0 = new QTableWidgetItem(QString::asprintf("%03d", x));
        item0->setTextAlignment(Qt::AlignCenter);
        mCompareTableWidget->setItem(row, 0, item0);
        const auto name = recordNameForID(x);
        mCompareTableWidget->setItem(row, 1, new QTableWidgetItem(QString("<font color='blue'><u>%1</u></font>").arg(name)));

        auto [caption, value] = valueForField(fieldData, addressOfRecordID(x));
        auto item = new TableWidgetItemNumeric(caption);
        item->setData(Qt::UserRole, value);
        mCompareTableWidget->setItem(row, 2, item);

        row++;
    }
    mCompareTableWidget->setSortingEnabled(true);
    mCompareTableWidget->sortItems(0);
}

void S2Plugin::AbstractDatabaseView::populateComparisonTreeWidget(const QVariant& fieldData)
{
    mCompareTreeWidget->setSortingEnabled(false);

    std::unordered_map<QString, QVariant> rootValues;
    std::unordered_map<QString, std::vector<ID_type>> groupedValues; // valueString -> vector<id's>
    for (ID_type x = 0; x <= highestRecordID(); ++x)
    {
        if (!isValidRecordID(x))
            continue;

        auto [caption, value] = valueForField(fieldData, addressOfRecordID(x));
        rootValues[caption] = value;

        if (auto it = groupedValues.find(caption); it != groupedValues.end())
        {
            it->second.push_back(x);
        }
        else
        {
            groupedValues[caption] = {x};
        }
    }

    for (const auto& [groupString, IDList] : groupedValues)
    {
        auto rootItem = new TreeWidgetItemNumeric(mCompareTreeWidget, QStringList(groupString));
        rootItem->setData(0, Qt::UserRole, rootValues.at(groupString));
        mCompareTreeWidget->insertTopLevelItem(0, rootItem);
        for (const auto& recordID : IDList)
        {
            auto name = recordNameForID(recordID);
            auto caption = QString("<font color='blue'><u>%1</u></font>").arg(name);
            auto childItem = new TreeWidgetItemNumeric(rootItem, QStringList(caption));
            childItem->setData(0, Qt::UserRole, recordID);
            mCompareTreeWidget->insertTopLevelItem(0, childItem);
        }
        rootItem->sortChildren(0, Qt::AscendingOrder);
    }
    mCompareTreeWidget->setSortingEnabled(true);
    mCompareTreeWidget->sortItems(0, Qt::AscendingOrder);
}

void S2Plugin::AbstractDatabaseView::comparisonCellClicked(int row, int column)
{
    if (column == 1)
    {
        mSearchLineEdit->clear();
        auto clickedID = mCompareTableWidget->item(row, 0)->data(Qt::DisplayRole).toUInt();
        showID(clickedID);
    }
}

void S2Plugin::AbstractDatabaseView::groupedComparisonItemClicked(QTreeWidgetItem* item)
{
    if (item->childCount() == 0)
    {
        mSearchLineEdit->clear();
        showID(item->data(0, Qt::UserRole).toUInt());
    }
}

size_t S2Plugin::AbstractDatabaseView::populateComparisonComboBox(const std::vector<MemoryField>& fields, size_t offset, std::string prefix)
{
    for (const auto& field : fields)
    {
        if (field.isPointer)
        {
            // we don't do pointers as i don't think there are any important ones for comparison in databases

            offset += sizeof(uintptr_t);
            continue;
        }
        switch (field.type)
        {
            case MemoryFieldType::Skip:
                break;
            case MemoryFieldType::Flags32:
            case MemoryFieldType::Flags16:
            case MemoryFieldType::Flags8:
            {
                ComparisonField parentFlag;
                parentFlag.type = field.type;
                parentFlag.offset = offset;
                parentFlag.refName = field.firstParameterType;
                mCompareFieldComboBox->addItem(QString::fromStdString(prefix + field.name), QVariant::fromValue(parentFlag));
                break;
            }
            case MemoryFieldType::DefaultStructType:
            {
                offset = populateComparisonComboBox(Configuration::get()->typeFieldsOfDefaultStruct(field.jsonName), offset, prefix + field.name + ".");
                continue;
            }
            default:
            {
                ComparisonField tmp;
                tmp.offset = offset;
                tmp.type = field.type;
                mCompareFieldComboBox->addItem(QString::fromStdString(prefix + field.name), QVariant::fromValue(tmp));
                break;
            }
        }
        offset += field.get_size();
    }
    return offset;
}

std::pair<QString, QVariant> S2Plugin::AbstractDatabaseView::valueForField(const QVariant& data, uintptr_t addr)
{
    ComparisonField compData = qvariant_cast<ComparisonField>(data);

    auto offset = addr + compData.offset;
    switch (compData.type)
    {
        // we only handle types that occur in the DB's
        case MemoryFieldType::Byte:
        case MemoryFieldType::State8:
        {
            int8_t value = static_cast<int8_t>(Script::Memory::ReadByte(offset));
            return std::make_pair(QString::asprintf("%d", value), QVariant::fromValue(value));
        }
        case MemoryFieldType::CharacterDBID:
        case MemoryFieldType::UnsignedByte:
        case MemoryFieldType::Flags8:
        {
            uint8_t value = Script::Memory::ReadByte(offset);
            return std::make_pair(QString::asprintf("%u", value), QVariant::fromValue(value));
        }
        case MemoryFieldType::Word:
        case MemoryFieldType::State16:
        {
            int16_t value = static_cast<int16_t>(Script::Memory::ReadWord(offset));
            return std::make_pair(QString::asprintf("%d", value), QVariant::fromValue(value));
        }
        case MemoryFieldType::UnsignedWord:
        case MemoryFieldType::Flags16:
        {
            uint16_t value = Script::Memory::ReadWord(offset);
            return std::make_pair(QString::asprintf("%u", value), QVariant::fromValue(value));
        }
        case MemoryFieldType::TextureDBID:
        case MemoryFieldType::Dword:
        case MemoryFieldType::State32:
        {
            int32_t value = static_cast<int32_t>(Script::Memory::ReadDword(offset));
            return std::make_pair(QString::asprintf("%ld", value), QVariant::fromValue(value));
        }
        case MemoryFieldType::ParticleDBID:
        case MemoryFieldType::EntityDBID:
        case MemoryFieldType::StringsTableID:
        case MemoryFieldType::UnsignedDword:
        case MemoryFieldType::Flags32:
        {
            uint32_t value = Script::Memory::ReadDword(offset);
            return std::make_pair(QString::asprintf("%lu", value), QVariant::fromValue(value));
        }
        case MemoryFieldType::Qword:
        {
            int64_t value = static_cast<int64_t>(Script::Memory::ReadQword(offset));
            return std::make_pair(QString::asprintf("%lld", value), QVariant::fromValue(value));
        }
        case MemoryFieldType::UnsignedQword:
        {
            uint64_t value = Script::Memory::ReadQword(offset);
            return std::make_pair(QString::asprintf("%llu", value), QVariant::fromValue(value));
        }
        case MemoryFieldType::Float:
        {
            uint32_t dword = Script::Memory::ReadDword(offset);
            float value = reinterpret_cast<float&>(dword);
            return std::make_pair(QString::asprintf("%f", value), QVariant::fromValue(value));
        }
        case MemoryFieldType::Double:
        {
            size_t qword = Script::Memory::ReadQword(offset);
            double value = reinterpret_cast<double&>(qword);
            return std::make_pair(QString::asprintf("%lf", value), QVariant::fromValue(value));
        }
        case MemoryFieldType::Bool:
        {
            auto b = Script::Memory::ReadByte(offset);
            bool value = b != 0;
            return std::make_pair(value ? "True" : "False", QVariant::fromValue(b));
        }
        case MemoryFieldType::Flag:
        {
            uint8_t flagToCheck = compData.flagIndex;
            bool isFlagSet = false;
            if (flagToCheck > 15)
                isFlagSet = ((Script::Memory::ReadDword(offset) & (1 << flagToCheck)) != 0);
            else if (flagToCheck > 7)
                isFlagSet = ((Script::Memory::ReadWord(offset) & (1 << flagToCheck)) != 0);
            else
                isFlagSet = ((Script::Memory::ReadByte(offset) & (1 << flagToCheck)) != 0);

            return std::make_pair(isFlagSet ? "True" : "False", QVariant::fromValue(isFlagSet));
        }
    }
    return std::make_pair("unknown", 0);
}
