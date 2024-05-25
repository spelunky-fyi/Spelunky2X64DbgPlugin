#pragma once

#include <QSize>
#include <QString>
#include <QWidget>
#include <cstdint>

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

      private:
        uintptr_t mPageAddress;
        TreeViewMemoryFields* mMainTreeView;
    };
} // namespace S2Plugin
