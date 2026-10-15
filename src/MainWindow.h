#pragma once

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>

class TetrisBoard;
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

private:
    TetrisBoard *m_board;
    PiecePreviewWidget *m_holdWidget;
    PiecePreviewWidget *m_nextWidget;

    QLabel *m_scoreLabel;
    QLabel *m_highScoreLabel;
    QLabel *m_linesLabel;
    QLabel *m_levelLabel;
    QProgressBar *m_levelProgressBar;
    QLabel *m_levelProgressLabel;
    QPushButton *m_restartButton;
};
