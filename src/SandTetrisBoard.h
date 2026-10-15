#pragma once

#include "Piece.h"
#include <QWidget>
#include <QBasicTimer>
#include <QTimer>
#include <QColor>
#include <vector>
#include <array>

struct SandParticle {
    float x;
    float y;
    float vx;
    float vy;
    float life;
    float maxLife;
    float size;
    QColor color;
};

struct SandFloatingText {
    QString text;
    float x;
    float y;
    float vy;
    float life;
    QColor color;
    int fontSize;
};

class SandTetrisBoard : public QWidget {
    Q_OBJECT

public:
    static constexpr int SandWidth = 80;    // 80 columnas de granos de arena
    static constexpr int SandHeight = 160;  // 160 filas de granos de arena
    static constexpr int GrainSize = 4;     // Cada grano mide 4x4 píxeles (320x640 total)
    static constexpr int BlockGrainScale = 8; // Cada bloque de Tetris equivale a 8x8 granos
    static constexpr int PointsPerLevel = 1000;

    explicit SandTetrisBoard(QWidget *parent = nullptr);

    void start();
    void pause();
    void restart();
    void resetToStandby();
    void holdCurrentPiece();

    bool isPaused() const { return m_isPaused; }
    bool isGameOver() const { return m_isGameOver; }
    bool isStarted() const { return m_isStarted; }
    int getHighScore() const { return m_highScore; }

signals:
    void scoreChanged(int score);
    void highScoreChanged(int highScore);
    void linesChanged(int lines); // En arena: cadenas de arena disueltas
    void levelChanged(int level);
    void levelProgressChanged(int currentPoints, int targetPoints);
    void nextPieceChanged(const Piece &nextPiece);
    void holdPieceChanged(const Piece &holdPiece);
    void gameOverSignal();

protected:
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void timerEvent(QTimerEvent *event) override;

private slots:
    void onAnimationFrame();

private:
    void updateSandPhysics();
    void checkSandClears();

    bool tryMovePiece(int newBlockX, int newBlockY, int newRotation);
    void meltPieceIntoSand();
    void spawnPiece();
    void dropStep();
    void hardDrop();
    void addScore(int pointsToAdd);
    int getDropInterval() const;

    void refillBag(bool isFirstBag);
    TetrominoType takeFromBag();

    void triggerScreenShake(float intensity);
    void spawnParticlesAt(float x, float y, const QColor &color, int count, float speed);
    void addFloatingText(const QString &text, float x, float y, const QColor &color, int size = 16);

    QBasicTimer m_dropTimer;
    QTimer m_animTimer;

    bool m_isStarted{false};
    bool m_isPaused{false};
    bool m_isGameOver{false};

    int m_score{0};
    int m_highScore{0};
    int m_clearsCount{0};
    int m_level{1};

    Piece m_currentPiece;
    Piece m_nextPiece;
    Piece m_holdPiece;
    bool m_canHold{true};

    bool m_isCountingDown{false};
    float m_countdownMs{0.0f};
    int m_countdownStep{3};

    // Matriz de arena: 0 = vacío, 1..4 = color de arena
    std::array<std::array<uint8_t, SandWidth>, SandHeight> m_sand;

    std::vector<TetrominoType> m_bag;
    float m_shakeIntensity{0.0f};
    std::vector<SandParticle> m_particles;
    std::vector<SandFloatingText> m_floatingTexts;
};
