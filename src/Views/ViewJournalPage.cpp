#include "Views/ViewJournalPage.h"

#include "Configuration.h"
#include "QtHelpers/TreeViewMemoryFields.h"
#include "QtHelpers/WidgetAutorefresh.h"
#include "QtPlugin.h"
#include <QComboBox>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

S2Plugin::ViewJournalPage::ViewJournalPage(uintptr_t address, QWidget* parent) : QWidget(parent), mPageAddress(address)
{
    setWindowIcon(getCavemanIcon());
    setWindowTitle("JournalPage");

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

    auto interpretAsComboBox = new QComboBox(this);

    auto config = Configuration::get();
    for (auto& pageName : config->getJournalPageNames())
        interpretAsComboBox->addItem(QString::fromStdString(pageName));

    QObject::connect(interpretAsComboBox, &QComboBox::currentTextChanged, this, &ViewJournalPage::interpretAsChanged);
    refreshLayout->addWidget(interpretAsComboBox);

    mMainTreeView = new TreeViewMemoryFields(this);
    mMainTreeView->addMemoryFields(config->typeFieldsOfDefaultStruct("JournalPage"), "JournalPage", mPageAddress);
    mMainTreeView->activeColumns.disable(gsColComparisonValue).disable(gsColComparisonValueHex);
    mainLayout->addWidget(mMainTreeView);

    mMainTreeView->setColumnWidth(gsColValue, 250);
    mMainTreeView->setColumnWidth(gsColField, 125);
    mMainTreeView->setColumnWidth(gsColValueHex, 125);
    mMainTreeView->setColumnWidth(gsColMemoryAddress, 125);
    mMainTreeView->setColumnWidth(gsColMemoryAddressDelta, 75);
    mMainTreeView->setColumnWidth(gsColType, 100);
    mMainTreeView->updateTableHeader();
    mMainTreeView->updateTree(0, 0, true);
    autoRefresh->toggleAutoRefresh(true);
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
            structs[0].jsonName = "JournalPage";
            structs[0].name = "JournalPage";
            structs[1].type = MemoryFieldType::DefaultStructType;
        }
        if (text == "JournalPage")
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
        mMainTreeView->setColumnWidth(gsColValue, 250);
        mMainTreeView->updateTableHeader();
        mMainTreeView->updateTree(0, 0, true);
    }
}
