#pragma once

#include "Configuration.h"
#include "LineEditEx.h"
#include "QtHelpers/TreeViewMemoryFields.h"
#include <QString>
#include <QTableWidget>
#include <QVariant>
#include <QWidget>
#include <cstdint>
#include <optional>
#include <string>
#include <utility>
#include <vector>

/* Things to implement:
 * setWindowTitle
 * Set up completer
 * update number of rows in mCompareTableWidget
 * sizeHint override
 * call show id on open
 * override pure virtuals
 */

class QComboBox;
class QStandardItem;
class QTreeWidgetItem;
class QModelIndex;
class QTabWidget;
class QTreeWidget;

namespace S2Plugin
{
    using ID_type = uint32_t;

    class AbstractDatabaseView : public QWidget
    {
        Q_OBJECT
      public:
        explicit AbstractDatabaseView(MemoryFieldType type, QWidget* parent = nullptr);
        virtual void showID(ID_type id) = 0;

      protected:
        QSize minimumSizeHint() const override;

        virtual ID_type highestRecordID() const = 0;
        // already capped with highestRecordID, only for structs with gaps in between id's
        virtual bool isValidRecordID(ID_type ID) const = 0;
        virtual std::optional<ID_type> getIDForName(QString name) const = 0;
        // valid id guaranteed via highestRecordID and isValidRecordID
        virtual QString recordNameForID(ID_type id) const = 0;
        // valid id guaranteed via highestRecordID and isValidRecordID
        virtual uintptr_t addressOfRecordID(ID_type id) const = 0;

        void switchToLookupTab()
        {
            mMainTabWidget->setCurrentIndex(0);
        }

      protected slots:
        virtual void label() const = 0;
        void searchFieldReturnPressed();

      private slots:
        void fieldUpdated(int row, QStandardItem* parent);
        void fieldExpanded(const QModelIndex& index);
        void comparisonFieldChosen();
        void comparisonFlagChosen(const QString& text);
        void compareGroupByCheckBoxClicked(int state);
        void comparisonCellClicked(int row, int column);
        void groupedComparisonItemClicked(QTreeWidgetItem* item);

      protected:
        LineEditEx* mSearchLineEdit;
        TreeViewMemoryFields* mMainTreeView;
        QTableWidget* mCompareTableWidget;
        bool mFieldChosen{false};

      private:
        QTabWidget* mMainTabWidget;
        QComboBox* mCompareFieldComboBox;
        QComboBox* mCompareFlagComboBox;
        QTreeWidget* mCompareTreeWidget;

        void populateComparisonTableWidget(const QVariant& fieldData);
        void populateComparisonTreeWidget(const QVariant& fieldData);
        size_t populateComparisonComboBox(const std::vector<MemoryField>& fields, size_t offset = 0, std::string prefix = {});
        static std::pair<QString, QVariant> valueForField(const QVariant& data, uintptr_t addr);
    };
} // namespace S2Plugin
