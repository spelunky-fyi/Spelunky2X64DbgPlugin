#pragma once

#include <QWidget>

namespace S2Plugin
{
    struct Settings
    {
        static Settings* get();

        Settings();
        enum SETTING
        {
            DEVELOPER_MODE,
        };

        bool checkB(SETTING s) const;
        void setB(SETTING s, bool b);
        void save();

      private:
        static Settings* _ptr;
        bool mDevMode{false};
    };

    class ViewSettings : public QWidget
    {
        Q_OBJECT;

      public:
        ViewSettings(QWidget* parent = nullptr);

      protected:
        void closeEvent(QCloseEvent* event) override;
        QSize sizeHint() const override;
        QSize minimumSizeHint() const override;
    };
} // namespace S2Plugin
