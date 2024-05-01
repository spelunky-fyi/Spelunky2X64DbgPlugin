#pragma once

#include <QSize>
#include <QWidget>
#include <cstdint>
#include <string>

namespace S2Plugin
{
    class TreeViewMemoryFields;

    class ViewStdVector : public QWidget
    {
        Q_OBJECT
      public:
        ViewStdVector(const std::string& vectorType, uintptr_t vectorOffset, QWidget* parent = nullptr);

      protected:
        QSize sizeHint() const override;
        QSize minimumSizeHint() const override;

      private slots:
        void refreshVectorContents();
        void refreshData();

      private:
        std::string mVectorType;
        uintptr_t mVectorOffset;
        size_t mVectorTypeSize;

        TreeViewMemoryFields* mMainTreeView;

        void initializeRefreshLayout();
    };
} // namespace S2Plugin
