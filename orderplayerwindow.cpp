#include "orderplayerwindow.h"
#include "ui_orderplayerwindow.h"

OrderPlayerWindow::OrderPlayerWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::OrderPlayerWindow),
      settings(QApplication::applicationDirPath()+"/musics.ini", QSettings::Format::IniFormat),
      musicsFileDir(QApplication::applicationDirPath()+"/musics"),
      player(new QMediaPlayer(this)),
      desktopLyric(new DesktopLyricWidget(settings, nullptr)),
      expandPlayingButton(new InteractiveButtonBase(this))
{
    starting = true;
    ui->setupUi(this);

    QHeaderView* header = ui->searchResultTable->horizontalHeader();
    header->setSectionResizeMode(QHeaderView::ResizeToContents);
    header->setMinimumSectionSize(QFontMetrics(this->font()).horizontalAdvance("哈哈哈哈哈哈"));
    header->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    header->setStyleSheet("QHeaderView { background-color: transparent; }");
    ui->searchResultTable->verticalHeader()->setStyleSheet("QHeaderView { background-color: transparent; }");

    new NoFocusDelegate(ui->searchResultTable, 4);
    new NoFocusDelegate(ui->orderSongsListView);
    new NoFocusDelegate(ui->normalSongsListView);
    new NoFocusDelegate(ui->favoriteSongsListView);
    new NoFocusDelegate(ui->listSongsListView);
    new NoFocusDelegate(ui->historySongsListView);

    QString vScrollBarSS("QScrollBar:vertical"
                         "{"
                         "    width:7px;"
                         "    background:rgba(128,128,128,0%);"
                         "    margin:0px,0px,0px,0px;"
                         "    padding-top:0px;"
                         "    padding-bottom:0px;"
                         "}"
                         "QScrollBar::handle:vertical"
                         "{"
                         "    width:7px;"
                         "    background:rgba(128,128,128,32%);"
                         "    border-radius:3px;"
                         "    min-height:20;"
                         "}"
                         "QScrollBar::handle:vertical:hover"
                         "{"
                         "    width:7px;"
                         "    background:rgba(128,128,128,50%);"
                         "    min-height:20;"
                         "}"
                         "QScrollBar::sub-line:vertical"
                         "{"
                         "    height:9px;width:8px;"
                         "    border-image:url(:/images/a/1.png);"
                         "    subcontrol-position:top;"
                         "}"
                         "QScrollBar::add-line:vertical"
                         "{"
                         "    height:9px;width:8px;"
                         "    border-image:url(:/images/a/3.png);"
                         "    subcontrol-position:bottom;"
                         "}"
                         "QScrollBar::add-page:vertical,QScrollBar::sub-page:vertical"
                         "{"
                         "    background:rgba(0,0,0,0%);"
                         "    border-radius:3px;"
                         "}");

    QString hScrollBarSS("QScrollBar:horizontal"
                         "{"
                         "    height:7px;"
                         "    background:rgba(128,128,128,0%);"
                         "    margin:0px,0px,0px,0px;"
                         "    padding-left:0px;"
                         "    padding-right:0px;"
                         "}"
                         "QScrollBar::handle:horizontal"
                         "{"
                         "    height:7px;"
                         "    background:rgba(128,128,128,32%);"
                         "    border-radius:3px;"
                         "    min-width:20;"
                         "}"
                         "QScrollBar::handle:horizontal:hover"
                         "{"
                         "    height:7px;"
                         "    background:rgba(128,128,128,50%);"
                         "    min-width:20;"
                         "}"
                         "QScrollBar::sub-line:horizontal"
                         "{"
                         "    width:9px;height:8px;"
                         "    border-image:url(:/images/a/1.png);"
                         "    subcontrol-position:top;"
                         "}"
                         "QScrollBar::add-line:horizontal"
                         "{"
                         "    width:9px;height:8px;"
                         "    border-image:url(:/images/a/3.png);"
                         "    subcontrol-position:bottom;"
                         "}"
                         "QScrollBar::add-page:horizontal,QScrollBar::sub-page:horizontal"
                         "{"
                         "    background:rgba(0,0,0,0%);"
                         "    border-radius:3px;"
                         "}");
    ui->orderSongsListView->verticalScrollBar()->setStyleSheet(vScrollBarSS);
    ui->orderSongsListView->horizontalScrollBar()->setStyleSheet(hScrollBarSS);
    ui->favoriteSongsListView->verticalScrollBar()->setStyleSheet(vScrollBarSS);
    ui->favoriteSongsListView->horizontalScrollBar()->setStyleSheet(hScrollBarSS);
    ui->normalSongsListView->verticalScrollBar()->setStyleSheet(vScrollBarSS);
    ui->normalSongsListView->horizontalScrollBar()->setStyleSheet(hScrollBarSS);
    ui->historySongsListView->verticalScrollBar()->setStyleSheet(vScrollBarSS);
    ui->historySongsListView->horizontalScrollBar()->setStyleSheet(hScrollBarSS);
    ui->scrollArea->verticalScrollBar()->setStyleSheet(vScrollBarSS);
    ui->searchResultTable->verticalScrollBar()->setStyleSheet(vScrollBarSS);
    ui->searchResultTable->horizontalScrollBar()->setStyleSheet(hScrollBarSS);
    ui->listTabWidget->removeTab(LISTTAB_PLAYLIST); // TOOD: 歌单部分没做好，先隐藏
    ui->titleButton->setText(settings.value("music/title", "Lazy点歌姬").toString());

    const int btnR = 5;
    ui->titleButton->setRadius(btnR);
    ui->settingsButton->setRadius(btnR);
    ui->playButton->setRadius(btnR);
    ui->nextSongButton->setRadius(btnR);
    ui->volumeButton->setRadius(btnR);
    ui->circleModeButton->setRadius(btnR);
    ui->desktopLyricButton->setRadius(btnR);
    expandPlayingButton->setRadius(btnR);

    if (settings.value("music/hideTab", false).toBool())
        ui->listTabWidget->tabBar()->hide();

    musicSource = static_cast<MusicSource>(settings.value("music/source", 0).toInt());
    setMusicIconBySource();

    QPalette pa;
    pa.setColor(QPalette::Highlight, QColor(100, 149, 237, 88));
    QApplication::setPalette(pa);

    connect(ui->searchResultTable->horizontalHeader(), SIGNAL(sectionClicked(int)), this, SLOT(sortSearchResult(int)));

    player->setNotifyInterval(100);
    connect(player, &QMediaPlayer::positionChanged, this, [=](qint64 position){
        ui->playingCurrentTimeLabel->setText(msecondToString(position));
        slotPlayerPositionChanged();
    });
    connect(player, &QMediaPlayer::durationChanged, this, [=](qint64 duration){
        ui->playProgressSlider->setMaximum(static_cast<int>(duration));
        ui->playingAllTimeLabel->setText(msecondToString(duration));
        if (setPlayPositionAfterLoad)
        {
            player->setPosition(setPlayPositionAfterLoad);
            setPlayPositionAfterLoad = 0;
        }
    });
    connect(player, &QMediaPlayer::mediaStatusChanged, this, [=](QMediaPlayer::MediaStatus status){
        if (status == QMediaPlayer::EndOfMedia)
        {
            slotSongPlayEnd();
        }
        else if (status == QMediaPlayer::InvalidMedia)
        {
            qDebug() << "无效媒体：" << playingSong.simpleString();
            playNext();
        }
    });
    connect(player, &QMediaPlayer::stateChanged, this, [=](QMediaPlayer::State state){
        if (state == QMediaPlayer::PlayingState)
        {
            ui->playButton->setIcon(QIcon(":/icons/pause"));
        }
        else
        {
            ui->playButton->setIcon(QIcon(":/icons/play"));
        }
    });

    connect(ui->lyricWidget, SIGNAL(signalAdjustLyricTime(QString)), this, SLOT(adjustCurrentLyricTime(QString)));

    connect(ui->playProgressSlider, &ClickSlider::signalClickMove, this, [=](int position){
        player->setPosition(position);
    });
    connect(ui->volumeSlider, &ClickSlider::signalClickMove, this, [=](int position){
        player->setVolume(position);
        settings.setValue("music/volume", position);
    });

    musicsFileDir.mkpath(musicsFileDir.absolutePath());
    QTime time;
    time= QTime::currentTime();
    qsrand(time.msec()+time.second()*1000);

    bool lyricStack = settings.value("music/lyricStream", false).toBool();
    if (lyricStack)
        ui->bodyStackWidget->setCurrentWidget(ui->lyricsPage);

    blurBg = settings.value("music/blurBg", blurBg).toBool();
    blurAlpha = settings.value("music/blurAlpha", blurAlpha).toInt();
    themeColor = settings.value("music/themeColor", themeColor).toBool();
    doubleClickToPlay = settings.value("music/doubleClickToPlay", false).toBool();

    // 读取数据
    ui->listTabWidget->setCurrentIndex(settings.value("orderplayerwindow/tabIndex").toInt());
    restoreSongList("music/order", orderSongs);
    restoreSongList("music/favorite", favoriteSongs);
    restoreSongList("music/normal", normalSongs);
    restoreSongList("music/history", historySongs);
    setSongModelToView(orderSongs, ui->orderSongsListView);
    setSongModelToView(favoriteSongs, ui->favoriteSongsListView);
    setSongModelToView(normalSongs, ui->normalSongsListView);
    setSongModelToView(historySongs, ui->historySongsListView);

    int volume = settings.value("music/volume", 50).toInt();
    bool mute = settings.value("music/mute", false).toBool();
    if (mute)
    {
        volume = 0;
        ui->volumeButton->setIcon(QIcon(":/icons/mute"));
        ui->volumeSlider->setSliderPosition(volume);
    }
    ui->volumeSlider->setSliderPosition(volume);
    player->setVolume(volume);

    circleMode = static_cast<PlayCircleMode>(settings.value("music/mode", 0).toInt());
    if (circleMode == OrderList)
        ui->circleModeButton->setIcon(QIcon(":/icons/order_list"));
    else
        ui->circleModeButton->setIcon(QIcon(":/icons/single_circle"));

    connectDesktopLyricSignals();

    bool showDesktopLyric = settings.value("music/desktopLyric", false).toBool();
    if (showDesktopLyric)
    {
        desktopLyric->show();
        ui->desktopLyricButton->setIcon(QIcon(":/icons/lyric_show"));
    }
    else
    {
        desktopLyric->hide();
        ui->desktopLyricButton->setIcon(QIcon(":/icons/lyric_hide"));
    }

    autoSwitchSource = settings.value("music/autoSwitchSource", true).toBool();

    // 读取cookie
    songBr = settings.value("music/br", 320000).toInt();
    neteaseCookies = settings.value("music/neteaseCookies").toString();
    neteaseCookiesVariant = getCookies(neteaseCookies);
    qqmusicCookies = settings.value("music/qqmusicCookies").toString();
    qqmusicCookiesVariant = getCookies(qqmusicCookies);
    unblockQQMusic = settings.value("music/unblockQQMusic").toBool();

    // 还原上次播放的歌曲
    Song currentSong = Song::fromJson(settings.value("music/currentSong").toJsonObject());
    if (currentSong.isValid())
    {
        startPlaySong(currentSong);// 还原位置

        qint64 playPosition = settings.value("music/playPosition", 0).toLongLong();
        if (playPosition)
        {
            setPlayPositionAfterLoad = playPosition;
            slotPlayerPositionChanged();
        }

        // 不自动播放
        player->stop();
    }
    settings.setValue("music/playPosition", 0);

    connect(expandPlayingButton, SIGNAL(clicked()), this, SLOT(slotExpandPlayingButtonClicked()));
    expandPlayingButton->show();

    // 还原搜索结果
    QString searchKey = settings.value("music/searchKey").toString();
    if (!searchKey.isEmpty())
    {
        QTimer::singleShot(100, [=]{
            if (!currentSong.isValid())
                ui->bodyStackWidget->setCurrentWidget(ui->searchResultPage);

            ui->searchEdit->setText(searchKey);
            searchMusic(searchKey);
        });
    }

    prevBlurBg = QPixmap(32, 32);
    prevBlurBg.fill(QColor(245, 245, 247));
    starting = false;

    clearHoaryFiles();
}

