#pragma once

#include "QtHelpers/AbstractContainerView.h"
#include <QWidget>
#include <cstdint>
#include <string>

namespace S2Plugin
{

    class ViewStdUnorderedMap : public AbstractContainerView
    {
        Q_OBJECT
      public:
        ViewStdUnorderedMap(uintptr_t address, const std::string& keytypeName, const std::string& valuetypeName, QWidget* parent = nullptr);

      protected:
        void reloadContainer() override;

      private:
        std::string mMapKeyType;
        std::string mMapValueType;
        uintptr_t mMapAddress;
        size_t mMapKeyTypeSize;
        uint8_t mMapKeyAlignment;
    };
} // namespace S2Plugin
