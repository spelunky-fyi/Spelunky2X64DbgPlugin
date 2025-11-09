#include "Views/ViewSettings.h"

#include "pluginmain.h"
#include <QCheckBox>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QString>
#include <QVBoxLayout>

S2Plugin::Settings* S2Plugin::Settings::_ptr{nullptr};

S2Plugin::Settings* S2Plugin::Settings::get()
{
    if (_ptr == nullptr)
        _ptr = new Settings();
    return _ptr;
}

static QString getSettingsPath()
{
    char buffer[MAX_PATH + 1] = {0};
    GetModuleFileNameA(nullptr, buffer, MAX_PATH);
    return QFileInfo(QString(buffer)).dir().filePath("plugins/spel2/settings.json");
}

S2Plugin::Settings::Settings()
{
    const QString path = getSettingsPath();

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QJsonDocument document = QJsonDocument::fromJson(file.readAll());
    file.close();
    QJsonObject jsonData = document.object();
    if (jsonData.isEmpty())
        return;

    QJsonValue value = jsonData.value("dev");
    if (!value.isNull())
        mDevMode = value.toBool();
}

void S2Plugin::Settings::save()
{
    static const QString path = getSettingsPath();

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        dprintf("Error: unable to save settings to settings.json\n");
        return;
    }

    QJsonDocument document(QJsonObject({
        {"dev", mDevMode},
    }));

    file.write(document.toJson());
    file.close();
}

bool S2Plugin::Settings::checkB(SETTING option) const
{
    switch (option)
    {
        case DEVELOPER_MODE:
        {
            return mDevMode;
        }
    }
    return false;
}

void S2Plugin::Settings::setB(SETTING option, bool b)
{
    switch (option)
    {
        case DEVELOPER_MODE:
        {
            mDevMode = b;
            break;
        }
    }
}

S2Plugin::ViewSettings::ViewSettings(QWidget* parent) : QWidget(parent)
{
    auto mainLayout = new QVBoxLayout(this);
    // mainLayout->setMargin(0);
    {
        auto secondaryLayout = new QHBoxLayout();
        secondaryLayout->addStretch();
        secondaryLayout->addWidget(new QLabel("Settings are saved on window close"));
        mainLayout->addLayout(secondaryLayout);
    }
    mainLayout->addSpacing(30);

    auto settings = Settings::get();
    auto check = new QCheckBox("Dev mode", this);
    check->setToolTip("Fills flag fields to max with \"unknown\" and shows skipped memory. Need to reload opened views to take effect");
    QObject::connect(check, &QCheckBox::clicked, this, [settings](bool b) { settings->setB(Settings::DEVELOPER_MODE, b); });
    check->setChecked(settings->checkB(Settings::DEVELOPER_MODE));
    mainLayout->addWidget(check);

    mainLayout->addStretch();
}

void S2Plugin::ViewSettings::closeEvent(QCloseEvent* event)
{
    Settings::get()->save();
    QWidget::closeEvent(event);
}

QSize S2Plugin::ViewSettings::sizeHint() const
{
    return QSize(200, 300);
}

QSize S2Plugin::ViewSettings::minimumSizeHint() const
{
    return QSize(150, 150);
}
