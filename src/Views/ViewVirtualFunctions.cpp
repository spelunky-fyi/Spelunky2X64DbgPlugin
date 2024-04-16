#include "Views/ViewVirtualFunctions.h"

#include "QtHelpers/ItemModelVirtualFunctions.h"
#include "QtHelpers/StyledItemDelegateHTML.h"
#include "QtPlugin.h"
#include "pluginmain.h"
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

S2Plugin::ViewVirtualFunctions::ViewVirtualFunctions(const std::string& typeName, size_t offset, QWidget* parent) : QWidget(parent), mTypeName(typeName), mMemoryOffset(offset)
{
    initializeUI();
    setWindowIcon(S2Plugin::getCavemanIcon());
    setWindowTitle(QString("Virtual Functions of %1").arg(QString::fromStdString(typeName)));
}

void S2Plugin::ViewVirtualFunctions::initializeUI()
{
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setMargin(5);
    setLayout(mainLayout);

    auto topLayout = new QHBoxLayout(this);
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

    auto model = new ItemModelVirtualFunctions(mTypeName, mMemoryOffset, this);
    auto sortFilterProxy = new SortFilterProxyModelVirtualFunctions(this);
    sortFilterProxy->setSourceModel(model);

    mFunctionsTable = new QTableView(this);
    mFunctionsTable->setModel(sortFilterProxy);
    mFunctionsTable->setAlternatingRowColors(true);
    mFunctionsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    mFunctionsTable->horizontalHeader()->setStretchLastSection(true);
    mFunctionsTable->setItemDelegate(HTMLDelegate);
    mFunctionsTable->setColumnWidth(gsColFunctionIndex, 50);
    mFunctionsTable->setColumnWidth(gsColFunctionTableAddress, 130);
    mFunctionsTable->setColumnWidth(gsColFunctionFunctionAddress, 130);
    sortFilterProxy->sort(0);
    mainLayout->addWidget(mFunctionsTable);

    QObject::connect(mFunctionsTable, &QTableView::clicked, this, &ViewVirtualFunctions::tableEntryClicked);
}

void S2Plugin::ViewVirtualFunctions::closeEvent(QCloseEvent* event)
{
    delete this;
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
    auto model = mFunctionsTable->model();
    switch (index.column())
    {
        case gsColFunctionIndex:
        case gsColFunctionSignature:
            // nop
            break;
        case gsColFunctionFunctionAddress:
        {
            auto address = model->data(index, gsRoleFunctionFunctionAddress).value<size_t>();
            GuiDisasmAt(address, GetContextData(UE_CIP));
            GuiShowCpu();
            break;
        }
        case gsColFunctionTableAddress:
        {
            auto address = model->data(index, gsRoleFunctionTableAddress).value<size_t>();
            GuiDumpAt(address);
            GuiShowCpu();
            break;
        }
    }
}

void S2Plugin::ViewVirtualFunctions::jumpToFunction(bool b)
{
    auto address = Script::Memory::ReadQword(mMemoryOffset + (mJumpToLineEdit->text().toUInt() * 8ull));
    GuiDisasmAt(address, GetContextData(UE_CIP));
    GuiShowCpu();
}
