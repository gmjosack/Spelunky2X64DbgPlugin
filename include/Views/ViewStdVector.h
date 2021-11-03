#pragma once

#include "QtHelpers/TreeViewMemoryFields.h"
#include "ViewToolbar.h"
#include <QCheckBox>
#include <QPushButton>

namespace S2Plugin
{
    class ViewStdVector : public QWidget
    {
        Q_OBJECT
      public:
        ViewStdVector(ViewToolbar* toolbar, const std::string& vectorType, size_t vectorOffset, QWidget* parent = nullptr);

      protected:
        void closeEvent(QCloseEvent* event) override;
        QSize sizeHint() const override;
        QSize minimumSizeHint() const override;

      private slots:
        void refreshVectorContents();
        void refreshData();
        void toggleAutoRefresh(int newState);
        void autoRefreshTimerTrigger();
        void autoRefreshIntervalChanged(const QString& text);

      private:
        std::string mVectorType;
        size_t mVectorOffset;
        size_t mVectorBegin;
        size_t mVectorTypeSize;
        std::vector<MemoryField> mMemoryFields;

        QVBoxLayout* mMainLayout;
        TreeViewMemoryFields* mMainTreeView;
        ViewToolbar* mToolbar;
        QPushButton* mRefreshDataButton;
        QCheckBox* mAutoRefreshCheckBox;
        QLineEdit* mAutoRefreshIntervalLineEdit;
        std::unique_ptr<QTimer> mAutoRefreshTimer;

        void initializeTreeView();
        void initializeRefreshLayout();
    };
} // namespace S2Plugin
