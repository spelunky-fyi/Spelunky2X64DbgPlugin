#pragma once

#include "QtHelpers/WidgetDatabaseView.h"

namespace S2Plugin
{
    class TreeViewMemoryFields;

    class ViewTextureDB : public WidgetDatabaseView
    {
        Q_OBJECT
      public:
        ViewTextureDB(QWidget* parent = nullptr);
        void showID(ID_type id) override;
        void showRAW(uintptr_t address);

      protected:
        QSize sizeHint() const override;
        void label() const override;
        ID_type highestRecordID() const override;
        bool isValidRecordID(ID_type id) const override;
        std::optional<ID_type> getIDForName(QString name) const override;
        QString recordNameForID(ID_type id) const override;
        uintptr_t addressOfRecordID(ID_type id) const override;

      private slots:
        void searchFieldCompleterActivated(const QString& text);
    };
} // namespace S2Plugin
