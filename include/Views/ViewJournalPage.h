#pragma once

#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QStandardItemModel>
#include <QTimer>
#include <QTreeView>
#include <QVBoxLayout>
#include <memory>
#include <string>

namespace S2Plugin
{
    class TreeViewMemoryFields;
    class JournalPage;

    class ViewJournalPage : public QWidget
    {
        Q_OBJECT
      public:
        ViewJournalPage(uintptr_t offset, const std::string& pageType, QWidget* parent = nullptr);

      protected:
        QSize sizeHint() const override;
        QSize minimumSizeHint() const override;

      private slots:
        void refreshJournalPage();
        void toggleAutoRefresh(int newState);
        void autoRefreshIntervalChanged(const QString& text);
        void label();
        void interpretAsChanged(const QString& text);

      private:
        uintptr_t mOffset;
        std::string mPageType;

        QVBoxLayout* mMainLayout;
        QHBoxLayout* mRefreshLayout;
        TreeViewMemoryFields* mMainTreeView;
        QPushButton* mRefreshButton;
        QCheckBox* mAutoRefreshCheckBox;
        QLineEdit* mAutoRefreshIntervalLineEdit;
        std::unique_ptr<QTimer> mAutoRefreshTimer;

        void initializeUI();
    };
} // namespace S2Plugin
