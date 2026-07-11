#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QTabWidget>
#include <QCursor>
#include <QSettings>
#include <QCloseEvent>
#include <QDebug>
#include <QIntValidator>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), m_isHardModeOn(false)
{
    m_pomodoroTimer = new QTimer(this);
    m_timeLeftSeconds = 25 * 60;
    m_isWorkSession = true;

    m_clickPlayer = new QMediaPlayer(this);
    m_clickAudioOutput = new QAudioOutput(this);
    m_clickPlayer->setAudioOutput(m_clickAudioOutput);
    m_clickPlayer->setSource(QUrl("qrc:/sounds/click.wav"));
    m_clickAudioOutput->setVolume(0.6);

    m_coinPlayer = new QMediaPlayer(this);
    m_coinAudioOutput = new QAudioOutput(this);
    m_coinPlayer->setAudioOutput(m_coinAudioOutput);
    m_coinPlayer->setSource(QUrl("qrc:/sounds/coin.wav"));
    m_coinAudioOutput->setVolume(0.7);

    setupUI();
    applyStyle();

    connect(m_pomodoroTimer, &QTimer::timeout, this, &MainWindow::updateTimer);
}

MainWindow::~MainWindow() {}

void MainWindow::setupUI()
{
    resize(480, 750);
    setWindowTitle("Eat the Frog");

    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    QVBoxLayout *rootLayout = new QVBoxLayout(centralWidget);

    // 0. Верхняя панель баланса (скрыта на главном экране)
    m_topPanelWidget = new QWidget(this);
    QHBoxLayout *topPanelLayout = new QHBoxLayout(m_topPanelWidget);
    topPanelLayout->setContentsMargins(0, 0, 0, 0);
    m_hardModeBtn = new QPushButton("Hard-Mode", this);
    m_hardModeBtn->setCheckable(true);
    m_hardModeBtn->setStyleSheet("background-color: #5D6D7E; min-width: 100px;");
    m_balanceLabel = new QLabel(this);
    m_balanceLabel->setObjectName("BalanceLabel");
    topPanelLayout->addWidget(m_hardModeBtn);
    topPanelLayout->addStretch();
    topPanelLayout->addWidget(m_balanceLabel);
    m_topPanelWidget->hide(); // Изначально скрываем баланс
    rootLayout->addWidget(m_topPanelWidget);

    m_stackedWidget = new QStackedWidget(this);
    rootLayout->addWidget(m_stackedWidget);


    // ==========================================
    // --- ЭКРАН 1: ГЛАВНЫЙ ПРИВЕТСТВЕННЫЙ ---
    // ==========================================
    m_welcomePage = new QWidget(this);
    QVBoxLayout *welcomeLayout = new QVBoxLayout(m_welcomePage);
    welcomeLayout->setAlignment(Qt::AlignCenter);

    QLabel *titleLabel = new QLabel("Eat the frog", this);
    titleLabel->setObjectName("WelcomeTitle"); // Имя для стилизации заголовка
    titleLabel->setAlignment(Qt::AlignCenter);
    welcomeLayout->addWidget(titleLabel);

    QPushButton *continueBtn = new QPushButton("Продолжить", this);
    continueBtn->setObjectName("ContinueButton"); // Имя для стилизации зеленой кнопки
    welcomeLayout->addWidget(continueBtn);

    QPushButton *resetBtn = new QPushButton("Начать сначала", this);
    resetBtn->setObjectName("ResetButton"); // Имя для стилизации красной кнопки
    welcomeLayout->addWidget(resetBtn);

    m_stackedWidget->addWidget(m_welcomePage);


    // ==========================================
    // --- ЭКРАН 2: ОСНОВНОЕ РАБОЧЕЕ ПРОСТРАНСТВО ---
    // ==========================================
    m_mainWorkspacePage = new QWidget(this);
    QVBoxLayout *workspaceLayout = new QVBoxLayout(m_mainWorkspacePage);
    workspaceLayout->setContentsMargins(0, 0, 0, 0);

    QTabWidget *tabs = new QTabWidget(this);

    // Вкладка задач
    QWidget *todoPage = new QWidget();
    QVBoxLayout *todoLayout = new QVBoxLayout(todoPage);

    m_taskAddWidget = new QWidget(this);
    QHBoxLayout *taskAddLayout = new QHBoxLayout(m_taskAddWidget);
    taskAddLayout->setContentsMargins(0, 0, 0, 0);
    m_taskInput = new QLineEdit(this);
    m_taskInput->setPlaceholderText("Новая задача...");
    m_taskCategoryCombo = new QComboBox(this);
    m_taskCategoryCombo->addItems({"Must Do", "Nice to Do"});
    QPushButton *addTaskBtn = new QPushButton("Добавить", this);
    taskAddLayout->addWidget(m_taskInput);
    taskAddLayout->addWidget(m_taskCategoryCombo);
    taskAddLayout->addWidget(addTaskBtn);
    todoLayout->addWidget(m_taskAddWidget);

    QGroupBox *mustDoBox = new QGroupBox("ОБЯЗАТЕЛЬНО (Must Do)", this);
    QVBoxLayout *mustLayout = new QVBoxLayout(mustDoBox);
    m_mustDoList = new QListWidget(this);
    mustLayout->addWidget(m_mustDoList);
    todoLayout->addWidget(mustDoBox);

    m_niceToDoBox = new QGroupBox("ЖЕЛАТЕЛЬНО (Nice to Do)", this);
    QVBoxLayout *niceLayout = new QVBoxLayout(m_niceToDoBox);
    m_niceToDoList = new QListWidget(this);
    niceLayout->addWidget(m_niceToDoList);
    todoLayout->addWidget(m_niceToDoBox);

    tabs->addTab(todoPage, "Задачи");

    // Вкладка Помодоро
    QWidget *pomodoroPage = new QWidget();
    QVBoxLayout *pomoLayout = new QVBoxLayout(pomodoroPage);
    pomoLayout->setAlignment(Qt::AlignCenter);

    m_timerLabel = new QLabel("25:00", this);
    m_timerLabel->setObjectName("TimerLabel");
    m_timerLabel->setAlignment(Qt::AlignCenter);
    pomoLayout->addWidget(m_timerLabel);

    QHBoxLayout *timerButtonsLayout = new QHBoxLayout();
    m_startTimerBtn = new QPushButton("Старт", this);
    QPushButton *resetTimerBtn = new QPushButton("Сброс", this);
    timerButtonsLayout->addWidget(m_startTimerBtn);
    timerButtonsLayout->addWidget(resetTimerBtn);
    pomoLayout->addLayout(timerButtonsLayout);

    tabs->addTab(pomodoroPage, "Таймер Pomodoro");

    // Вкладка наград
    QWidget *rewardPage = new QWidget();
    QVBoxLayout *rewardLayout = new QVBoxLayout(rewardPage);

    QGroupBox *addRewardBox = new QGroupBox("Создать свою награду", this);
    QHBoxLayout *rewardAddLayout = new QHBoxLayout(addRewardBox);
    m_rewardNameInput = new QLineEdit(this);
    m_rewardNameInput->setPlaceholderText("Название отдыха...");
    m_rewardPriceInput = new QLineEdit(this);
    m_rewardPriceInput->setPlaceholderText("Цена");
    m_rewardPriceInput->setMaximumWidth(60);
    m_rewardPriceInput->setValidator(new QIntValidator(1, 9999, this));
    QPushButton *addRewardBtn = new QPushButton("Создать", this);
    rewardAddLayout->addWidget(m_rewardNameInput);
    rewardAddLayout->addWidget(m_rewardPriceInput);
    rewardAddLayout->addWidget(addRewardBtn);
    rewardLayout->addWidget(addRewardBox);

    m_rewardList = new QListWidget(this);
    QPushButton *buyRewardBtn = new QPushButton("Купить награду", this);
    rewardLayout->addWidget(new QLabel("Обменяйте монеты на отдых:"));
    rewardLayout->addWidget(m_rewardList);
    rewardLayout->addWidget(buyRewardBtn);

    tabs->addTab(rewardPage, "Магазин наград");

    workspaceLayout->addWidget(tabs);
    m_stackedWidget->addWidget(m_mainWorkspacePage);

    // Связи главного экрана
    connect(continueBtn, &QPushButton::clicked, this, &MainWindow::onContinuePressed);
    connect(resetBtn, &QPushButton::clicked, this, &MainWindow::onResetPressed);

    // Связи рабочего пространства
    connect(addTaskBtn, &QPushButton::clicked, this, &MainWindow::addTask);
    connect(addRewardBtn, &QPushButton::clicked, this, &MainWindow::addCustomReward);
    connect(m_mustDoList, &QListWidget::itemDoubleClicked, this, &MainWindow::completeTask);
    connect(m_niceToDoList, &QListWidget::itemDoubleClicked, this, &MainWindow::completeTask);
    connect(buyRewardBtn, &QPushButton::clicked, this, &MainWindow::buyReward);

    connect(m_startTimerBtn, &QPushButton::clicked, this, &MainWindow::toggleTimer);
    connect(resetTimerBtn, &QPushButton::clicked, this, &MainWindow::resetTimer);
    connect(m_hardModeBtn, &QPushButton::clicked, this, &MainWindow::toggleHardMode);
}

