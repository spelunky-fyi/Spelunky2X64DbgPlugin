#pragma once

#include <QSize>
#include <QWidget>
#include <cstdint>
#include <string>

namespace S2Plugin
{
    class TreeViewMemoryFields;

    class ViewStdMap : public QWidget
    {
        Q_OBJECT
      public:
        ViewStdMap(const std::string& keytypeName, const std::string& valuetypeName, uintptr_t address, QWidget* parent = nullptr);

      protected:
        QSize sizeHint() const override;
        QSize minimumSizeHint() const override;

      private slots:
        void refreshMapContents();
        void refreshData();

      private:
        std::string mMapKeyType;
        std::string mMapValueType;
        uintptr_t mMapAddress;
        size_t mMapKeyTypeSize;
        size_t mMapValueTypeSize;
        uint8_t mMapKeyAlignment;
        uint8_t mMapValueAlignment;

        TreeViewMemoryFields* mMainTreeView;
    };
} // namespace S2Plugin
