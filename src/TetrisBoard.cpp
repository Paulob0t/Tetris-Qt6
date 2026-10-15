#include "TetrisBoard.h"
#include <QPainter>
#include <QPainterPath>
#include <QKeyEvent>
#include <QRandomGenerator>
#include <QSettings>
#include <algorithm>
#include <cmath>

TetrisBoard::TetrisBoard(QWidget *parent)
    : QWidget(parent) {
    setFocusPolicy(Qt::StrongFocus);
    setFixedSize(BoardWidth * BlockSize, BoardHeight * BlockSize);

    // Cargar puntuación máxima guardada en el sistema
    QSettings settings("TetrisQt6", "TetrisGame");
    m_highScore = settings.value("highScore", 0).toInt();

    // Iniciar temporizador de animación a 60 FPS (~16ms)
    connect(&m_animTimer, &QTimer::timeout, this, &TetrisBoard::onAnimationFrame);
    m_animTimer.start(16);

    for (auto &row : m_board) {
        row.fill(TetrominoType::None);
    }
}

void TetrisBoard::start() {
    m_isStarted = true;
    m_isPaused = false;
    m_isGameOver = false;
    m_score = 0;
    m_linesCleared = 0;
    m_level = 1;
    m_canHold = true;
    m_holdPiece = Piece(TetrominoType::None);
    m_particles.clear();
    m_floatingTexts.clear();
    m_isClearingAnimation = false;
    m_animatingRows.clear();

    for (auto &row : m_board) {
        row.fill(TetrominoType::None);
    }

    emit scoreChanged(m_score);
    emit highScoreChanged(m_highScore);
    emit linesChanged(m_linesCleared);
    emit levelChanged(m_level);
    emit holdPieceChanged(m_holdPiece);

    m_nextPiece = Piece::getRandomPiece();
    spawnPiece();

    m_dropTimer.start(getDropInterval(), this);
    addFloatingText("¡LISTO! A JUGAR", width() / 2.0f, height() / 2.0f, QColor(0, 240, 240), 20);
}

void TetrisBoard::pause() {
    if (!m_isStarted || m_isGameOver) return;

    m_isPaused = !m_isPaused;
    if (m_isPaused) {
        m_dropTimer.stop();
    } else {
        m_dropTimer.start(getDropInterval(), this);
    }
}

void TetrisBoard::restart() {
    start();
}

int TetrisBoard::getDropInterval() const {
    int interval = 500 - (m_level - 1) * 35;
    return std::max(interval, 75);
}

void TetrisBoard::holdCurrentPiece() {
    if (!m_canHold || !m_isStarted || m_isPaused || m_isGameOver || m_isClearingAnimation) {
        return;
    }

    TetrominoType currentType = m_currentPiece.getType();
    if (m_holdPiece.getType() == TetrominoType::None) {
        m_holdPiece = Piece(currentType);
        spawnPiece();
    } else {
        TetrominoType temp = m_holdPiece.getType();
        m_holdPiece = Piece(currentType);
        m_currentPiece = Piece(temp);
        m_currentPiece.setPosition(3, 0);

        if (!tryMove(m_currentPiece, 3, 0, m_currentPiece.getRotation())) {
            m_isGameOver = true;
            m_dropTimer.stop();
            emit gameOverSignal();
        }
    }

    m_canHold = false;
    emit holdPieceChanged(m_holdPiece);
    triggerScreenShake(2.0f);
}

void TetrisBoard::spawnPiece() {
    m_currentPiece = m_nextPiece;
    m_nextPiece = Piece::getRandomPiece();
    emit nextPieceChanged(m_nextPiece);

    m_currentPiece.setPosition(3, 0);

    if (!tryMove(m_currentPiece, 3, 0, m_currentPiece.getRotation())) {
        m_isGameOver = true;
        m_dropTimer.stop();
        triggerScreenShake(12.0f);
        addFloatingText("GAME OVER", width() / 2.0f, height() / 2.0f - 20, QColor(255, 60, 60), 26);
        emit gameOverSignal();
    }
}

bool TetrisBoard::tryMove(const Piece &piece, int newX, int newY, int newRotation) {
    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 4; ++c) {
            if (piece.isBlockAt(r, c, newRotation)) {
                int x = newX + c;
                int y = newY + r;

                if (x < 0 || x >= BoardWidth || y >= BoardHeight) {
                    return false;
                }
                if (y >= 0 && m_board[y][x] != TetrominoType::None) {
                    return false;
                }
            }
        }
    }
    return true;
}

