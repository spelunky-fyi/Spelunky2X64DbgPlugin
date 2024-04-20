#pragma once

#include <QCloseEvent>
#include <QComboBox>
#include <QLineEdit>
#include <QModelIndex>
#include <QSize>
#include <QStandardItem>
#include <QString>
#include <QTabWidget>
#include <QTableWidget>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QWidget>
#include <cstdint>
#include <optional>

/* Things to implement:
 * setWindowTitle
 * Set up completer
 * update number of rows in mCompareTableWidget
 * sizeHint override
 * call show id on open
 * override pure virtuals
 */

namespace S2Plugin
{
    class TreeViewMemoryFields;
    enum class MemoryFieldType;
    struct MemoryField;
    using ID_type = uint32_t;

    class WidgetDatabaseView : public QWidget
    {
        Q_OBJECT
      public:
        WidgetDatabaseView(MemoryFieldType type, QWidget* parent = nullptr);
        virtual void showID(ID_type id) = 0;

      protected:
        void closeEvent(QCloseEvent* event) override;
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
        void fieldUpdated(int row, QStandardItem* parrent);
        void fieldExpanded(const QModelIndex& index);
        void comparisonFieldChosen();
        void compareGroupByCheckBoxClicked(int state);
        void comparisonCellClicked(int row, int column);
        void groupedComparisonItemClicked(QTreeWidgetItem* item);

      protected:
        QLineEdit* mSearchLineEdit;
        TreeViewMemoryFields* mMainTreeView;
        QTableWidget* mCompareTableWidget;

      private:
        QTabWidget* mMainTabWidget;
        QComboBox* mCompareFieldComboBox;
        QTreeWidget* mCompareTreeWidget;

        void populateComparisonTableWidget();
        void populateComparisonTreeWidget();
        size_t populateComparisonCombobox(const std::vector<MemoryField>& fields, size_t offset = 0, std::string prefix = {});
        static std::pair<QString, QVariant> valueForField(const QVariant& data, uintptr_t addr);
    };
} // namespace S2Plugin
