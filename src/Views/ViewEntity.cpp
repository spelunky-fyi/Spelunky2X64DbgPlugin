#include "Views/ViewEntity.h"

#include "Configuration.h"
#include "Data/CPPGenerator.h"
#include "Data/Entity.h"
#include "QtHelpers/CPPSyntaxHighlighter.h"
#include "QtHelpers/TreeViewMemoryFields.h"
#include "QtHelpers/WidgetAutorefresh.h"
#include "QtHelpers/WidgetMemoryView.h"
#include "QtHelpers/WidgetSpelunkyLevel.h"
#include "QtPlugin.h"
#include "pluginmain.h"
#include <QAbstractItemView>
#include <QColor>
#include <QFont>
#include <QFontMetrics>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QStandardItemModel>
#include <QVBoxLayout>
#include <string>
#include <vector>

S2Plugin::ViewEntity::ViewEntity(size_t entityOffset, QWidget* parent) : QWidget(parent), mEntityPtr(entityOffset)
{
    initializeUI();
    setWindowIcon(getCavemanIcon());

    setWindowTitle(QString::asprintf("Entity %s 0x%016llX", Entity{mEntityPtr}.entityTypeName().c_str(), entityOffset));
    mMainTreeView->setVisible(true);

    mMainTreeView->setColumnWidth(gsColField, 175);
    mMainTreeView->setColumnWidth(gsColValueHex, 125);
    mMainTreeView->setColumnWidth(gsColMemoryAddress, 125);
    mMainTreeView->setColumnWidth(gsColMemoryAddressDelta, 75);
    mMainTreeView->setColumnWidth(gsColType, 100);
    updateMemoryViewOffsetAndSize();

    mSpelunkyLevel->paintFloor(QColor(160, 160, 160));
    mSpelunkyLevel->paintEntity(mEntityPtr, QColor(222, 52, 235));
    mSpelunkyLevel->update();
    auto entityClassName = Entity{mEntityPtr}.entityClassName();

    // the combobox is set as Entity by default, so we have to manually call interpretAsChanged
    if (entityClassName == "Entity")
        interpretAsChanged(QString::fromStdString(entityClassName));
    else
        mInterpretAsComboBox->setCurrentText(QString::fromStdString(entityClassName));
}

enum TABS
{
    FIELDS = 0,
    MEMORY = 1,
    LEVEL = 2,
    CPP = 3,
};