void TetrisBoard::dropStep() {
    if (m_isClearingAnimation || m_isGameOver || m_isPaused) return;

    if (tryMove(m_currentPiece, m_currentPiece.getX(), m_currentPiece.getY() + 1, m_currentPiece.getRotation())) {
        m_currentPiece.move(0, 1);
    } else {
        lockPiece();
    }
}

void TetrisBoard::hardDrop() {
    if (m_isClearingAnimation || m_isGameOver || m_isPaused) return;

    int dropDistance = 0;
    while (tryMove(m_currentPiece, m_currentPiece.getX(), m_currentPiece.getY() + 1, m_currentPiece.getRotation())) {
        m_currentPiece.move(0, 1);
        dropDistance++;
    }

    if (dropDistance > 0) {
        m_score += dropDistance * 2;
        emit scoreChanged(m_score);
        triggerScreenShake(std::min(2.0f + dropDistance * 0.4f, 8.0f));

        // Partículas en el fondo del impacto
        QColor col = Piece::getColorForType(m_currentPiece.getType());
        float impactY = (m_currentPiece.getY() + 3) * BlockSize;
        float impactX = (m_currentPiece.getX() + 1.5f) * BlockSize;
        spawnParticlesAt(impactX, impactY, col, 14, 3.5f);
    }

    lockPiece();
}

void TetrisBoard::lockPiece() {
    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 4; ++c) {
            if (m_currentPiece.isBlockAt(r, c)) {
                int x = m_currentPiece.getX() + c;
                int y = m_currentPiece.getY() + r;
                if (y >= 0 && y < BoardHeight && x >= 0 && x < BoardWidth) {
                    m_board[y][x] = m_currentPiece.getType();
                }
            }
        }
    }

    m_canHold = true;
    checkLines();
}

void TetrisBoard::checkLines() {
    m_animatingRows.clear();
    for (int r = 0; r < BoardHeight; ++r) {
        bool fullLine = true;
        for (int c = 0; c < BoardWidth; ++c) {
            if (m_board[r][c] == TetrominoType::None) {
                fullLine = false;
                break;
            }
        }
        if (fullLine) {
            m_animatingRows.push_back(r);
        }
    }

    if (!m_animatingRows.empty()) {
        m_isClearingAnimation = true;
        m_clearAnimProgress = 0.0f;
        m_dropTimer.stop();

        int count = static_cast<int>(m_animatingRows.size());
        float shake = (count == 4) ? 16.0f : (count * 3.5f);
        triggerScreenShake(shake);

        // Explosión de partículas en cada fila completada
        for (int row : m_animatingRows) {
            float py = row * BlockSize + BlockSize / 2.0f;
            for (int col = 0; col < BoardWidth; ++col) {
                float px = col * BlockSize + BlockSize / 2.0f;
                QColor blockColor = Piece::getColorForType(m_board[row][col]);
                spawnParticlesAt(px, py, blockColor, 6, 4.0f);
            }
        }
    } else {
        spawnPiece();
    }
}

void TetrisBoard::finalizeLineClear() {
    int clearedCount = static_cast<int>(m_animatingRows.size());

    for (int row : m_animatingRows) {
        for (int k = row; k > 0; --k) {
            m_board[k] = m_board[k - 1];
        }
        m_board[0].fill(TetrominoType::None);
    }

    m_linesCleared += clearedCount;
    emit linesChanged(m_linesCleared);

    int points = 0;
    QString popupText;
    QColor popupColor = QColor(255, 255, 100);

    switch (clearedCount) {
    case 1:
        points = 100 * m_level;
        popupText = "+100";
        break;
    case 2:
        points = 300 * m_level;
        popupText = "+300 DOBLE";
        popupColor = QColor(0, 240, 240);
        break;
    case 3:
        points = 500 * m_level;
        popupText = "+500 TRIPLE";
        popupColor = QColor(240, 160, 0);
        break;
    case 4:
        points = 800 * m_level;
        popupText = "¡TETRIS! +800";
        popupColor = QColor(160, 0, 240);
        break;
    }

    m_score += points;
    emit scoreChanged(m_score);

    if (m_score > m_highScore) {
        m_highScore = m_score;
        emit highScoreChanged(m_highScore);
        QSettings settings("TetrisQt6", "TetrisGame");
        settings.setValue("highScore", m_highScore);
    }

    addFloatingText(popupText, width() / 2.0f, (m_animatingRows.front()) * BlockSize, popupColor, clearedCount == 4 ? 22 : 16);

    int newLevel = (m_linesCleared / 10) + 1;
    if (newLevel != m_level) {
        m_level = newLevel;
        emit levelChanged(m_level);
        addFloatingText("¡NIVEL " + QString::number(m_level) + "!", width() / 2.0f, height() / 3.0f, QColor(0, 255, 120), 24);
    }

    m_isClearingAnimation = false;
    m_animatingRows.clear();
    m_dropTimer.start(getDropInterval(), this);

    spawnPiece();
}

