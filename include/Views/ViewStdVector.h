#pragma once

#include "Configuration.h"
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
        ViewStdVector(uintptr_t vectoraddr, const std::string& vectorType, QWidget* parent = nullptr);

      protected:
        void reloadContainer() override;

      private:
        MemoryField mValueField;
        uintptr_t mVectorAddress;
    };
} // namespace S2Plugin
