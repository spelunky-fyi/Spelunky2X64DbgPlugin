#pragma once

#include <QWidget>
#include <cstdint>

class QString;
class QComboBox;
class QMenu;

namespace S2Plugin
{
    class TreeViewMemoryFields;

    class ViewJournalPage : public QWidget
    {
        Q_OBJECT
      public:
        ViewJournalPage(uintptr_t address, QWidget* parent = nullptr);

      protected:
        QSize sizeHint() const override;
        QSize minimumSizeHint() const override;

      private slots:
        void refreshJournalPage();
        void label();
        void interpretAsChanged(const QString& text);
        void viewContextMenu(QMenu* menu);

      private:
        uintptr_t mPageAddress;
        TreeViewMemoryFields* mMainTreeView;
        QComboBox* mInterpretAsComboBox;
    };
} // namespace S2Plugin
