#ifndef TAGEDITORWIDGET_H
#define TAGEDITORWIDGET_H

#include "./previousvaluehandling.h"

#include <tagparser/mediafileinfo.h>

#include <QWidget>
#include <QByteArray>

#include <mutex>
#include <functional>

#if defined(TAGEDITOR_NO_WEBVIEW)
# error "not supported (yet)."
#elif defined(TAGEDITOR_USE_WEBENGINE)
# define WEB_VIEW_PROVIDER QWebEngineView
#else
# define WEB_VIEW_PROVIDER QWebView
#endif

QT_FORWARD_DECLARE_CLASS(QFileSystemWatcher)
QT_FORWARD_DECLARE_CLASS(QMenu)
#ifndef TAGEDITOR_NO_WEBVIEW
QT_FORWARD_DECLARE_CLASS(WEB_VIEW_PROVIDER)
#endif

namespace Media {
DECLARE_ENUM(TagType, unsigned int)
}

namespace QtGui {

namespace Ui {
class TagEditorWidget;
}

class TagEdit;

class TagEditorWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString currentPath READ currentPath)
    Q_PROPERTY(QByteArray fileInfoHtml READ fileInfoHtml)
    Q_PROPERTY(bool fileNameVisible READ isFileNameVisible WRITE setFileNameVisible)
    Q_PROPERTY(bool buttonsVisible READ areButtonsVisible WRITE setButtonVisible)
    
public:
    explicit TagEditorWidget(QWidget *parent = nullptr);
    virtual ~TagEditorWidget();

public:
    std::mutex &fileOperationMutex();
    const QString &currentPath() const;
    Media::MediaFileInfo &fileInfo();
    const QByteArray &fileInfoHtml() const;
    bool isFileNameVisible() const;
    void setFileNameVisible(bool visible);
    bool areButtonsVisible() const;
    void setButtonVisible(bool visible);

public slots:
    // operations with the currently opened file: load, save, delete, close
    bool startParsing(const QString &path, bool forceRefresh = false);
    bool startSaving();
    void saveAndShowNextFile();
    bool reparseFile();
    bool applyEntriesAndSaveChangings();
    bool deleteAllTagsAndSave();
    void closeFile();

signals:
    /*!
     * \brief Emitted when loading the next file has been triggered.
     */
    void nextFileSelected();

    /*!
     * \brief Emitted to show a status message.
     */
    void statusMessage(const QString &message, int timeout = 0);

    /*!
     * \brief Emmited when the file status (opened/closed) has changed.
     */
    void fileStatusChange(bool opened, bool hasTag);

    /*!
     * \brief Emitted when the current path changed.
     */
    void fileSaved(const QString &newPath);

protected:
    bool event(QEvent *event);

private slots:
    // editor
    void fileChangedOnDisk(const QString &path);
    void showFile(char result);
    void handleReturnPressed();
    void handleKeepPreviousValuesActionTriggered(QAction *action);
    void applySettingsFromDialog();
    void addTag(const std::function<Media::Tag *(Media::MediaFileInfo &)> &createTag);
    void removeTag(Media::Tag *tag);
    void changeTarget(Media::Tag *tag);

    // saving
    void showSavingResult(bool processingError, bool ioError);

    // web view
    void updateInfoWebView();
    void showInfoWebViewContextMenu(const QPoint &);
    void copyInfoWebViewSelection();

private:
    void updateDocumentTitleEdits();
    void updateTagEditsAndAttachmentEdits(bool updateUi = true, PreviousValueHandling previousValueHandling = PreviousValueHandling::Auto);
    void updateTagSelectionComboBox();
    void updateFileStatusStatus();
    void updateTagManagementMenu();
    void insertTitleFromFilename();
    void foreachTagEdit(const std::function<void (TagEdit *)> &function);
    bool confirmCreationOfId3TagForUnsupportedFile();

    // UI
    std::unique_ptr<Ui::TagEditorWidget> m_ui;
    QMenu *m_keepPreviousValuesMenu;
    QMenu *m_tagOptionsMenu;
    QMenu *m_addTagMenu;
    QMenu *m_removeTagMenu;
    QMenu *m_changeTargetMenu;
    WEB_VIEW_PROVIDER *m_infoWebView;
    // tag, file, directory management
    QString m_currentPath;
    QFileSystemWatcher *m_fileWatcher;
    bool m_fileChangedOnDisk;
    Media::MediaFileInfo m_fileInfo;
    std::vector<Media::Tag *> m_tags;
    QByteArray m_fileInfoHtml;
    /*!
     * \brief This is the actual direcotry of the opened file which may differ from the directory selected in the tree view of the main window.
     */
    QString m_currentDir;
    QString m_lastDir;
    // status
    bool m_nextFileAfterSaving;
    bool m_makingResultsAvailable;
    Media::NotificationList m_originalNotifications;
    bool m_abortClicked;
    std::mutex m_fileOperationMutex;
};

/*!
 * \brief Returns the mutex which is internally used for thread-synchronization.
 */
inline std::mutex &TagEditorWidget::fileOperationMutex()
{
    return m_fileOperationMutex;
}

/*!
 * \brief Returns the current path.
 */
inline const QString &TagEditorWidget::currentPath() const
{
    return m_currentPath;
}

/*!
 * \brief Return file info.
 */
inline Media::MediaFileInfo &TagEditorWidget::fileInfo()
{
    return m_fileInfo;
}

/*!
 * \brief Returns the HTML source of the info website.
 */
inline const QByteArray &TagEditorWidget::fileInfoHtml() const
{
    return m_fileInfoHtml;
}

}

#endif // TAGEDITORWIDGET_H