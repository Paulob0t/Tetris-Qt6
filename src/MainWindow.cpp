#include "MainWindow.h"
#include "TetrisBoard.h"
#include "SandTetrisBoard.h"
#include "PiecePreviewWidget.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {
    setWindowTitle("Tetris Qt6 - Clásico & Modo Arena (Linux & Windows)");

    setStyleSheet(R"(
        QMainWindow {
            background-color: #13141C;
        }
        QLabel {
            color: #DCE0EC;
            font-family: 'Segoe UI', 'Ubuntu', 'DejaVu Sans', sans-serif;
        }
        QPushButton {
            background-color: #2E344A;
            color: #D0D5E6;
            border: 1px solid #404866;
            border-radius: 8px;
            padding: 8px 12px;
            font-weight: 600;
            font-size: 12px;
        }
        QPushButton:hover {
            background-color: #3C4460;
        }
        QPushButton:pressed {
            background-color: #24283A;
        }
        QPushButton#activeMode {
            background-color: #4C567A;
            color: #FFFFFF;
            border: 1px solid #7080B0;
        }
        QPushButton#restartBtn {
            background-color: #3B4261;
            color: #FFFFFF;
            border: 1px solid #4C567A;
            padding: 10px 16px;
            font-size: 13px;
        }
        QPushButton#restartBtn:hover {
            background-color: #495278;
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
    // PANEL IZQUIERDO: SELECTOR DE MODO, HOLD & CONTROLES
    // ==========================================
    auto *leftPanel = new QFrame(this);
    leftPanel->setProperty("class", "card");
    leftPanel->setFixedWidth(165);

    auto *leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(15, 15, 15, 15);
    leftLayout->setSpacing(8);

    auto *modeTitle = new QLabel("MODO DE JUEGO", leftPanel);
    modeTitle->setStyleSheet("font-size: 10px; font-weight: bold; color: #7A819E; letter-spacing: 1px;");
    modeTitle->setAlignment(Qt::AlignCenter);
    leftLayout->addWidget(modeTitle);

    auto *modeLayout = new QHBoxLayout();
    modeLayout->setSpacing(6);

    m_btnClassicMode = new QPushButton("Clásico", leftPanel);
    m_btnClassicMode->setObjectName("activeMode");
    connect(m_btnClassicMode, &QPushButton::clicked, this, &MainWindow::switchToClassicMode);
    modeLayout->addWidget(m_btnClassicMode);

    m_btnSandMode = new QPushButton("Arena", leftPanel);
    connect(m_btnSandMode, &QPushButton::clicked, this, &MainWindow::switchToSandMode);
    modeLayout->addWidget(m_btnSandMode);

    leftLayout->addLayout(modeLayout);
    leftLayout->addSpacing(10);

    auto *holdTitle = new QLabel("HOLD", leftPanel);
    holdTitle->setStyleSheet("font-size: 11px; font-weight: bold; color: #7A819E; letter-spacing: 1px;");
    holdTitle->setAlignment(Qt::AlignCenter);
    leftLayout->addWidget(holdTitle);

    m_holdWidget = new PiecePreviewWidget(leftPanel);
    leftLayout->addWidget(m_holdWidget, 0, Qt::AlignCenter);

    leftLayout->addSpacing(15);

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
    // CENTRO: STACK DE TABLEROS (CLÁSICO vs ARENA)
    // ==========================================
    m_boardStack = new QStackedWidget(this);
    m_classicBoard = new TetrisBoard(this);
    m_sandBoard = new SandTetrisBoard(this);

    m_boardStack->addWidget(m_classicBoard);
    m_boardStack->addWidget(m_sandBoard);
    mainLayout->addWidget(m_boardStack);

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

    m_highScoreLabel = new QLabel(QString::number(m_classicBoard->getHighScore()), rightPanel);
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
    m_linesTitle = new QLabel("LÍNEAS", rightPanel);
    m_linesTitle->setStyleSheet("font-size: 10px; font-weight: bold; color: #7A819E; letter-spacing: 1px;");
    rightLayout->addWidget(m_linesTitle);

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
    m_restartButton->setObjectName("restartBtn");
    connect(m_restartButton, &QPushButton::clicked, [this]() {
        if (m_isSandMode) {
            m_sandBoard->restart();
            m_sandBoard->setFocus();
        } else {
            m_classicBoard->restart();
            m_classicBoard->setFocus();
        }
    });
    rightLayout->addWidget(m_restartButton);

    mainLayout->addWidget(rightPanel);
    setCentralWidget(centralWidget);

    // Conectar señales del Tablero Clásico
    connect(m_classicBoard, &TetrisBoard::scoreChanged, this, [this](int s) { if (!m_isSandMode) updateScore(s); });
    connect(m_classicBoard, &TetrisBoard::highScoreChanged, this, [this](int hs) { if (!m_isSandMode) updateHighScore(hs); });
    connect(m_classicBoard, &TetrisBoard::linesChanged, this, [this](int l) { if (!m_isSandMode) updateLines(l); });
    connect(m_classicBoard, &TetrisBoard::levelChanged, this, [this](int lvl) { if (!m_isSandMode) updateLevel(lvl); });
    connect(m_classicBoard, &TetrisBoard::levelProgressChanged, this, [this](int c, int t) { if (!m_isSandMode) updateLevelProgress(c, t); });
    connect(m_classicBoard, &TetrisBoard::nextPieceChanged, this, [this](const Piece &p) { if (!m_isSandMode) m_nextWidget->setPiece(p); });
    connect(m_classicBoard, &TetrisBoard::holdPieceChanged, this, [this](const Piece &p) { if (!m_isSandMode) m_holdWidget->setPiece(p); });

    // Conectar señales del Tablero de Arena
    connect(m_sandBoard, &SandTetrisBoard::scoreChanged, this, [this](int s) { if (m_isSandMode) updateScore(s); });
    connect(m_sandBoard, &SandTetrisBoard::highScoreChanged, this, [this](int hs) { if (m_isSandMode) updateHighScore(hs); });
    connect(m_sandBoard, &SandTetrisBoard::linesChanged, this, [this](int l) { if (m_isSandMode) updateLines(l); });
    connect(m_sandBoard, &SandTetrisBoard::levelChanged, this, [this](int lvl) { if (m_isSandMode) updateLevel(lvl); });
    connect(m_sandBoard, &SandTetrisBoard::levelProgressChanged, this, [this](int c, int t) { if (m_isSandMode) updateLevelProgress(c, t); });
    connect(m_sandBoard, &SandTetrisBoard::nextPieceChanged, this, [this](const Piece &p) { if (m_isSandMode) m_nextWidget->setPiece(p); });
    connect(m_sandBoard, &SandTetrisBoard::holdPieceChanged, this, [this](const Piece &p) { if (m_isSandMode) m_holdWidget->setPiece(p); });

    setFixedSize(sizeHint());
}

