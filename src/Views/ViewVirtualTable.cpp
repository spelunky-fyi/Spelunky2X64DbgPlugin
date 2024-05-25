#include "Views/ViewVirtualTable.h"

#include "Data/VirtualTableLookup.h"
#include "QtHelpers/ItemModelGatherVirtualData.h"
#include "QtHelpers/ItemModelVirtualTable.h"
#include "QtHelpers/StyledItemDelegateHTML.h"
#include "QtHelpers/TableWidgetItemNumeric.h"
#include "QtPlugin.h"
#include "Spelunky2.h"
#include "pluginmain.h"
#include <QCheckBox>
#include <QClipBoard>
#include <QFileDialog>
#include <QGuiApplication>
#include <QHeaderView>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <fstream>

S2Plugin::ViewVirtualTable::ViewVirtualTable(QWidget* parent) : QWidget(parent)
{
    mModel = new ItemModelVirtualTable(this);
    mSortFilterProxy = new SortFilterProxyModelVirtualTable(this);
    mGatherModel = new ItemModelGatherVirtualData(this);
    mGatherSortFilterProxy = new SortFilterProxyModelGatherVirtualData(this);
    mGatherSortFilterProxy->sort(gsColGatherID);

    setWindowIcon(getCavemanIcon());
    setWindowTitle("Virtual Table");
    initializeUI();
}

enum TABS
{
    DATA = 0,
    LOOKUP = 1,
    GATHER = 2,
};

