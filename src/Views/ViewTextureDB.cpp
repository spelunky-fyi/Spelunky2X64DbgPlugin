#include "Views/ViewTextureDB.h"

#include "Configuration.h"
#include "Data/TextureDB.h"
#include "QtHelpers/TreeViewMemoryFields.h"
#include "Spelunky2.h"
#include <QCompleter>

S2Plugin::ViewTextureDB::ViewTextureDB(QWidget* parent) : AbstractDatabaseView(MemoryFieldType::TextureDB, parent)
{
    auto& textureDB = Spelunky2::get()->get_TextureDB();
    setWindowTitle(QString("Texture DB (%1 textures)").arg(textureDB.count()));
    auto textureNameCompleter = new QCompleter(textureDB.namesStringList(), this);
    textureNameCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    textureNameCompleter->setFilterMode(Qt::MatchContains);
    QObject::connect(textureNameCompleter, static_cast<void (QCompleter::*)(const QString&)>(&QCompleter::activated), this, &ViewTextureDB::searchFieldCompleterActivated);
    mSearchLineEdit->setCompleter(textureNameCompleter);
    mCompareTableWidget->setRowCount(static_cast<int>(textureDB.count()));
    showID(0);
}

QSize S2Plugin::ViewTextureDB::sizeHint() const
{
    return QSize(750, 375);
}

void S2Plugin::ViewTextureDB::searchFieldCompleterActivated(const QString& text)
{
    auto id = getIDForName(text);
    if (id.has_value())
    {
        showID(id.value());
    }
}

void S2Plugin::ViewTextureDB::showRAW(uintptr_t address)
{
    mMainTreeView->updateTree(address);
}

void S2Plugin::ViewTextureDB::showID(ID_type id)
{
    if (!isValidRecordID(id))
        return;

    switchToLookupTab();
    auto offset = Spelunky2::get()->get_TextureDB().addressOfID(id);
    mMainTreeView->updateTree(offset);
}

void S2Plugin::ViewTextureDB::label() const
{
    auto model = mMainTreeView->model();
    std::string name;
    auto& textureDB = Spelunky2::get()->get_TextureDB();
    for (int idx = 0; idx < model->rowCount(); ++idx)
    {
        if (model->data(model->index(idx, gsColField), Qt::DisplayRole).toString() == "id")
        {
            auto id = model->data(model->index(idx, gsColValue), gsRoleRawValue).toUInt();
            name = '[' + textureDB.nameForID(id) + ']';
            break;
        }
    }
    mMainTreeView->labelAll(name);
}

S2Plugin::ID_type S2Plugin::ViewTextureDB::highestRecordID() const
{
    return static_cast<ID_type>(Spelunky2::get()->get_TextureDB().highestID());
}

bool S2Plugin::ViewTextureDB::isValidRecordID(ID_type id) const
{
    return Spelunky2::get()->get_TextureDB().isValidID(id);
}

std::optional<S2Plugin::ID_type> S2Plugin::ViewTextureDB::getIDForName(QString name) const
{
    static const QRegularExpression r("^Texture ([0-9]+)");
    auto m = r.match(name);
    if (m.isValid())
        return m.captured(1).toUInt();

    return std::nullopt;
}

QString S2Plugin::ViewTextureDB::recordNameForID(ID_type id) const
{
    return QString::fromStdString(Spelunky2::get()->get_TextureDB().nameForID(id));
}

uintptr_t S2Plugin::ViewTextureDB::addressOfRecordID(ID_type id) const
{
    return Spelunky2::get()->get_TextureDB().addressOfID(id);
}
