#pragma once

#include "QtHelpers/AbstractContainerView.h"
#include <QWidget>
#include <cstdint>
#include <string>

namespace S2Plugin
{

    class ViewStdMap : public AbstractContainerView
    {
        Q_OBJECT
      public:
        ViewStdMap(const std::string& keytypeName, const std::string& valuetypeName, uintptr_t address, QWidget* parent = nullptr);

      protected:
        void reloadContainer() override;

      private:
        std::string mMapKeyType;
        std::string mMapValueType;
        uintptr_t mMapAddress;
        size_t mMapKeyTypeSize;
        size_t mMapValueTypeSize;
        uint8_t mMapKeyAlignment;
        uint8_t mMapValueAlignment;
    };
} // namespace S2Plugin
