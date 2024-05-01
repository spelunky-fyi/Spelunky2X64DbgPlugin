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
    initializeUI();
    setWindowIcon(getCavemanIcon());
    setWindowTitle("JournalPage");

    mMainTreeView->setColumnWidth(gsColField, 125);
    mMainTreeView->setColumnWidth(gsColValueHex, 125);
    mMainTreeView->setColumnWidth(gsColMemoryAddress, 125);
    mMainTreeView->setColumnWidth(gsColMemoryAddressDelta, 75);
    mMainTreeView->setColumnWidth(gsColType, 100);
}

void S2Plugin::ViewJournalPage::initializeUI()
{
    auto mainLayout = new QVBoxLayout(this);
    auto refreshLayout = new QHBoxLayout();
    mainLayout->addLayout(refreshLayout);

    auto autoRefresh = new WidgetAutorefresh("100", this);
    QObject::connect(autoRefresh, &WidgetAutorefresh::refresh, this, &ViewJournalPage::refreshJournalPage);
    refreshLayout->addWidget(autoRefresh);

    refreshLayout->addStretch();
    auto labelButton = new QPushButton("Label", this);
    QObject::connect(labelButton, &QPushButton::clicked, this, &ViewJournalPage::label);
    refreshLayout->addWidget(labelButton);
    refreshLayout->addWidget(new QLabel("Interpret as:", this));

    auto interpretAsComboBox = new QComboBox(this);
    // TODO get from json
    // also, guess page by the vtable maybe?
    interpretAsComboBox->addItem("JournalPage");
    interpretAsComboBox->addItem("JournalPageProgress");
    interpretAsComboBox->addItem("JournalPageJournalMenu");
    interpretAsComboBox->addItem("JournalPagePlaces");
    interpretAsComboBox->addItem("JournalPagePeople");
    interpretAsComboBox->addItem("JournalPageBestiary");
    interpretAsComboBox->addItem("JournalPageItems");
    interpretAsComboBox->addItem("JournalPageTraps");
    interpretAsComboBox->addItem("JournalPageStory");
    interpretAsComboBox->addItem("JournalPageFeats");
    interpretAsComboBox->addItem("JournalPageDeathCause");
    interpretAsComboBox->addItem("JournalPageDeathMenu");
    interpretAsComboBox->addItem("JournalPageRecap");
    interpretAsComboBox->addItem("JournalPagePlayerProfile");
    interpretAsComboBox->addItem("JournalPageLastGamePlayed");

    QObject::connect(interpretAsComboBox, &QComboBox::currentTextChanged, this, &ViewJournalPage::interpretAsChanged);
    refreshLayout->addWidget(interpretAsComboBox);

    mMainTreeView = new TreeViewMemoryFields(this);
    mMainTreeView->addMemoryFields(Configuration::get()->typeFieldsOfDefaultStruct("JournalPage"), "JournalPage", mPageAddress);
    mMainTreeView->activeColumns.disable(gsColComparisonValue).disable(gsColComparisonValueHex);
    mainLayout->addWidget(mMainTreeView);

    mMainTreeView->setColumnWidth(gsColValue, 250);
    mMainTreeView->updateTableHeader();

    mainLayout->setMargin(5);
    mMainTreeView->setVisible(true);
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
        mMainTreeView->addMemoryFields(Configuration::get()->typeFieldsOfDefaultStruct(pageType), pageType, mPageAddress);
        mMainTreeView->setColumnWidth(gsColValue, 250);
        mMainTreeView->updateTableHeader();
        mMainTreeView->updateTree(0, 0, true);
    }
}
