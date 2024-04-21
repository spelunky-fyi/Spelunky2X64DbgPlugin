#pragma once

#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>
#include <QStandardItemModel>
#include <QTimer>
#include <QTreeView>
#include <QVBoxLayout>
#include <memory>

namespace S2Plugin
{
    class TreeViewMemoryFields;

    class ViewState : public QWidget
    {
        Q_OBJECT
      public:
        ViewState(uintptr_t state, QWidget* parent = nullptr);

      protected:
        QSize sizeHint() const override;
        QSize minimumSizeHint() const override;

      private slots:
        void refreshState();
        void toggleAutoRefresh(int newState);
        void autoRefreshIntervalChanged(const QString& text);
        void label();

      private:
        uintptr_t mState;

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
