#pragma once

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>

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

private:
    TetrisBoard *m_board;
    PiecePreviewWidget *m_holdWidget;
    PiecePreviewWidget *m_nextWidget;

    QLabel *m_scoreLabel;
    QLabel *m_highScoreLabel;
    QLabel *m_linesLabel;
    QLabel *m_levelLabel;
    QPushButton *m_restartButton;
};
