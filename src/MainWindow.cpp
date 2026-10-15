#include "MainWindow.h"
#include "TetrisBoard.h"
#include "PiecePreviewWidget.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {
    setWindowTitle("Tetris Qt6 - Pastel Edition (Linux & Windows)");

    setStyleSheet(R"(
        QMainWindow {
            background-color: #13141C;
        }
        QLabel {
            color: #DCE0EC;
            font-family: 'Segoe UI', 'Ubuntu', 'DejaVu Sans', sans-serif;
        }
        QPushButton {
            background-color: #3B4261;
            color: #FFFFFF;
            border: 1px solid #4C567A;
            border-radius: 8px;
            padding: 10px 16px;
            font-weight: 600;
            font-size: 13px;
        }
        QPushButton:hover {
            background-color: #495278;
        }
        QPushButton:pressed {
            background-color: #2F354F;
        }
        QFrame.card {
            background-color: #1A1C28;
            border-radius: 10px;
            border: 1px solid #282C40;
        }
        QProgressBar {
            background-color: #202436;
            border: 1px solid #2F3650;
            border-radius: 4px;
            height: 7px;
        }
        QProgressBar::chunk {
            background-color: #96D7AA;
            border-radius: 3px;
        }
    )");

    auto *centralWidget = new QWidget(this);
    auto *mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setContentsMargins(25, 25, 25, 25);
    mainLayout->setSpacing(20);

    // ==========================================
    // PANEL IZQUIERDO: HOLD & CONTROLES
    // ==========================================
    auto *leftPanel = new QFrame(this);
    leftPanel->setProperty("class", "card");
    leftPanel->setFixedWidth(160);

    auto *leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(15, 15, 15, 15);
    leftLayout->setSpacing(10);

    auto *holdTitle = new QLabel("HOLD", leftPanel);
    holdTitle->setStyleSheet("font-size: 11px; font-weight: bold; color: #7A819E; letter-spacing: 1px;");
    holdTitle->setAlignment(Qt::AlignCenter);
    leftLayout->addWidget(holdTitle);

    m_holdWidget = new PiecePreviewWidget(leftPanel);
    leftLayout->addWidget(m_holdWidget, 0, Qt::AlignCenter);

    leftLayout->addSpacing(20);

    auto *controlsTitle = new QLabel("CONTROLES", leftPanel);
    controlsTitle->setStyleSheet("font-size: 11px; font-weight: bold; color: #7A819E; letter-spacing: 1px;");
    leftLayout->addWidget(controlsTitle);

    auto *controlsDesc = new QLabel(
        "<b>← / →</b> : Mover<br>"
        "<b>↑ / W</b> : Rotar<br>"
        "<b>↓ / S</b> : Bajar suave<br>"
        "<b>ESPACIO</b> : Caída rápida<br>"
        "<b>C / SHIFT</b> : Guardar<br>"
        "<b>P</b> : Pausa<br>"
        "<b>R</b> : Reiniciar",
        leftPanel);
    controlsDesc->setStyleSheet("font-size: 11px; color: #9AA2C0; line-height: 1.5;");
    leftLayout->addWidget(controlsDesc);

    leftLayout->addStretch();
    mainLayout->addWidget(leftPanel);

    // ==========================================
    // CENTRO: TABLERO DE TETRIS
    // ==========================================
    m_board = new TetrisBoard(this);
    mainLayout->addWidget(m_board);

    // ==========================================
    // PANEL DERECHO: NEXT, PUNTUACIÓN, NIVELES
    // ==========================================
    auto *rightPanel = new QFrame(this);
    rightPanel->setProperty("class", "card");
    rightPanel->setFixedWidth(175);

    auto *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(15, 15, 15, 15);
    rightLayout->setSpacing(8);

    auto *nextTitle = new QLabel("SIGUIENTE", rightPanel);
    nextTitle->setStyleSheet("font-size: 11px; font-weight: bold; color: #7A819E; letter-spacing: 1px;");
    nextTitle->setAlignment(Qt::AlignCenter);
    rightLayout->addWidget(nextTitle);

    m_nextWidget = new PiecePreviewWidget(rightPanel);
    rightLayout->addWidget(m_nextWidget, 0, Qt::AlignCenter);

    rightLayout->addSpacing(8);

    // Récord
    auto *highScoreTitle = new QLabel("RÉCORD", rightPanel);
    highScoreTitle->setStyleSheet("font-size: 10px; font-weight: bold; color: #7A819E; letter-spacing: 1px;");
    rightLayout->addWidget(highScoreTitle);

    m_highScoreLabel = new QLabel(QString::number(m_board->getHighScore()), rightPanel);
    m_highScoreLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #FA8CA0;");
    rightLayout->addWidget(m_highScoreLabel);

    // Puntuación
    auto *scoreTitle = new QLabel("PUNTUACIÓN", rightPanel);
    scoreTitle->setStyleSheet("font-size: 10px; font-weight: bold; color: #7A819E; letter-spacing: 1px;");
    rightLayout->addWidget(scoreTitle);

    m_scoreLabel = new QLabel("0", rightPanel);
    m_scoreLabel->setStyleSheet("font-size: 22px; font-weight: bold; color: #64D7E6;");
    rightLayout->addWidget(m_scoreLabel);

    // Líneas
    auto *linesTitle = new QLabel("LÍNEAS", rightPanel);
    linesTitle->setStyleSheet("font-size: 10px; font-weight: bold; color: #7A819E; letter-spacing: 1px;");
    rightLayout->addWidget(linesTitle);

    m_linesLabel = new QLabel("0", rightPanel);
    m_linesLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #FFB482;");
    rightLayout->addWidget(m_linesLabel);

    // Nivel con Barra de Progreso
    auto *levelTitle = new QLabel("NIVEL", rightPanel);
    levelTitle->setStyleSheet("font-size: 10px; font-weight: bold; color: #7A819E; letter-spacing: 1px;");
    rightLayout->addWidget(levelTitle);

    m_levelLabel = new QLabel("1", rightPanel);
    m_levelLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #96D7AA;");
    rightLayout->addWidget(m_levelLabel);

    m_levelProgressBar = new QProgressBar(rightPanel);
    m_levelProgressBar->setRange(0, TetrisBoard::PointsPerLevel);
    m_levelProgressBar->setValue(0);
    m_levelProgressBar->setTextVisible(false);
    rightLayout->addWidget(m_levelProgressBar);

    m_levelProgressLabel = new QLabel("0 / 1000 pts", rightPanel);
    m_levelProgressLabel->setStyleSheet("font-size: 10px; color: #7A819E; font-weight: 500;");
    rightLayout->addWidget(m_levelProgressLabel);

    rightLayout->addStretch();

    m_restartButton = new QPushButton("Reiniciar (R)", rightPanel);
    connect(m_restartButton, &QPushButton::clicked, [this]() {
        m_board->restart();
        m_board->setFocus();
    });
    rightLayout->addWidget(m_restartButton);

    mainLayout->addWidget(rightPanel);
    setCentralWidget(centralWidget);

    // Conectar señales del tablero
    connect(m_board, &TetrisBoard::scoreChanged, this, &MainWindow::updateScore);
    connect(m_board, &TetrisBoard::highScoreChanged, this, &MainWindow::updateHighScore);
    connect(m_board, &TetrisBoard::linesChanged, this, &MainWindow::updateLines);
    connect(m_board, &TetrisBoard::levelChanged, this, &MainWindow::updateLevel);
    connect(m_board, &TetrisBoard::levelProgressChanged, this, &MainWindow::updateLevelProgress);
    connect(m_board, &TetrisBoard::nextPieceChanged, m_nextWidget, &PiecePreviewWidget::setPiece);
    connect(m_board, &TetrisBoard::holdPieceChanged, m_holdWidget, &PiecePreviewWidget::setPiece);

    setFixedSize(sizeHint());
}

void MainWindow::updateScore(int score) {
    m_scoreLabel->setText(QString::number(score));
}

void MainWindow::updateHighScore(int highScore) {
    m_highScoreLabel->setText(QString::number(highScore));
}

void MainWindow::updateLines(int lines) {
    m_linesLabel->setText(QString::number(lines));
}

void MainWindow::updateLevel(int level) {
    m_levelLabel->setText(QString::number(level));
}

void MainWindow::updateLevelProgress(int currentPoints, int targetPoints) {
    m_levelProgressBar->setMaximum(targetPoints);
    m_levelProgressBar->setValue(currentPoints);
    m_levelProgressLabel->setText(QString("%1 / %2 pts").arg(currentPoints).arg(targetPoints));
}
