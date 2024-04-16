#pragma once

#include <QComboBox>
#include <QDialog>
#include <QLineEdit>

namespace S2Plugin
{
    enum class MemoryFieldType;
    class ItemModelStates;
    class SortFilterProxyModelStates;

    class DialogEditState : public QDialog
    {
        Q_OBJECT

      public:
        DialogEditState(const QString& fieldName, const std::string& refName, uintptr_t memoryAddress, MemoryFieldType type, QWidget* parent = nullptr);

      protected:
        QSize minimumSizeHint() const override;
        QSize sizeHint() const override;

      private slots:
        void cancelBtnClicked();
        void changeBtnClicked();
        void stateComboBoxChanged();

      private:
        uintptr_t mMemoryAddress;
        MemoryFieldType mFieldType;

        QComboBox* mStatesComboBox;
        ItemModelStates* mStatesModel;
        SortFilterProxyModelStates* mStatesSortProxy;
        QLineEdit* mStateLineEdit;
    };
} // namespace S2Plugin
