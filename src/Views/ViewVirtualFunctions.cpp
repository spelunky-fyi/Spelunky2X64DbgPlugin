#include "Views/ViewVirtualFunctions.h"

#include "QtHelpers/ItemModelVirtualFunctions.h"
#include "QtHelpers/QStrFromStringView.h"
#include "QtHelpers/StyledItemDelegateHTML.h"
#include "QtPlugin.h"
#include "pluginmain.h"
#include <QHeaderView> // for horizontalHeader
#include <QLabel>
#include <QLineEdit>
#include <QModelIndex>
#include <QPushButton>
#include <QTableView>
#include <QVBoxLayout>

S2Plugin::ViewVirtualFunctions::ViewVirtualFunctions(uintptr_t address, std::string_view typeName, QWidget* parent) : QWidget(parent), mMemoryAddress(address)
{
    setWindowIcon(getCavemanIcon());
    setWindowTitle(QString("Virtual Functions of %1").arg(QStrFromStringView(typeName)));

    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setMargin(5);
    auto topLayout = new QHBoxLayout();
    mainLayout->addLayout(topLayout);

    topLayout->addWidget(new QLabel("Jump to function at index:", this));

    mJumpToLineEdit = new QLineEdit(this);
    mJumpToLineEdit->setValidator(new QIntValidator(0, 5000, this));
    mJumpToLineEdit->setFixedWidth(100);
    topLayout->addWidget(mJumpToLineEdit);

    auto jumpBtn = new QPushButton("Jump", this);
    QObject::connect(jumpBtn, &QPushButton::clicked, this, &ViewVirtualFunctions::jumpToFunction);
    topLayout->addWidget(jumpBtn);
    topLayout->addStretch();

    auto HTMLDelegate = new StyledItemDelegateHTML(this);
    HTMLDelegate->setCenterVertically(true);

    auto functionsTable = new QTableView(this);
    auto model = new ItemModelVirtualFunctions(typeName, mMemoryAddress, functionsTable);
    functionsTable->setModel(model);
    functionsTable->setAlternatingRowColors(true);
    functionsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    functionsTable->horizontalHeader()->setStretchLastSection(true);
    functionsTable->setItemDelegate(HTMLDelegate);
    functionsTable->setColumnWidth(ItemModelVirtualFunctions::Index, 30);
    functionsTable->setColumnWidth(ItemModelVirtualFunctions::Offset, 50);
    functionsTable->setColumnWidth(ItemModelVirtualFunctions::TableAddress, 130);
    functionsTable->setColumnWidth(ItemModelVirtualFunctions::FunctionAddress, 130);
    functionsTable->setColumnWidth(ItemModelVirtualFunctions::Signature, 300);
    mainLayout->addWidget(functionsTable);

    QObject::connect(functionsTable, &QTableView::clicked, this, &ViewVirtualFunctions::tableEntryClicked);
}

QSize S2Plugin::ViewVirtualFunctions::sizeHint() const
{
    return QSize(650, 450);
}

QSize S2Plugin::ViewVirtualFunctions::minimumSizeHint() const
{
    return QSize(150, 150);
}

void S2Plugin::ViewVirtualFunctions::tableEntryClicked(const QModelIndex& index)
{
    switch (index.column())
    {
        case ItemModelVirtualFunctions::Index:
        case ItemModelVirtualFunctions::Signature:
        case ItemModelVirtualFunctions::Comment:
        case ItemModelVirtualFunctions::Offset:
            // nop
            break;
        case ItemModelVirtualFunctions::FunctionAddress:
        {
            auto address = index.data(gsRoleFunctionFunctionAddress).toULongLong();
            GuiDisasmAt(address, GetContextData(UE_CIP));
            GuiShowCpu();
            break;
        }
        case ItemModelVirtualFunctions::TableAddress:
        {
            auto address = index.data(gsRoleFunctionTableAddress).toULongLong();
            GuiDumpAt(address);
            GuiShowCpu();
            break;
        }
    }
}

void S2Plugin::ViewVirtualFunctions::jumpToFunction()
{
    auto address = Script::Memory::ReadQword(mMemoryAddress + (mJumpToLineEdit->text().toUInt() * 8ull));
    GuiDisasmAt(address, GetContextData(UE_CIP));
    GuiShowCpu();
}
