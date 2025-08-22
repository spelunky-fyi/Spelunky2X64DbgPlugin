#include "QtHelpers/DialogEditString.h"

#include "QtPlugin.h"
#include "pluginmain.h"
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

S2Plugin::DialogEditString::DialogEditString(const QString& fieldName, QString value, uintptr_t memoryAddress, int size, MemoryFieldType type, QWidget* parent)
    : QDialog(parent, Qt::WindowCloseButtonHint | Qt::WindowTitleHint), mMemoryAddress(memoryAddress), mFieldType(type)
{
    setModal(true);
    setWindowTitle("Change value");
    setWindowIcon(getCavemanIcon());
    auto layout = new QVBoxLayout(this);

    // FIELDS
    auto gridLayout = new QGridLayout();

    gridLayout->addWidget(new QLabel(QString("Change value of %1").arg(fieldName), this), 0, 0, 1, 2);
    gridLayout->addWidget(new QLabel("New text:", this), 1, 0);

    mLineEdit = new QLineEdit(this);
    mLineEdit->setMaxLength(size);
    mLineEdit->setText(value);
    gridLayout->addWidget(mLineEdit, 1, 1);

    // BUTTONS
    auto buttonLayout = new QHBoxLayout();

    auto cancelBtn = new QPushButton("Cancel", this);
    QObject::connect(cancelBtn, &QPushButton::clicked, this, &DialogEditString::cancelBtnClicked);
    cancelBtn->setAutoDefault(false);
    auto changeBtn = new QPushButton("Change", this);
    QObject::connect(changeBtn, &QPushButton::clicked, this, &DialogEditString::changeBtnClicked);
    changeBtn->setAutoDefault(true);

    buttonLayout->addStretch();
    buttonLayout->addWidget(cancelBtn);
    buttonLayout->addWidget(changeBtn);

    layout->addLayout(gridLayout);
    layout->addStretch();
    layout->addLayout(buttonLayout);

    mLineEdit->setFocus();
    mLineEdit->selectAll();
}

QSize S2Plugin::DialogEditString::minimumSizeHint() const
{

    return QSize(350, 150);
}

QSize S2Plugin::DialogEditString::sizeHint() const
{
    return minimumSizeHint();
}

void S2Plugin::DialogEditString::cancelBtnClicked()
{
    reject();
}

void S2Plugin::DialogEditString::changeBtnClicked()
{
    switch (mFieldType)
    {
        case MemoryFieldType::UTF8Char:
        {
            char v = mLineEdit->text().isEmpty() ? 0 : mLineEdit->text().toStdString()[0];
            Script::Memory::WriteByte(mMemoryAddress, static_cast<unsigned char>(v));
            break;
        }
        case MemoryFieldType::UTF16Char:
        {
            ushort v = mLineEdit->text().isEmpty() ? 0u : mLineEdit->text()[0].unicode();
            Script::Memory::WriteWord(mMemoryAddress, v);
            break;
        }
        case MemoryFieldType::UTF16StringFixedSize:
        {
            auto v = mLineEdit->text().toStdWString();
            Script::Memory::Write(mMemoryAddress, v.data(), (v.size() + 1) * 2, nullptr); // +1 to include null character
            break;
        }
        case MemoryFieldType::ConstCharPointer:
        case MemoryFieldType::UTF8StringFixedSize:
        {
            auto v = mLineEdit->text().toStdString();
            Script::Memory::Write(mMemoryAddress, v.data(), v.size() + 1, nullptr); // +1 to include null character
            break;
        }
    }
    accept();
}
