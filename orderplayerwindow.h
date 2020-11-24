#ifndef ORDERPLAYERWINDOW_H
#define ORDERPLAYERWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QSettings>
#include <QDebug>
#include <QApplication>
#include <QClipboard>
#include <QNetworkCookie>
#include <QMessageBox>
#include <QInputDialog>
#include <QTextCodec>
#include <stdio.h>
#include <iostream>
#include <QtWebSockets/QWebSocket>
#include <QAuthenticator>
#include <QtConcurrent/QtConcurrent>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QStyledItemDelegate>
#include <QListView>
#include <QStringListModel>
#include <QScrollBar>
#include <QMediaPlayer>
#include "facilemenu.h"
#include "songbeans.h"

QT_BEGIN_NAMESPACE
namespace Ui { class OrderPlayerWindow; }
QT_END_NAMESPACE



class OrderPlayerWindow : public QMainWindow
{
    Q_OBJECT
public:
    OrderPlayerWindow(QWidget *parent = nullptr);
    ~OrderPlayerWindow() override;

    enum PlayCircleMode
    {
        OrderList,
        SingleCircle
    };

private slots:
    void on_searchEdit_returnPressed();

    void on_searchButton_clicked();

    void slotSearchAndAutoAppend(QString key);

    void on_searchResultTable_cellActivated(int row, int);

    void on_searchResultTable_customContextMenuRequested(const QPoint &);

    void on_listTabWidget_currentChanged(int index);

    void sortSearchResult(int col);

    void on_playProgressSlider_sliderReleased();

    void on_volumeSlider_sliderMoved(int position);

    void on_playButton_clicked();

    void on_volumeButton_clicked();

    void on_circleModeButton_clicked();

    void slotSongPlayEnd();

    void on_orderSongsListView_customContextMenuRequested(const QPoint &);

    void on_favoriteSongsListView_customContextMenuRequested(const QPoint &);

    void on_historySongsListView_customContextMenuRequested(const QPoint &);

    void on_listSongsListView_customContextMenuRequested(const QPoint &pos);

    void on_normalSongsListView_customContextMenuRequested(const QPoint &pos);

    void on_orderSongsListView_activated(const QModelIndex &index);

    void on_favoriteSongsListView_activated(const QModelIndex &index);

    void on_normalSongsListView_activated(const QModelIndex &index);

    void on_historySongsListView_activated(const QModelIndex &index);

private:
    void searchMusic(QString key);
    void setSearchResultTable(SongList songs);
    void setSearchResultTable(PlayListList playLists);
    void addFavorite(SongList songs);
    void removeFavorite(SongList songs);
    void saveSongList(QString key, const SongList &songs);
    void restoreSongList(QString key, SongList& songs);
    void setSongModelToView(const SongList& songs, QListView* listView);
    QString songPath(const Song &song) const;
    QString lyricPath(const Song &song) const;
    QString coverPath(const Song &song) const;
    bool isSongDownloaded(Song song);
    QString msecondToString(qint64 msecond);
    void activeSong(Song song);
    bool isNotPlaying() const;

    void startPlaySong(Song song);
    void playNext();
    void appendOrderSongs(SongList songs);
    void appendNextSongs(SongList songs);

    void playLocalSong(Song song);
    void addDownloadSong(Song song);
    void downloadNext();
    void downloadSong(Song song);
    void downloadSongLyric(Song song);
    void downloadSongCover(Song song);
    void setCurrentLyric(QString lyric);

protected:
    void showEvent(QShowEvent*) override;
    void closeEvent(QCloseEvent*) override;

signals:
    void signalSongDownloadFinished(Song song);
    void signalLyricDownloadFinished(Song song);
    void signalCoverDownloadFinished(Song song);
    void signalSongPlayFinished(Song song);

private:
    Ui::OrderPlayerWindow *ui;
    QSettings settings;
    QDir musicsFileDir;
    const QString API_DOMAIN = "http://iwxyi.com:3000/";
    SongList searchResultSongs;
    PlayListList searchResultPlayLists;

    SongList orderSongs;
    SongList favoriteSongs;
    SongList normalSongs;
    SongList historySongs;
    PlayListList myPlayLists;
    SongList toDownloadSongs;
    Song playAfterDownloaded;
    Song downloadingSong;

    QMediaPlayer* player;
    PlayCircleMode circleMode = OrderList;
    Song playingSong;

    bool doubleClickToPlay = false; // 双击是立即播放，还是添加到列表
    bool searchAndAppend = false;
    qint64 setPlayPositionAfterLoad = 0; // 加载后跳转到时间
};

class NoFocusDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    NoFocusDelegate(){}
    ~NoFocusDelegate(){}

    void paint(QPainter* painter, const QStyleOptionViewItem & option, const QModelIndex &index) const
    {
        QStyleOptionViewItem itemOption(option);
        if (itemOption.state & QStyle::State_Selected)
        {
            painter->fillRect(option.rect, QColor(100, 149, 237, 88));
            /*int radius = option.rect.height() / 2;
            QPainterPath path;
            path.addRoundedRect(option.rect, radius, radius);
            painter->fillPath(path, QColor(100, 149, 237, 128));*/
        }
        QRect rect = option.rect;
        rect.setLeft(rect.left() + 4);
        painter->drawText(rect, Qt::AlignLeft | Qt::AlignVCenter, index.data(Qt::DisplayRole).toString());
    }
};

#endif // ORDERPLAYERWINDOW_H
