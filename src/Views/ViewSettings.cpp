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

    for (auto& [data, name] : cache)
    {
        QJsonValue value = jsonData.value(name);
        data = value.toVariant();
    }
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

    QJsonObject object;
    for (const auto& [data, name] : cache)
        object.insert(name, QJsonValue::fromVariant(data));

    file.write(QJsonDocument(object).toJson());
    file.close();
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
    {
        auto check = new QCheckBox("Dev mode", this);
        check->setToolTip("Fills flag fields and virtual functions view to max with \"unknown\" and shows skipped memory. Need to reload opened views to take effect");
        QObject::connect(check, &QCheckBox::clicked, this, [settings](bool b) { settings->setB(Settings::DEVELOPER_MODE, b); });
        check->setChecked(settings->checkB(Settings::DEVELOPER_MODE));
        mainLayout->addWidget(check);
    }
    {
        auto check = new QCheckBox("Comments as tooltip", this);
        check->setToolTip("Hides the comments column and instead shows them as tooltip when hovering over the name");
        QObject::connect(check, &QCheckBox::clicked, this, [settings](bool b) { settings->setB(Settings::COMMENTS_AS_TOOLTIP, b); });
        check->setChecked(settings->checkB(Settings::COMMENTS_AS_TOOLTIP));
        mainLayout->addWidget(check);
    }

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
