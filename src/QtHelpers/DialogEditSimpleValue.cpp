#include "QtHelpers/DialogEditSimpleValue.h"
#include "Configuration.h"
#include "QtHelpers/LongLongSpinBox.h"
#include "QtPlugin.h"
#include "read_helpers.h"
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <iomanip>
#include <sstream>

S2Plugin::DialogEditSimpleValue::DialogEditSimpleValue(const QString& fieldName, uintptr_t memoryAddress, MemoryFieldType type, QWidget* parent)
    : QDialog(parent, Qt::WindowCloseButtonHint | Qt::WindowTitleHint), mMemoryAddress(memoryAddress), mFieldType(type)
{
    setModal(true);
    setWindowTitle("Change value");
    setWindowIcon(getCavemanIcon());
    auto layout = new QVBoxLayout(this);

    // FIELDS
    auto gridLayout = new QGridLayout(this);

    gridLayout->addWidget(new QLabel(QString("Change value of %1").arg(fieldName), this), 0, 0, 1, 2);

    gridLayout->addWidget(new QLabel("New value (dec):", this), 1, 0);
    gridLayout->addWidget(new QLabel("New value (hex):", this), 2, 0);

    mLineEditHexValue = new QLineEdit(this);
    mLineEditHexValue->setDisabled(true);

    switch (mFieldType)
    {
        case MemoryFieldType::Byte:
        {
            auto spinBox = new QSpinBox(this);
            QObject::connect(spinBox, static_cast<void (QSpinBox::*)(const QString&)>(&QSpinBox::valueChanged), this, &DialogEditSimpleValue::decValueChanged);
            spinBox->setRange(std::numeric_limits<int8_t>::min(), std::numeric_limits<int8_t>::max());
            auto v = Read<int8_t>(mMemoryAddress);
            spinBox->setValue(v);
            mSpinBox = spinBox;
            break;
        }
        case MemoryFieldType::UnsignedByte:
        {
            auto spinBox = new QSpinBox(this);
            QObject::connect(spinBox, static_cast<void (QSpinBox::*)(const QString&)>(&QSpinBox::valueChanged), this, &DialogEditSimpleValue::decValueChanged);
            spinBox->setRange(std::numeric_limits<uint8_t>::min(), std::numeric_limits<uint8_t>::max());
            auto v = Read<uint8_t>(mMemoryAddress);
            spinBox->setValue(v);
            mSpinBox = spinBox;
            break;
        }
        case MemoryFieldType::Word:
        {
            auto spinBox = new QSpinBox(this);
            QObject::connect(spinBox, static_cast<void (QSpinBox::*)(const QString&)>(&QSpinBox::valueChanged), this, &DialogEditSimpleValue::decValueChanged);
            spinBox->setRange(std::numeric_limits<int16_t>::min(), std::numeric_limits<int16_t>::max());
            auto v = Read<int16_t>(mMemoryAddress);
            spinBox->setValue(v);
            mSpinBox = spinBox;
            break;
        }
        case MemoryFieldType::UnsignedWord:
        {
            auto spinBox = new QSpinBox(this);
            QObject::connect(spinBox, static_cast<void (QSpinBox::*)(const QString&)>(&QSpinBox::valueChanged), this, &DialogEditSimpleValue::decValueChanged);
            spinBox->setRange(std::numeric_limits<uint16_t>::min(), std::numeric_limits<uint16_t>::max());
            auto v = Read<uint16_t>(mMemoryAddress);
            spinBox->setValue(v);
            mSpinBox = spinBox;
            break;
        }
        case MemoryFieldType::Dword:
        {
            auto spinBox = new QSpinBox(this);
            QObject::connect(spinBox, static_cast<void (QSpinBox::*)(const QString&)>(&QSpinBox::valueChanged), this, &DialogEditSimpleValue::decValueChanged);
            spinBox->setRange(std::numeric_limits<int32_t>::min(), std::numeric_limits<int32_t>::max());
            auto v = Read<int32_t>(mMemoryAddress);
            spinBox->setValue(v);
            mSpinBox = spinBox;
            break;
        }
        case MemoryFieldType::UnsignedDword:
        case MemoryFieldType::StringsTableID:
        {
            // no point of making SpinBox for uint32_t type, we can just use bigger type
            auto spinBox = new ULongLongSpinBox(this);
            QObject::connect(spinBox, &ULongLongSpinBox::valueChanged, this, &DialogEditSimpleValue::decValueChanged);
            spinBox->setRange(std::numeric_limits<uint32_t>::min(), std::numeric_limits<uint32_t>::max());
            auto v = Read<uint32_t>(mMemoryAddress);
            spinBox->setValue(v);
            mSpinBox = spinBox;
            break;
        }
        case MemoryFieldType::Qword:
        {
            auto spinBox = new LongLongSpinBox(this);
            QObject::connect(spinBox, &LongLongSpinBox::valueChanged, this, &DialogEditSimpleValue::decValueChanged);
            // spinBox->setRange(std::numeric_limits<int64_t>::min(), std::numeric_limits<int64_t>::max());
            auto v = Read<int64_t>(mMemoryAddress);
            spinBox->setValue(v);
            mSpinBox = spinBox;
            break;
        }
        case MemoryFieldType::UnsignedQword:
        {
            auto spinBox = new ULongLongSpinBox(this);
            QObject::connect(spinBox, &ULongLongSpinBox::valueChanged, this, &DialogEditSimpleValue::decValueChanged);
            // spinBox->setRange(std::numeric_limits<int64_t>::min(), std::numeric_limits<int64_t>::max());
            auto v = Read<uint64_t>(mMemoryAddress);
            spinBox->setValue(v);
            mSpinBox = spinBox;
            break;
        }
        case MemoryFieldType::Float:
        {
            auto spinBox = new QDoubleSpinBox(this);
            spinBox->setDecimals(6);
            spinBox->setRange(-std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
            QObject::connect(spinBox, static_cast<void (QDoubleSpinBox::*)(const QString&)>(&QDoubleSpinBox::valueChanged), this, &DialogEditSimpleValue::decValueChanged);
            uint32_t tmp = Script::Memory::ReadDword(mMemoryAddress);
            auto v = reinterpret_cast<float&>(tmp);
            spinBox->setValue(v);
            mSpinBox = spinBox;
            break;
        }
        case MemoryFieldType::Double:
        {
            auto spinBox = new QDoubleSpinBox(this);
            spinBox->setDecimals(15);
            spinBox->setRange(-std::numeric_limits<double>::max(), std::numeric_limits<double>::max());
            QObject::connect(spinBox, static_cast<void (QDoubleSpinBox::*)(const QString&)>(&QDoubleSpinBox::valueChanged), this, &DialogEditSimpleValue::decValueChanged);
            size_t tmp = Script::Memory::ReadQword(mMemoryAddress);
            double v = reinterpret_cast<double&>(tmp);
            spinBox->setValue(v);
            mSpinBox = spinBox;
            break;
        }
    }
    decValueChanged(mSpinBox->text());

    gridLayout->addWidget(mSpinBox, 1, 1);
    gridLayout->addWidget(mLineEditHexValue, 2, 1);

    // BUTTONS
    auto buttonLayout = new QHBoxLayout();

    auto cancelBtn = new QPushButton("Cancel", this);
    QObject::connect(cancelBtn, &QPushButton::clicked, this, &DialogEditSimpleValue::cancelBtnClicked);
    cancelBtn->setAutoDefault(false);
    auto changeBtn = new QPushButton("Change", this);
    QObject::connect(changeBtn, &QPushButton::clicked, this, &DialogEditSimpleValue::changeBtnClicked);
    changeBtn->setAutoDefault(true);

    buttonLayout->addStretch();
    buttonLayout->addWidget(cancelBtn);
    buttonLayout->addWidget(changeBtn);

    layout->addLayout(gridLayout);
    layout->addStretch();
    layout->addLayout(buttonLayout);

    mSpinBox->setFocus();
}

QSize S2Plugin::DialogEditSimpleValue::minimumSizeHint() const
{

    return QSize(350, 150);
}

QSize S2Plugin::DialogEditSimpleValue::sizeHint() const
{
    return minimumSizeHint();
}

void S2Plugin::DialogEditSimpleValue::cancelBtnClicked()
{
    reject();
}

void S2Plugin::DialogEditSimpleValue::changeBtnClicked()
{

    switch (mFieldType)
    {
        case MemoryFieldType::Byte:
        case MemoryFieldType::UnsignedByte:
        {
            int v = 0;
            auto obj = qobject_cast<QSpinBox*>(mSpinBox);
            if (obj) // probably not needed but just in case
                v = obj->value();

            Script::Memory::WriteByte(mMemoryAddress, static_cast<uint8_t>(v));
            break;
        }
        case MemoryFieldType::Word:
        case MemoryFieldType::UnsignedWord:
        {
            int v = 0;
            auto obj = qobject_cast<QSpinBox*>(mSpinBox);
            if (obj) // probably not needed but just in case
                v = obj->value();

            Script::Memory::WriteWord(mMemoryAddress, static_cast<uint16_t>(v));
            break;
        }
        case MemoryFieldType::Dword:
        {
            int v = 0;
            auto obj = qobject_cast<QSpinBox*>(mSpinBox);
            if (obj) // probably not needed but just in case
                v = obj->value();

            Script::Memory::WriteDword(mMemoryAddress, static_cast<uint32_t>(v));
            break;
        }
        case MemoryFieldType::UnsignedDword:
        case MemoryFieldType::StringsTableID:
        {
            uint32_t v = 0;
            auto obj = qobject_cast<ULongLongSpinBox*>(mSpinBox);
            if (obj) // probably not needed but just in case
                v = static_cast<uint32_t>(obj->value());

            Script::Memory::WriteDword(mMemoryAddress, v);
            break;
        }
        case MemoryFieldType::Qword:
        {
            int64_t v = 0;
            auto obj = qobject_cast<LongLongSpinBox*>(mSpinBox);
            if (obj) // probably not needed but just in case
                v = obj->value();

            Script::Memory::WriteQword(mMemoryAddress, static_cast<uint64_t>(v));
            break;
        }
        case MemoryFieldType::UnsignedQword:
        {
            uint64_t v = 0;
            auto obj = qobject_cast<ULongLongSpinBox*>(mSpinBox);
            if (obj) // probably not needed but just in case
                v = obj->value();

            Script::Memory::WriteQword(mMemoryAddress, v);
            break;
        }
        case MemoryFieldType::Float:
        {
            float v = 0;
            auto obj = qobject_cast<QDoubleSpinBox*>(mSpinBox);
            if (obj) // probably not needed but just in case
                v = obj->value();

            Script::Memory::WriteDword(mMemoryAddress, reinterpret_cast<uint32_t&>(v));
            break;
        }
        case MemoryFieldType::Double:
        {
            double v = 0;
            auto obj = qobject_cast<QDoubleSpinBox*>(mSpinBox);
            if (obj) // probably not needed but just in case
                v = obj->value();

            Script::Memory::WriteQword(mMemoryAddress, reinterpret_cast<uint64_t&>(v));
            break;
        }
    }
    accept();
}

void S2Plugin::DialogEditSimpleValue::decValueChanged(const QString& text)
{
    QString result;
    switch (mFieldType)
    {
        case MemoryFieldType::Byte:
        {
            int8_t v = static_cast<int8_t>(text.toShort()); // there is no conversion to 8bit
            result = QString::asprintf("0x%02X", static_cast<uint8_t>(v));
            break;
        }
        case MemoryFieldType::UnsignedByte:
        {
            uint8_t v = static_cast<uint8_t>(text.toUShort());
            result = QString::asprintf("0x%02X", v);
            break;
        }
        case MemoryFieldType::Word:
        {
            int16_t v = text.toShort();
            result = QString::asprintf("0x%04X", static_cast<uint16_t>(v));
            break;
        }
        case MemoryFieldType::UnsignedWord:
        {
            uint16_t v = text.toUShort();
            result = QString::asprintf("0x%04X", v);
            break;
        }
        case MemoryFieldType::Dword:
        {
            int32_t v = text.toLong();
            result = QString::asprintf("0x%08X", static_cast<uint32_t>(v));
            break;
        }
        case MemoryFieldType::UnsignedDword:
        case MemoryFieldType::StringsTableID:
        {
            uint32_t v = text.toULong();
            result = QString::asprintf("0x%08X", v);
            break;
        }
        case MemoryFieldType::Qword:
        {
            int64_t v = text.toLongLong();
            result = QString::asprintf("0x%016llX", static_cast<uint64_t>(v));
            break;
        }
        case MemoryFieldType::UnsignedQword:
        {
            uint64_t v = text.toULongLong();
            result = QString::asprintf("0x%016llX", v);
            break;
        }
        case MemoryFieldType::Float:
        {
            float v = QLocale::system().toFloat(text);
            uint32_t tmp = reinterpret_cast<uint32_t&>(v);
            result = QString::asprintf("0x%08X", tmp);
            break;
        }
        case MemoryFieldType::Double:
        {
            double v = QLocale::system().toDouble(text);
            uint64_t tmp = reinterpret_cast<uint64_t&>(v);
            result = QString::asprintf("0x%016llX", tmp);
            break;
        }
    }
    mLineEditHexValue->setText(result);
}
