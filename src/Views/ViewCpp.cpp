#include "Views/ViewCpp.h"

#include "Configuration.h"
#include "JsonNameDefinitions.h"
#include "QtHelpers/CPPSyntaxHighlighter.h"
#include "QtHelpers/QStrFromStringView.h"
#include <QCheckBox>
#include <QString>
#include <QTextEdit>
#include <QTextStream>
#include <QVBoxLayout>
#include <regex>
#include <sstream>

S2Plugin::ViewCpp::ViewCpp(std::string_view typeName, QWidget* parent) : QWidget(parent), mTypeName(typeName)
{
    auto config = Configuration::get();
    auto type = config->getBuiltInType(mTypeName);
    if (type == MemoryFieldType::None)
        setWindowTitle(QStrFromStringView(typeName));
    else
        setWindowTitle(QStrFromStringView(config->getTypeDisplayName(type)));

    auto mainLayout = new QVBoxLayout(this);

    auto check = new QCheckBox("Include Dependencies");
    QObject::connect(check, &QCheckBox::clicked, this, &ViewCpp::refresh);
    mainLayout->addWidget(check);

    mCPPTextEdit = new QTextEdit();
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
    mainLayout->addWidget(mCPPTextEdit);
    refresh(false);
}

void S2Plugin::ViewCpp::refresh(bool addDependencies)
{
    mCPPSyntaxHighlighter->clearRules();
    mCPPTextEdit->clear();

    generate(mTypeName);

    if (addDependencies)
        for (size_t i = 0; i < mDependencies.size(); ++i)
            generate(mDependencies[i]);

    mCPPTextEdit->moveCursor(QTextCursor::Start);
}