void MainWindow::switchToClassicMode() {
    if (!m_isSandMode) return;
    m_isSandMode = false;

    m_btnClassicMode->setObjectName("activeMode");
    m_btnSandMode->setObjectName("");
    m_btnClassicMode->setStyle(m_btnClassicMode->style());
    m_btnSandMode->setStyle(m_btnSandMode->style());

    m_linesTitle->setText("LÍNEAS");
    m_highScoreLabel->setText(QString::number(m_classicBoard->getHighScore()));

    m_sandBoard->pause();
    m_boardStack->setCurrentIndex(0);
    m_classicBoard->setFocus();
    m_classicBoard->restart();
}

void MainWindow::switchToSandMode() {
    if (m_isSandMode) return;
    m_isSandMode = true;

    m_btnSandMode->setObjectName("activeMode");
    m_btnClassicMode->setObjectName("");
    m_btnSandMode->setStyle(m_btnSandMode->style());
    m_btnClassicMode->setStyle(m_btnClassicMode->style());

    m_linesTitle->setText("DISOLUCIONES");
    m_highScoreLabel->setText(QString::number(m_sandBoard->getHighScore()));

    m_classicBoard->pause();
    m_boardStack->setCurrentIndex(1);
    m_sandBoard->setFocus();
    m_sandBoard->restart();
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