OrderPlayerWindow::~OrderPlayerWindow()
{
    delete ui;
    desktopLyric->deleteLater();
}

bool OrderPlayerWindow::hasSongInOrder(QString by)
{
    foreach (Song song, orderSongs)
    {
        if (song.addBy == by)
            return true;
    }
    return false;
}

void OrderPlayerWindow::on_searchEdit_returnPressed()
{
    QString text = ui->searchEdit->text();
    if (text.isEmpty())
        return ;
    settings.setValue("music/searchKey", text);
    searchMusic(text);
    ui->bodyStackWidget->setCurrentWidget(ui->searchResultPage);
}

void OrderPlayerWindow::on_searchButton_clicked()
{
    on_searchEdit_returnPressed();
}

/**
 * 点歌并且添加到末尾
 */
void OrderPlayerWindow::slotSearchAndAutoAppend(QString key, QString by)
{
    ui->searchEdit->setText(key);
    searchMusic(key, by, true);
}

/**
 * 搜索音乐
 */
void OrderPlayerWindow::searchMusic(QString key, QString addBy, bool notify)
{
    if (key.trimmed().isEmpty())
        return ;
    MusicSource source = musicSource; // 需要暂存一个备份，因为可能会变回去
    QString url;
    switch (source) {
    case NeteaseCloudMusic:
        url = NETEASE_SERVER + "/search?keywords=" + key.toUtf8().toPercentEncoding() + "&limit=80";
        break;
    case QQMusic:
        url = QQMUSIC_SERVER + "/getSearchByKey?key=" + key.toUtf8().toPercentEncoding() + "&limit=80";
        break;
    }
    fetch(url, [=](QJsonObject json){
        bool insertOnce = this->insertOrderOnce;
        this->insertOrderOnce = false;
        currentResultOrderBy = addBy;
        prevOrderSong = Song();

        QJsonObject response;
        switch (source) {
        case NeteaseCloudMusic:
            if (json.value("code").toInt() != 200)
            {
                qDebug() << ("返回结果不为200：") << json.value("message").toString();
                return ;
            }
            break;
        case QQMusic:
            QJsonValue val = json.value("response");
            if (!val.isObject()) // 不是正常搜索结果对象
            {
                qDebug() << val;
                return ;
            }
            response = val.toObject();
            if (response.value("code").toInt() != 0)
            {
                qDebug() << ("返回结果不为0：") << json.value("message").toString();
                return ;
            }
            break;
        }

        QJsonArray songs;
        switch (source) {
        case NeteaseCloudMusic:
            songs = json.value("result").toObject().value("songs").toArray();
            break;
        case QQMusic:
            songs = response.value("data").toObject().value("song").toObject().value("list").toArray();
            break;
        }
        searchResultSongs.clear();
        switch (source) {
        case NeteaseCloudMusic:
            foreach (QJsonValue val, songs)
                searchResultSongs << Song::fromJson(val.toObject());
            break;
        case QQMusic:
            foreach (QJsonValue val, songs)
                searchResultSongs << Song::fromQQMusicJson(val.toObject());
            break;
        }

        setSearchResultTable(searchResultSongs);

        // 没有搜索结果
        if (!searchResultSongs.size())
            return ;

        // 从点歌的槽进来的
        if (!addBy.isEmpty())
        {
            Song song = getSuiableSong(key);
            song.setAddDesc(addBy);
            prevOrderSong = song;

            // 添加到点歌列表
            if (playingSong == song || orderSongs.contains(song)) // 重复点歌
                return ;

            if (insertOnce) // 可能是换源过来的
            {
                if (isNotPlaying()) // 很可能是换源过来的
                    startPlaySong(song);
                else
                    appendNextSongs(SongList{song});
            }
            else
                appendOrderSongs(SongList{song});

            // 发送点歌成功的信号
            if (notify)
            {
                qint64 sumLatency = isNotPlaying() ? 0 : player->duration() - player->position();
                for (int i = 0; i < orderSongs.size()-1; i++)
                {
                    if (orderSongs.at(i).id == song.id) // 同一首歌，如果全都不同，那就是下一首
                        break;
                    sumLatency += orderSongs.at(i).duration;
                }
                emit signalOrderSongSucceed(song, sumLatency, orderSongs.size());
            }
        }
        else if (insertOnce) // 强制播放啊
        {
            Song song = getSuiableSong(key);
            if (isNotPlaying()) // 很可能是换源过来的
                startPlaySong(song);
            else
                appendNextSongs(SongList{song});
        }
    });
}

void OrderPlayerWindow::searchMusicBySource(QString key, MusicSource source, QString addBy)
{
    MusicSource originSource = musicSource;
    musicSource = source;
    searchMusic(key, addBy);
    musicSource = originSource;
}

/**
 * 搜索结果数据到Table
 */
void OrderPlayerWindow::setSearchResultTable(SongList songs)
{
    QTableWidget* table = ui->searchResultTable;
    table->clear();
    searchResultPlayLists.clear();
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    enum {
        titleCol,
        artistCol,
        albumCol,
        durationCol
    };
    table->setColumnCount(4);
    QStringList headers{"标题", "艺术家", "专辑", "时长"};
    table->setHorizontalHeaderLabels(headers);

    QFontMetrics fm(font());
    int fw = fm.horizontalAdvance("哈哈哈哈哈哈哈哈哈哈哈哈哈哈哈哈");
    auto createItem = [=](QString s){
        QTableWidgetItem *item = new QTableWidgetItem();
        if (s.length() > 16 && fm.horizontalAdvance(s) > fw)
        {
            item->setToolTip(s);
            s = s.left(15) + "...";
        }
        item->setText(s);
        return item;
    };

    table->setRowCount(songs.size());
    for (int row = 0; row < songs.size(); row++)
    {
        Song song = songs.at(row);
        table->setItem(row, titleCol, createItem(song.name));
        table->setItem(row, artistCol, createItem(song.artistNames));
        table->setItem(row, albumCol, createItem(song.album.name));
        table->setItem(row, durationCol, createItem(msecondToString(song.duration)));
    }
    table->verticalScrollBar()->setSliderPosition(0); // 置顶

    QTimer::singleShot(0, [=]{
        table->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    });
}

void OrderPlayerWindow::setSearchResultTable(PlayListList playLists)
{
    QTableWidget* table = ui->searchResultTable;
    table->clear();
    searchResultSongs.clear();

}

void OrderPlayerWindow::addFavorite(SongList songs)
{
    QPoint center = ui->listTabWidget->tabBar()->tabRect(LISTTAB_FAVORITE).center();
    foreach (auto song, songs)
    {
        if (favoriteSongs.contains(song))
        {
            qDebug() << "歌曲已收藏：" << song.simpleString();
            continue;
        }
        favoriteSongs.append(song);
        showTabAnimation(center, "+1");
        qDebug() << "添加收藏：" << song.simpleString();
    }
    saveSongList("music/favorite", favoriteSongs);
    setSongModelToView(favoriteSongs, ui->favoriteSongsListView);
}

void OrderPlayerWindow::removeFavorite(SongList songs)
{
    QPoint center = ui->listTabWidget->tabBar()->tabRect(LISTTAB_FAVORITE).center();
    foreach (Song song, songs)
    {
        if (favoriteSongs.removeOne(song))
        {
            showTabAnimation(center, "-1");
            qDebug() << "取消收藏：" << song.simpleString();
        }
    }
    saveSongList("music/favorite", favoriteSongs);
    setSongModelToView(favoriteSongs, ui->favoriteSongsListView);
}

void OrderPlayerWindow::addNormal(SongList songs)
{
    QPoint center = ui->listTabWidget->tabBar()->tabRect(LISTTAB_NORMAL).center();
    foreach (Song song, songs)
    {
        normalSongs.removeOne(song);
    }
    for (int i = songs.size()-1; i >= 0; i--)
    {
        normalSongs.insert(0, songs.at(i));
        showTabAnimation(center, "+1");
    }
    saveSongList("music/normal", normalSongs);
    setSongModelToView(normalSongs, ui->normalSongsListView);
}

void OrderPlayerWindow::removeNormal(SongList songs)
{
    QPoint center = ui->listTabWidget->tabBar()->tabRect(LISTTAB_NORMAL).center();
    foreach (Song song, songs)
    {
        if (normalSongs.removeOne(song))
            showTabAnimation(center, "-1");
    }
    saveSongList("music/normal", normalSongs);
    setSongModelToView(normalSongs, ui->normalSongsListView);
}

void OrderPlayerWindow::removeOrder(SongList songs)
{
    QPoint center = ui->listTabWidget->tabBar()->tabRect(LISTTAB_ORDER).center();
    foreach (Song song, songs)
    {
        if (orderSongs.removeOne(song))
            showTabAnimation(center, "-1");
    }
    saveSongList("music/order", orderSongs);
    setSongModelToView(orderSongs, ui->orderSongsListView);
}

void OrderPlayerWindow::saveSongList(QString key, const SongList &songs)
{
    QJsonArray array;
    foreach (Song song, songs)
        array.append(song.toJson());
    settings.setValue(key, array);
}

void OrderPlayerWindow::restoreSongList(QString key, SongList &songs)
{
    QJsonArray array = settings.value(key).toJsonArray();
    foreach (QJsonValue val, array)
        songs.append(Song::fromJson(val.toObject()));
}

/**
 * 更新Model到ListView
 */
void OrderPlayerWindow::setSongModelToView(const SongList &songs, QListView *listView)
{
    QStringList sl;
    foreach (Song song, songs)
    {
        sl << song.simpleString();
    }
    QAbstractItemModel *model = listView->model();
    if (model)
        delete model;
    model = new QStringListModel(sl);
    listView->setModel(model);
}

/**
 * 音乐文件的路径
 */
QString OrderPlayerWindow::songPath(const Song &song) const
{
    switch (song.source) {
    case NeteaseCloudMusic:
        return musicsFileDir.absoluteFilePath("netease_" + snum(song.id) + ".mp3");
    case QQMusic:
        return musicsFileDir.absoluteFilePath("qq_" + snum(song.id) + ".mp3");
    }
}

QString OrderPlayerWindow::lyricPath(const Song &song) const
{
    switch (song.source) {
    case NeteaseCloudMusic:
        return musicsFileDir.absoluteFilePath("netease_" + snum(song.id) + ".lrc");
    case QQMusic:
        return musicsFileDir.absoluteFilePath("qq_" + snum(song.id) + ".lrc");
    }
}

QString OrderPlayerWindow::coverPath(const Song &song) const
{
    switch (song.source) {
    case NeteaseCloudMusic:
        return musicsFileDir.absoluteFilePath("netease_" + snum(song.id) + ".jpg");
    case QQMusic:
        return musicsFileDir.absoluteFilePath("qq_" + snum(song.id) + ".jpg");
    }
}

/**
 * 歌曲是否已经被下载了
 */
bool OrderPlayerWindow::isSongDownloaded(Song song)
{
    return QFileInfo(songPath(song)).exists();
}

QString OrderPlayerWindow::msecondToString(qint64 msecond)
{
    return QString("%1:%2").arg(msecond/1000 / 60, 2, 10, QLatin1Char('0'))
            .arg(msecond/1000 % 60, 2, 10, QLatin1Char('0'));
}

void OrderPlayerWindow::activeSong(Song song)
{
    if (doubleClickToPlay)
        startPlaySong(song);
    else
        appendOrderSongs(SongList{song});
}

bool OrderPlayerWindow::isNotPlaying() const
{
    return player->state() != QMediaPlayer::PlayingState
            && (!playingSong.isValid() || player->position() == 0);
}

/**
 * 从结果中获取最适合的歌曲
 * 优先：歌名-歌手
 * 其次：歌名 歌手
 * 默认：搜索结果第一首歌
 */
