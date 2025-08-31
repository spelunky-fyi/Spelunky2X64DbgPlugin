#include "Views/ViewEntity.h"

#include "Configuration.h"
#include "Data/Entity.h"
#include "JsonNameDefinitions.h"
#include "QtHelpers/QStrFromStringView.h"
#include "QtHelpers/TreeViewMemoryFields.h"
#include "QtHelpers/WidgetAutorefresh.h"
#include "QtHelpers/WidgetMemoryView.h"
#include "QtHelpers/WidgetSpelunkyLevel.h"
#include "QtPlugin.h"
#include "Views/ViewToolbar.h"
#include "pluginmain.h" // for dprintf
#include <QAction>
#include <QColor>
#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QStandardItemModel>
#include <QString>
#include <QTabWidget>
#include <QVBoxLayout>
#include <string>
#include <vector>

enum TABS
{
    FIELDS = 0,
    MEMORY = 1,
    LEVEL = 2,
};

S2Plugin::ViewEntity::ViewEntity(size_t entityOffset, QWidget* parent) : QWidget(parent), mEntityPtr(entityOffset)
{
    setWindowIcon(getCavemanIcon());
    setWindowTitle(QString::asprintf("Entity %s 0x%016llX", Entity{mEntityPtr}.entityTypeName().c_str(), entityOffset));

    initializeUI();
    updateMemoryViewOffsetAndSize();

    auto entityClassName = Entity{mEntityPtr}.entityClassName();
    // the comboBox is set as Entity by default, so we have to manually call interpretAsChanged
    if (entityClassName == JsonName::Entity)
        interpretAsChanged(QStrFromStringView(entityClassName));
    else
        mInterpretAsComboBox->setCurrentText(QStrFromStringView(entityClassName));
}

void S2Plugin::ViewEntity::initializeUI()
{
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setMargin(5);
    auto topLayout = new QHBoxLayout();
    mainLayout->addLayout(topLayout);

    // TOP LAYOUT
    auto autoRefresh = new WidgetAutorefresh(100, this);
    QObject::connect(autoRefresh, &WidgetAutorefresh::refresh, this, &ViewEntity::refreshEntity);
    topLayout->addWidget(autoRefresh);

    topLayout->addStretch();

    auto labelButton = new QPushButton("Label", this);
    QObject::connect(labelButton, &QPushButton::clicked, this, &ViewEntity::label);
    topLayout->addWidget(labelButton);

    topLayout->addWidget(new QLabel("Interpret as:", this));
    mInterpretAsComboBox = new QComboBox(this);
    mInterpretAsComboBox->addItem("Reset");
    mInterpretAsComboBox->addItem(QStrFromStringView(JsonName::Entity));
    mInterpretAsComboBox->insertSeparator(2);
    std::vector<std::string> classNames;
    for (const auto& [classType, parentClassType] : Configuration::get()->entityClassHierarchy())
    {
        classNames.emplace_back(classType);
    }
    std::sort(classNames.begin(), classNames.end());
    for (const auto& className : classNames)
    {
        mInterpretAsComboBox->addItem(QString::fromStdString(className));
    }
    QObject::connect(mInterpretAsComboBox, &QComboBox::currentTextChanged, this, &ViewEntity::interpretAsChanged);
    topLayout->addWidget(mInterpretAsComboBox);

    // TABS
    mMainTabWidget = new QTabWidget(this);
    mMainTabWidget->setDocumentMode(false);
    mainLayout->addWidget(mMainTabWidget);

    mMainTreeView = new TreeViewMemoryFields();
    auto tabMemory = new QWidget();
    auto tabLevel = new QScrollArea();

    tabMemory->setLayout(new QHBoxLayout());
    tabMemory->layout()->setMargin(0);

    mMainTabWidget->addTab(mMainTreeView, "Fields");
    mMainTabWidget->addTab(tabMemory, "Memory");
    mMainTabWidget->addTab(tabLevel, "Level");

    // TAB FIELDS
    {
        mMainTreeView->mActiveColumns.disable(gsColComparisonValue).disable(gsColComparisonValueHex);
        mMainTreeView->updateTableHeader(false);
        mMainTreeView->setColumnWidth(gsColValue, 250);
        mMainTreeView->setColumnWidth(gsColField, 175);
        mMainTreeView->setColumnWidth(gsColValueHex, 125);
        mMainTreeView->setColumnWidth(gsColMemoryAddress, 120);
        mMainTreeView->setColumnWidth(gsColMemoryAddressDelta, 75);
        mMainTreeView->setColumnWidth(gsColType, 100);
        QObject::connect(mMainTreeView, &TreeViewMemoryFields::offsetDropped, this, &ViewEntity::entityOffsetDropped);
        QObject::connect(mMainTreeView, &TreeViewMemoryFields::onContextMenu, this, &ViewEntity::viewContextMenu);
    }
    // TAB MEMORY
    {
        mMemoryScrollArea = new QScrollArea(tabMemory);
        mMemoryView = new WidgetMemoryView(mMemoryScrollArea);
        mMemoryScrollArea->setStyleSheet("background-color: #FFF;");
        mMemoryScrollArea->setWidget(mMemoryView);
        tabMemory->layout()->addWidget(mMemoryScrollArea);

        mMemoryComparisonScrollArea = new QScrollArea(tabMemory);
        mMemoryComparisonView = new WidgetMemoryView(mMemoryComparisonScrollArea);
        mMemoryComparisonScrollArea->setStyleSheet("background-color: #FFF;");
        mMemoryComparisonScrollArea->setWidget(mMemoryComparisonView);
        tabMemory->layout()->addWidget(mMemoryComparisonScrollArea);
        mMemoryComparisonScrollArea->setVisible(false);

        connect(mMemoryScrollArea->horizontalScrollBar(), &QScrollBar::valueChanged, mMemoryComparisonScrollArea->horizontalScrollBar(), &QScrollBar::setValue);
        connect(mMemoryScrollArea->verticalScrollBar(), &QScrollBar::valueChanged, mMemoryComparisonScrollArea->verticalScrollBar(), &QScrollBar::setValue);
        connect(mMemoryComparisonScrollArea->horizontalScrollBar(), &QScrollBar::valueChanged, mMemoryScrollArea->horizontalScrollBar(), &QScrollBar::setValue);
        connect(mMemoryComparisonScrollArea->verticalScrollBar(), &QScrollBar::valueChanged, mMemoryScrollArea->verticalScrollBar(), &QScrollBar::setValue);
    }
    // TAB LEVEL
    {
        mSpelunkyLevel = new WidgetSpelunkyLevel(mEntityPtr, tabLevel);
        mSpelunkyLevel->paintFloor(QColor(160, 160, 160));
        mSpelunkyLevel->paintEntity(mEntityPtr, QColor(222, 52, 235));
        // to many entities
        // mSpelunkyLevel->paintEntityMask(0x4000, QColor(255, 87, 6));  // lava
        // mSpelunkyLevel->paintEntityMask(0x2000, QColor(6, 213, 249)); // water
        mSpelunkyLevel->paintEntityMask(0x80, QColor(85, 170, 170)); // active floors
        tabLevel->setStyleSheet("background-color: #FFF;");
        tabLevel->setWidget(mSpelunkyLevel);
    }
    autoRefresh->toggleAutoRefresh(true);
}

