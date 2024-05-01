#include "QtHelpers/WidgetAutorefresh.h"
#include <QHBoxLayout>
#include <QIntValidator>
#include <QLabel>

S2Plugin::WidgetAutorefresh::WidgetAutorefresh(const QString& initialInterval, QWidget* parrent) : QWidget(parrent)
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

    mAutoRefreshIntervalLineEdit = new QLineEdit(this);
    mAutoRefreshIntervalLineEdit->setFixedWidth(50);
    // TODO: for some reason it does not limit the input, only force it to int
    mAutoRefreshIntervalLineEdit->setValidator(new QIntValidator(100, 5000, this));
    mAutoRefreshIntervalLineEdit->setText(initialInterval);
    refreshLayout->addWidget(mAutoRefreshIntervalLineEdit);
    QObject::connect(mAutoRefreshIntervalLineEdit, &QLineEdit::textChanged, this, &WidgetAutorefresh::autoRefreshIntervalChanged);

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
        mAutoRefreshTimer->setInterval(mAutoRefreshIntervalLineEdit->text().toUInt());
        mAutoRefreshTimer->start();
        mRefreshButton->setEnabled(false);
    }
}

void S2Plugin::WidgetAutorefresh::autoRefreshIntervalChanged(const QString& text)
{
    if (mAutoRefreshCheckBox->checkState() == Qt::Checked)
    {
        mAutoRefreshTimer->setInterval(text.toUInt());
    }
}
