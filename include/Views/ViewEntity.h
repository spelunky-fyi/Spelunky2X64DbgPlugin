#pragma once

#include <QWidget>
#include <cstdint>

class QComboBox;
class QScrollArea;
class QString;
class QTabWidget;
class QTextEdit;
class QMenu;

namespace S2Plugin
{
    class TreeViewMemoryFields;
    class WidgetMemoryView;
    class CPPSyntaxHighlighter;
    class WidgetSpelunkyLevel;

    class ViewEntity : public QWidget
    {
        Q_OBJECT
      public:
        ViewEntity(size_t entityOffset, QWidget* parent = nullptr);

      protected:
        QSize sizeHint() const override;
        QSize minimumSizeHint() const override;

      private slots:
        void refreshEntity();
        void interpretAsChanged(const QString& text);
        void label();
        void entityOffsetDropped(uintptr_t entityOffset);
        void viewContextMenu(QMenu* menu);

      private:
        QTabWidget* mMainTabWidget;

        uintptr_t mEntityPtr;
        uintptr_t mComparisonEntityPtr{0};
        size_t mEntitySize{0};

        // TOP LAYOUT
        QComboBox* mInterpretAsComboBox;

        // TAB FIELDS
        TreeViewMemoryFields* mMainTreeView;

        // TAB MEMORY
        WidgetMemoryView* mMemoryView;
        WidgetMemoryView* mMemoryComparisonView;
        QScrollArea* mMemoryScrollArea;
        QScrollArea* mMemoryComparisonScrollArea;

        // TAB LEVEL
        WidgetSpelunkyLevel* mSpelunkyLevel;

        void initializeUI();
        void updateMemoryViewOffsetAndSize();
        void updateComparedMemoryViewHighlights();
    };
} // namespace S2Plugin