void S2Plugin::ViewEntity::refreshEntity()
{
    mMainTreeView->updateTree();
    if (mMainTabWidget->currentIndex() == TABS::MEMORY)
    {
        mMemoryView->updateMemory();
        mMemoryComparisonView->updateMemory();
        updateComparedMemoryViewHighlights();
    }
    else if (mMainTabWidget->currentIndex() == TABS::LEVEL)
    {
        mSpelunkyLevel->updateLevel();
    }
}

QSize S2Plugin::ViewEntity::sizeHint() const
{
    return QSize(750, 550);
}

QSize S2Plugin::ViewEntity::minimumSizeHint() const
{
    return QSize(150, 150);
}

void S2Plugin::ViewEntity::interpretAsChanged(const QString& classType)
{
    if (!classType.isEmpty())
    {
        Entity entity{mEntityPtr};
        if (classType == "Reset")
        {
            mInterpretAsComboBox->setCurrentText(QStrFromStringView(entity.entityClassName()));
            return;
        }

        static const auto colors = {QColor(255, 214, 222), QColor(232, 206, 227), QColor(199, 186, 225), QColor(187, 211, 236), QColor(236, 228, 197), QColor(193, 219, 204)};
        auto config = Configuration::get();
        auto hierarchy = Entity::classHierarchy(classType.toStdString());

        mMainTreeView->clear();
        mMemoryView->clearHighlights();
        mMainTreeView->updateTableHeader();
        size_t delta = 0;
        uint8_t colorIndex = 0;
        auto recursiveHighlight = [&](std::string prefix, const std::vector<MemoryField>& fields, auto&& self) -> void
        {
            for (auto& field : fields)
            {
                if (!field.isPointer)
                {
                    // note: this will not work with arrays and probably other stuff, in the future prefer to use mMainTreeView
                    if (field.type == MemoryFieldType::DefaultStructType)
                    {
                        self(prefix + field.name + ".", config->typeFieldsOfDefaultStruct(field.jsonName), self);
                        continue;
                    }
                    else if (auto& typeFields = config->typeFields(field.type); !typeFields.empty())
                    {
                        // paint elements of types like map, vector etc.
                        self(prefix + field.name + ".", typeFields, self);
                        continue;
                    }
                }

                int size = static_cast<int>(field.get_size());
                if (size == 0)
                    continue;
                mMemoryView->addHighlightedField(prefix + field.name, mEntityPtr + delta, size, *(colors.begin() + colorIndex));
                delta += size;
            }
        };

        MemoryField headerField; // potentially not safe if used get_size() since it won't update
        headerField.type = MemoryFieldType::EntitySubclass;
        for (auto it = hierarchy.rbegin(); it != hierarchy.rend(); ++it, ++colorIndex)
        {
            if (colorIndex > colors.size())
                colorIndex = 0;

            headerField.name = "<b>" + *it + "</b>";
            headerField.jsonName = *it;
            mMainTreeView->addMemoryField(headerField, *it, mEntityPtr + delta, delta);
            // highlights fields in memory view, also updates delta
            recursiveHighlight(*it + ".", config->typeFieldsOfEntitySubclass(*it), recursiveHighlight);
        }

        mMainTreeView->updateTree(0, 0, true);
        mMainTreeView->expandLast();
        mEntitySize = delta;
        updateMemoryViewOffsetAndSize();
    }
}