Song OrderPlayerWindow::getSuiableSong(QString key) const
{
    Q_ASSERT(searchResultSongs.size());
    key = key.trimmed();

    // 歌名 - 歌手
    if (key.indexOf("-"))
    {
        QRegularExpression re("^\\s*(.+?)\\s*-\\s*(.+?)\\s*$");
        QRegularExpressionMatch match;
        if (key.indexOf(re, 0, &match) > -1)
        {
            QString name = match.captured(1);   // 歌名
            QString author = match.captured(2); // 歌手

            // 歌名、歌手全匹配
            foreach (Song song, searchResultSongs)
            {
                if (song.name == name && song.artistNames == author)
                    return song;
            }

            // 歌名全匹配、歌手包含
            foreach (Song song, searchResultSongs)
            {
                if (song.name == name && song.artistNames.contains(author))
                    return song;
            }

            // 歌名包含、歌手全匹配
            foreach (Song song, searchResultSongs)
            {
                if (song.name.contains(name) && song.artistNames == author)
                    return song;
            }

            // 歌名包含、歌手包含
            foreach (Song song, searchResultSongs)
            {
                if (song.name.contains(name) && song.artistNames.contains(author))
                    return song;
            }
        }
    }

    // 歌 名 歌 手
    if (key.contains(" "))
    {
        int pos = key.lastIndexOf(" ");
        while (pos > -1)
        {
            QString name = key.left(pos).trimmed();
            QString author = key.right(key.length() - pos - 1).trimmed();

            // 歌名、歌手全匹配
            foreach (Song song, searchResultSongs)
            {
                if (song.name == name && song.artistNames == author)
                    return song;
            }

            // 歌名全匹配、歌手包含
            foreach (Song song, searchResultSongs)
            {
                if (song.name == name && song.artistNames.contains(author))
                    return song;
            }

            // 歌名包含(cover)、歌手全匹配
            foreach (Song song, searchResultSongs)
            {
                if (song.name.contains(name) && song.artistNames == author)
                    return song;
            }

            // 歌名包含、歌手包含
            foreach (Song song, searchResultSongs)
            {
                if (song.name.contains(name) && song.artistNames.contains(author))
                    return song;
            }

            pos = key.lastIndexOf(" ", pos-1);
        }
    }

    // 没指定歌手，随便来一个就行
    return searchResultSongs.first();
}

void OrderPlayerWindow::showEvent(QShowEvent *e)
{
    QMainWindow::showEvent(e);

    static bool firstShow = true;
    if (firstShow)
    {
        firstShow = false;
    }

    restoreGeometry(settings.value("orderplayerwindow/geometry").toByteArray());
    restoreState(settings.value("orderplayerwindow/state").toByteArray());
    ui->splitter->restoreState(settings.value("orderplayerwindow/splitterState").toByteArray());
    auto sizes = ui->splitter->sizes();
    if (sizes.at(0)+1 >= sizes.at(1))
    {
        int sum = sizes.at(0) + sizes.at(1);
        int w = sum / 3;
        ui->splitter->setSizes(QList<int>{w, sum - w});
    }

    adjustExpandPlayingButton();

    if (settings.value("music/desktopLyric", false).toBool())
        desktopLyric->show();
}

void OrderPlayerWindow::closeEvent(QCloseEvent *)
{
    settings.setValue("orderplayerwindow/geometry", this->saveGeometry());
    settings.setValue("orderplayerwindow/state", this->saveState());
    settings.setValue("orderplayerwindow/splitterState", ui->splitter->saveState());
    settings.setValue("music/playPosition", player->position());

    if (player->state() == QMediaPlayer::PlayingState)
        player->pause();

    // 保存位置
    if (!desktopLyric->isHidden())
        desktopLyric->close();

    emit signalWindowClosed();
}

void OrderPlayerWindow::resizeEvent(QResizeEvent *)
{
    adjustExpandPlayingButton();
    // setBlurBackground(currentCover); // 太消耗性能了
}

void OrderPlayerWindow::paintEvent(QPaintEvent *e)
{
    QMainWindow::paintEvent(e);

    if (blurBg)
    {
        QPainter painter(this);
        if (!currentBlurBg.isNull())
        {
            painter.setOpacity((double)currentBgAlpha / 255);
            painter.drawPixmap(rect(), currentBlurBg);
        }
        if (!prevBlurBg.isNull() && prevBgAlpha)
        {
            painter.setOpacity((double)prevBgAlpha / 255);
            painter.drawPixmap(rect(), prevBlurBg);
        }
    }
}

void OrderPlayerWindow::setLyricScroll(int x)
{
    this->lyricScroll = x;
}

int OrderPlayerWindow::getLyricScroll() const
{
    return this->lyricScroll;
}

void OrderPlayerWindow::setAppearBgProg(int x)
{
    this->currentBgAlpha = x;
}

int OrderPlayerWindow::getAppearBgProg() const
{
    return this->currentBgAlpha;
}

void OrderPlayerWindow::setDisappearBgProg(int x)
{
    this->prevBgAlpha = x;
}

int OrderPlayerWindow::getDisappearBgProg() const
{
    return this->prevBgAlpha;
}

void OrderPlayerWindow::showTabAnimation(QPoint center, QString text)
{
    center = ui->listTabWidget->mapTo(this, center);
    QColor color = QColor::fromHsl(rand()%360,rand()%256,rand()%200); // 深色的颜色
    NumberAnimation *animation = new NumberAnimation(text, color, this);
    animation->setCenter(center + QPoint(rand() % 32 - 16, rand() % 32 - 16));
    animation->startAnimation();
}

void OrderPlayerWindow::setPaletteBgProg(double x)
{
    this->paletteAlpha = x;
}

double OrderPlayerWindow::getPaletteBgProg() const
{
    return paletteAlpha;
}

/**
 * 搜索结果双击
 * 还没想好怎么做……
 */
void OrderPlayerWindow::on_searchResultTable_cellActivated(int row, int)
{
    if (searchResultSongs.size())
    {
        Song song = searchResultSongs.at(row);
        removeOrder(SongList{song});
        activeSong(song);
    }
    else if (searchResultPlayLists.size())
    {

    }
}

/**
 * 搜索结果的菜单
 */
void OrderPlayerWindow::on_searchResultTable_customContextMenuRequested(const QPoint &)
{
    auto items = ui->searchResultTable->selectedItems();

    // 是歌曲搜索结果
    if (searchResultSongs.size())
    {
        QList<Song> songs;
        foreach (auto item, items)
        {
            int row = ui->searchResultTable->row(item);
            int col = ui->searchResultTable->column(item);
            if (col != 0)
                continue;
            songs.append(searchResultSongs.at(row));
        }
        int row = ui->searchResultTable->currentRow();
        Song currentSong;
        if (row > -1)
            currentSong = searchResultSongs.at(row);

        FacileMenu* menu = new FacileMenu(this);

        if (!currentResultOrderBy.isEmpty())
        {
            currentSong.setAddDesc(currentResultOrderBy);
            menu->addAction(currentResultOrderBy)->disable();
            menu->addAction("换这首歌", [=]{
                int index = orderSongs.indexOf(prevOrderSong);
                if (index == -1)
                {
                    if (playingSong == prevOrderSong) // 正在播放
                    {
                        startPlaySong(currentSong);
                        prevOrderSong = currentSong;
                        return ;
                    }
                    else // 可能被手动删除了
                    {
                        orderSongs.insert(0, currentSong);
                    }
                }
                else // 等待播放，自动替换
                {
                    orderSongs[index] = currentSong;
                }
                prevOrderSong = currentSong;
                saveSongList("music/order", orderSongs);
                setSongModelToView(orderSongs, ui->orderSongsListView);
                addDownloadSong(currentSong);
                downloadNext();
            })->disable(!currentSong.isValid() || !prevOrderSong.isValid());
            menu->split();
        }

        menu->addAction("立即播放", [=]{
            startPlaySong(currentSong);
        })->disable(songs.size() != 1);

        menu->addAction("下一首播放", [=]{
            appendNextSongs(songs);
        })->disable(!currentSong.isValid());

        menu->addAction("添加到播放列表", [=]{
            appendOrderSongs(songs);
        })->disable(!currentSong.isValid());

        menu->addAction("添加固定播放", [=]{
            addNormal(songs);
        })->disable(!currentSong.isValid());

        menu->split()->addAction("收藏", [=]{
            if (!favoriteSongs.contains(currentSong))
                addFavorite(songs);
            else
                removeFavorite(songs);
        })->disable(!currentSong.isValid())
                ->text(favoriteSongs.contains(currentSong), "从收藏中移除", "添加到收藏");

        menu->addAction("打开分享的歌单", [=]{
            QString url = QInputDialog::getText(this, "查看歌单", "支持网易云音乐、QQ音乐的分享链接");
            if (url.isEmpty())
                return ;
            openPlayList(url);
        });

        menu->exec();
    }
    // 是歌单搜索结果
    else if (searchResultPlayLists.size())
    {
        PlayListList lists;
        foreach (auto item, items)
        {
            int row = ui->searchResultTable->row(item);
            int col = ui->searchResultTable->column(item);
            if (col != 0)
                continue;
            lists.append(searchResultPlayLists.at(row));
        }
        int row = ui->searchResultTable->currentRow();
        PlayList currentList;
        if (row > -1)
            currentList = searchResultPlayLists.at(row);

        FacileMenu* menu = new FacileMenu(this);

        menu->addAction("添加到我的歌单", [=]{

        });

        menu->exec();
    }
}

/**
 * 立即播放
 */
void OrderPlayerWindow::startPlaySong(Song song)
{
    if (isSongDownloaded(song))
    {
        playLocalSong(song);
    }
    else
    {
        playAfterDownloaded = song;
        downloadSong(song);
    }
}

void OrderPlayerWindow::playNext()
{
    if (!orderSongs.size()) // 播放列表全部结束
    {
        // 查看固定列表
        if (!normalSongs.size())
            return ;

        int r = qrand() % normalSongs.size();
        startPlaySong(normalSongs.at(r));
        return ;
    }

    Song song = orderSongs.takeFirst();
    saveSongList("music/order", orderSongs);
    setSongModelToView(orderSongs, ui->orderSongsListView);

    startPlaySong(song);
    emit signalOrderSongPlayed(song);
}

/**
 * 添加到点歌队列（末尾）
 */
void OrderPlayerWindow::appendOrderSongs(SongList songs)
{
    QPoint center = ui->listTabWidget->tabBar()->tabRect(LISTTAB_ORDER).center();
    foreach (Song song, songs)
    {
        if (orderSongs.contains(song))
            continue;
        orderSongs.append(song);
        addDownloadSong(song);
        showTabAnimation(center, "+1");
    }

    if (isNotPlaying() && orderSongs.size())
    {
        qDebug() << "当前未播放，开始播放列表";
        startPlaySong(orderSongs.takeFirst());
    }

    saveSongList("music/order", orderSongs);
    setSongModelToView(orderSongs, ui->orderSongsListView);

    downloadNext();
}

/**
 * 下一首播放（点歌队列置顶）
 */
void OrderPlayerWindow::appendNextSongs(SongList songs)
{
    QPoint center = ui->listTabWidget->tabBar()->tabRect(LISTTAB_ORDER).center();
    foreach (Song song, songs)
    {
        if (orderSongs.contains(song))
            orderSongs.removeOne(song);
        orderSongs.insert(0, song);
        addDownloadSong(song);
        showTabAnimation(center, "+1");
    }

    // 一般不会自动播放，除非没有在放的歌
    /*if (isNotPlaying() && !playingSong.isValid() && songs.size())
    {
        qDebug() << "当前未播放，开始播放本首歌";
        startPlaySong(orderSongs.takeFirst());
    }*/

    saveSongList("music/order", orderSongs);
    setSongModelToView(orderSongs, ui->orderSongsListView);

    downloadNext();
}

/**
 * 立刻开始播放音乐
 */