void TetrisBoard::triggerScreenShake(float intensity) {
    m_shakeIntensity = std::max(m_shakeIntensity, intensity);
}

void TetrisBoard::spawnParticlesAt(float x, float y, const QColor &color, int count, float speed) {
    for (int i = 0; i < count; ++i) {
        float angle = QRandomGenerator::global()->generateDouble() * 2.0 * M_PI;
        float spd = (0.5f + QRandomGenerator::global()->generateDouble()) * speed;
        float life = 350.0f + QRandomGenerator::global()->generateDouble() * 300.0f;
        float size = 3.0f + QRandomGenerator::global()->generateDouble() * 3.5f;

        m_particles.push_back({
            x, y,
            std::cos(angle) * spd,
            std::sin(angle) * spd,
            1.0f, life, size, color
        });
    }
}

void TetrisBoard::addFloatingText(const QString &text, float x, float y, const QColor &color, int size) {
    m_floatingTexts.push_back({text, x, y, -1.2f, 1.0f, color, size});
}

void TetrisBoard::onAnimationFrame() {
    // 1. Amortiguación de sacudida
    if (m_shakeIntensity > 0.05f) {
        m_shakeIntensity *= 0.86f;
    } else {
        m_shakeIntensity = 0.0f;
    }

    // 2. Progreso de animación de líneas completadas
    if (m_isClearingAnimation) {
        m_clearAnimProgress += 0.07f;
        if (m_clearAnimProgress >= 1.0f) {
            finalizeLineClear();
        }
    }

    // 3. Física de partículas
    for (auto it = m_particles.begin(); it != m_particles.end();) {
        it->x += it->vx;
        it->y += it->vy;
        it->vy += 0.15f; // Gravedad suave
        it->vx *= 0.98f;
        it->life -= 16.0f / it->maxLife;

        if (it->life <= 0.0f) {
            it = m_particles.erase(it);
        } else {
            ++it;
        }
    }

    // 4. Textos flotantes
    for (auto it = m_floatingTexts.begin(); it != m_floatingTexts.end();) {
        it->y += it->vy;
        it->life -= 16.0f / 1000.0f; // Se desvanecen en 1 segundo

        if (it->life <= 0.0f) {
            it = m_floatingTexts.erase(it);
        } else {
            ++it;
        }
    }

    update();
}

void TetrisBoard::timerEvent(QTimerEvent *event) {
    if (event->timerId() == m_dropTimer.timerId()) {
        dropStep();
    } else {
        QWidget::timerEvent(event);
    }
}

