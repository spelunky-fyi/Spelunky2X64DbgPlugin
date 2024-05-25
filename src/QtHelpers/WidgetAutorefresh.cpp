#include "QtHelpers/WidgetAutorefresh.h"
#include <QHBoxLayout>
#include <QLabel>

S2Plugin::WidgetAutorefresh::WidgetAutorefresh(int initialInterval, QWidget* parrent) : QWidget(parrent)
{
    auto refreshLayout = new QHBoxLayout(this);
    refreshLayout->setMargin(0);
    mRefreshButton = new QPushButton("Refresh", this);
    refreshLayout->addWidget(mRefreshButton);
    QObject::connect(mRefreshButton, &QPushButton::clicked, this, &WidgetAutorefresh::refresh);

    mAutoRefreshTimer = new QTimer(this);
    QObject::connect(mAutoRefreshTimer, &QTimer::timeout, this, &WidgetAutorefresh::refresh);

    mAutoRefreshCheckBox = new QCheckBox("Auto-refresh every", this);
    mAutoRefreshCheckBox->setCheckState(Qt::Checked);
    refreshLayout->addWidget(mAutoRefreshCheckBox);
    QObject::connect(mAutoRefreshCheckBox, &QCheckBox::clicked, this, &WidgetAutorefresh::toggleAutoRefresh);

    mAutoRefreshIntervalQSpinBox = new QSpinBox(this);
    mAutoRefreshIntervalQSpinBox->setFixedWidth(50);
    mAutoRefreshIntervalQSpinBox->setRange(10, 5000);
    mAutoRefreshIntervalQSpinBox->setValue(initialInterval);
    refreshLayout->addWidget(mAutoRefreshIntervalQSpinBox);
    QObject::connect(mAutoRefreshIntervalQSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &WidgetAutorefresh::autoRefreshIntervalChanged);

    refreshLayout->addWidget(new QLabel("milliseconds", this));
    refreshLayout->addStretch();
}

void S2Plugin::WidgetAutorefresh::toggleAutoRefresh(bool checked)
{
    if (!checked)
    {
        mAutoRefreshTimer->stop();
        mRefreshButton->setEnabled(true);
    }
    else
    {
        mAutoRefreshTimer->setInterval(mAutoRefreshIntervalQSpinBox->value());
        mAutoRefreshTimer->start();
        mRefreshButton->setEnabled(false);
    }
}

void S2Plugin::WidgetAutorefresh::autoRefreshIntervalChanged(int val)
{
    if (mAutoRefreshCheckBox->checkState() == Qt::Checked)
    {
        mAutoRefreshTimer->setInterval(val);
    }
}