void S2Plugin::ViewEntity::initializeUI()
{
    auto mainLayout = new QVBoxLayout(this);
    auto topLayout = new QHBoxLayout();
    mainLayout->addLayout(topLayout);

    // TOP LAYOUT

    auto autoRefresh = new WidgetAutorefresh("100", this);
    QObject::connect(autoRefresh, &WidgetAutorefresh::refresh, this, &ViewEntity::refreshEntity);
    topLayout->addWidget(autoRefresh);

    topLayout->addStretch();

    auto labelButton = new QPushButton("Label", this);
    QObject::connect(labelButton, &QPushButton::clicked, this, &ViewEntity::label);
    topLayout->addWidget(labelButton);

    topLayout->addWidget(new QLabel("Interpret as:", this));
    mInterpretAsComboBox = new QComboBox(this);
    mInterpretAsComboBox->addItem("Reset");
    mInterpretAsComboBox->addItem("Entity");
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
    QObject::connect(mMainTabWidget, &QTabWidget::currentChanged, this, &ViewEntity::tabChanged);
    mainLayout->addWidget(mMainTabWidget);

    auto tabFields = new QWidget();
    auto tabMemory = new QWidget();
    auto tabLevel = new QWidget();
    auto tabCPP = new QWidget();
    tabFields->setLayout(new QVBoxLayout());
    tabFields->layout()->setMargin(0);
    tabMemory->setLayout(new QHBoxLayout());
    tabMemory->layout()->setMargin(0);
    tabLevel->setLayout(new QVBoxLayout());
    tabLevel->layout()->setMargin(0);
    tabCPP->setLayout(new QVBoxLayout());
    tabCPP->layout()->setMargin(0);

    mMainTabWidget->addTab(tabFields, "Fields");
    mMainTabWidget->addTab(tabMemory, "Memory");
    mMainTabWidget->addTab(tabLevel, "Level");
    mMainTabWidget->addTab(tabCPP, "C++");

    // TAB FIELDS
    mMainTreeView = new TreeViewMemoryFields(this);
    mMainTreeView->setColumnWidth(gsColValue, 250);
    mMainTreeView->setVisible(false);
    mMainTreeView->activeColumns.disable(gsColComparisonValue).disable(gsColComparisonValueHex);
    mMainTreeView->updateTableHeader();
    mMainTreeView->setDragDropMode(QAbstractItemView::DragDropMode::DragDrop);
    mMainTreeView->setAcceptDrops(true);
    QObject::connect(mMainTreeView, &TreeViewMemoryFields::entityOffsetDropped, this, &ViewEntity::entityOffsetDropped);
    tabFields->layout()->addWidget(mMainTreeView);

    // TAB MEMORY
    auto scroll = new QScrollArea(tabMemory);
    mMemoryView = new WidgetMemoryView(scroll);
    scroll->setStyleSheet("background-color: #fff;");
    scroll->setWidget(mMemoryView);
    scroll->setVisible(true);
    tabMemory->layout()->addWidget(scroll);

    mMemoryComparisonScrollArea = new QScrollArea(tabMemory);
    mMemoryComparisonView = new WidgetMemoryView(mMemoryComparisonScrollArea);
    mMemoryComparisonScrollArea->setStyleSheet("background-color: #fff;");
    mMemoryComparisonScrollArea->setWidget(mMemoryComparisonView);
    mMemoryComparisonScrollArea->setVisible(true);
    tabMemory->layout()->addWidget(mMemoryComparisonScrollArea);
    mMemoryComparisonScrollArea->setVisible(false);

    // TAB LEVEL
    scroll = new QScrollArea(tabLevel);
    mSpelunkyLevel = new WidgetSpelunkyLevel(mEntityPtr, scroll);
    scroll->setStyleSheet("background-color: #fff;");
    scroll->setWidget(mSpelunkyLevel);
    scroll->setVisible(true);
    tabLevel->layout()->addWidget(scroll);

    // TAB CPP
    mCPPTextEdit = new QTextEdit(this);
    mCPPTextEdit->setReadOnly(true);
    auto font = QFont("Courier", 10);
    font.setFixedPitch(true);
    font.setStyleHint(QFont::Monospace);
    auto fontMetrics = QFontMetrics(font);
    mCPPTextEdit->setFont(font);
    mCPPTextEdit->setTabStopWidth(4 * fontMetrics.width(' '));
    mCPPTextEdit->setLineWrapMode(QTextEdit::LineWrapMode::NoWrap);
    QPalette palette = mCPPTextEdit->palette();
    palette.setColor(QPalette::Base, QColor("#1E1E1E"));
    palette.setColor(QPalette::Text, QColor("#D4D4D4"));
    mCPPTextEdit->setPalette(palette);
    mCPPTextEdit->document()->setDocumentMargin(10);
    mCPPSyntaxHighlighter = new CPPSyntaxHighlighter(mCPPTextEdit->document());

    tabCPP->layout()->addWidget(mCPPTextEdit);
    mainLayout->setMargin(5);
    autoRefresh->toggleAutoRefresh(true);
}

