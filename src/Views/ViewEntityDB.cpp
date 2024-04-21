#include "Views/ViewEntityDB.h"

#include "Configuration.h"
#include "Data/EntityDB.h"
#include "QtHelpers/TreeViewMemoryFields.h"
#include "QtPlugin.h"
#include "Spelunky2.h"
#include <QCompleter>

S2Plugin::ViewEntityDB::ViewEntityDB(QWidget* parent) : WidgetDatabaseView(MemoryFieldType::EntityDB, parent)
{
    auto config = Configuration::get();
    setWindowTitle(QString("Entity DB (%1 entities)").arg(config->entityList().count()));
    auto entityNameCompleter = new QCompleter(config->entityList().names(), this);
    entityNameCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    entityNameCompleter->setFilterMode(Qt::MatchContains);
    QObject::connect(entityNameCompleter, static_cast<void (QCompleter::*)(const QString&)>(&QCompleter::activated), this, &ViewEntityDB::searchFieldCompleterActivated);
    mSearchLineEdit->setCompleter(entityNameCompleter);
    mCompareTableWidget->setRowCount(static_cast<int>(config->entityList().count()));
    showID(1);
}

QSize S2Plugin::ViewEntityDB::sizeHint() const
{
    return QSize(750, 1050);
}

void S2Plugin::ViewEntityDB::searchFieldCompleterActivated()
{
    searchFieldReturnPressed();
}

void S2Plugin::ViewEntityDB::showID(ID_type id)
{
    if (id > Configuration::get()->entityList().highestID())
        return;

    switchToLookupTab();
    // id == 0 is valid, but not used as of right now
    mMainTreeView->updateTree(Spelunky2::get()->get_EntityDB().addressOfIndex(id));
}

void S2Plugin::ViewEntityDB::label() const
{
    auto model = mMainTreeView->model();
    auto& entityDB = Spelunky2::get()->get_EntityDB();
    auto offset = entityDB.addressOfIndex(0); // ptr
    uintptr_t indexOffset = model->data(model->index(0, gsColField), gsRoleMemoryAddress).toULongLong();
    uint32_t index = static_cast<uint32_t>((indexOffset - offset) / entityDB.entitySize());
    std::string name = '[' + Configuration::get()->entityList().nameForID(index) + ']';
    mMainTreeView->labelAll(name);
}

S2Plugin::ID_type S2Plugin::ViewEntityDB::highestRecordID() const
{
    return Configuration::get()->entityList().highestID();
}

bool S2Plugin::ViewEntityDB::isValidRecordID(ID_type id) const
{
    return Configuration::get()->entityList().isValidID(id);
}

std::optional<S2Plugin::ID_type> S2Plugin::ViewEntityDB::getIDForName(QString name) const
{
    auto id = Configuration::get()->entityList().idForName(name.toStdString());
    if (id == 0)
        return std::nullopt;

    return id;
}

QString S2Plugin::ViewEntityDB::recordNameForID(ID_type id) const
{
    return QString::fromStdString(Configuration::get()->entityList().nameForID(id));
}

uintptr_t S2Plugin::ViewEntityDB::addressOfRecordID(ID_type id) const
{
    return Spelunky2::get()->get_EntityDB().addressOfIndex(id);
}
