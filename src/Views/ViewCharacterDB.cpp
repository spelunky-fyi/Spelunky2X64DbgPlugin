#include "Views/ViewCharacterDB.h"

#include "Configuration.h"
#include "QtHelpers/TreeViewMemoryFields.h"
#include "Spelunky2.h"
#include <QCompleter>

S2Plugin::ViewCharacterDB::ViewCharacterDB(QWidget* parent) : AbstractDatabaseView(MemoryFieldType::CharacterDB, parent)
{
    setWindowTitle("Character DB");
    auto& charDB = Spelunky2::get()->get_CharacterDB();
    auto characterNameCompleter = new QCompleter(charDB.characterNamesStringList(), this);
    characterNameCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    characterNameCompleter->setFilterMode(Qt::MatchContains);
    QObject::connect(characterNameCompleter, static_cast<void (QCompleter::*)(const QString&)>(&QCompleter::activated), this, &ViewCharacterDB::searchFieldCompleterActivated);
    mSearchLineEdit->setCompleter(characterNameCompleter);
    mCompareTableWidget->setRowCount(charDB.charactersCount());
    showID(0);
}

QSize S2Plugin::ViewCharacterDB::sizeHint() const
{
    return QSize(750, 650);
}

void S2Plugin::ViewCharacterDB::searchFieldCompleterActivated()
{
    searchFieldReturnPressed();
}

void S2Plugin::ViewCharacterDB::showID(ID_type id)
{
    auto& charDB = Spelunky2::get()->get_CharacterDB();
    if (id >= charDB.charactersCount())
        return;

    switchToLookupTab();
    auto offset = charDB.addressOfIndex(id);
    mMainTreeView->updateTree(offset);
}

void S2Plugin::ViewCharacterDB::label() const
{
    auto model = mMainTreeView->model();
    std::string name = "[UNKNOWN_CHARACTER]";
    auto& characterDB = Spelunky2::get()->get_CharacterDB();
    auto offset = characterDB.addressOfIndex(0); // ptr
    uintptr_t indexOffset = model->data(model->index(0, gsColField), gsRoleMemoryAddress).toULongLong();
    uint8_t index = static_cast<uint8_t>((indexOffset - offset) / characterDB.characterSize());
    if (index < characterDB.charactersCount())
    {
        name = '[' + characterDB.characterNamesStringList()[index].toStdString() + ']';
    }
    mMainTreeView->labelAll(name);
}

std::optional<S2Plugin::ID_type> S2Plugin::ViewCharacterDB::getIDForName(QString name) const
{
    ID_type count = 0;
    for (auto& characterName : Spelunky2::get()->get_CharacterDB().characterNamesStringList())
    {
        if (characterName == name)
            return count;

        ++count;
    }
    return std::nullopt;
}

S2Plugin::ID_type S2Plugin::ViewCharacterDB::highestRecordID() const
{
    return Spelunky2::get()->get_CharacterDB().charactersCount() - 1;
}

QString S2Plugin::ViewCharacterDB::recordNameForID(ID_type id) const
{
    return Spelunky2::get()->get_CharacterDB().characterNamesStringList()[id];
}

uintptr_t S2Plugin::ViewCharacterDB::addressOfRecordID(ID_type id) const
{
    return Spelunky2::get()->get_CharacterDB().addressOfIndex(id);
}