void S2Plugin::ViewEntity::refreshEntity()
{
    mMainTreeView->updateTree();
    if (mMainTabWidget->currentIndex() == TABS::MEMORY)
    {
        mMemoryView->update();
        mMemoryComparisonView->update();
        updateComparedMemoryViewHighlights();
    }
    else if (mMainTabWidget->currentIndex() == TABS::LEVEL)
    {
        mSpelunkyLevel->update();
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
            mInterpretAsComboBox->setCurrentText(QString::fromStdString(entity.entityClassName()));
            return;
        }

        static const auto colors = {QColor(255, 214, 222), QColor(232, 206, 227), QColor(199, 186, 225), QColor(187, 211, 236), QColor(236, 228, 197), QColor(193, 219, 204)};
        auto config = Configuration::get();
        auto hierarchy = Entity::classHierarchy(classType.toStdString());

        mMainTreeView->clear();
        mMemoryView->clearHighlights();
        mMainTreeView->updateTableHeader();
        uint8_t counter = 0;
        size_t delta = 0;
        uint8_t colorIndex = 0;
        auto recursiveHighlight = [&](std::string prefix, const std::vector<MemoryField>& fields, auto&& self) -> void
        {
            for (auto& field : fields)
            {
                if (!field.isPointer)
                {
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

        for (auto it = hierarchy.rbegin(); it != hierarchy.rend(); ++it, ++colorIndex)
        {
            if (colorIndex > colors.size())
                colorIndex = 0;

            MemoryField headerField;
            headerField.name = "<b>" + *it + "</b>";
            headerField.type = MemoryFieldType::EntitySubclass;
            headerField.jsonName = *it;
            auto item = mMainTreeView->addMemoryField(headerField, *it, mEntityPtr + delta, delta);
            if (++counter == hierarchy.size()) // expand last subclass
            {
                mMainTreeView->expand(item->index());
            }
            // highlights fields in memory view, also updates delta
            recursiveHighlight(*it + ".", config->typeFieldsOfEntitySubclass(*it), recursiveHighlight);
        }

        mMainTreeView->updateTree(0, 0, true);
        mEntitySize = delta;
        updateMemoryViewOffsetAndSize();
    }
}

void S2Plugin::ViewEntity::updateMemoryViewOffsetAndSize()
{
    constexpr size_t smallEntityBucket = 0xD0;
    constexpr size_t bigEntityBucket = 0x188;

    size_t bytesShown = mEntitySize > smallEntityBucket ? bigEntityBucket : smallEntityBucket;

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
    auto config = Configuration::get();
    size_t offset = 0;
    std::string fieldName;
    QColor color;
    MemoryFieldType type{};

    auto highlightFields = [&](QStandardItem* parrent, auto&& self) -> void
    {
        for (int idx = 0; idx < parrent->rowCount(); ++idx)
        {
            auto field = parrent->child(idx, gsColField);
            type = field->data(gsRoleType).value<MemoryFieldType>();
            bool isPointer = field->data(gsRoleIsPointer).toBool();
            if (!isPointer && (type == MemoryFieldType::DefaultStructType || !config->typeFields(type).empty()))
            {
                self(field, self);
                continue;
            }
            auto deltaField = parrent->child(idx, gsColMemoryAddressDelta);
            size_t delta = deltaField->data(gsRoleRawValue).toULongLong();
            // get the size by the difference in offset delta
            // [Known Issue]: this will fail in getting the correct size if there is a skip element between fields
            int size = static_cast<int>(delta - offset);
            if (size != 0)
            {
                mMemoryComparisonView->addHighlightedField(std::move(fieldName), mComparisonEntityPtr + offset, size, std::move(color));
            }
            offset = delta;
            fieldName = field->data(gsRoleUID).toString().toStdString();
            color = parrent->child(idx, gsColComparisonValueHex)->background().color();
        }
    };

    for (int idx = 0; idx < root->rowCount(); ++idx)
    {
        QStandardItem* currentClass = root->child(idx, gsColField);
        if (currentClass->data(gsRoleType).value<MemoryFieldType>() != MemoryFieldType::EntitySubclass)
        {
            displayError("Error in `updateComparedMemoryViewHighlights`, found non EntitySubclass member in main tree");
            return;
        }
        highlightFields(currentClass, highlightFields);
    }
    // update last element
    int size = static_cast<int>(Configuration::getBuiltInTypeSize(type));
    if (size != 0)
    {
        mMemoryComparisonView->addHighlightedField(std::move(fieldName), mComparisonEntityPtr + offset, size, std::move(color));
    }
}

void S2Plugin::ViewEntity::label()
{
    std::stringstream ss;
    ss << "[Entity uid:" << Entity{mEntityPtr}.uid() << "]";
    mMainTreeView->labelAll(ss.str());
}

void S2Plugin::ViewEntity::entityOffsetDropped(size_t entityOffset)
{
    if (mComparisonEntityPtr != 0)
    {
        mSpelunkyLevel->clearPaintedEntity(mComparisonEntityPtr);
    }

    mComparisonEntityPtr = entityOffset;
    mMainTreeView->activeColumns.enable(gsColComparisonValue).enable(gsColComparisonValueHex);
    mMainTreeView->setColumnHidden(gsColComparisonValue, false);
    mMainTreeView->setColumnHidden(gsColComparisonValueHex, false);
    mMemoryComparisonScrollArea->setVisible(true);
    mMainTreeView->updateTree(0, mComparisonEntityPtr);
    updateMemoryViewOffsetAndSize();
}

void S2Plugin::ViewEntity::tabChanged()
{
    if (mMainTabWidget->currentIndex() == TABS::CPP)
    {
        mCPPSyntaxHighlighter->clearRules();
        CPPGenerator g{};
        g.generate(mInterpretAsComboBox->currentText().toStdString(), mCPPSyntaxHighlighter);
        mCPPSyntaxHighlighter->finalCustomRuleAdded();
        mCPPTextEdit->setText(QString::fromStdString(g.result()));
    }
}
