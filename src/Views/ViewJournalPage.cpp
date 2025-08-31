#include "Views/ViewJournalPage.h"

#include "Configuration.h"
#include "JsonNameDefinitions.h"
#include "QtHelpers/QStrFromStringView.h"
#include "QtHelpers/TreeViewMemoryFields.h"
#include "QtHelpers/WidgetAutorefresh.h"
#include "QtPlugin.h"
#include "Views/ViewCpp.h"
#include "Views/ViewToolbar.h"
#include <QComboBox>
#include <QHeaderView>
#include <QLabel>
#include <QMenu>
#include <QPushButton>
#include <QVBoxLayout>

S2Plugin::ViewJournalPage::ViewJournalPage(uintptr_t address, QWidget* parent) : QWidget(parent), mPageAddress(address)
{
    setWindowIcon(getCavemanIcon());
    setWindowTitle(QStrFromStringView(JsonName::JournalPage));

    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setMargin(5);
    auto refreshLayout = new QHBoxLayout();
    mainLayout->addLayout(refreshLayout);

    auto autoRefresh = new WidgetAutorefresh(100, this);
    QObject::connect(autoRefresh, &WidgetAutorefresh::refresh, this, &ViewJournalPage::refreshJournalPage);
    refreshLayout->addWidget(autoRefresh);

    refreshLayout->addStretch();
    auto labelButton = new QPushButton("Label", this);
    QObject::connect(labelButton, &QPushButton::clicked, this, &ViewJournalPage::label);
    refreshLayout->addWidget(labelButton);
    refreshLayout->addWidget(new QLabel("Interpret as:", this));

    mInterpretAsComboBox = new QComboBox(this);

    auto config = Configuration::get();
    for (auto& pageName : config->getJournalPageNames())
        mInterpretAsComboBox->addItem(QString::fromStdString(pageName));

    QObject::connect(mInterpretAsComboBox, &QComboBox::currentTextChanged, this, &ViewJournalPage::interpretAsChanged);
    refreshLayout->addWidget(mInterpretAsComboBox);

    mMainTreeView = new TreeViewMemoryFields(this);
    mainLayout->addWidget(mMainTreeView);
    mMainTreeView->mActiveColumns.disable(gsColComparisonValue).disable(gsColComparisonValueHex);
    mMainTreeView->updateTableHeader(false);
    mMainTreeView->setColumnWidth(gsColValue, 250);
    mMainTreeView->setColumnWidth(gsColField, 125);
    mMainTreeView->setColumnWidth(gsColValueHex, 125);
    mMainTreeView->setColumnWidth(gsColMemoryAddress, 120);
    mMainTreeView->setColumnWidth(gsColMemoryAddressDelta, 75);
    mMainTreeView->setColumnWidth(gsColType, 100);
    mMainTreeView->addMemoryFields(config->typeFieldsOfDefaultStruct(std::string(JsonName::JournalPage)), std::string(JsonName::JournalPage), mPageAddress);
    mMainTreeView->updateTree(0, 0, true);
    autoRefresh->toggleAutoRefresh(true);
    QObject::connect(mMainTreeView, &TreeViewMemoryFields::onContextMenu, this, &ViewJournalPage::viewContextMenu);
}

void S2Plugin::ViewJournalPage::refreshJournalPage()
{
    mMainTreeView->updateTree();
}

QSize S2Plugin::ViewJournalPage::sizeHint() const
{
    return QSize(750, 750);
}

QSize S2Plugin::ViewJournalPage::minimumSizeHint() const
{
    return QSize(150, 150);
}

void S2Plugin::ViewJournalPage::label()
{
    mMainTreeView->labelAll();
}

void S2Plugin::ViewJournalPage::interpretAsChanged(const QString& text)
{
    if (!text.isEmpty())
    {
        auto pageType = text.toStdString();
        mMainTreeView->clear();
        static std::vector<MemoryField> structs;
        if (structs.empty())
        {
            structs.resize(2);
            structs[0].type = MemoryFieldType::DefaultStructType;
            structs[0].jsonName = JsonName::JournalPage;
            structs[0].name = JsonName::JournalPage;
            structs[1].type = MemoryFieldType::DefaultStructType;
        }
        if (text == QStrFromStringView(JsonName::JournalPage))
        {
            mMainTreeView->addMemoryFields({structs[0]}, pageType, mPageAddress);
        }
        else
        {
            structs[1].name = pageType;
            structs[1].jsonName = pageType;
            mMainTreeView->addMemoryFields(structs, pageType, mPageAddress);
        }
        mMainTreeView->expandLast();
        mMainTreeView->updateTableHeader();
        mMainTreeView->updateTree(0, 0, true);
    }
}

void S2Plugin::ViewJournalPage::viewContextMenu(QMenu* menu)
{
    auto action = menu->addAction("View Code");
    QObject::connect(action, &QAction::triggered, menu,
                     [this]()
                     {
                         auto type = mInterpretAsComboBox->currentText().toStdString();
                         auto codeWindow = getToolbar()->showCode(type);
                         if (type != JsonName::JournalPage)
                             codeWindow->addDependency(JsonName::JournalPage);
                     });
}
