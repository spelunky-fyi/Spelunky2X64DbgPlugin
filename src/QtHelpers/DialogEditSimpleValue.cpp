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
            auto spinbox = new QDoubleSpinBox(this);
            spinbox->setDecimals(6);
            spinbox->setRange(-std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
            QObject::connect(spinbox, static_cast<void (QDoubleSpinBox::*)(const QString&)>(&QDoubleSpinBox::valueChanged), this, &DialogEditSimpleValue::decValueChanged);
            uint32_t tmp = Script::Memory::ReadDword(mMemoryAddress);
            auto v = reinterpret_cast<float&>(tmp);
            spinbox->setValue(v);
            mSpinBox = spinbox;
            break;
        }
        case MemoryFieldType::Double:
        {
            auto spinbox = new QDoubleSpinBox(this);
            spinbox->setDecimals(15);
            spinbox->setRange(-std::numeric_limits<double>::max(), std::numeric_limits<double>::max());
            QObject::connect(spinbox, static_cast<void (QDoubleSpinBox::*)(const QString&)>(&QDoubleSpinBox::valueChanged), this, &DialogEditSimpleValue::decValueChanged);
            size_t tmp = Script::Memory::ReadQword(mMemoryAddress);
            double v = reinterpret_cast<double&>(tmp);
            spinbox->setValue(v);
            mSpinBox = spinbox;
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
            int v = 0;
            auto obj = qobject_cast<ULongLongSpinBox*>(mSpinBox);
            if (obj) // probably not needed but just in case
                v = obj->value();

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
    std::stringstream ss;
    switch (mFieldType)
    {
        case MemoryFieldType::Byte:
        {
            int8_t v = text.toInt();
            ss << QString::asprintf("0x%02x", static_cast<uint8_t>(v)).toStdString();
            break;
        }
        case MemoryFieldType::UnsignedByte:
        {
            uint8_t v = text.toInt();
            ss << "0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<uint32_t>(v);
            break;
        }
        case MemoryFieldType::Word:
        {
            int16_t v = text.toShort();
            ss << "0x" << std::hex << std::setw(4) << std::setfill('0') << v;
            break;
        }
        case MemoryFieldType::UnsignedWord:
        {
            uint16_t v = text.toUShort();
            ss << "0x" << std::hex << std::setw(4) << std::setfill('0') << v;
            break;
        }
        case MemoryFieldType::Dword:
        {
            int32_t v = text.toLong();
            ss << "0x" << std::hex << std::setw(8) << std::setfill('0') << v;
            break;
        }
        case MemoryFieldType::UnsignedDword:
        case MemoryFieldType::StringsTableID:
        {
            uint32_t v = text.toULong();
            ss << "0x" << std::hex << std::setw(8) << std::setfill('0') << v;
            break;
        }
        case MemoryFieldType::Qword:
        {
            int64_t v = text.toLongLong();
            ss << "0x" << std::hex << std::setw(16) << std::setfill('0') << v;
            break;
        }
        case MemoryFieldType::UnsignedQword:
        {
            uint64_t v = text.toULongLong();
            ss << "0x" << std::hex << std::setw(16) << std::setfill('0') << v;
            break;
        }
        case MemoryFieldType::Float:
        {
            float v = QLocale::system().toFloat(text);
            uint32_t tmp = reinterpret_cast<uint32_t&>(v);
            ss << "0x" << std::hex << std::setw(8) << std::setfill('0') << tmp;
            break;
        }
        case MemoryFieldType::Double:
        {
            double v = QLocale::system().toDouble(text);
            size_t tmp = reinterpret_cast<size_t&>(v);
            ss << "0x" << std::hex << std::setw(16) << std::setfill('0') << tmp;
            break;
        }
    }
    mLineEditHexValue->setText(QString::fromStdString(ss.str()));
}
