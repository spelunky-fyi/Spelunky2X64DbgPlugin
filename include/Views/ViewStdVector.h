#pragma once

#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <memory>
#include <vector>

namespace S2Plugin
{
    class TreeViewMemoryFields;

    class ViewStdVector : public QWidget
    {
        Q_OBJECT
      public:
        ViewStdVector(const std::string& vectorType, uintptr_t vectorOffset, QWidget* parent = nullptr);

      protected:
        void closeEvent(QCloseEvent* event) override;
        QSize sizeHint() const override;
        QSize minimumSizeHint() const override;

      private slots:
        void refreshVectorContents();
        void refreshData();
        void toggleAutoRefresh(int newState);
        void autoRefreshIntervalChanged(const QString& text);

      private:
        std::string mVectorType;
        uintptr_t mVectorOffset;
        size_t mVectorTypeSize;

        QVBoxLayout* mMainLayout;
        TreeViewMemoryFields* mMainTreeView;
        QPushButton* mRefreshDataButton;
        QCheckBox* mAutoRefreshCheckBox;
        QLineEdit* mAutoRefreshIntervalLineEdit;
        std::unique_ptr<QTimer> mAutoRefreshTimer;

        void initializeRefreshLayout();
    };
} // namespace S2Plugin
