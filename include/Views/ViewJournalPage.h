#pragma once

#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QStandardItemModel>
#include <QTimer>
#include <QTreeView>
#include <QVBoxLayout>
#include <memory>
#include <string>

namespace S2Plugin
{
    struct ViewToolbar;
    struct TreeViewMemoryFields;
    struct JournalPage;

    class ViewJournalPage : public QWidget
    {
        Q_OBJECT
      public:
        ViewJournalPage(ViewToolbar* toolbar, size_t offset, const std::string& pageType, QWidget* parent = nullptr);

      protected:
        void closeEvent(QCloseEvent* event) override;
        QSize sizeHint() const override;
        QSize minimumSizeHint() const override;

      private slots:
        void refreshJournalPage();
        void toggleAutoRefresh(int newState);
        void autoRefreshTimerTrigger();
        void autoRefreshIntervalChanged(const QString& text);
        void label();
        void interpretAsChanged(const QString& text);

      private:
        size_t mOffset;
        std::string mPageType;
        ViewToolbar* mToolbar;

        QVBoxLayout* mMainLayout;
        QHBoxLayout* mRefreshLayout;
        TreeViewMemoryFields* mMainTreeView;
        QPushButton* mRefreshButton;
        QCheckBox* mAutoRefreshCheckBox;
        QLineEdit* mAutoRefreshIntervalLineEdit;
        std::unique_ptr<QTimer> mAutoRefreshTimer;
        QComboBox* mInterpretAsComboBox;

        std::unique_ptr<JournalPage> mJournalPage;
        void initializeUI();
    };
} // namespace S2Plugin