void TetrisBoard::keyPressEvent(QKeyEvent *event) {
    if (!m_isStarted) {
        if (event->key() == Qt::Key_Space || event->key() == Qt::Key_Return) {
            start();
        }
        return;
    }

    if (event->key() == Qt::Key_P) {
        pause();
        return;
    }

    if (event->key() == Qt::Key_R) {
        restart();
        return;
    }

    if (event->key() == Qt::Key_C || event->key() == Qt::Key_Shift) {
        holdCurrentPiece();
        return;
    }

    if (m_isPaused || m_isGameOver || m_isClearingAnimation) {
        return;
    }

    switch (event->key()) {
    case Qt::Key_Left:
    case Qt::Key_A:
        if (tryMove(m_currentPiece, m_currentPiece.getX() - 1, m_currentPiece.getY(), m_currentPiece.getRotation())) {
            m_currentPiece.move(-1, 0);
        }
        break;

    case Qt::Key_Right:
    case Qt::Key_D:
        if (tryMove(m_currentPiece, m_currentPiece.getX() + 1, m_currentPiece.getY(), m_currentPiece.getRotation())) {
            m_currentPiece.move(1, 0);
        }
        break;

    case Qt::Key_Down:
    case Qt::Key_S:
        if (tryMove(m_currentPiece, m_currentPiece.getX(), m_currentPiece.getY() + 1, m_currentPiece.getRotation())) {
            m_currentPiece.move(0, 1);
            m_score += 1;
            emit scoreChanged(m_score);
        }
        break;

    case Qt::Key_Up:
    case Qt::Key_W: {
        int nextRot = (m_currentPiece.getRotation() + 1) % 4;
        if (tryMove(m_currentPiece, m_currentPiece.getX(), m_currentPiece.getY(), nextRot)) {
            m_currentPiece.rotateClockwise();
        } else if (tryMove(m_currentPiece, m_currentPiece.getX() - 1, m_currentPiece.getY(), nextRot)) {
            m_currentPiece.move(-1, 0);
            m_currentPiece.rotateClockwise();
        } else if (tryMove(m_currentPiece, m_currentPiece.getX() + 1, m_currentPiece.getY(), nextRot)) {
            m_currentPiece.move(1, 0);
            m_currentPiece.rotateClockwise();
        }
        break;
    }

    case Qt::Key_Space:
        hardDrop();
        break;

    default:
        QWidget::keyPressEvent(event);
    }
}

