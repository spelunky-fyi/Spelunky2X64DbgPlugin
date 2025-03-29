#pragma once

#include "QtHelpers/AbstractDatabaseView.h"

class QResizeEvent;
class QPushButton;

namespace S2Plugin
{
    class TreeViewMemoryFields;

    class ViewTextureDB : public AbstractDatabaseView
    {
        Q_OBJECT
      public:
        ViewTextureDB(QWidget* parent = nullptr);
        void showID(ID_type id) override;
        void showRAW(uintptr_t address);

      protected:
        QSize sizeHint() const override;
        QSize minimumSizeHint() const override;
        void label() const override;
        ID_type highestRecordID() const override;
        bool isValidRecordID(ID_type id) const override;
        std::optional<ID_type> getIDForName(QString name) const override;
        QString recordNameForID(ID_type id) const override;
        uintptr_t addressOfRecordID(ID_type id) const override;
        void resizeEvent(QResizeEvent* event) override;

      private slots:
        void searchFieldCompleterActivated(const QString& text);

      private:
        QPushButton* mReloadCacheButton;
    };
} // namespace S2Plugin
