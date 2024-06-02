#pragma once

#include "QtHelpers/AbstractContainerView.h"
#include <QWidget>
#include <cstdint>
#include <string>

namespace S2Plugin
{
    class ViewStdVector : public AbstractContainerView
    {
        Q_OBJECT
      public:
        ViewStdVector(const std::string& vectorType, uintptr_t vectoraddr, QWidget* parent = nullptr);

      protected:
        void reloadContainer() override;

      private:
        std::string mVectorType;
        uintptr_t mVectorAddress;
        size_t mVectorTypeSize;
    };
} // namespace S2Plugin