void S2Plugin::ViewEntity::updateMemoryViewOffsetAndSize()
{
    size_t bytesShown = mEntitySize > gSmallEntityBucket ? gBigEntityBucket : gSmallEntityBucket;

    mMemoryView->setOffsetAndSize(mEntityPtr, bytesShown);
    mMemoryComparisonView->setOffsetAndSize(mComparisonEntityPtr, bytesShown);
}

void S2Plugin::ViewEntity::updateComparedMemoryViewHighlights()
{
    if (mComparisonEntityPtr == 0)
        return;

    // TODO: don't clear tooltip if the interpretAs was not changed, maybe consider adding updateHighlightedField
    mMemoryComparisonView->clearHighlights();
    auto root = qobject_cast<QStandardItemModel*>(mMainTreeView->model())->invisibleRootItem();
    size_t offset = 0;
    std::string fieldName;
    QColor color;
    MemoryFieldType type{};

    auto highlightFields = [&](QStandardItem* parent, auto&& self) -> void
    {
        for (int idx = 0; idx < parent->rowCount(); ++idx)
        {
            auto field = parent->child(idx, gsColField);
            type = field->data(gsRoleType).value<MemoryFieldType>();
            bool isPointer = field->data(gsRoleIsPointer).toBool();
            // [Known Issue]: This may need update if we ever add field types that have children with not actual memory representation
            if (!isPointer && field->hasChildren() && type != MemoryFieldType::Flags8 && type != MemoryFieldType::Flags16 && type != MemoryFieldType::Flags32)
            {
                self(field, self);
                continue;
            }
            auto deltaField = parent->child(idx, gsColMemoryAddressDelta);
            size_t delta = deltaField->data(gsRoleRawValue).toULongLong();
            // get the size by the difference in offset delta
            // TODO: this will fail in getting the correct size if there is a skip element between fields
            int size = static_cast<int>(delta - offset);
            if (size != 0)
            {
                mMemoryComparisonView->addHighlightedField(std::move(fieldName), mComparisonEntityPtr + offset, size, std::move(color));
            }
            offset = delta;
            fieldName = field->data(gsRoleUID).toString().toStdString();
            color = parent->child(idx, gsColComparisonValueHex)->background().color();
        }
    };

    for (int idx = 0; idx < root->rowCount(); ++idx)
    {
        QStandardItem* currentClass = root->child(idx, gsColField);
        if (currentClass->data(gsRoleType).value<MemoryFieldType>() != MemoryFieldType::EntitySubclass)
        {
            dprintf("Error in `updateComparedMemoryViewHighlights`, found non EntitySubclass member in main tree");
            return;
        }
        highlightFields(currentClass, highlightFields);
    }
    // update last element
    int size = static_cast<int>(Configuration::getBuiltInTypeSize(type));
    if (size != 0)
        mMemoryComparisonView->addHighlightedField(std::move(fieldName), mComparisonEntityPtr + offset, size, std::move(color));
}

void S2Plugin::ViewEntity::label()
{
    std::string ss("[Entity uid:" + std::to_string(Entity{mEntityPtr}.uid()) + "]");
    mMainTreeView->labelAll(ss);
}

void S2Plugin::ViewEntity::entityOffsetDropped(uintptr_t entityOffset)
{
    if (mComparisonEntityPtr != 0)
        mSpelunkyLevel->clearPaintedEntity(mComparisonEntityPtr);

    mComparisonEntityPtr = entityOffset;
    mSpelunkyLevel->paintEntity(entityOffset, QColor(232, 134, 30));
    mMemoryComparisonScrollArea->setVisible(true);
    updateMemoryViewOffsetAndSize();
    mMemoryScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

void S2Plugin::ViewEntity::viewContextMenu(QMenu* menu)
{
    auto action = menu->addAction("View Code");
    QObject::connect(action, &QAction::triggered, menu,
                     [this]()
                     {
                         auto type = mInterpretAsComboBox->currentText().toStdString();
                         getToolbar()->showCode(type);
                     });
}
