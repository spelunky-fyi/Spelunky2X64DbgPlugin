#pragma once

#include <QHash>
#include <QVariant>
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
            COMMENTS_AS_TOOLTIP,
        };

        bool checkB(SETTING s) const
        {
            return cache[s].data.toBool();
        }
        void setB(SETTING s, bool b)
        {
            cache[s].data = b;
        }
        QVariant getData(SETTING s) const
        {
            return cache[s].data;
        }
        void setData(SETTING s, const QVariant& data)
        {
            cache[s].data = data;
        }

      private:
        void save();

        struct Storage
        {
            QVariant data;
            QString name;
        };

        static Settings* _ptr;
        QHash<SETTING, Storage> cache = {
            // enum, {default value, name},
            {DEVELOPER_MODE, {false, "dev"}},
            {COMMENTS_AS_TOOLTIP, {false, "tooltip_comments"}},
        };
        friend class ViewSettings;
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
