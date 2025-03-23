#include "Views/ViewParticleDB.h"

#include "Configuration.h"
#include "QtHelpers/TreeViewMemoryFields.h"
#include "Spelunky2.h"
#include <QCompleter>

S2Plugin::ViewParticleDB::ViewParticleDB(QWidget* parent) : AbstractDatabaseView(MemoryFieldType::ParticleDB, parent)
{
    auto& particleEmitters = Configuration::get()->particleEmittersList();
    setWindowTitle(QString("Particle DB (%1 particles)").arg(particleEmitters.count()));
    auto particleNameCompleter = new QCompleter(particleEmitters.names(), this);
    particleNameCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    particleNameCompleter->setFilterMode(Qt::MatchContains);
    QObject::connect(particleNameCompleter, static_cast<void (QCompleter::*)(const QString&)>(&QCompleter::activated), this, &ViewParticleDB::searchFieldCompleterActivated);
    mSearchLineEdit->setCompleter(particleNameCompleter);
    mCompareTableWidget->setRowCount(static_cast<int>(particleEmitters.count()));
    showID(1);
}

QSize S2Plugin::ViewParticleDB::sizeHint() const
{
    return QSize(750, 1050);
}

void S2Plugin::ViewParticleDB::searchFieldCompleterActivated()
{
    searchFieldReturnPressed();
}

void S2Plugin::ViewParticleDB::showRAW(uintptr_t address)
{
    mMainTreeView->updateTree(address);
}

void S2Plugin::ViewParticleDB::showID(ID_type id)
{
    // ids start from 1
    if (id == 0 || id > Configuration::get()->particleEmittersList().highestID())
        return;

    switchToLookupTab();
    mMainTreeView->updateTree(Spelunky2::get()->get_ParticleDB().addressOfIndex(id - 1));
}

void S2Plugin::ViewParticleDB::label() const
{
    auto model = mMainTreeView->model();
    auto& particleDB = Spelunky2::get()->get_ParticleDB();
    auto offset = particleDB.addressOfIndex(0); // ptr
    uintptr_t indexOffset = model->data(model->index(0, gsColField), gsRoleMemoryAddress).toULongLong();
    uint32_t index = static_cast<uint32_t>((indexOffset - offset) / particleDB.particleSize());
    std::string name = '[' + Configuration::get()->particleEmittersList().nameForID(index + 1) + ']';
    mMainTreeView->labelAll(name);
}

S2Plugin::ID_type S2Plugin::ViewParticleDB::highestRecordID() const
{
    return Configuration::get()->particleEmittersList().highestID();
}

bool S2Plugin::ViewParticleDB::isValidRecordID(ID_type id) const
{
    return id != 0;
}

std::optional<S2Plugin::ID_type> S2Plugin::ViewParticleDB::getIDForName(QString name) const
{
    auto id = Configuration::get()->particleEmittersList().idForName(name.toStdString());
    if (id == 0)
        return std::nullopt;

    return id;
}

QString S2Plugin::ViewParticleDB::recordNameForID(ID_type id) const
{
    return QString::fromStdString(Configuration::get()->particleEmittersList().nameForID(id));
}

uintptr_t S2Plugin::ViewParticleDB::addressOfRecordID(ID_type id) const
{
    return Spelunky2::get()->get_ParticleDB().addressOfIndex(id - 1);
}