void OrderPlayerWindow::playLocalSong(Song song)
{
    qDebug() << "开始播放：" << song.simpleString();
    if (!isSongDownloaded(song))
    {
        qDebug() << "error: 未下载歌曲" << song.simpleString() << "开始下载";
        playAfterDownloaded = song;
        downloadSong(song);
        return ;
    }

    // 设置信息
    auto max16 = [=](QString s){
        if (s.length() > 16)
            s = s.left(15) + "...";
        return s;
    };
    ui->playingNameLabel->setText(max16(song.name));
    ui->playingArtistLabel->setText(max16(song.artistNames));

    // 设置封面
    if (QFileInfo(coverPath(song)).exists())
    {
        QPixmap pixmap(coverPath(song), "1"); // 这里读取要加个参数，原因未知……
        if (pixmap.isNull())
            qDebug() << "warning: 本地封面是空的" << song.simpleString() << coverPath(song);
        pixmap = pixmap.scaledToHeight(ui->playingCoverLabel->height());
        ui->playingCoverLabel->setPixmap(pixmap);
        setCurrentCover(pixmap);
    }
    else
    {
        downloadSongCover(song);
    }
    // 设置歌词
    if (QFileInfo(lyricPath(song)).exists())
    {
        QFile file(lyricPath(song));
        file.open(QIODevice::ReadOnly | QIODevice::Text);
        QTextStream stream(&file);
        QString lyric;
        QString line;
        while (!stream.atEnd())
        {
            line = stream.readLine();
            lyric.append(line+"\n");
        }
        file.close();

        setCurrentLyric(lyric);
    }
    else
    {
        setCurrentLyric("");
        downloadSongLyric(song);
    }

    // 开始播放
    playingSong = song;
    player->setMedia(QUrl::fromLocalFile(songPath(song)));
    player->setPosition(0);
    player->play();
    emit signalSongPlayStarted(song);
    setWindowTitle(song.name);

    // 添加到历史记录
    historySongs.removeOne(song);
    historySongs.insert(0, song);
    saveSongList("music/history", historySongs);
    setSongModelToView(historySongs, ui->historySongsListView);

    // 保存当前歌曲
    settings.setValue("music/currentSong", song.toJson());
}

/**
 * 放入下载队列，准备下载（并不立即下载）
 */
void OrderPlayerWindow::addDownloadSong(Song song)
{
    if (isSongDownloaded(song) || toDownloadSongs.contains(song) || downloadingSong == song)
        return ;
    toDownloadSongs.append(song);
}

/**
 * 放入下载队列、或一首歌下载完毕，开始下载下一个
 */
void OrderPlayerWindow::downloadNext()
{
    if (downloadingSong.isValid() || !toDownloadSongs.size())
        return ;
    Song song = toDownloadSongs.takeFirst();
    if (!song.isValid())
        return downloadNext();

    downloadSong(song);
}

/**
 * 下载音乐
 */
