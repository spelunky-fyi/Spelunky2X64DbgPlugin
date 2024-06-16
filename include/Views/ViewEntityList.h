#pragma once

#include "Configuration.h"
#include "QtHelpers/AbstractContainerView.h"
#include <QWidget>
#include <cstdint>
#include <string>

namespace S2Plugin
{
    class ViewEntityList : public AbstractContainerView
    {
        Q_OBJECT
      public:
        ViewEntityList(uintptr_t address, QWidget* parent = nullptr);

      protected:
        void reloadContainer() override;

      private:
        MemoryField mEntityField;
        uintptr_t mEntityListAddress;
    };
} // namespace S2Plugin
