#pragma once

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QStackedWidget>

class TetrisBoard;
class SandTetrisBoard;
class PiecePreviewWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void updateScore(int score);
    void updateHighScore(int highScore);
    void updateLines(int lines);
    void updateLevel(int level);
    void updateLevelProgress(int currentPoints, int targetPoints);
    void switchToClassicMode();
    void switchToSandMode();

private:
    QStackedWidget *m_boardStack;
    TetrisBoard *m_classicBoard;
    SandTetrisBoard *m_sandBoard;
    PiecePreviewWidget *m_holdWidget;
    PiecePreviewWidget *m_nextWidget;

    QPushButton *m_btnClassicMode;
    QPushButton *m_btnSandMode;

    QLabel *m_scoreLabel;
    QLabel *m_highScoreLabel;
    QLabel *m_linesTitle;
    QLabel *m_linesLabel;
    QLabel *m_levelLabel;
    QProgressBar *m_levelProgressBar;
    QLabel *m_levelProgressLabel;
    QPushButton *m_restartButton;

    bool m_isSandMode{false};
};
