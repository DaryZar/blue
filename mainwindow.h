#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QTimer>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QParallelAnimationGroup>
#include <QGraphicsOpacityEffect>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QVariant>
#include <QGroupBox>
#include <QStackedWidget>

class WidgetScaler : public QObject {
    Q_OBJECT
    Q_PROPERTY(double scale READ scale WRITE setScale)
public:
    WidgetScaler(QWidget *target) : m_target(target) {}
    double scale() const { return m_scale; }
    void setScale(double s) {
        m_scale = s;
        if (m_target) {
            QFont font = m_target->font();
            font.setPointSizeF(16.0 * s);
            m_target->setFont(font);
        }
    }
private:
    QWidget *m_target;
    double m_scale = 1.0;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    static int parseRewardPrice(const QString &text);

protected:
    // ВАЖНО: Эта строчка исправляет ошибку со скриншота!
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onContinuePressed();
    void onResetPressed();

    void addTask();
    void addCustomReward();
    void completeTask(QListWidgetItem *item);
    void buyReward();

    void toggleTimer();
    void updateTimer();
    void resetTimer();

    void toggleHardMode();

private:
    void setupUI();
    void applyStyle();
    void updateBalance(int amount);
    void animateFlyingCoin(QPoint startPos);
    void playClickSound();

    void saveSettings();
    void loadSettings();
    void clearAllProgress();

    int m_balance = 0;
    QLabel *m_balanceLabel;
    QWidget *m_topPanelWidget;

    QStackedWidget *m_stackedWidget;
    QWidget *m_welcomePage;
    QWidget *m_mainWorkspacePage;

    QListWidget *m_mustDoList;
    QListWidget *m_niceToDoList;
    QLineEdit *m_taskInput;
    QComboBox *m_taskCategoryCombo;

    QListWidget *m_rewardList;
    QLineEdit *m_rewardNameInput;
    QLineEdit *m_rewardPriceInput;

    QTimer *m_pomodoroTimer;
    QLabel *m_timerLabel;
    QPushButton *m_startTimerBtn;
    int m_timeLeftSeconds;
    bool m_isWorkSession;

    QMediaPlayer *m_clickPlayer;
    QAudioOutput *m_clickAudioOutput;
    QMediaPlayer *m_coinPlayer;
    QAudioOutput *m_coinAudioOutput;

    QPushButton *m_hardModeBtn;
    bool m_isHardModeOn;
    QGroupBox *m_niceToDoBox;
    QWidget *m_taskAddWidget;
};

#endif // MAINWINDOW_H