void MainWindow::onContinuePressed()
{
    playClickSound();
    loadSettings(); // Загружаем старый прогресс
    m_topPanelWidget->show(); // Показываем кошелек баланса
    m_stackedWidget->setCurrentWidget(m_mainWorkspacePage); // Открываем задачи
}

void MainWindow::onResetPressed()
{
    playClickSound();

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Сброс", "Вы уверены, что хотите обнулить весь прогресс?",
                                  QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        clearAllProgress(); // Зачищаем настройки на диске
        m_topPanelWidget->show();
        m_stackedWidget->setCurrentWidget(m_mainWorkspacePage);
    }
}

void MainWindow::clearAllProgress()
{
    m_balance = 0;
    updateBalance(0);
    m_mustDoList->clear();
    m_niceToDoList->clear();
    m_rewardList->clear();

    // Пересоздаем дефолтный список магазина
    m_rewardList->addItem("Посмотреть серию сериала [20 🪙]");
    m_rewardList->addItem("Съесть вкусняшку [40 🪙]");
    m_rewardList->addItem("15 минут скроллинга [100 🪙]");

    saveSettings(); // Синхронизируем пустые данные с диском
}

// ====================================================================
// НИЖНЯЯ ЧАСТЬ С ФУНКЦИЯМИ ОРГАНАЙЗЕРА (ДЛЯ КОПИРОВАНИЯ ИЗ ПРОШЛОГО ШАГА)
// ====================================================================
void MainWindow::applyStyle()
{
    QString qss = R"(
        /* Общие настройки главного окна */
        QMainWindow { background-color: #EBF4FA; }

        /* Стилизация главного приветственного экрана */
        QLabel#WelcomeTitle {
            font-size: 42px;
            font-weight: bold;
            color: #1F618D;
            margin-bottom: 30px;
            font-family: 'Segoe UI', sans-serif;
        }

        QPushButton#ContinueButton {
            background-color: #3498DB;
            color: white;
            border-radius: 8px;
            padding: 12px 24px;
            font-size: 16px;
            font-weight: bold;
            min-width: 220px;
            margin-bottom: 10px;
        }
        QPushButton#ContinueButton:hover { background-color: #2980B9; }
        QPushButton#ContinueButton:pressed { background-color: #21618C; }

        QPushButton#ResetButton {
            background-color: #BDC3C7;
            color: #2C3E50;
            border-radius: 8px;
            padding: 10px 24px;
            font-size: 14px;
            font-weight: bold;
            min-width: 220px;
        }
        QPushButton#ResetButton:hover { background-color: #95A5A6; color: white; }
        QPushButton#ResetButton:pressed { background-color: #7F8C8D; }

        /* Стилизация рабочей области (вкладок, списков и элементов) */
        QTabWidget::pane { border: 1px solid #B0C4DE; background: #FFFFFF; border-radius: 8px; }
        QTabBar::tab { background: #D4E6F1; border: 1px solid #A9CCE3; padding: 8px 16px; border-top-left-radius: 6px; border-top-right-radius: 6px; color: #2E4053; font-weight: bold; }
        QTabBar::tab:selected { background: #FFFFFF; border-bottom-color: transparent; color: #1F618D; }

        QGroupBox { font-weight: bold; color: #1A5276; border: 2px solid #AED6F1; border-radius: 6px; margin-top: 12px; padding-top: 8px; }
        QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 3px; }

        QListWidget { background-color: #F7F9FA; border: 1px solid #D4E6F1; border-radius: 4px; color: #2C3E50; }
        QListWidget::item { padding: 6px; border-bottom: 1px solid #E5E8E8; }
        QListWidget::item:hover { background-color: #E8F4F8; }

        QLineEdit, QComboBox { border: 1px solid #A9CCE3; border-radius: 4px; padding: 5px; background: #FFFFFF; }

        /* Стандартные кнопки внутри вкладок */
        QPushButton { background-color: #3498DB; color: white; border-radius: 4px; padding: 6px 12px; font-weight: bold; }
        QPushButton:hover { background-color: #2980B9; }
        QPushButton:pressed { background-color: #21618C; }

        QLabel#BalanceLabel { font-size: 16px; font-weight: bold; color: #1F618D; background-color: #D4E6F1; padding: 6px 12px; border-radius: 12px; }
        QLabel#TimerLabel { font-size: 64px; font-weight: bold; color: #2980B9; font-family: 'Courier New', monospace; background-color: #F2F4F4; border: 3px dashed #A9CCE3; border-radius: 20px; padding: 20px; margin: 20px; }
    )";
    this->setStyleSheet(qss);
}

void MainWindow::playClickSound()
{
    m_clickPlayer->setPosition(0);
    m_clickPlayer->play();
}

void MainWindow::addCustomReward()
{
    playClickSound();
    QString name = m_rewardNameInput->text().trimmed();
    QString priceStr = m_rewardPriceInput->text().trimmed();
    if (name.isEmpty() || priceStr.isEmpty()) return;

    m_rewardList->addItem(QString("%1 [%2 🪙]").arg(name, priceStr));
    m_rewardNameInput->clear();
    m_rewardPriceInput->clear();
}

void MainWindow::toggleHardMode()
{
    playClickSound();
    m_isHardModeOn = !m_isHardModeOn;

    // ИСПРАВЛЕНО: Добавлен указатель *
    QGraphicsOpacityEffect *niceEffect = qobject_cast<QGraphicsOpacityEffect*>(m_niceToDoBox->graphicsEffect());
    if (!niceEffect) {
        niceEffect = new QGraphicsOpacityEffect(m_niceToDoBox);
        m_niceToDoBox->setGraphicsEffect(niceEffect);
    }

    // ИСПРАВЛЕНО: Добавлен указатель *
    QGraphicsOpacityEffect *inputEffect = qobject_cast<QGraphicsOpacityEffect*>(m_taskAddWidget->graphicsEffect());
    if (!inputEffect) {
        inputEffect = new QGraphicsOpacityEffect(m_taskAddWidget);
        m_taskAddWidget->setGraphicsEffect(inputEffect);
    }

    QParallelAnimationGroup *hardModeGroup = new QParallelAnimationGroup(this);
    double startOpacity = m_isHardModeOn ? 1.0 : 0.0;
    double endOpacity = m_isHardModeOn ? 0.0 : 1.0;

    QPropertyAnimation *a1 = new QPropertyAnimation(niceEffect, "opacity"); a1->setDuration(300); a1->setStartValue(startOpacity); a1->setEndValue(endOpacity);
    QPropertyAnimation *a3 = new QPropertyAnimation(inputEffect, "opacity"); a3->setDuration(300); a3->setStartValue(startOpacity); a3->setEndValue(endOpacity);
    hardModeGroup->addAnimation(a1);
    hardModeGroup->addAnimation(a3);

    if (m_isHardModeOn) {
        m_hardModeBtn->setText("Hard Mode: ON");
        m_hardModeBtn->setStyleSheet("background-color: #E67E22; color: white; font-weight: bold;");

        // СИНТАКСИС ИСПРАВЛЕН: убрано дублирование "this, this"
        connect(hardModeGroup, &QParallelAnimationGroup::finished, this, [this]() {
            m_niceToDoBox->hide();
            m_taskAddWidget->hide();
        });
    } else {
        m_niceToDoBox->show();
        m_taskAddWidget->show();
        m_hardModeBtn->setText("Hard-Mode");
        m_hardModeBtn->setStyleSheet("background-color: #5D6D7E;");
    }
    hardModeGroup->start();
}

int MainWindow::parseRewardPrice(const QString &text)
{
    int startIndex = text.lastIndexOf('[');
    int endIndex = text.lastIndexOf(' ');
    if (startIndex != -1 && endIndex != -1 && endIndex > startIndex) {
        return text.mid(startIndex + 1, endIndex - startIndex - 1).toInt();
    }
    return 0;
}

void MainWindow::saveSettings()
{
    // Сохраняем прогресс только если мы уже зашли в само приложение (чтобы не затереть данные при старте)
    if (m_stackedWidget->currentWidget() != m_mainWorkspacePage) return;

    QSettings settings("BlueCompany", "BlueProductivity");
    settings.setValue("balance", m_balance);

    QStringList mustTasks;
    for (int i = 0; i < m_mustDoList->count(); ++i) mustTasks << m_mustDoList->item(i)->text();
    settings.setValue("mustDoTasks", mustTasks);

    QStringList niceTasks;
    for (int i = 0; i < m_niceToDoList->count(); ++i) niceTasks << m_niceToDoList->item(i)->text();
    settings.setValue("niceToDoTasks", niceTasks);

    QStringList rewards;
    for (int i = 0; i < m_rewardList->count(); ++i) rewards << m_rewardList->item(i)->text();
    settings.setValue("rewards", rewards);
}
void MainWindow::loadSettings()
{
    QSettings settings("BlueCompany", "BlueProductivity");
    m_balance = settings.value("balance", 0).toInt();
    updateBalance(0);

    m_mustDoList->clear();
    m_niceToDoList->clear();
    m_rewardList->clear();

    m_mustDoList->addItems(settings.value("mustDoTasks").toStringList());
    m_niceToDoList->addItems(settings.value("niceToDoTasks").toStringList());

    QStringList savedRewards = settings.value("rewards").toStringList();
    if (!savedRewards.isEmpty()) {
        m_rewardList->addItems(savedRewards);
    } else {
        m_rewardList->addItem("Посмотреть серию сериала [20 🪙]");
        m_rewardList->addItem("Съесть вкусняшку [40 🪙]");
        m_rewardList->addItem("15 минут скроллинга [100 🪙]");
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    saveSettings();
    event->accept();
}

void MainWindow::animateFlyingCoin(QPoint startPos)
{
    playClickSound();
    QLabel *coin = new QLabel("🪙", this);
    coin->setStyleSheet("font-size: 24px; background: transparent;");
    coin->setAttribute(Qt::WA_DeleteOnClose);
    coin->resize(30, 30);

    QPoint localStart = this->mapFromGlobal(startPos);
    coin->move(localStart);
    coin->show();

    QPoint targetPos = m_balanceLabel->mapTo(this, QPoint(20, 10));

    QPropertyAnimation *moveAnim = new QPropertyAnimation(coin, "pos");
    moveAnim->setDuration(600);
    moveAnim->setStartValue(localStart);
    moveAnim->setEndValue(targetPos);
    moveAnim->setEasingCurve(QEasingCurve::OutQuad);

    QGraphicsOpacityEffect *opacityEffect = new QGraphicsOpacityEffect(coin);
    coin->setGraphicsEffect(opacityEffect);

    QPropertyAnimation *fadeAnim = new QPropertyAnimation(opacityEffect, "opacity");
    fadeAnim->setDuration(600);
    fadeAnim->setStartValue(1.0);
    fadeAnim->setEndValue(0.0);

    QParallelAnimationGroup *group = new QParallelAnimationGroup(this);
    group->addAnimation(moveAnim);
    group->addAnimation(fadeAnim);

    // СИНТАКСИС ИСПРАВЛЕН: убрано дублирование "this, this"
    connect(group, &QParallelAnimationGroup::finished, this, [this]() {
        m_coinPlayer->setPosition(0);
        m_coinPlayer->play();
    });

    connect(group, &QParallelAnimationGroup::finished, coin, &QWidget::close);
    connect(group, &QParallelAnimationGroup::finished, group, &QParallelAnimationGroup::deleteLater);
    group->start();
}

void MainWindow::updateBalance(int amount)
{
    m_balance += amount;
    m_balanceLabel->setText(QString("Баланс: %1 🪙").arg(m_balance));
    if (amount == 0) return;

    WidgetScaler *scaler = new WidgetScaler(m_balanceLabel);
    QPropertyAnimation *animUp = new QPropertyAnimation(scaler, "scale");
    animUp->setDuration(100);
    animUp->setStartValue(1.0);
    animUp->setEndValue(1.3);
    animUp->setEasingCurve(QEasingCurve::OutQuad);

    QPropertyAnimation *animDown = new QPropertyAnimation(scaler, "scale");
    animDown->setDuration(180);
    animDown->setStartValue(1.3);
    animDown->setEndValue(1.0);
    animDown->setEasingCurve(QEasingCurve::InQuad);

    QSequentialAnimationGroup *pulseAnimation = new QSequentialAnimationGroup(this);
    pulseAnimation->addAnimation(animUp);
    pulseAnimation->addAnimation(animDown);

    connect(pulseAnimation, &QSequentialAnimationGroup::finished, pulseAnimation, &QSequentialAnimationGroup::deleteLater);
    connect(pulseAnimation, &QSequentialAnimationGroup::finished, scaler, &WidgetScaler::deleteLater);
    pulseAnimation->start();
}

void MainWindow::addTask()
{
    playClickSound();
    QString text = m_taskInput->text().trimmed();
    if (text.isEmpty()) return;

    if (m_taskCategoryCombo->currentIndex() == 0)
        m_mustDoList->addItem(text + " (+10 🪙)");
    else
        m_niceToDoList->addItem(text + " (+5 🪙)");
    m_taskInput->clear();
}

void MainWindow::completeTask(QListWidgetItem *item)
{
    animateFlyingCoin(QCursor::pos());
    int reward = item->text().contains("+10") ? 10 : 5;
    updateBalance(reward);
    delete item;
}

void MainWindow::buyReward()
{
    playClickSound();
    QListWidgetItem *current = m_rewardList->currentItem();
    if (!current) return;

    QString text = current->text();
    int price = parseRewardPrice(text);

    if (m_balance >= price) {
        updateBalance(-price);
        QMessageBox::information(this, "Магазин", QString("Вы приобрели: %1\nСписано: %2 🪙").arg(text.split(" [").first()).arg(price));
    } else {
        QMessageBox::warning(this, "Магазин", QString("Недостаточно монет! Стоимость: %1 🪙.").arg(price));
    }
}

void MainWindow::toggleTimer()
{
    playClickSound();
    if (m_pomodoroTimer->isActive()) {
        m_pomodoroTimer->stop();
        m_startTimerBtn->setText("Start");
    } else {
        m_pomodoroTimer->start(1000);
        m_startTimerBtn->setText("Pause");
    }
}

void MainWindow::updateTimer()
{
    if (m_timeLeftSeconds > 0) {
        m_timeLeftSeconds--;
        int mins = m_timeLeftSeconds / 60;
        int secs = m_timeLeftSeconds % 60;
        m_timerLabel->setText(QString("%1:%2")
                                  .arg(mins, 2, 10, QChar('0'))
                                  .arg(secs, 2, 10, QChar('0')));
    } else {
        m_pomodoroTimer->stop();
        m_startTimerBtn->setText("Start");
        if (m_isWorkSession) {
            animateFlyingCoin(this->mapToGlobal(QPoint(width()/2, height()/2)));
            updateBalance(15);
            QMessageBox::information(this, "Помодоро", "Отличная работа! +15 🪙.");
            m_isWorkSession = false;
            m_timeLeftSeconds = 5 * 60;
            m_timerLabel->setText("05:00");
            m_timerLabel->setStyleSheet("color: #27AE60;");
        } else {
            QMessageBox::information(this, "Помодоро", "Отдых окончен!");
            m_isWorkSession = true;
            m_timeLeftSeconds = 25 * 60;
            m_timerLabel->setText("25:00");
            m_timerLabel->setStyleSheet("color: #2980B9;");
        }
    }
}

void MainWindow::resetTimer()
{
    playClickSound();
    m_pomodoroTimer->stop();
    m_startTimerBtn->setText("Start");
    m_isWorkSession = true;
    m_timeLeftSeconds = 25 * 60;
    m_timerLabel->setText("25:00");
    m_timerLabel->setStyleSheet("color: #2980B9;");
}