void OrderPlayerWindow::downloadSong(Song song)
{
    if (isSongDownloaded(song))
        return ;
    downloadingSong = song;
    bool unblockQQMusic = this->unblockQQMusic; // 保存状态，避免下载的时候改变

    QString url;
    switch (song.source) {
    case NeteaseCloudMusic:
        url = NETEASE_SERVER + "/song/url?id=" + snum(song.id) + "&br=" + snum(songBr);
        break;
    case QQMusic:
        if (unblockQQMusic)
            url = "http://www.douqq.com/qqmusic/qqapi.php?mid=" + song.mid;
        else
            url = url = QQMUSIC_SERVER + "/getMusicPlay?songmid=" + song.mid;
        break;
    }

    MUSIC_DEB << "获取歌曲信息：" << song.simpleString() << url;
    fetch(url, [=](QNetworkReply* reply){
        QByteArray baData = reply->readAll();

        if (song.source == QQMusic && unblockQQMusic)
        {
            // 这个API是野生找的，需要额外处理
            baData.replace("\\\"", "\"").replace("\\\\", "\\").replace("\\/", "/");
            if (baData.startsWith("\""))
                baData.replace(0, 1, "");
            if (baData.endsWith("\""))
                baData.replace(baData.length()-1, 1, "");
        }

        QJsonParseError error;
        QJsonDocument document = QJsonDocument::fromJson(baData, &error);
        if (error.error != QJsonParseError::NoError)
        {
            qWarning() << "解析歌曲信息出错" << error.errorString() << baData << url;
            return ;
        }
        QJsonObject json = document.object();
        QString fileUrl;
        switch (song.source) {
        case NeteaseCloudMusic:
        {
            if (json.value("code").toInt() != 200)
            {
                qDebug() << ("歌曲信息返回结果不为200：") << json.value("message").toString();
                return ;
            }

            QJsonArray array = json.value("data").toArray();
            if (!array.size())
            {
                qDebug() << "未找到歌曲：" << song.simpleString();
                downloadingSong = Song();
                downloadNext();
                return ;
            }
            json = array.first().toObject();
            fileUrl = JVAL_STR(url);
            int br = JVAL_INT(br); // 比率320000
            int size = JVAL_INT(size);
            QString type = JVAL_STR(type); // mp3
            QString encodeType = JVAL_STR(encodeType); // mp3
            qDebug() << "    信息：" << br << size << type << fileUrl;
            if (size == 0)
            {
                qDebug() << "无法下载，可能没有版权" << song.simpleString();
                if (playAfterDownloaded == song)
                {
                    if (orderSongs.contains(song))
                    {
                        orderSongs.removeOne(song);
                        saveSongList("music/order", orderSongs);
                        setSongModelToView(orderSongs, ui->orderSongsListView);
                    }
                    if (autoSwitchSource && song.source == musicSource
                            /*&& !song.addBy.isEmpty()*/) // 只有点歌才自动换源，普通播放自动跳过
                    {
                        slotSongPlayEnd(); // 先停止播放，然后才会开始播放新的；否则会插入到下一首
                        qDebug() << "无法播放：" << song.name << "，开始换源";
                        insertOrderOnce = true;
                        searchMusicBySource(song.name, musicSource == NeteaseCloudMusic
                                            ? QQMusic : NeteaseCloudMusic, song.addBy);
                    }
                    else
                    {
                        playNext();
                    }
                }

                downloadingSong = Song();
                downloadNext();
                return ;
            }
            break;
        }
        case QQMusic:
        {
            if (unblockQQMusic)
            {
                /*{
                    "mid": "001fApzM4Rymgq",
                    "m4a": "http:\/\/aqqmusic.tc.qq.com\/amobile.music.tc.qq.com\/C400001fApzM4Rymgq.m4a?guid=2095717240&vkey=23EB99E476D5EB7936AF440461D097689A00AE211B2D1725B9703A9459A1239301BF19E0A098B30CC9F4503C8DBDA65FD85E60E133944F13&uin=0&fromtag=38",
                    "mp3_l": "http:\/\/mobileoc.music.tc.qq.com\/M500001fApzM4Rymgq.mp3?guid=2095717240&vkey=23EB99E476D5EB7936AF440461D097689A00AE211B2D1725B9703A9459A1239301BF19E0A098B30CC9F4503C8DBDA65FD85E60E133944F13&uin=0&fromtag=53",
                    "mp3_h": "http:\/\/mobileoc.music.tc.qq.com\/M800001fApzM4Rymgq.mp3?guid=2095717240&vkey=23EB99E476D5EB7936AF440461D097689A00AE211B2D1725B9703A9459A1239301BF19E0A098B30CC9F4503C8DBDA65FD85E60E133944F13&uin=0&fromtag=53",
                    "ape": "http:\/\/mobileoc.music.tc.qq.com\/A000001fApzM4Rymgq.ape?guid=2095717240&vkey=23EB99E476D5EB7936AF440461D097689A00AE211B2D1725B9703A9459A1239301BF19E0A098B30CC9F4503C8DBDA65FD85E60E133944F13&uin=0&fromtag=53",
                    "flac": "http:\/\/mobileoc.music.tc.qq.com\/F000001fApzM4Rymgq.flac?guid=2095717240&vkey=23EB99E476D5EB7936AF440461D097689A00AE211B2D1725B9703A9459A1239301BF19E0A098B30CC9F4503C8DBDA65FD85E60E133944F13&uin=0&fromtag=53",
                    "songname": "\u6000\u74a7\u8d74\u524d\u5c18",
                    "albumname": "\u603b\u507f\u76f8\u601d",
                    "singername": "\u53f8\u590f",
                    "pic": "https:\/\/y.gtimg.cn\/music\/photo_new\/T002R300x300M0000022qglb0tOda5_1.jpg?max_age=2592000",
                    "mv": "\u6682\u65e0MV",
                    "lrc": "[ti:]\n[ar:]\n[al:]\n[by:\u65f6\u95f4\u6233\u5de5\u5177V2.0_20200505]\n[offset:0]\n[00:00.00]\u6000\u74a7\u8d74\u524d\u5c18\n[00:01.50]\u4f5c\u8bcd\uff1a\u6d41\u5149\n[00:03.01]\u4f5c\/\u7f16\u66f2\uff1a\u4e00\u53ea\u9c7c\u5361\n[00:04.51]\u6f14\u5531\uff1a\u53f8\u590f\n[00:06.02]\u4fee\u97f3\uff1a\u6c64\u5706w\n[00:07.53]\u6df7\u97f3\uff1aWuli\u5305\u5b50\n[00:09.03]\u6587\u6848\uff1a\u8c22\u77e5\u8a00\n[00:10.54]\u7f8e\u5de5\uff1a\u4f5c\u8086\n[00:12.04]\u5236\u4f5c\u4eba\uff1a\u6c90\u4e88\n[00:13.55]\u51fa\u54c1\uff1a\u7c73\u6f2b\u4f20\u5a92\n[00:15.06]\u5236\u4f5c\uff1a\u8d4f\u4e50\u8ba1\u5212\n[00:16.56]\u827a\u672f\u7edf\u7b79\uff1a\u697c\u5c0f\u6728\n[00:18.07]\u53d1\u884c\uff1a\u7396\u8bb8\n[00:19.59]\u9879\u76ee\u76d1\u7763\uff1a\u5b59\u534e\u5b81\n[00:27.51]\u522b\u6765\u5df2\u662f\u767e\u5e74\u8eab\n[00:33.39]\u788c\u788c\u7ea2\u5c18\u8001\u82b1\u6708\u60c5\u6839\n[00:40.08]\u9752\u9752\u9b02\u4ed8\u9752\u9752\u575f\n[00:43.56]\u76f8\u601d\u4e24\u5904\u5173\u5c71\u963b\u68a6\u9b42\n[00:52.77]\u541b\u5b50\u6380\u5e18\u773a\u6625\u6df1\n[00:58.71]\u6c49\u768b\u89e3\u4f69\u6000\u74a7\u9057\u4f73\u4eba\n[01:05.34]\u98de\u5149\u4e0d\u6765\u96ea\u76c8\u6a3d\n[01:11.28]\u4f73\u4eba\u53bb\u4e5f\u70df\u6708\u4ff1\u6c89\u6c89\n[01:15.15]\u5979\u6709\u51b0\u96ea\u9b42\u9b44\u4e03\u5e74\u9010\u70df\u6ce2\n[01:23.76]\u6f14\u4e00\u526f\u591a\u60c5\u8eab\u7ea2\u5c18\u91cc\u6d88\u78e8\n[01:30.30]\u6b4c\u6b7b\u751f\u5951\u9614\u5531\u4e0e\u5b50\u6210\u8bf4\n[01:36.15]\u5374\u5c06\u75f4\u5fc3\u4e00\u63e1\u6258\u4f53\u540c\u5c71\u963f\n[02:08.52]\u541b\u5b50\u6380\u5e18\u773a\u6625\u6df1\n[02:14.46]\u6c49\u768b\u89e3\u4f69\u6000\u74a7\u9057\u4f73\u4eba\n[02:21.12]\u98de\u5149\u4e0d\u6765\u96ea\u76c8\u6a3d\n[02:27.03]\u4f73\u4eba\u53bb\u4e5f\u70df\u6708\u4ff1\u6c89\u6c89\n[02:32.67]\u5979\u6709\u51b0\u96ea\u9b42\u9b44\u4e03\u5e74\u9010\u70df\u6ce2\n[02:39.00]\u6f14\u4e00\u526f\u591a\u60c5\u8eab\u7ea2\u5c18\u91cc\u6d88\u78e8\n[02:46.08]\u6b4c\u6b7b\u751f\u5951\u9614\u5531\u4e0e\u5b50\u6210\u8bf4\n[02:51.99]\u5374\u5c06\u75f4\u5fc3\u4e00\u63e1\u6258\u4f53\u540c\u5c71\u963f\n[02:56.13]\u5979\u6709\u51b0\u96ea\u9b42\u9b44\u4e03\u5e74\u9010\u70df\u6ce2\n[03:05.04]\u6f14\u4e00\u526f\u591a\u60c5\u8eab\u7ea2\u5c18\u91cc\u6d88\u78e8\n[03:11.37]\u6b4c\u6b7b\u751f\u5951\u9614\u5531\u4e0e\u5b50\u6210\u8bf4\n[03:17.22]\u5374\u5c06\u75f4\u5fc3\u4e00\u63e1\u6258\u4f53\u540c\u5c71\u963f\n[04:00.00]"
                }*/
                QString m4a = json.value("m4a").toString(); // 视频？但好像能直接播放
                QString mp3_l = json.value("mp3_l").toString(); // 普通品质
                QString mp3_h = json.value("mp3_h").toString(); // 高品质
                QString ape = json.value("ape").toString(); // 高品无损
                QString flac = json.value("flac").toString(); // 无损音频

                // 实测只有 m4a 能播放……
                if (!m4a.isEmpty())
                    fileUrl = m4a;
                else if (!mp3_h.isEmpty())
                    fileUrl = mp3_h;
                else if (!mp3_l.isEmpty())
                    fileUrl = mp3_l;
                else if (!ape.isEmpty())
                    fileUrl = ape;
                else if (!flac.isEmpty())
                    fileUrl = flac;
            }
            else
            {
                QJsonObject playUrl = json.value("data").toObject().value("playUrl").toObject();
                fileUrl = playUrl.value(song.mid).toObject().value("url").toString();
                if (fileUrl.isEmpty())
                {
                    QString error = playUrl.value(song.mid).toObject().value("error").toString();
                    if (!error.isEmpty())
                        qDebug() << "无法播放音乐：" << song.simpleString() << error;
                }
            }

            if (fileUrl.isEmpty())
            {
                qDebug() << "无法下载，歌曲不存在或没有版权" << song.simpleString();
                if (playAfterDownloaded == song)
                {
                    if (orderSongs.contains(song))
                    {
                        orderSongs.removeOne(song);
                    }
                    playNext();
                }

                downloadingSong = Song();
                downloadNext();
                return ;
            }

            break;
        }
        }
        MUSIC_DEB << "歌曲下载直链：" << fileUrl;

        // 开始下载歌曲本身
        QNetworkAccessManager manager;
        QEventLoop loop;
        QNetworkReply *reply1 = manager.get(QNetworkRequest(QUrl(fileUrl)));
        //请求结束并下载完成后，退出子事件循环
        connect(reply1, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        //开启子事件循环
        loop.exec();
        QByteArray mp3Ba = reply1->readAll();
        if (mp3Ba.isEmpty())
        {
            qWarning() << "无法下载歌曲，可能缺少版权：" << song.simpleString();
            downloadingSong = Song();
            downloadNext();
            return ;
        }

        // 解析MP3标签
        /*try {
            readMp3Data(mp3Ba);
        } catch(...) {
            qDebug() << "读取音乐标签出错";
        }*/

        // 保存到文件
        QFile file(songPath(song));
        file.open(QIODevice::WriteOnly);
        file.write(mp3Ba);
        file.flush();
        file.close();

        emit signalSongDownloadFinished(song);

        if (playAfterDownloaded == song)
            playLocalSong(song);

        downloadingSong = Song();
        downloadNext();
    });

    downloadSongLyric(song);
    downloadSongCover(song);
}

void OrderPlayerWindow::downloadSongLyric(Song song)
{
    if (QFileInfo(lyricPath(song)).exists())
        return ;

//    downloadingSong = song; // 忘了为什么要加这句了？

    QString url;
    switch (song.source) {
    case NeteaseCloudMusic:
        url = NETEASE_SERVER + "/lyric?id=" + snum(song.id);
        break;
    case QQMusic:
        url = QQMUSIC_SERVER + "/getLyric?songmid=" + song.mid;
        break;
    }

    fetch(url, [=](QJsonObject json){
        QString lrc;
        switch (song.source) {
        case NeteaseCloudMusic:
            if (json.value("code").toInt() != 200)
            {
                qDebug() << ("返回结果不为200：") << json.value("message").toString();
                return ;
            }
            lrc = json.value("lrc").toObject().value("lyric").toString();
            break;
        case QQMusic:
            lrc = json.value("response").toObject().value("lyric").toString();
            break;
        }
        if (!lrc.isEmpty())
        {
            QFile file(lyricPath(song));
            file.open(QIODevice::WriteOnly);
            QTextStream stream(&file);
            stream << lrc;
            file.flush();
            file.close();

            MUSIC_DEB << "下载歌词完成：" << song.simpleString();
            if (playAfterDownloaded == song || playingSong == song)
            {
                setCurrentLyric(lrc);
            }

            emit signalLyricDownloadFinished(song);
        }
        else
        {
            qWarning() << "warning: 下载的歌词是空的" << song.simpleString() << url;
        }
    });
}

void OrderPlayerWindow::downloadSongCover(Song song)
{
    if (QFileInfo(coverPath(song)).exists())
        return ;
//    downloadingSong = song; // 忘了为什么要加这句了？

    QString url;
    switch (song.source) {
    case NeteaseCloudMusic:
        url = NETEASE_SERVER + "/song/detail?ids=" + snum(song.id);
        break;
    case QQMusic:
        url = QQMUSIC_SERVER + "/getImageUrl?id=" + song.album.mid;
        break;
    }

    MUSIC_DEB << "封面信息url:" << url;
    fetch(url, [=](QJsonObject json){
        QString url;
        switch (song.source) {
        case NeteaseCloudMusic:
        {
            if (json.value("code").toInt() != 200)
            {
                qDebug() << ("返回结果不为200：") << json.value("message").toString();
                return ;
            }
            QJsonArray array = json.value("songs").toArray();
            if (!array.size())
            {
                qDebug() << "未找到歌曲：" << song.simpleString();
                return ;
            }

            json = array.first().toObject();
            url = json.value("al").toObject().value("picUrl").toString();
            break;
        }
        case QQMusic:
            if (json.value("code").toInt() != 0)
            {
                qDebug() << ("封面返回结果不为0：") << json.value("message").toString();
                return ;
            }
            url = json.value("response").toObject().value("data").toObject().value("imageUrl").toString();
            break;
        }

        MUSIC_DEB << "封面地址：" << url;

        // 开始下载封面数据
        QNetworkAccessManager manager;
        QEventLoop loop;
        QNetworkReply *reply1 = manager.get(QNetworkRequest(QUrl(url)));
        //请求结束并下载完成后，退出子事件循环
        connect(reply1, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        //开启子事件循环
        loop.exec();
        QByteArray baData1 = reply1->readAll();
        QPixmap pixmap;
        pixmap.loadFromData(baData1);
        if (!pixmap.isNull())
        {
            QFile file(coverPath(song));
            file.open(QIODevice::WriteOnly);
            file.write(baData1);
            file.flush();
            file.close();

            emit signalCoverDownloadFinished(song);

            // 正是当前要播放的歌曲
            if (playAfterDownloaded == song || playingSong == song)
            {
                pixmap = pixmap.scaledToHeight(ui->playingCoverLabel->height());
                ui->playingCoverLabel->setPixmap(pixmap);
                setCurrentCover(pixmap);
            }
        }
        else
        {
            qDebug() << "warning: 下载的封面是空的" << song.simpleString();
        }
    });
}

/**
 * 设置当前歌曲的歌词到歌词控件
 */
void OrderPlayerWindow::setCurrentLyric(QString lyric)
{
    desktopLyric->setLyric(lyric);
    ui->lyricWidget->setLyric(lyric);
}

void OrderPlayerWindow::openPlayList(QString shareUrl)
{
    MusicSource source;
    QString playlistUrl;
    QString id;
    qDebug() << "歌单URL：" << shareUrl;
    if (shareUrl.contains("music.163.com/playlist?")) // 网易云音乐的分享
    {
        source = this->musicSource;
        QRegularExpression re("id=(\\d+)");
        QRegularExpressionMatch match;
        if (shareUrl.indexOf(re, 0, &match) == -1)
        {
            qWarning() << "无法解析歌单链接：" << shareUrl;
            return ;
        }
        id = match.captured(1);
        playlistUrl = NETEASE_SERVER + "/playlist/detail?id=" + id;
    }
    else if (shareUrl.contains("c.y.qq.com/base/fcgi-bin/u?__=")) // QQ音乐压缩
    {
        // https://c.y.qq.com/base/fcgi-bin/u?__=HdLP6Kp
        if (!shareUrl.startsWith("http"))
            shareUrl = "https://" + shareUrl;

        // 检测重定向
        fetch(shareUrl, [=](QNetworkReply* reply){
            QString url = reply->rawHeader("Location");
            return openPlayList(url);
        });
        return ;
    }
    else if (shareUrl.contains("y.qq.com/w/taoge.html")) // QQ音乐短网址第一次重定向
    {
        // https://y.qq.com/w/taoge.html?ADTAG=erweimashare&channelId=10036163&id=7845417918&openinqqmusic=1
        source = QQMusic;
        QRegularExpression re("[\\?&]id=(\\d+)");
        QRegularExpressionMatch match;
        if (shareUrl.indexOf(re, 0, &match) == -1)
        {
            qWarning() << "无法解析歌单链接：" << shareUrl;
            return ;
        }
        id = match.captured(1);
        playlistUrl = QQMUSIC_SERVER + "/getSongListDetail?disstid=" + id;
    }
    else if (shareUrl.contains("y.qq.com/n/yqq/playlist/")) // QQ音乐短网址第二次重定向（网页打开的）
    {
        // https://y.qq.com/n/yqq/playlist/7845417918.html
        source = QQMusic;
        QRegularExpression re("y.qq.com/n/yqq/playlist/(\\d+)");
        QRegularExpressionMatch match;
        if (shareUrl.indexOf(re, 0, &match) == -1)
        {
            qWarning() << "无法解析歌单链接：" << shareUrl;
            return ;
        }
        id = match.captured(1);
        playlistUrl = QQMUSIC_SERVER + "/getSongListDetail?disstid=" + id;
    }
    else
    {
        qWarning() << "无法解析歌单链接：" << shareUrl;
        return ;
    }

    MUSIC_DEB << "歌单接口：" << playlistUrl;
    fetch(playlistUrl, [=](QJsonObject json){
        QJsonObject response;
        SongList songs;
        switch (source) {
        case NeteaseCloudMusic:
        {
            if (json.value("code").toInt() != 200)
            {
                qDebug() << ("歌单返回结果不为200：") << json.value("message").toString();
                return ;
            }
            QJsonArray array = json.value("playlist").toObject().value("tracks").toArray();
            searchResultSongs.clear();
            foreach (QJsonValue val, array)
            {
                searchResultSongs << Song::fromNeteaseShareJson(val.toObject());
            }
            setSearchResultTable(searchResultSongs);
            ui->bodyStackWidget->setCurrentWidget(ui->searchResultPage);
            break;
        }
        case QQMusic:
        {
            response = json.value("response").toObject();
            if (response.value("code").toInt() != 0)
            {
                qDebug() << ("返回结果不为0：") << json.value("message").toString();
                return ;
            }
            QJsonArray array = response.value("cdlist").toArray().first().toObject().value("songlist").toArray();
            searchResultSongs.clear();
            foreach (QJsonValue val, array)
            {
                searchResultSongs << Song::fromQQMusicJson(val.toObject());
            }
            setSearchResultTable(searchResultSongs);
            ui->bodyStackWidget->setCurrentWidget(ui->searchResultPage);
            break;
        }
        }
    });

}

void OrderPlayerWindow::clearDownloadFiles()
{
    QList<QFileInfo> files = musicsFileDir.entryInfoList(QDir::Files);
    foreach (QFileInfo info, files)
    {
        QFile f;
        f.remove(info.absoluteFilePath());
    }
}

void OrderPlayerWindow::clearHoaryFiles()
{
    qint64 current = QDateTime::currentSecsSinceEpoch();
    QList<QFileInfo> files = musicsFileDir.entryInfoList(QDir::Files);
    foreach (QFileInfo info, files)
    {
        if (info.lastModified().toSecsSinceEpoch() + 604800 < current) // 七天前的
        {
            QFile f;
            f.remove(info.absoluteFilePath());
        }
    }
}

void OrderPlayerWindow::adjustExpandPlayingButton()
{
    QRect rect(ui->playingCoverLabel->mapTo(this, QPoint(0,0)), QSize(ui->listTabWidget->width(), ui->playingCoverLabel->height()));
    expandPlayingButton->setGeometry(rect);
    expandPlayingButton->raise();
}

void OrderPlayerWindow::connectDesktopLyricSignals()
{
    connect(desktopLyric, &DesktopLyricWidget::signalhide, this, [=]{
        ui->desktopLyricButton->setIcon(QIcon(":/icons/lyric_hide"));
        settings.setValue("music/desktopLyric", false);
    });
    connect(desktopLyric, &DesktopLyricWidget::signalSwitchTrans, this, [=]{
        desktopLyric->close();
        desktopLyric->deleteLater();

        desktopLyric = new DesktopLyricWidget(settings, nullptr);
        connectDesktopLyricSignals();
        desktopLyric->show();

        if (playingSong.isValid())
        {
            Song song = playingSong;
            if (QFileInfo(lyricPath(song)).exists())
            {
                QFile file(lyricPath(song));
                file.open(QIODevice::ReadOnly | QIODevice::Text);
                QTextStream stream(&file);
                QString lyric;
                QString line;
                while (!stream.atEnd())
                {
                    line = stream.readLine();
                    lyric.append(line+"\n");
                }
                file.close();

                desktopLyric->setLyric(lyric);
                desktopLyric->setPosition(player->position());
            }
        }
    });
}

void OrderPlayerWindow::setCurrentCover(const QPixmap &pixmap)
{
    currentCover = pixmap;
    if (themeColor)
        setThemeColor(currentCover);
    if (blurBg)
        setBlurBackground(currentCover);
}

void OrderPlayerWindow::setBlurBackground(const QPixmap &bg)
{
    if (bg.isNull())
        return ;

    // 当前图片变为上一张图
    prevBgAlpha = currentBgAlpha;
    prevBlurBg = currentBlurBg;

    // 开始模糊
    const int radius = qMax(20, qMin(width(), height())/5);
    QPixmap pixmap = bg;
    pixmap = pixmap.scaled(this->width()+radius*2, this->height() + radius*2, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    QImage img = pixmap.toImage(); // img -blur-> painter(pixmap)
    QPainter painter( &pixmap );
    qt_blurImage( &painter, img, radius, true, false );

    // 裁剪掉边缘（模糊后会有黑边）
    int c = qMin(bg.width(), bg.height());
    c = qMin(c/2, radius);
    QPixmap clip = pixmap.copy(c, c, pixmap.width()-c*2, pixmap.height()-c*2);

    // 抽样获取背景，设置之后的透明度
    qint64 rgbSum = 0;
    QImage image = clip.toImage();
    int w = image.width(), h = image.height();
    const int m = 16;
    for (int y = 0; y < m; y++)
    {
        for (int x = 0; x < m; x++)
        {
            QColor c = image.pixelColor(w*x/m, h*x/m);
            rgbSum += c.red() + c.green() + c.blue();
        }
    }
    int addin = rgbSum * blurAlpha / (255*3*m*m);

    // 半透明
    currentBlurBg = clip;
    currentBgAlpha = qMin(255, blurAlpha + addin);

    // 出现动画
    QPropertyAnimation* ani1 = new QPropertyAnimation(this, "appearBgProg");
    ani1->setStartValue(0);
    ani1->setEndValue(currentBgAlpha);
    ani1->setDuration(1000);
    ani1->setEasingCurve(QEasingCurve::OutCubic);
    connect(ani1, &QPropertyAnimation::valueChanged, this, [=](const QVariant& val){
        update();
    });
    connect(ani1, &QPropertyAnimation::finished, this, [=]{
        ani1->deleteLater();
    });
    currentBgAlpha = 0;
    ani1->start();

    // 消失动画
    QPropertyAnimation* ani2 = new QPropertyAnimation(this, "disappearBgProg");
    ani2->setStartValue(prevBgAlpha);
    ani2->setEndValue(0);
    ani2->setDuration(1000);
    ani2->setEasingCurve(QEasingCurve::OutCubic);
    connect(ani2, &QPropertyAnimation::valueChanged, this, [=](const QVariant& val){
        prevBgAlpha = val.toInt();
        update();
    });
    connect(ani2, &QPropertyAnimation::finished, this, [=]{
        prevBlurBg = QPixmap();
        ani2->deleteLater();
        update();
    });
    ani2->start();
}

void OrderPlayerWindow::setThemeColor(const QPixmap &cover)
{
    QColor bg, fg, sbg, sfg;
    auto colors = ImageUtil::extractImageThemeColors(cover.toImage(), 7);
    ImageUtil::getBgFgSgColor(colors, &bg, &fg, &sbg, &sfg);

    prevPa = BFSColor::fromPalette(palette());
    currentPa = BFSColor(QList<QColor>{bg, fg,sbg, sfg});

    QPropertyAnimation* ani = new QPropertyAnimation(this, "paletteProg");
    ani->setStartValue(0);
    ani->setEndValue(1.0);
    ani->setDuration(500);
    connect(ani, &QPropertyAnimation::valueChanged, this, [=](const QVariant& val){
        double d = val.toDouble();
        BFSColor bfs = prevPa + (currentPa - prevPa) * d;
        QColor bg, fg, sbg, sfg;
        bfs.toColors(&bg, &fg, &sbg, &sfg);

        QPalette pa;
        pa.setColor(QPalette::Window, bg);
        pa.setColor(QPalette::Background, bg);
        pa.setColor(QPalette::Button, bg);

        pa.setColor(QPalette::Foreground, fg);
        pa.setColor(QPalette::Text, fg);
        pa.setColor(QPalette::ButtonText, fg);
        pa.setColor(QPalette::WindowText, fg);

        pa.setColor(QPalette::Highlight, sbg);
        pa.setColor(QPalette::HighlightedText, sfg);

        QApplication::setPalette(pa);
        setPalette(pa);

        ui->lyricWidget->setColors(sfg, fg);
        desktopLyric->setColors(sfg, fg);
        ui->playingNameLabel->setPalette(pa);
        ui->titleButton->setPalette(pa);
        ui->titleButton->setTextColor(fg);
    });
    connect(ani, SIGNAL(finished()), ani, SLOT(deleteLater()));
    ani->start();

    // 菜单直接切换，不进行动画
    QColor halfSg = sfg;
    halfSg.setAlpha(halfSg.alpha() / 2);
    FacileMenu::normal_bg = bg;
    FacileMenu::hover_bg = halfSg;
    FacileMenu::press_bg = sfg;
    FacileMenu::text_fg = fg;

    MUSIC_DEB << "当前颜色：" << bg << fg << sfg;
}

/**
 * 参考链接：https://blog.csdn.net/weixin_37608233/article/details/82930197
 * 仅供测试
 */
void OrderPlayerWindow::readMp3Data(const QByteArray &array)
{
    // ID3V2 标签头
    std::string header = array.mid(0, 3).toStdString(); // [3] 必须为ID3
    char ver = *array.mid(3, 1).data(); // [1] 版本号03=v2.3, 04=v2.4
    char revision = *array.mid(4, 1).data(); // [1] 副版本号
    char flag = *array.mid(5, 1).data(); // [1] 标志
    char* sizes = array.mid(6, 4).data(); // [4] 标签大小，包括标签帧和标签头（不包括扩展标签头的10字节）
    int size = (sizes[0]<<24)+(sizes[1]<<16)+(sizes[2]<<8)+sizes[3];
    qDebug() << QString::fromStdString(header) << size;

    QHash<QString, QString>frameIds;
    frameIds.insert("TIT2", "标题");
    frameIds.insert("TPE1", "作者");
    frameIds.insert("TALB", "专辑");
    frameIds.insert("TRCK", "音轨"); // 直接数字 // ???格式：N/M 其中N为专集中的第N首，M为专集中共M首，N和M为ASCII码表示的数字
    frameIds.insert("TYER", "年代"); // 用ASCII码表示的数字
    frameIds.insert("TCON", "类型"); // 直接用字符串表示
    frameIds.insert("COMM", "备注"); // 格式："eng\0备注内容"，其中eng表示备注所使用的自然语言
    frameIds.insert("APIC", "专辑图片"); // png文件标志的前两个字节为89 50；jpeg文件标志的前两个字节为FF,D8；
    frameIds.insert("TSSE", "专辑图片"); // Lavf56.4.101

    // 标签帧
    int pos = 10; // 标签头10字节
    while (pos < size)
    {
        std::string frameId = array.mid(pos, 4).toStdString(); // [4] 帧标志
        char* sizes = array.mid(pos+4, 4).data(); // [4] 帧内容大小（不包括帧头）
        char* frameFlag = array.mid(pos+8, 2).data(); // [2] 存放标志
        int frameSize = (sizes[0]<<24)+(sizes[1]<<16)+(sizes[2]<<8)+sizes[3];
        qDebug() << "pos =" << pos << "    id =" << QString::fromStdString(frameId) << "   size =" << frameSize;
        pos += 10; // 帧标签10字节
        if (frameIds.contains(QString::fromStdString(frameId)))
        {
            QByteArray ba;
            if (*array.mid(pos, 1).data() == 0 && *array.mid(pos+frameSize-1, 1).data()==0)
                ba = array.mid(pos+1, frameSize-2);
            else
                ba = array.mid(pos, frameSize);
            qDebug() << frameIds.value(QString::fromStdString(frameId)) << ba;
        }
        else if (frameSize < 1000)
            qDebug() << array.mid(pos, frameSize);
        else
            qDebug() << array.mid(pos, 100) << "...";
        pos += frameSize; // 帧内容x字节
    }

}

void OrderPlayerWindow::setMusicIconBySource()
{
    switch (musicSource) {
    case NeteaseCloudMusic:
        ui->musicSourceButton->setIcon(QIcon(":/musics/netease"));
        ui->musicSourceButton->setToolTip("当前播放源：网易云音乐\n点击切换");
        break;
    case QQMusic:
        ui->musicSourceButton->setIcon(QIcon(":/musics/qq"));
        ui->musicSourceButton->setToolTip("当前播放源：QQ音乐\n点击切换");
        break;
    }
}

void OrderPlayerWindow::fetch(QString url, NetStringFunc func)
{
    fetch(url, [=](QNetworkReply* reply){
        func(reply->readAll());
    });
}

void OrderPlayerWindow::fetch(QString url, NetJsonFunc func)
{
    fetch(url, [=](QNetworkReply* reply){
        QJsonParseError error;
        QJsonDocument document = QJsonDocument::fromJson(reply->readAll(), &error);
        if (error.error != QJsonParseError::NoError)
        {
            qDebug() << error.errorString();
            return ;
        }
        func(document.object());
    });
}

void OrderPlayerWindow::fetch(QString url, NetReplyFunc func)
{
    QNetworkAccessManager* manager = new QNetworkAccessManager;
    QNetworkRequest* request = new QNetworkRequest(url);
    request->setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded; charset=UTF-8");
    request->setHeader(QNetworkRequest::UserAgentHeader, "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/86.0.4240.111 Safari/537.36");
    if (!neteaseCookies.isEmpty() && url.startsWith(NETEASE_SERVER))
        request->setHeader(QNetworkRequest::CookieHeader, neteaseCookiesVariant);
    else if (!qqmusicCookies.isEmpty() && url.startsWith(QQMUSIC_SERVER))
        request->setHeader(QNetworkRequest::CookieHeader, qqmusicCookiesVariant);
    connect(manager, &QNetworkAccessManager::finished, this, [=](QNetworkReply* reply){
        manager->deleteLater();
        delete request;

        func(reply);
        reply->deleteLater();
    });
    manager->get(*request);
}

QVariant OrderPlayerWindow::getCookies(QString cookieString)
{
    QList<QNetworkCookie> cookies;

    // 设置cookie
    QString cookieText = cookieString;
    QStringList sl = cookieText.split(";");
    foreach (auto s, sl)
    {
        s = s.trimmed();
        int pos = s.indexOf("=");
        QString key = s.left(pos);
        QString val = s.right(s.length() - pos - 1);
        cookies.push_back(QNetworkCookie(key.toUtf8(), val.toUtf8()));
    }

    // 请求头里面加入cookies
    QVariant var;
    var.setValue(cookies);
    return var;
}

/**
 * 列表项改变
 */
void OrderPlayerWindow::on_listTabWidget_currentChanged(int index)
{
    if (starting)
        return ;
    settings.setValue("orderplayerwindow/tabIndex", index);
}

/**
 * 改变排序
 */
void OrderPlayerWindow::sortSearchResult(int col)
{
    Q_UNUSED(col)
}

/**
 * 播放进度条被拖动
 */
void OrderPlayerWindow::on_playProgressSlider_sliderReleased()
{
    int position = ui->playProgressSlider->sliderPosition();
    player->setPosition(position);
}

/**
 * 播放进度条被拖动
 */
void OrderPlayerWindow::on_playProgressSlider_sliderMoved(int position)
{
    player->setPosition(position);
}

/**
 * 音量进度被拖动
 */
void OrderPlayerWindow::on_volumeSlider_sliderMoved(int position)
{
    player->setVolume(position);
    settings.setValue("music/volume", position);
}

/**
 * 暂停/播放/随机推荐
 */
void OrderPlayerWindow::on_playButton_clicked()
{
    if (player->state() == QMediaPlayer::PlayingState) // 暂停播放
    {
        player->pause();
    }
    else // 开始播放
    {
        if (!playingSong.isValid())
        {
            playNext();
            return ;
        }
        player->play();
    }
}

/**
 * 静音/恢复音量
 */
void OrderPlayerWindow::on_volumeButton_clicked()
{
    int volume = ui->volumeSlider->sliderPosition();
    if (volume == 0) // 恢复音量
    {
        volume = settings.value("music/volume", 50).toInt();
        if (volume == 0)
            volume = 50;
        ui->volumeButton->setIcon(QIcon(":/icons/volume"));
        ui->volumeSlider->setSliderPosition(volume);
        settings.setValue("music/mute", false);
        settings.setValue("music/volume", volume);
    }
    else // 静音
    {
        volume = 0;
        ui->volumeButton->setIcon(QIcon(":/icons/mute"));
        ui->volumeSlider->setSliderPosition(0);
        settings.setValue("music/mute", true);
    }
    player->setVolume(volume);
}

/**
 * 循环方式
 */
void OrderPlayerWindow::on_circleModeButton_clicked()
{
    if (circleMode == OrderList)
    {
        circleMode = SingleCircle;
        ui->circleModeButton->setIcon(QIcon(":/icons/single_circle"));
    }
    else
    {
        circleMode = OrderList;
        ui->circleModeButton->setIcon(QIcon(":/icons/order_list"));
    }
    settings.setValue("music/mode", circleMode);
}

void OrderPlayerWindow::slotSongPlayEnd()
{
    emit signalSongPlayFinished(playingSong);
    // 根据循环模式
    if (circleMode == OrderList) // 列表顺序
    {
        // 清除播放
        player->stop();
        playingSong = Song();
        settings.setValue("music/currentSong", "");
        ui->playingNameLabel->clear();
        ui->playingArtistLabel->clear();
        ui->playingCoverLabel->clear();
        setCurrentLyric("");

        // 下一首歌，没有就不放
        playNext();
    }
    else if (circleMode == SingleCircle) // 单曲循环
    {
        // 不用管，会自己放下去
        player->setPosition(0);
        player->play();
    }
}

void OrderPlayerWindow::on_orderSongsListView_customContextMenuRequested(const QPoint &)
{
    auto indexes = ui->orderSongsListView->selectionModel()->selectedRows(0);
    SongList songs;
    foreach (auto index, indexes)
        songs.append(orderSongs.at(index.row()));
    int row = ui->orderSongsListView->currentIndex().row();
    Song currentSong;
    if (row > -1)
        currentSong = orderSongs.at(row);

    FacileMenu* menu = new FacileMenu(this);

    if (!currentSong.addBy.isEmpty())
    {
        currentSong.setAddDesc(currentSong.addBy);
        menu->addAction(currentSong.addBy)->disable();
        menu->split();
    }

    menu->addAction("立即播放", [=]{
        Song song = orderSongs.takeAt(row);
        startPlaySong(song);
        orderSongs.removeOne(song);
        saveSongList("music/order", orderSongs);
        setSongModelToView(orderSongs, ui->orderSongsListView);
    })->disable(songs.size() != 1 || !currentSong.isValid());

    menu->addAction("下一首播放", [=]{
        appendNextSongs(songs);
    })->disable(!songs.size());

    menu->split()->addAction("添加固定播放", [=]{
        addNormal(songs);
    })->disable(!currentSong.isValid());

    menu->addAction("收藏", [=]{
        if (!favoriteSongs.contains(currentSong))
            addFavorite(songs);
        else
            removeFavorite(songs);
    })->disable(!currentSong.isValid())
            ->text(favoriteSongs.contains(currentSong), "从收藏中移除", "添加到收藏");

    menu->split()->addAction("上移", [=]{
        orderSongs.swapItemsAt(row, row-1);
        saveSongList("music/order", orderSongs);
        setSongModelToView(orderSongs, ui->orderSongsListView);
    })->disable(songs.size() != 1 || row < 1);

    menu->addAction("下移", [=]{
        orderSongs.swapItemsAt(row, row+1);
        saveSongList("music/order", orderSongs);
        setSongModelToView(orderSongs, ui->orderSongsListView);
    })->disable(songs.size() != 1 || row >= orderSongs.size()-1);

    menu->split()->addAction("删除", [=]{
        removeOrder(songs);
    })->disable(!songs.size());

    menu->exec();
}

void OrderPlayerWindow::on_favoriteSongsListView_customContextMenuRequested(const QPoint &)
{
    auto indexes = ui->favoriteSongsListView->selectionModel()->selectedRows(0);
    SongList songs;
    foreach (auto index, indexes)
        songs.append(favoriteSongs.at(index.row()));
    int row = ui->favoriteSongsListView->currentIndex().row();
    Song currentSong;
    if (row > -1)
        currentSong = favoriteSongs.at(row);

    FacileMenu* menu = new FacileMenu(this);

    menu->addAction("立即播放", [=]{
        Song song = favoriteSongs.takeAt(row);
        startPlaySong(song);
    })->disable(songs.size() != 1 || !currentSong.isValid());

    menu->split()->addAction("下一首播放", [=]{
        appendNextSongs(songs);
    })->disable(!songs.size());

    menu->addAction("添加到播放列表", [=]{
        appendOrderSongs(songs);
    })->disable(!songs.size());

    menu->addAction("添加固定播放", [=]{
        addNormal(songs);
    })->disable(!currentSong.isValid());

    menu->split()->addAction("上移", [=]{
        favoriteSongs.swapItemsAt(row, row-1);
        saveSongList("music/order", favoriteSongs);
        setSongModelToView(favoriteSongs, ui->favoriteSongsListView);
    })->disable(songs.size() != 1 || row < 1);

    menu->addAction("下移", [=]{
        favoriteSongs.swapItemsAt(row, row+1);
        saveSongList("music/order", favoriteSongs);
        setSongModelToView(favoriteSongs, ui->favoriteSongsListView);
    })->disable(songs.size() != 1 || row >= favoriteSongs.size()-1);

    menu->addAction("打开分享的歌单", [=]{
        QString url = QInputDialog::getText(this, "查看歌单", "支持网易云音乐、QQ音乐的分享链接");
        if (url.isEmpty())
            return ;
        openPlayList(url);
    });

    menu->split()->addAction("移出收藏", [=]{
        removeFavorite(songs);
    })->disable(!songs.size());

    menu->exec();
}

void OrderPlayerWindow::on_listSongsListView_customContextMenuRequested(const QPoint &pos)
{
    auto indexes = ui->favoriteSongsListView->selectionModel()->selectedRows(0);
    int row = ui->listSongsListView->currentIndex().row();
    PlayList pl;
    if (row > -1)
        ;

    FacileMenu* menu = new FacileMenu(this);

    menu->addAction("加载歌单", [=]{

    });

    menu->addAction("播放里面歌曲", [=]{

    });

    menu->split()->addAction("删除歌单", [=]{

    });

    menu->exec();
}

void OrderPlayerWindow::on_normalSongsListView_customContextMenuRequested(const QPoint &pos)
{
    auto indexes = ui->normalSongsListView->selectionModel()->selectedRows(0);
    SongList songs;
    foreach (auto index, indexes)
        songs.append(normalSongs.at(index.row()));
    int row = ui->normalSongsListView->currentIndex().row();
    Song currentSong;
    if (row > -1)
        currentSong = normalSongs.at(row);

    FacileMenu* menu = new FacileMenu(this);

    menu->addAction("立即播放", [=]{
        Song song = normalSongs.takeAt(row);
        startPlaySong(song);
    })->disable(songs.size() != 1 || !currentSong.isValid());

    menu->split()->addAction("下一首播放", [=]{
        appendNextSongs(songs);
    })->disable(!songs.size());

    menu->addAction("添加到播放列表", [=]{
        appendOrderSongs(songs);
    })->disable(!songs.size());

    menu->addAction("收藏", [=]{
        if (!favoriteSongs.contains(currentSong))
            addFavorite(songs);
        else
            removeFavorite(songs);
    })->disable(!currentSong.isValid())
            ->text(favoriteSongs.contains(currentSong), "从收藏中移除", "添加到收藏");

    menu->split()->addAction("上移", [=]{
        normalSongs.swapItemsAt(row, row-1);
        saveSongList("music/order", normalSongs);
        setSongModelToView(normalSongs, ui->normalSongsListView);
    })->disable(songs.size() != 1 || row < 1);

    menu->addAction("下移", [=]{
        normalSongs.swapItemsAt(row, row+1);
        saveSongList("music/order", normalSongs);
        setSongModelToView(normalSongs, ui->normalSongsListView);
    })->disable(songs.size() != 1 || row >= normalSongs.size()-1);

    menu->addAction("打开分享的歌单", [=]{
        QString url = QInputDialog::getText(this, "查看歌单", "支持网易云音乐、QQ音乐的分享链接");
        if (url.isEmpty())
            return ;
        openPlayList(url);
    });

    menu->split()->addAction("移出固定播放", [=]{
        removeNormal(songs);
    })->disable(!songs.size());

    menu->exec();
}

void OrderPlayerWindow::on_historySongsListView_customContextMenuRequested(const QPoint &)
{
    auto indexes = ui->historySongsListView->selectionModel()->selectedRows(0);
    SongList songs;
    foreach (auto index, indexes)
        songs.append(historySongs.at(index.row()));
    int row = ui->historySongsListView->currentIndex().row();
    Song currentSong;
    if (row > -1)
        currentSong = historySongs.at(row);

    FacileMenu* menu = new FacileMenu(this);

    if (!currentSong.addBy.isEmpty())
    {
        currentSong.setAddDesc(currentSong.addBy);
        menu->addAction(currentSong.addBy)->disable();
        menu->split();
    }

    menu->addAction("立即播放", [=]{
        Song song = historySongs.takeAt(row);
        startPlaySong(song);
    })->disable(songs.size() != 1 || !currentSong.isValid());

    menu->split()->addAction("下一首播放", [=]{
        appendNextSongs(songs);
    })->disable(!songs.size());

    menu->addAction("添加到播放列表", [=]{
        appendOrderSongs(songs);
    })->disable(!songs.size());

    menu->addAction("添加固定播放", [=]{
        addNormal(songs);
    })->disable(!currentSong.isValid());

    menu->addAction("收藏", [=]{
        if (!favoriteSongs.contains(currentSong))
            addFavorite(songs);
        else
            removeFavorite(songs);
    })->disable(!currentSong.isValid())
            ->text(favoriteSongs.contains(currentSong), "从收藏中移除", "添加到收藏");

    menu->split()->addAction("清理下载文件", [=]{
        foreach (Song song, songs)
        {
            QString path = songPath(song);
            if (QFileInfo(path).exists())
                QFile(path).remove();
            path = coverPath(song);
            if (QFileInfo(path).exists())
                QFile(path).remove();
            path = lyricPath(song);
            if (QFileInfo(path).exists())
                QFile(path).remove();
        }
    })->disable(!currentSong.isValid());

    menu->addAction("删除记录", [=]{
        QPoint center = ui->listTabWidget->tabBar()->tabRect(LISTTAB_HISTORY).center();
        foreach (Song song, songs)
        {
            if (historySongs.removeOne(song))
                showTabAnimation(center, "+1");
        }
        saveSongList("music/history", historySongs);
        setSongModelToView(historySongs, ui->historySongsListView);
    })->disable(!songs.size());

    menu->exec();
}

void OrderPlayerWindow::on_orderSongsListView_activated(const QModelIndex &index)
{
    Song song = orderSongs.takeAt(index.row());
    setSongModelToView(orderSongs, ui->orderSongsListView);
    startPlaySong(song);
}

void OrderPlayerWindow::on_favoriteSongsListView_activated(const QModelIndex &index)
{
    Song song = favoriteSongs.at(index.row());
    activeSong(song);
}

void OrderPlayerWindow::on_normalSongsListView_activated(const QModelIndex &index)
{
    Song song = normalSongs.at(index.row());
    activeSong(song);
}

void OrderPlayerWindow::on_historySongsListView_activated(const QModelIndex &index)
{
    Song song = historySongs.at(index.row());
    activeSong(song);
}

void OrderPlayerWindow::on_desktopLyricButton_clicked()
{
    bool showDesktopLyric = !settings.value("music/desktopLyric", false).toBool();
    settings.setValue("music/desktopLyric", showDesktopLyric);
    if (showDesktopLyric)
    {
        desktopLyric->show();
        ui->desktopLyricButton->setIcon(QIcon(":/icons/lyric_show"));
    }
    else
    {
        desktopLyric->hide();
        ui->desktopLyricButton->setIcon(QIcon(":/icons/lyric_hide"));
    }
}

void OrderPlayerWindow::slotExpandPlayingButtonClicked()
{
    if (ui->bodyStackWidget->currentWidget() == ui->lyricsPage) // 隐藏歌词
    {
        QRect rect(ui->bodyStackWidget->mapTo(this, QPoint(5, 0)), ui->bodyStackWidget->size()-QSize(5, 0));
        QPixmap pixmap(rect.size());
        render(&pixmap, QPoint(0, 0), rect);
        QLabel* label = new QLabel(this);
        label->setScaledContents(true);
        label->setGeometry(rect);
        label->setPixmap(pixmap);
        QPropertyAnimation* ani = new QPropertyAnimation(label, "geometry");
        ani->setStartValue(rect);
        ani->setEndValue(QRect(ui->playingCoverLabel->geometry().center(), QSize(1,1)));
        ani->setEasingCurve(QEasingCurve::InOutCubic);
        ani->setDuration(300);
        connect(ani, &QPropertyAnimation::finished, this, [=]{
            ani->deleteLater();
            label->deleteLater();
        });
        label->show();
        ani->start();
        ui->bodyStackWidget->setCurrentWidget(ui->searchResultPage);
        settings.setValue("music/lyricStream", false);
    }
    else // 显示歌词
    {
        ui->bodyStackWidget->setCurrentWidget(ui->lyricsPage);
        QRect rect(ui->bodyStackWidget->mapTo(this, QPoint(5, 0)), ui->bodyStackWidget->size()-QSize(5, 0));
        QPixmap pixmap(rect.size());
        render(&pixmap, QPoint(0, 0), rect);
        QLabel* label = new QLabel(this);
        label->setScaledContents(true);
        label->setGeometry(rect);
        label->setPixmap(pixmap);
        QPropertyAnimation* ani = new QPropertyAnimation(label, "geometry");
        ani->setStartValue(QRect(ui->playingCoverLabel->geometry().center(), QSize(1,1)));
        ani->setEndValue(rect);
        ani->setDuration(300);
        ani->setEasingCurve(QEasingCurve::InOutCubic);
        connect(ani, &QPropertyAnimation::finished, this, [=]{
            ui->bodyStackWidget->setCurrentWidget(ui->lyricsPage);
            ani->deleteLater();
            label->deleteLater();
        });
        label->show();
        ani->start();
        ui->bodyStackWidget->setCurrentWidget(ui->searchResultPage);
        settings.setValue("music/lyricStream", true);
    }
}

void OrderPlayerWindow::slotPlayerPositionChanged()
{
    qint64 position = player->position();
    if (desktopLyric && !desktopLyric->isHidden())
        desktopLyric->setPosition(position);
    if (ui->lyricWidget->setPosition(position))
    {
        // 开始滚动
        QPropertyAnimation* ani = new QPropertyAnimation(this, "lyricScroll");
        ani->setStartValue(ui->scrollArea->verticalScrollBar()->sliderPosition());
        ani->setEndValue(qMax(0, ui->lyricWidget->getCurrentTop() - this->height()/2));
        ani->setDuration(200);
        connect(ani, &QPropertyAnimation::valueChanged, this, [=]{
            ui->scrollArea->verticalScrollBar()->setSliderPosition(lyricScroll);
        });
        connect(ani, SIGNAL(finished()), ani, SLOT(deleteLater()));
        ani->start();
    }
    ui->playProgressSlider->setSliderPosition(static_cast<int>(position));
    update();
}

void OrderPlayerWindow::on_splitter_splitterMoved(int pos, int index)
{
    adjustExpandPlayingButton();
}

void OrderPlayerWindow::on_titleButton_clicked()
{
    QString text = settings.value("music/title").toString();
    bool ok;
    QString rst = QInputDialog::getText(this, "请输入名称", "显示在左上角，不影响其他任何效果", QLineEdit::Normal, text, &ok);
    if (!ok)
        return ;
    settings.setValue("music/title", rst);
    ui->titleButton->setText(rst);
}

/**
 * 调整当前歌词的时间
 */
void OrderPlayerWindow::adjustCurrentLyricTime(QString lyric)
{
    if (!playingSong.isValid())
        return ;
    QFile file(lyricPath(playingSong));
    file.open(QIODevice::WriteOnly);
    QTextStream stream(&file);
    stream << lyric;
    file.flush();
    file.close();

    // 调整桌面歌词
    desktopLyric->setLyric(lyric);
    desktopLyric->setPosition(player->position());
}

void OrderPlayerWindow::on_settingsButton_clicked()
{
    FacileMenu* menu = new FacileMenu(this);

    FacileMenu* accountMenu = menu->addMenu("账号");

    accountMenu->addAction("添加账号", [=]{
        LoginDialog* dialog = new LoginDialog(this);
        connect(dialog, &LoginDialog::signalLogined, this, [=](MusicSource source, QString cookies){
            switch (source) {
            case NeteaseCloudMusic:
                neteaseCookies = cookies;
                settings.setValue("music/neteaseCookies", cookies);
                neteaseCookiesVariant = getCookies(neteaseCookies);
                break;
            case QQMusic:
                qqmusicCookies = cookies;
                settings.setValue("music/qqmusicCookies", cookies);
                qqmusicCookiesVariant = getCookies(qqmusicCookies);
                if (unblockQQMusic)
                    settings.setValue("music/unblockQQMusic", unblockQQMusic = false);
                break;
            }
            clearDownloadFiles();
        });
        dialog->exec();
    })->uncheck();

    accountMenu->split();
    accountMenu->addAction("网易云音乐", [=]{
        if (QMessageBox::question(this, "网易云音乐", "是否退出网易云音乐账号？") == QMessageBox::Yes)
        {
            neteaseCookies = "";
            neteaseCookiesVariant.clear();
            settings.setValue("music/neteaseCookies", "");
        }
    })->check(!neteaseCookies.isEmpty())->disable(neteaseCookies.isEmpty());

    accountMenu->addAction("QQ音乐", [=]{
        if (QMessageBox::question(this, "QQ音乐", "是否退出QQ音乐账号？") == QMessageBox::Yes)
        {
            qqmusicCookies = "";
            qqmusicCookiesVariant.clear();
            settings.setValue("music/qqmusicCookies", "");
        }
    })->check(!qqmusicCookies.isEmpty())->disable(qqmusicCookies.isEmpty());

    FacileMenu* playMenu = menu->addMenu("播放");

    playMenu->addAction("音乐品质", [=]{
        bool ok = false;
        int br = QInputDialog::getInt(this, "设置码率", "请输入音乐码率，越高越清晰，体积也更大\n修改后将清理所有缓存，重新下载歌曲文件", songBr, 128000, 1280000, 10000, &ok);
        if (!ok)
            return ;
        if (songBr != br)
            clearDownloadFiles();
        settings.setValue("music/br", songBr = br);
    })->check(songBr >= 320000);

    playMenu->addAction("双击播放", [=]{
        settings.setValue("music/doubleClickToPlay", doubleClickToPlay = !doubleClickToPlay);
    })->check(doubleClickToPlay);

    playMenu->split()->addAction("自动换源", [=]{
        settings.setValue("music/autoSwitchSource", autoSwitchSource = !autoSwitchSource);
    })->setChecked(autoSwitchSource);

    playMenu->addAction("特殊接口", [=]{
        settings.setValue("music/unblockQQMusic", unblockQQMusic = !unblockQQMusic);
        if (unblockQQMusic)
            QMessageBox::information(this, "特殊接口", "可在不登录的情况下试听QQ音乐的VIP歌曲1分钟\n若已登录QQ音乐的会员用户，十分建议关掉");
    })->check(unblockQQMusic);

    playMenu->split()->addAction("清理缓存", [=]{
        clearDownloadFiles();
    })->uncheck();

    FacileMenu* stMenu = menu->addMenu("设置");

    bool h = settings.value("music/hideTab", false).toBool();

    stMenu->addAction("模糊背景", [=]{
        settings.setValue("music/blurBg", blurBg = !blurBg);
        if (blurBg)
            setBlurBackground(currentCover);
        update();
    })->setChecked(blurBg);

    QStringList sl{"32", "64", "96", "128"/*, "160", "192", "224", "256"*/};
    auto blurAlphaMenu = stMenu->addMenu("模糊透明");
    stMenu->lastAction()->hide(!blurBg)->uncheck();
    blurAlphaMenu->addOptions(sl, blurAlpha / 32 - 1, [=](int index){
        blurAlpha = (index+1) * 32;
        settings.setValue("music/blurAlpha", blurAlpha);
        setBlurBackground(currentCover);
    });

    stMenu->addAction("主题变色", [=]{
        settings.setValue("music/themeColor", themeColor = !themeColor);
        if (themeColor)
            setThemeColor(currentCover);
        update();
    })->setChecked(themeColor);

    stMenu->split()->addAction("隐藏Tab", [=]{
        settings.setValue("music/hideTab", !h);
        if (h)
            ui->listTabWidget->tabBar()->show();
        else
            ui->listTabWidget->tabBar()->hide();
    })->check(h);

    menu->exec();
}

void OrderPlayerWindow::on_musicSourceButton_clicked()
{
    musicSource = (MusicSource)(((int)musicSource + 1) % 2);
    settings.setValue("music/source", musicSource);

    setMusicIconBySource();

    QString text = ui->searchEdit->text();
    if (!text.isEmpty())
    {
        searchMusic(text, currentResultOrderBy);
        ui->bodyStackWidget->setCurrentWidget(ui->searchResultPage);
    }
    else
    {
        ui->searchResultTable->clear();
    }
}

void OrderPlayerWindow::on_nextSongButton_clicked()
{
    if (orderSongs.size())
        playNext();
    else
        slotSongPlayEnd(); // 顺序停止播放；单曲重新播放
}