void S2Plugin::ViewVirtualTable::initializeUI()
{
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setMargin(5);

    mMainTabWidget = new QTabWidget();
    mMainTabWidget->setDocumentMode(false);
    mainLayout->addWidget(mMainTabWidget);

    auto tabData = new QWidget();
    tabData->setLayout(new QVBoxLayout());
    tabData->layout()->setMargin(10);
    tabData->setObjectName("datawidget");
    tabData->setStyleSheet("QWidget#datawidget {border: 1px solid #999;}");

    auto tabLookup = new QWidget();
    tabLookup->setLayout(new QVBoxLayout());
    tabLookup->layout()->setMargin(10);
    tabLookup->setObjectName("lookupwidget");
    tabLookup->setStyleSheet("QWidget#lookupwidget {border: 1px solid #999;}");

    auto tabGather = new QWidget();
    tabGather->setLayout(new QVBoxLayout());
    tabGather->layout()->setMargin(10);
    tabGather->setObjectName("gatherwidget");
    tabGather->setStyleSheet("QWidget#gatherwidget {border: 1px solid #999;}");

    mMainTabWidget->addTab(tabData, "Data");
    mMainTabWidget->addTab(tabLookup, "Lookup");
    mMainTabWidget->addTab(tabGather, "Gather");

    // TAB DATA
    {
        auto topLayout = new QGridLayout(this);

        auto detectEntitiesBtn = new QPushButton("Detect entities", this);
        QObject::connect(detectEntitiesBtn, &QPushButton::clicked, this, &ViewVirtualTable::detectEntities);
        topLayout->addWidget(detectEntitiesBtn, 0, 0);

        auto showImportedSymbolsCheckBox = new QCheckBox("Show imported symbols", this);
        showImportedSymbolsCheckBox->setCheckState(Qt::Checked);
        QObject::connect(showImportedSymbolsCheckBox, &QCheckBox::stateChanged, this, &ViewVirtualTable::showImportedSymbolsCheckBoxStateChanged);
        topLayout->addWidget(showImportedSymbolsCheckBox, 0, 1);

        auto showNonAddressEntriesCheckBox = new QCheckBox("Show non-address entries", this);
        showNonAddressEntriesCheckBox->setCheckState(Qt::Checked);
        QObject::connect(showNonAddressEntriesCheckBox, &QCheckBox::stateChanged, this, &ViewVirtualTable::showNonAddressEntriesCheckBoxStateChanged);
        topLayout->addWidget(showNonAddressEntriesCheckBox, 0, 2);

        auto showSymbollessEntriesCheckBox = new QCheckBox("Show symbol-less entries", this);
        showSymbollessEntriesCheckBox->setCheckState(Qt::Checked);
        QObject::connect(showSymbollessEntriesCheckBox, &QCheckBox::stateChanged, this, &ViewVirtualTable::showSymbollessEntriesCheckBoxStateChanged);
        topLayout->addWidget(showSymbollessEntriesCheckBox, 0, 3);

        auto searchFilterLineEdit = new QLineEdit(this);
        searchFilterLineEdit->setPlaceholderText("Search symbol name");
        QObject::connect(searchFilterLineEdit, &QLineEdit::textChanged, this, &ViewVirtualTable::filterTextChanged);
        topLayout->addWidget(searchFilterLineEdit, 1, 1, 1, 3);

        auto tmpLayout = new QHBoxLayout(this);
        tmpLayout->addLayout(topLayout);
        tmpLayout->addStretch();
        dynamic_cast<QVBoxLayout*>(tabData->layout())->addLayout(tmpLayout);

        mDataTable = new QTableView(this);
        mSortFilterProxy->setSourceModel(mModel);
        mDataTable->setModel(mSortFilterProxy);
        mDataTable->setAlternatingRowColors(true);
        mDataTable->setSelectionBehavior(QAbstractItemView::SelectRows);
        mDataTable->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
        mDataTable->verticalHeader()->setDefaultSectionSize(19);
        mDataTable->verticalHeader()->setVisible(false);
        mDataTable->setItemDelegate(new StyledItemDelegateHTML(this));
        mDataTable->setColumnWidth(gsColTableOffset, 100);
        mDataTable->setColumnWidth(gsColCodeAddress, 125);
        mDataTable->setColumnWidth(gsColTableAddress, 125);
        mDataTable->horizontalHeader()->setStretchLastSection(true);

        QObject::connect(mDataTable, &QTableView::clicked, this, &ViewVirtualTable::tableEntryClicked);

        dynamic_cast<QVBoxLayout*>(tabData->layout())->addWidget(mDataTable);
    }

    // TAB LOOKUP
    {
        auto topLayout = new QHBoxLayout(this);
        mLookupAddressLineEdit = new QLineEdit(this);
        mLookupAddressLineEdit->setPlaceholderText("Lookup address");
        topLayout->addWidget(mLookupAddressLineEdit);

        auto lookupBtn = new QPushButton("Lookup", this);
        QObject::connect(lookupBtn, &QPushButton::clicked, this, &ViewVirtualTable::processLookupAddressText);
        topLayout->addWidget(lookupBtn);

        topLayout->addStretch();

        dynamic_cast<QVBoxLayout*>(tabLookup->layout())->addLayout(topLayout);

        mLookupResultsTable = new QTableWidget(this);
        mLookupResultsTable->setAlternatingRowColors(true);
        mLookupResultsTable->verticalHeader()->setVisible(false);
        mLookupResultsTable->horizontalHeader()->setStretchLastSection(true);
        mLookupResultsTable->setColumnCount(3);
        mLookupResultsTable->setHorizontalHeaderLabels(QStringList() << "Base offset"
                                                                     << "Name"
                                                                     << "Relative offset");
        mLookupResultsTable->setColumnWidth(0, 75);
        mLookupResultsTable->setColumnWidth(1, 325);
        mLookupResultsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

        dynamic_cast<QVBoxLayout*>(tabLookup->layout())->addWidget(mLookupResultsTable);
    }

    // TAB GATHER
    {
        auto topLayout = new QHBoxLayout(this);

        auto gatherEntitiesBtn = new QPushButton("Gather entities", this);
        QObject::connect(gatherEntitiesBtn, &QPushButton::clicked, this, &ViewVirtualTable::gatherEntities);
        topLayout->addWidget(gatherEntitiesBtn);

        auto gatherExtraObjectsBtn = new QPushButton("Gather extra", this);
        QObject::connect(gatherExtraObjectsBtn, &QPushButton::clicked, this, &ViewVirtualTable::gatherExtraObjects);
        topLayout->addWidget(gatherExtraObjectsBtn);

        auto gatherVirtsBtn = new QPushButton("Gather virts", this);
        QObject::connect(gatherVirtsBtn, &QPushButton::clicked, this, &ViewVirtualTable::gatherAvailableVirtuals);
        topLayout->addWidget(gatherVirtsBtn);

        mGatherProgressLabel = new QLabel("", this);
        topLayout->addWidget(mGatherProgressLabel);

        auto hideCompletedCheckbox = new QCheckBox("Hide completed", this);
        QObject::connect(hideCompletedCheckbox, &QCheckBox::stateChanged, this, &ViewVirtualTable::showGatherHideCompletedCheckBoxStateChanged);
        topLayout->addWidget(hideCompletedCheckbox);

        topLayout->addStretch();

        auto exportJSONBtn = new QPushButton("Export JSON", this);
        QObject::connect(exportJSONBtn, &QPushButton::clicked, this, &ViewVirtualTable::exportGatheredData);
        topLayout->addWidget(exportJSONBtn);

        auto exportVirtTableBtn = new QPushButton("Export Virt Table", this);
        QObject::connect(exportVirtTableBtn, &QPushButton::clicked, this, &ViewVirtualTable::exportVirtTable);
        topLayout->addWidget(exportVirtTableBtn);

        auto exportCppEnumBtn = new QPushButton("Export C++ enum", this);
        QObject::connect(exportCppEnumBtn, &QPushButton::clicked, this, &ViewVirtualTable::exportCppEnum);
        topLayout->addWidget(exportCppEnumBtn);

        dynamic_cast<QVBoxLayout*>(tabGather->layout())->addLayout(topLayout);

        auto gatherTable = new QTableView(this);
        mGatherSortFilterProxy->setSourceModel(mGatherModel);
        gatherTable->setModel(mGatherSortFilterProxy);
        gatherTable->setAlternatingRowColors(true);
        gatherTable->setSelectionBehavior(QAbstractItemView::SelectRows);
        gatherTable->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
        gatherTable->verticalHeader()->setDefaultSectionSize(19);
        gatherTable->verticalHeader()->setVisible(false);
        gatherTable->setItemDelegate(new StyledItemDelegateHTML(this));
        gatherTable->setColumnWidth(gsColGatherID, 50);
        gatherTable->setColumnWidth(gsColGatherName, 200);
        gatherTable->setColumnWidth(gsColGatherVirtualTableOffset, 125);
        gatherTable->setColumnWidth(gsColGatherCollision1Present, 75);
        gatherTable->setColumnWidth(gsColGatherCollision2Present, 75);
        gatherTable->horizontalHeader()->setStretchLastSection(true);

        dynamic_cast<QVBoxLayout*>(tabGather->layout())->addWidget(gatherTable);
        updateGatherProgress();
    }
}

