#pragma once

#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>
#include <QStandardItem>
#include <QVBoxLayout>
#include <memory>
#include <vector>

namespace S2Plugin
{
    class TreeViewMemoryFields;

    class ViewStdMap : public QWidget
    {
        Q_OBJECT
      public:
        ViewStdMap(const std::string& keytypeName, const std::string& valuetypeName, uintptr_t mapOffset, QWidget* parent = nullptr);

      protected:
        QSize sizeHint() const override;
        QSize minimumSizeHint() const override;

      private slots:
        void refreshMapContents();
        void refreshData();
        void toggleAutoRefresh(int newState);
        void autoRefreshIntervalChanged(const QString& text);

      private:
        std::string mMapKeyType;
        std::string mMapValueType;
        uintptr_t mmapOffset;
        size_t mMapKeyTypeSize;
        size_t mMapValueTypeSize;
        uint8_t mMapKeyAlignment;
        uint8_t mMapValueAlignment;

        QVBoxLayout* mMainLayout;
        TreeViewMemoryFields* mMainTreeView;
        QPushButton* mRefreshDataButton;
        QCheckBox* mAutoRefreshCheckBox;
        QLineEdit* mAutoRefreshIntervalLineEdit;
        std::unique_ptr<QTimer> mAutoRefreshTimer;

        void initializeRefreshLayout();
    };
} // namespace S2Plugin
