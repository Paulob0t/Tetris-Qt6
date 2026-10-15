#pragma once

#include "Piece.h"
#include <QWidget>
#include <QBasicTimer>
#include <QTimer>
#include <QColor>
#include <vector>
#include <array>

struct Particle {
    float x;
    float y;
    float vx;
    float vy;
    float life;
    float maxLife;
    float size;
    QColor color;
};

struct FloatingText {
    QString text;
    float x;
    float y;
    float vy;
    float life;
    QColor color;
    int fontSize;
};

class TetrisBoard : public QWidget {
    Q_OBJECT

public:
    static constexpr int BoardWidth = 10;
    static constexpr int BoardHeight = 20;
    static constexpr int BlockSize = 32;

    explicit TetrisBoard(QWidget *parent = nullptr);

    void start();
    void pause();
    void restart();
    void holdCurrentPiece();

    bool isPaused() const { return m_isPaused; }
    bool isGameOver() const { return m_isGameOver; }
    int getHighScore() const { return m_highScore; }

signals:
    void scoreChanged(int score);
    void highScoreChanged(int highScore);
    void linesChanged(int lines);
    void levelChanged(int level);
    void nextPieceChanged(const Piece &nextPiece);
    void holdPieceChanged(const Piece &holdPiece);
    void gameOverSignal();

protected:
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void timerEvent(QTimerEvent *event) override;

private slots:
    void onAnimationFrame(); // Bucle a 60 FPS

private:
    bool tryMove(const Piece &newPiece, int newX, int newY, int newRotation);
    bool isGrounded() const;
    void lockPiece();
    void checkLines();
    void finalizeLineClear();
    void spawnPiece();
    void dropStep();
    void hardDrop();
    int getDropInterval() const;

    // Algoritmo 7-Bag Randomizer
    void refillBag(bool isFirstBag);
    TetrominoType takeFromBag();

    // Lock Delay (Tiempo de gracia al tocar suelo)
    void resetLockDelayIfGrounded();
    static constexpr float LockDelayMaxMs = 500.0f;
    static constexpr int LockMaxMoves = 15;
    bool m_isLocking{false};
    float m_lockTimerMs{0.0f};
    int m_lockMoveCount{0};

    // Cuenta regresiva de inicio
    bool m_isCountingDown{false};
    float m_countdownMs{0.0f};
    int m_countdownStep{3}; // 3, 2, 1, 0 (GO)

    // Efectos visuales
    void triggerScreenShake(float intensity);
    void spawnParticlesAt(float x, float y, const QColor &color, int count, float speed);
    void addFloatingText(const QString &text, float x, float y, const QColor &color, int size = 16);

    // Relojes
    QBasicTimer m_dropTimer;
    QTimer m_animTimer;

    // Estado de juego
    bool m_isStarted{false};
    bool m_isPaused{false};
    bool m_isGameOver{false};

    int m_score{0};
    int m_highScore{0};
    int m_linesCleared{0};
    int m_level{1};

    Piece m_currentPiece;
    Piece m_nextPiece;
    Piece m_holdPiece;
    bool m_canHold{true};

    // Bolsa de 7 piezas
    std::vector<TetrominoType> m_bag;

    // Tablero [fila][columna]
    std::array<std::array<TetrominoType, BoardWidth>, BoardHeight> m_board;

    // Animación de líneas
    bool m_isClearingAnimation{false};
    float m_clearAnimProgress{0.0f};
    std::vector<int> m_animatingRows;

    // Screen Shake
    float m_shakeIntensity{0.0f};

    // Partículas y textos
    std::vector<Particle> m_particles;
    std::vector<FloatingText> m_floatingTexts;
};