QSize S2Plugin::ViewVirtualTable::sizeHint() const
{
    return QSize(800, 650);
}

QSize S2Plugin::ViewVirtualTable::minimumSizeHint() const
{
    return QSize(150, 150);
}

void S2Plugin::ViewVirtualTable::tableEntryClicked(const QModelIndex& index)
{
    auto mappedIndex = mSortFilterProxy->mapToSource(index);
    auto offset = mappedIndex.row();
    const auto& entry = Spelunky2::get()->get_VirtualTableLookup().entryForOffset(offset);
    auto column = mappedIndex.column();
    switch (column)
    {
        case gsColTableOffset:
        case gsColSymbolName:
            // nop
            break;
        case gsColCodeAddress:
            if (entry.isValidAddress)
            {
                GuiDisasmAt(entry.value, GetContextData(UE_CIP));
                GuiShowCpu();
            }
            break;
        case gsColTableAddress:
            GuiDumpAt(Spelunky2::get()->get_VirtualTableLookup().tableAddressForEntry(entry));
            GuiShowCpu();
            break;
    }
}

void S2Plugin::ViewVirtualTable::detectEntities()
{
    mModel->detectEntities();
}

void S2Plugin::ViewVirtualTable::showImportedSymbolsCheckBoxStateChanged(int state)
{
    mSortFilterProxy->setShowImportedSymbols(state == Qt::Checked);
}

void S2Plugin::ViewVirtualTable::showNonAddressEntriesCheckBoxStateChanged(int state)
{
    mSortFilterProxy->setShowNonAddressEntries(state == Qt::Checked);
}

void S2Plugin::ViewVirtualTable::showSymbollessEntriesCheckBoxStateChanged(int state)
{
    mSortFilterProxy->setShowSymbollessEntries(state == Qt::Checked);
}

void S2Plugin::ViewVirtualTable::filterTextChanged(const QString& text)
{
    mSortFilterProxy->setFilterString(text);
    if (mSortFilterProxy->symbollessEntriesShown())
    {
        for (auto x = 0; x < mSortFilterProxy->rowCount(); ++x)
        {
            auto indexData = mSortFilterProxy->index(x, gsColSymbolName).data(Qt::DisplayRole);
            if (indexData.toString().contains(text, Qt::CaseInsensitive))
            {
                mDataTable->selectRow(x);
                break;
            }
        }
    }
}

void S2Plugin::ViewVirtualTable::processLookupAddressText()
{
    auto enteredAddress = mLookupAddressLineEdit->text();
    bool conversionOK = false;
    size_t address = enteredAddress.toULongLong(&conversionOK, 16);
    if (conversionOK)
    {
        lookupAddress(address);
    }
}

void S2Plugin::ViewVirtualTable::showLookupAddress(size_t address)
{
    mMainTabWidget->setCurrentIndex(TABS::LOOKUP);
    mLookupAddressLineEdit->setText(QString::asprintf("%016llX", address));
    lookupAddress(address);
}

