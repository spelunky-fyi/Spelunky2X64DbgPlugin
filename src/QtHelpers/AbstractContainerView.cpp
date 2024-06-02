#include "QtHelpers/AbstractContainerView.h"

#include "Configuration.h"
#include "Data/EntityList.h"
#include "QtHelpers/TreeViewMemoryFields.h"
#include "QtHelpers/WidgetAutorefresh.h"
#include "QtPlugin.h"
#include <QPushButton>
#include <QString>
#include <QVBoxLayout>

S2Plugin::AbstractContainerView::AbstractContainerView(QWidget* parent) : QWidget(parent)
{
    setWindowIcon(getCavemanIcon());

    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setMargin(5);
    auto refreshLayout = new QHBoxLayout();
    mainLayout->addLayout(refreshLayout);

    auto refreshVectorButton = new QPushButton("Reload", this);
    QObject::connect(refreshVectorButton, &QPushButton::clicked, this, &AbstractContainerView::reloadContainer);
    refreshLayout->addWidget(refreshVectorButton);

    auto autoRefresh = new WidgetAutorefresh(300, this);
    refreshLayout->addWidget(autoRefresh);

    mMainTreeView = new TreeViewMemoryFields(this);
    QObject::connect(autoRefresh, &WidgetAutorefresh::refresh, mMainTreeView, static_cast<void (TreeViewMemoryFields::*)()>(&TreeViewMemoryFields::updateTree));
    mainLayout->addWidget(mMainTreeView);
    autoRefresh->toggleAutoRefresh(true);
}

QSize S2Plugin::AbstractContainerView::sizeHint() const
{
    return QSize(750, 550);
}

QSize S2Plugin::AbstractContainerView::minimumSizeHint() const
{
    return QSize(150, 150);
}