void TetrisBoard::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // Fondo plano suave y moderno
    painter.fillRect(rect(), QColor(22, 24, 34));

    // Aplicar Screen Shake suave
    painter.save();
    if (m_shakeIntensity > 0.0f) {
        float shakeX = (QRandomGenerator::global()->generateDouble() * 2.0f - 1.0f) * m_shakeIntensity;
        float shakeY = (QRandomGenerator::global()->generateDouble() * 2.0f - 1.0f) * m_shakeIntensity;
        painter.translate(shakeX, shakeY);
    }

    // Cuadrícula plana sutil
    painter.setPen(QPen(QColor(34, 38, 52), 1));
    for (int x = 0; x <= BoardWidth; ++x) {
        painter.drawLine(x * BlockSize, 0, x * BlockSize, BoardHeight * BlockSize);
    }
    for (int y = 0; y <= BoardHeight; ++y) {
        painter.drawLine(0, y * BlockSize, BoardWidth * BlockSize, y * BlockSize);
    }

    // Dibujado plano con colores pastel
    auto drawBlock = [&](int x, int y, QColor color, float alpha = 1.0f, bool isGhost = false) {
        float px = x * BlockSize;
        float py = y * BlockSize;
        QRectF bRect(px + 2, py + 2, BlockSize - 4, BlockSize - 4);

        if (isGhost) {
            QColor ghostColor = color;
            ghostColor.setAlphaF(0.7f);
            painter.setPen(QPen(ghostColor, 1.5, Qt::DashLine));
            painter.setBrush(QColor(color.red(), color.green(), color.blue(), 30));
            painter.drawRoundedRect(bRect, 5, 5);
        } else {
            QColor c = color;
            c.setAlphaF(std::clamp(alpha, 0.0f, 1.0f));

            // Color plano pastel con borde sutil
            painter.setPen(QPen(c.darker(118), 1.2));
            painter.setBrush(c);
            painter.drawRoundedRect(bRect, 5, 5);
        }
    };

    // 1. Dibujar bloques colocados en el tablero
    for (int r = 0; r < BoardHeight; ++r) {
        bool isClearingRow = std::find(m_animatingRows.begin(), m_animatingRows.end(), r) != m_animatingRows.end();

        for (int c = 0; c < BoardWidth; ++c) {
            if (m_board[r][c] != TetrominoType::None) {
                if (isClearingRow) {
                    float alpha = 1.0f - m_clearAnimProgress;
                    QColor flashColor = QColor(255, 255, 240);
                    drawBlock(c, r, flashColor, alpha);
                } else {
                    drawBlock(c, r, Piece::getColorForType(m_board[r][c]));
                }
            }
        }

        if (isClearingRow) {
            float lineAlpha = (1.0f - m_clearAnimProgress) * 160.0f;
            painter.fillRect(0, r * BlockSize, width(), BlockSize, QColor(255, 255, 240, static_cast<int>(lineAlpha)));
        }
    }

    // 2. Dibujar pieza activa y fantasma
    if (m_isStarted && !m_isGameOver && !m_isClearingAnimation) {
        int ghostY = m_currentPiece.getY();
        while (tryMove(m_currentPiece, m_currentPiece.getX(), ghostY + 1, m_currentPiece.getRotation())) {
            ghostY++;
        }

        QColor pieceColor = Piece::getColorForType(m_currentPiece.getType());

        // Pieza fantasma
        for (int r = 0; r < 4; ++r) {
            for (int c = 0; c < 4; ++c) {
                if (m_currentPiece.isBlockAt(r, c)) {
                    int x = m_currentPiece.getX() + c;
                    int y = ghostY + r;
                    if (y >= 0) drawBlock(x, y, pieceColor, 0.5f, true);
                }
            }
        }

        // Pieza activa
        for (int r = 0; r < 4; ++r) {
            for (int c = 0; c < 4; ++c) {
                if (m_currentPiece.isBlockAt(r, c)) {
                    int x = m_currentPiece.getX() + c;
                    int y = m_currentPiece.getY() + r;
                    if (y >= 0) drawBlock(x, y, pieceColor, 1.0f, false);
                }
            }
        }
    }

    // 3. Dibujar partículas (puntos pastel suaves)
    for (const auto &p : m_particles) {
        painter.setPen(Qt::NoPen);
        QColor pc = p.color;
        pc.setAlphaF(std::clamp(p.life, 0.0f, 1.0f));
        painter.setBrush(pc);
        painter.drawEllipse(QPointF(p.x, p.y), p.size * p.life, p.size * p.life);
    }

    // 4. Dibujar textos flotantes
    for (const auto &ft : m_floatingTexts) {
        QFont font = painter.font();
        font.setPointSize(ft.fontSize);
        font.setBold(true);
        painter.setFont(font);

        QColor tc = ft.color;
        tc.setAlphaF(std::clamp(ft.life, 0.0f, 1.0f));

        painter.setPen(QColor(0, 0, 0, static_cast<int>(140 * ft.life)));
        painter.drawText(QRectF(ft.x - 150 + 1, ft.y - 20 + 1, 300, 40), Qt::AlignCenter, ft.text);

        painter.setPen(tc);
        painter.drawText(QRectF(ft.x - 150, ft.y - 20, 300, 40), Qt::AlignCenter, ft.text);
    }

    painter.restore();

    // 5. Pantallas de superposición (Inicio / Pausa / Game Over)
    if (!m_isStarted || m_isPaused || m_isGameOver) {
        painter.fillRect(rect(), QColor(14, 16, 24, 210));

        QRect card(width() / 2 - 120, height() / 2 - 80, 240, 160);
        painter.setPen(QPen(QColor(50, 56, 75), 1.5));
        painter.setBrush(QColor(28, 32, 46, 240));
        painter.drawRoundedRect(card, 12, 12);

        QFont titleFont = painter.font();
        titleFont.setPointSize(19);
        titleFont.setBold(true);
        painter.setFont(titleFont);

        QString mainText;
        QString subText;
        QColor mainColor;

        if (!m_isStarted) {
            mainText = "TETRIS";
            subText = "ESPACIO para jugar\nC / Shift para Hold";
            mainColor = QColor(100, 215, 230);
        } else if (m_isGameOver) {
            mainText = "GAME OVER";
            subText = "Presiona 'R' para reiniciar";
            mainColor = QColor(250, 140, 160);
        } else if (m_isPaused) {
            mainText = "PAUSA";
            subText = "Presiona 'P' para continuar";
            mainColor = QColor(255, 225, 130);
        }

        painter.setPen(mainColor);
        painter.drawText(card.adjusted(0, 20, 0, -80), Qt::AlignCenter, mainText);

        QFont subFont = painter.font();
        subFont.setPointSize(10);
        subFont.setBold(false);
        painter.setFont(subFont);
        painter.setPen(QColor(180, 185, 205));
        painter.drawText(card.adjusted(10, 75, -10, -10), Qt::AlignCenter, subText);
    }
}