void S2Plugin::ViewCpp::generate(std::string typeName)
{
    std::stringstream outputStream;
    std::string_view parentClassName;
    bool asClass = false;
    const std::vector<MemoryField>* fields;
    std::vector<VirtualFunction> functions;
    auto config = Configuration::get();
    auto resolveType = [config, this](const std::string& typeNamex)
    {
        if (config->isJsonStruct(typeNamex))
        {
            addDependency(typeNamex);
            if (config->isPermanentPointer(typeNamex))
                return typeNamex + '*';

            return typeNamex;
        }
        else
        {
            auto type = config->getBuiltInType(typeNamex);
            if (type == MemoryFieldType::None)
                return "ERRORTYPE(" + typeNamex + ')';
            else
            {
                if (config->isPointerType(type))
                    return std::string(config->getCPPTypeName(type)) + '*';

                return std::string(config->getCPPTypeName(type));
            }
        }
    };
    auto cleanAndAddTypeRule = [this](std::string_view typeNamex)
    {
        if (typeNamex.rfind("std::", 0) != std::string::npos)
            typeNamex = typeNamex.substr(5);

        auto starPos = typeNamex.size();
        while (starPos != 0 && typeNamex[starPos - 1] == '*')
            --starPos;

        typeNamex = typeNamex.substr(0, starPos);

        QString qVariableType = "\\b" + QRegularExpression::escape(QStrFromStringView(typeNamex)) + "\\b";
        mCPPSyntaxHighlighter->addRule(qVariableType, HighlightColor::Type);
    };

    if (config->isEntitySubclass(typeName))
    {
        asClass = true;
        const auto& hierarchy = config->entityClassHierarchy();
        if (auto it = hierarchy.find(typeName); it != hierarchy.end())
        {
            parentClassName = it->second;
            addDependency(it->second);

            std::string currentType(parentClassName);
            it = hierarchy.find(currentType);
            while (it != hierarchy.end())
            {
                addDependency(it->second);
                currentType = it->second;
                it = hierarchy.find(currentType);
            }
        }
        fields = &config->typeFieldsOfEntitySubclass(typeName);
    }
    else if (config->isJsonStruct(typeName))
    {
        if (config->isJournalPage(typeName))
        {
            asClass = true;
            if (typeName != JsonName::JournalPage)
                parentClassName = JsonName::JournalPage;
        }
        fields = &config->typeFieldsOfDefaultStruct(typeName);
    }
    else
    {
        auto type = config->getBuiltInType(typeName);
        if (type == MemoryFieldType::None)
        {
            mCPPTextEdit->moveCursor(QTextCursor::Start);
            mCPPTextEdit->insertPlainText("// ViewCpp::generate() called for unknown type: (" + QString::fromStdString(typeName) + ")\n\n");
            return;
        }
        fields = &config->typeFields(type);
        if (fields->empty())
        {
            std::string_view svTypeName = config->getCPPTypeName(type);
            QString qtStringName = QStrFromStringView(svTypeName);
            mCPPTextEdit->moveCursor(QTextCursor::Start);
            mCPPTextEdit->insertPlainText(qtStringName + "\n\n");

            cleanAndAddTypeRule(svTypeName);
            return;
        }
    }
    cleanAndAddTypeRule(typeName);

    outputStream << (asClass ? "class " : "struct ");
    outputStream << typeName;

    if (!parentClassName.empty())
    {
        QString qClassName = QString("\\b" + QRegularExpression::escape(QStrFromStringView(parentClassName)) + "\\b");
        mCPPSyntaxHighlighter->addRule(qClassName, HighlightColor::Type);

        outputStream << " : public " << parentClassName;
    }
    outputStream << "\n{\n";
    if (asClass)
        outputStream << "\tpublic:\n";

    for (const auto& field : *fields)
    {
        if (field.type == MemoryFieldType::VirtualFunctionTable)
        {
            functions = config->virtualFunctionsOfType(typeName);
            if (functions.empty())
                outputStream << "uintptr_t _vtable; // no functions found\n";
            continue;
        }

        std::string variableType;
        if (field.type == MemoryFieldType::DefaultStructType)
        {
            variableType = field.jsonName;
            addDependency(field.jsonName);
        }
        else if (auto str = Configuration::getCPPTypeName(field.type); !str.empty())
        {
            variableType = str;
        }
        else
        {
            outputStream << "\tunknown field type (" << static_cast<uint32_t>(field.type) << ") name: (" << field.name << ")\n";
            continue;
        }

        if (!field.comment.empty())
            outputStream << "\t/// " << field.comment << "\n";

        switch (field.type)
        {
            case MemoryFieldType::Array:
            {
                std::string format = variableType;
                format.replace(format.find('S'), 1, std::to_string(field.numberOfElements));
                variableType = resolveType(field.firstParameterType);
                format.replace(format.find('T'), 1, variableType);
                outputStream << '\t' << format;

                mCPPSyntaxHighlighter->addRule("\\barray\\b", HighlightColor::Type);
                break;
            }
            case MemoryFieldType::Matrix:
            {
                variableType = resolveType(field.firstParameterType);
                outputStream << '\t' << variableType;
                break;
            }

            case MemoryFieldType::StdMap:          // std::map<K, V>
            case MemoryFieldType::StdUnorderedMap: // std::unordered_map<K, V>
            {
                if (!field.secondParameterType.empty()) // map
                {
                    std::string format = variableType;
                    variableType = resolveType(field.firstParameterType);
                    auto extraType = resolveType(field.secondParameterType);
                    format.replace(format.find('K'), 1, variableType);
                    format.replace(format.find("V>"), 1, extraType);
                    outputStream << '\t' << format;

                    cleanAndAddTypeRule(extraType);
                    mCPPSyntaxHighlighter->addRule("\\bmap\\b", HighlightColor::Type);
                    mCPPSyntaxHighlighter->addRule("\\bunordered_map\\b", HighlightColor::Type);
                    break;
                }
                // else -> set
                {
                    constexpr std::string_view replace("map<K, V>");
                    variableType.replace(variableType.find(replace), replace.size(), "set<T>");
                    mCPPSyntaxHighlighter->addRule("\\bset\\b", HighlightColor::Type);
                    mCPPSyntaxHighlighter->addRule("\\bunordered_set\\b", HighlightColor::Type);
                }
                [[fallthrough]];
            }
            case MemoryFieldType::StdList:
            case MemoryFieldType::StdVector:
            {
                std::string format = variableType;
                if (field.firstParameterType.empty())
                    variableType = "size_t";
                else
                    variableType = resolveType(field.firstParameterType);

                outputStream << '\t' << format.replace(format.find('T'), 1, variableType);

                mCPPSyntaxHighlighter->addRule("\\bvector\\b", HighlightColor::Type);
                mCPPSyntaxHighlighter->addRule("\\blist\\b", HighlightColor::Type);
                break;
            }
            case MemoryFieldType::OnHeapPointer:
            {
                std::string format = variableType;
                if (field.jsonName.empty())
                    variableType = "size_t";
                else
                    variableType = resolveType(field.jsonName);

                outputStream << '\t' << format.replace(format.find('T'), 1, variableType);

                mCPPSyntaxHighlighter->addRule("\\bOnHeapPointer\\b", HighlightColor::Type);
                break;
            }

            case MemoryFieldType::EntityPointer:
            case MemoryFieldType::EntityDBPointer:
            case MemoryFieldType::ParticleDBPointer:
            case MemoryFieldType::TextureDBPointer:
            case MemoryFieldType::UndeterminedThemeInfoPointer:
            case MemoryFieldType::COThemeInfoPointer:
            case MemoryFieldType::LiquidPhysicsPointer:
            case MemoryFieldType::JournalPagePointer:
            case MemoryFieldType::LevelGenPointer:
            case MemoryFieldType::EntityList:
            {
                outputStream << '\t' << variableType;
                addDependency(variableType);
                break;
            }
            case MemoryFieldType::StdString:
            {
                outputStream << '\t' << variableType;
                variableType = "string";
                break;
            }
            case MemoryFieldType::StdWstring:
            {
                outputStream << '\t' << variableType;
                variableType = "wstring";
                break;
            }
            case MemoryFieldType::OldStdList:
            {
                outputStream << '\t' << variableType;
                variableType = "uintptr_t";
                mCPPSyntaxHighlighter->addRule("\\bpair\\b", HighlightColor::Type);
                break;
            }
            case MemoryFieldType::CodePointer:
            {
                if (!field.firstParameterType.empty())
                {
                    // return (*name)(params);
                    outputStream << '\t' << field.firstParameterType << " (";
                    break;
                }
                [[fallthrough]];
            }
            default:
            {
                outputStream << '\t' << variableType;
                break;
            }
        }

        if (field.isPointer)
            outputStream << '*';

        if (!(field.type == MemoryFieldType::CodePointer && !field.firstParameterType.empty()))
            outputStream << ' ';

        outputStream << field.name;

        if (field.type == MemoryFieldType::Skip)
            outputStream << '[' << field.get_size() << ']';
        else if (field.type == MemoryFieldType::Matrix)
            outputStream << '[' << field.rows << "][" << field.getNumColumns() << ']';
        else if (field.type == MemoryFieldType::CodePointer)
            if (!field.firstParameterType.empty())
                outputStream << ")(" << field.secondParameterType << ')';

        outputStream << ";\n";

        QString qVariableName;
        if (field.type == MemoryFieldType::UTF8StringFixedSize || field.type == MemoryFieldType::UTF16StringFixedSize)
        {
            auto pos = static_cast<int>(field.name.rfind('['));
            qVariableName = "\\b" + QRegularExpression::escape(QString::fromUtf8(field.name.c_str(), pos)) + "\\b";
        }
        else
            qVariableName = "\\b" + QRegularExpression::escape(QString::fromStdString(field.name)) + "\\b";

        mCPPSyntaxHighlighter->addRule(qVariableName, HighlightColor::Variable);
        cleanAndAddTypeRule(variableType);
    }
    if (functions.empty() && !parentClassName.empty()) // for entity sub-classes
        functions = config->virtualFunctionsOfType(typeName, !parentClassName.empty());

    if (!functions.empty())
    {
        outputStream << '\n';
        size_t index{0};
        if (!parentClassName.empty()) // if class has parent, functions don't start at 0
            index = functions[0].index;

        for (auto& func : functions)
        {
            while (func.index > index) // for the gaps
            {
                outputStream << "\tvirtual void unknown" << index << "() = 0;\n";
                QString qFunctionName = QString("\\bunknown%1\\b").arg(index);
                mCPPSyntaxHighlighter->addRule(qFunctionName, HighlightColor::Function);
                ++index;
            }
            if (!func.comment.empty())
            {
                std::string comment = func.comment;
                auto idx = comment.find('\n');
                while (idx != std::string::npos)
                {
                    comment.replace(idx, 1, "\n\t/// ");
                    idx += 6;
                    idx = comment.find('\n', idx);
                }
                outputStream << "\t/// " << comment << '\n';
            }
            outputStream << "\tvirtual " << func.returnValue << ' ' << func.name << '(' << func.params << ") = 0;\n";

            QString qFunctionName = "\\s" + QRegularExpression::escape(QString::fromStdString(func.name)) + "\\b";
            mCPPSyntaxHighlighter->addRule(qFunctionName, HighlightColor::Function);
            cleanAndAddTypeRule(func.returnValue);
            ++index;
        }
    }

    outputStream << "};\n\n";

    mCPPTextEdit->moveCursor(QTextCursor::Start);
    mCPPTextEdit->insertPlainText(QString::fromStdString(outputStream.str()));
}

QSize S2Plugin::ViewCpp::sizeHint() const
{
    return QSize(650, 750);
}

QSize S2Plugin::ViewCpp::minimumSizeHint() const
{
    return QSize(150, 150);
}
