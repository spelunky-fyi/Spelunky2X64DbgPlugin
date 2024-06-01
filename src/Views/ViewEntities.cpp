#include "Views/ViewEntities.h"

#include "Configuration.h"
#include "Data/Entity.h"
#include "Data/Entitylist.h"
#include "Data/StdMap.h"
#include "QtHelpers/TreeViewMemoryFields.h"
#include "QtPlugin.h"
#include "Spelunky2.h"
#include "pluginmain.h"
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

S2Plugin::ViewEntities::ViewEntities(QWidget* parent) : QWidget(parent)
{
    setWindowIcon(getCavemanIcon());
    setWindowTitle("Entities");

    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setMargin(5);
    auto config = Configuration::get();
    mLayer0Address = config->offsetForField(MemoryFieldType::State, "layer0", Spelunky2::get()->get_StatePtr());
    mLayer1Address = config->offsetForField(MemoryFieldType::State, "layer1", Spelunky2::get()->get_StatePtr());
    mLayerMapOffset = config->offsetForField(config->typeFieldsOfDefaultStruct("LayerPointer"), "entities_by_mask");

    // initializeRefreshAndFilter
    {
        auto filterLayout = new QGridLayout();

        auto refreshButton = new QPushButton("Refresh", this);
        QObject::connect(refreshButton, &QPushButton::clicked, this, &ViewEntities::refreshEntities);
        filterLayout->addWidget(refreshButton, 0, 0);

        auto label = new QLabel("Filter:", this);
        filterLayout->addWidget(label, 0, 1);

        mFilterLineEdit = new QLineEdit(this);
        mFilterLineEdit->setPlaceholderText("Search for UID (dec or hex starting with 0x) or (part of) the entity name");
        QObject::connect(mFilterLineEdit, &QLineEdit::textChanged, this, &ViewEntities::refreshEntities);
        filterLayout->addWidget(mFilterLineEdit, 0, 2, 1, 6);

        mCheckboxLayer0 = new QCheckBox("Front layer (0)", this);
        mCheckboxLayer0->setChecked(true);
        QObject::connect(mCheckboxLayer0, &QCheckBox::stateChanged, this, &ViewEntities::refreshEntities);
        filterLayout->addWidget(mCheckboxLayer0, 2, 2);

        mCheckboxLayer1 = new QCheckBox("Back layer (0)", this);
        QObject::connect(mCheckboxLayer1, &QCheckBox::stateChanged, this, &ViewEntities::refreshEntities);
        filterLayout->addWidget(mCheckboxLayer1, 2, 3);

        int row = 3;
        int col = 2;
        for (auto& checkbox : mCheckbox)
        {
            checkbox.mCheckbox = new QCheckBox(checkbox.name + " (0)", this);
            QObject::connect(checkbox.mCheckbox, &QCheckBox::stateChanged, this, &ViewEntities::refreshEntities);
            filterLayout->addWidget(checkbox.mCheckbox, row, col);
            ++col;
            if (col == 9)
            {
                col = 2;
                ++row;
            }
        }

        auto horLayout = new QHBoxLayout();
        horLayout->addLayout(filterLayout);
        horLayout->addStretch();
        mainLayout->addLayout(horLayout);
    }
    mMainTreeView = new TreeViewMemoryFields(this);
    mMainTreeView->setEnableChangeHighlighting(false);
    mMainTreeView->activeColumns.disable(gsColComparisonValue).disable(gsColComparisonValueHex).disable(gsColMemoryAddressDelta).disable(gsColMemoryAddress).disable(gsColComment);

    mainLayout->addWidget(mMainTreeView);

    refreshEntities();
    mFilterLineEdit->setFocus();
}