void S2Plugin::ViewVirtualTable::lookupAddress(size_t address)
{
    mLookupResultsTable->setSortingEnabled(false);
    mLookupResultsTable->clearContents();
    std::vector<std::vector<QTableWidgetItem*>> items;
    auto& vtl = Spelunky2::get()->get_VirtualTableLookup();
    auto tableOffsets = vtl.tableOffsetForFunctionAddress(address);

    if (tableOffsets.size() == 0)
    {
        mLookupResultsTable->setRowCount(1);
        mLookupResultsTable->setItem(0, 1, new QTableWidgetItem("Address not found in virtual table"));
    }
    else
    {
        for (const auto& tableOffset : tableOffsets)
        {
            auto precedingEntry = vtl.findPrecedingEntryWithSymbols(tableOffset);
            if (precedingEntry.symbols.size() > 0)
            {
                for (const auto& symbol : precedingEntry.symbols)
                {
                    std::vector<QTableWidgetItem*> tmp;
                    tmp.emplace_back(new QTableWidgetItem(QString::fromStdString(std::to_string(precedingEntry.offset))));
                    tmp.emplace_back(new QTableWidgetItem(QString::fromStdString(symbol)));
                    auto item = new TableWidgetItemNumeric(QString::fromStdString("+" + std::to_string(tableOffset - precedingEntry.offset)));
                    item->setData(Qt::UserRole, tableOffset - precedingEntry.offset);
                    tmp.emplace_back(item);
                    items.emplace_back(tmp);
                }
            }
        }

        if (items.size() == 0)
        {
            mLookupResultsTable->setRowCount(static_cast<int>(tableOffsets.size()));
            auto counter = 0;
            for (const auto& tableOffset : tableOffsets)
            {
                mLookupResultsTable->setItem(counter, 1, new QTableWidgetItem("No preceding functions found with a name"));
                mLookupResultsTable->setItem(counter++, 2, new TableWidgetItemNumeric(QString::fromStdString("+" + std::to_string(tableOffset))));
            }
        }
        else
        {
            mLookupResultsTable->setRowCount(static_cast<int>(items.size()));
            auto counter = 0;
            for (const auto& item : items)
            {
                mLookupResultsTable->setItem(counter, 0, item.at(0));
                mLookupResultsTable->setItem(counter, 1, item.at(1));
                mLookupResultsTable->setItem(counter++, 2, item.at(2));
            }
        }
    }
    mLookupResultsTable->setSortingEnabled(true);
    mLookupResultsTable->sortItems(2);
}

void S2Plugin::ViewVirtualTable::gatherEntities()
{
    mGatherModel->gatherEntities();
    updateGatherProgress();
}

void S2Plugin::ViewVirtualTable::gatherExtraObjects()
{
    mGatherModel->gatherExtraObjects();
    updateGatherProgress();
}

void S2Plugin::ViewVirtualTable::gatherAvailableVirtuals()
{
    mGatherModel->gatherAvailableVirtuals();
}

void S2Plugin::ViewVirtualTable::updateGatherProgress()
{
    auto progress = mGatherModel->completionPercentage();
    mGatherProgressLabel->setText(QString("%1% complete").arg(progress));
}

void S2Plugin::ViewVirtualTable::exportGatheredData()
{
    auto fileName = QFileDialog::getSaveFileName(this, "Save gathered data", "Spelunky2VirtualTableData.json", "JSON files (*.json)");
    if (!fileName.isEmpty())
    {
        try
        {
            auto fp = std::ofstream(fileName.toStdString());
            fp << mGatherModel->dumpJSON();
        }
        catch (...)
        {
            QMessageBox msgBox;
            msgBox.setIcon(QMessageBox::Critical);
            msgBox.setWindowIcon(getCavemanIcon());
            msgBox.setText("The file could not be written");
            msgBox.setWindowTitle("Spelunky2");
            msgBox.exec();
        }
    }
}

void S2Plugin::ViewVirtualTable::exportVirtTable()
{
    auto tbl = mGatherModel->dumpVirtTable();
    auto clipboard = QGuiApplication::clipboard();
    clipboard->setText(QString::fromStdString(tbl));

    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setWindowIcon(getCavemanIcon());
    msgBox.setText("The table was copied to the clipboard");
    msgBox.setWindowTitle("Spelunky2");
    msgBox.exec();
}

void S2Plugin::ViewVirtualTable::exportCppEnum()
{
    auto tbl = mGatherModel->dumpCppEnum();
    auto clipboard = QGuiApplication::clipboard();
    clipboard->setText(QString::fromStdString(tbl));

    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setWindowIcon(getCavemanIcon());
    msgBox.setText("The C++ enum was copied to the clipboard");
    msgBox.setWindowTitle("Spelunky2");
    msgBox.exec();
}

void S2Plugin::ViewVirtualTable::showGatherHideCompletedCheckBoxStateChanged(int state)
{
    mGatherSortFilterProxy->setHideCompleted(state == Qt::Checked);
}