void S2Plugin::ViewEntities::refreshEntities()
{
    mMainTreeView->clear();

    bool isUIDlookupSuccess = false;
    uint enteredUID = 0;
    if (!mFilterLineEdit->text().isEmpty())
    {
        enteredUID = mFilterLineEdit->text().toUInt(&isUIDlookupSuccess, 0);
    }

    size_t entitiesShown = 0;
    auto AddEntity = [&](size_t entity_ptr)
    {
        auto entity = Entity{Script::Memory::ReadQword(entity_ptr)};
        QString entityName = QString::fromStdString(Configuration::get()->getEntityName(entity.entityTypeID()));

        if (!isUIDlookupSuccess && !mFilterLineEdit->text().isEmpty())
        {
            if (!entityName.contains(mFilterLineEdit->text(), Qt::CaseInsensitive))
                return;
        }

        MemoryField field;
        field.name = "entity_uid_" + std::to_string(entity.uid());
        field.type = MemoryFieldType::EntityPointer;
        field.isPointer = true;
        mMainTreeView->addMemoryField(field, {}, entity_ptr, 0);
        ++entitiesShown;
    };

    auto layer0 = Script::Memory::ReadQword(mLayer0Address);
    EntityList entListLayer0{layer0 + 0x8};
    auto layer1 = Script::Memory::ReadQword(mLayer1Address);
    EntityList entListLayer1{layer1 + 0x8};
    mCheckboxLayer0->setText(QString("Front layer (%1)").arg(entListLayer0.size()));
    mCheckboxLayer1->setText(QString("Back layer (%1)").arg(entListLayer1.size()));

    auto check_layer0 = mCheckboxLayer0->checkState() == Qt::Checked;
    auto check_layer1 = mCheckboxLayer1->checkState() == Qt::Checked;

    if (isUIDlookupSuccess)
    {
        // loop thru all entities to find the uid
        auto uidList0 = entListLayer0.getAllUids();
        bool found_uid = false;
        for (uint idx = 0; idx < entListLayer0.size(); ++idx)
        {
            if (enteredUID == uidList0[idx])
            {
                AddEntity(entListLayer0.entities() + idx * sizeof(size_t));
                found_uid = true;
                break;
            }
        }

        if (found_uid == false)
        {
            auto uidList1 = entListLayer1.getAllUids();
            for (uint idx = 0; idx < entListLayer1.size(); ++idx)
            {
                if (enteredUID == uidList1[idx])
                {
                    AddEntity(entListLayer1.entities() + idx * sizeof(size_t));
                    break;
                }
            }
        }
    }

    StdMap<MASK, size_t> map0{layer0 + mLayerMapOffset};
    StdMap<MASK, size_t> map1{layer1 + mLayerMapOffset};

    for (auto& checkbox : mCheckbox)
    {
        int field_count = 0;

        if (check_layer0)
        {
            auto itr = map0.find(checkbox.mask);
            if (itr != map0.end())
            {
                EntityList maskEntList{itr.value_ptr()};

                field_count += maskEntList.size();
                // loop only if uid was not entered and the mask was choosen
                if (!isUIDlookupSuccess && checkbox.mCheckbox->checkState() == Qt::Checked)
                {
                    for (size_t i = 0; i < maskEntList.size(); ++i)
                        AddEntity(maskEntList.entities() + i * sizeof(uintptr_t));
                }
            }
        }
        if (check_layer1)
        {
            auto itr = map1.find(checkbox.mask);
            if (itr != map1.end())
            {
                EntityList maskEntList{itr.value_ptr()};
                field_count += maskEntList.size();
                if (!isUIDlookupSuccess && checkbox.mCheckbox->checkState() == Qt::Checked)
                {
                    for (size_t i = 0; i < maskEntList.size(); ++i)
                        AddEntity(maskEntList.entities() + i * sizeof(uintptr_t));
                }
            }
        }
        checkbox.mCheckbox->setText(QString(checkbox.name + " (%1)").arg(field_count));
    }
    setWindowTitle(QString("%1 Entities").arg(entitiesShown));
    mMainTreeView->updateTableHeader();
    mMainTreeView->setColumnWidth(gsColField, 145);
    mMainTreeView->setColumnWidth(gsColValueHex, 125);
    mMainTreeView->setColumnWidth(gsColType, 100);
    mMainTreeView->setColumnWidth(gsColValue, 300);
    mMainTreeView->updateTree();
}

QSize S2Plugin::ViewEntities::sizeHint() const
{
    return QSize(850, 550);
}

QSize S2Plugin::ViewEntities::minimumSizeHint() const
{
    return QSize(150, 150);
}
