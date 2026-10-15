#include "SandTetrisBoard.h"
#include <QPainter>
#include <QKeyEvent>
#include <QRandomGenerator>
#include <QSettings>
#include <queue>
#include <algorithm>
#include <cmath>

SandTetrisBoard::SandTetrisBoard(QWidget *parent)
    : QWidget(parent) {
    setFocusPolicy(Qt::StrongFocus);
    setFixedSize(SandWidth * GrainSize, SandHeight * GrainSize); // 320 x 640 px

    QSettings settings("TetrisQt6", "SandTetris");
    m_highScore = settings.value("sandHighScore", 0).toInt();

    connect(&m_animTimer, &QTimer::timeout, this, &SandTetrisBoard::onAnimationFrame);
    m_animTimer.start(16);

    for (auto &row : m_sand) {
        row.fill(0);
    }
}

void SandTetrisBoard::refillBag(bool isFirstBag) {
    m_bag = {
        TetrominoType::I,
        TetrominoType::J,
        TetrominoType::L,
        TetrominoType::O,
        TetrominoType::S,
        TetrominoType::T,
        TetrominoType::Z
    };

    auto *rng = QRandomGenerator::global();
    for (size_t i = m_bag.size() - 1; i > 0; --i) {
        size_t j = rng->bounded(static_cast<quint32>(i + 1));
        std::swap(m_bag[i], m_bag[j]);
    }

    if (isFirstBag) {
        auto isGoodStart = [](TetrominoType t) {
            return t == TetrominoType::I || t == TetrominoType::J ||
                   t == TetrominoType::L || t == TetrominoType::T;
        };

        if (!isGoodStart(m_bag.front())) {
            for (size_t i = 1; i < m_bag.size(); ++i) {
                if (isGoodStart(m_bag[i])) {
                    std::swap(m_bag[0], m_bag[i]);
                    break;
                }
            }
        }
    }
}

TetrominoType SandTetrisBoard::takeFromBag() {
    if (m_bag.empty()) {
        refillBag(false);
    }
    TetrominoType type = m_bag.front();
    m_bag.erase(m_bag.begin());
    return type;
}

void SandTetrisBoard::addScore(int pointsToAdd) {
    if (pointsToAdd <= 0) return;

    m_score += pointsToAdd;
    emit scoreChanged(m_score);

    if (m_score > m_highScore) {
        m_highScore = m_score;
        emit highScoreChanged(m_highScore);
        QSettings settings("TetrisQt6", "SandTetris");
        settings.setValue("sandHighScore", m_highScore);
    }

    int currentPtsInLevel = m_score % PointsPerLevel;
    emit levelProgressChanged(currentPtsInLevel, PointsPerLevel);

    int newLevel = (m_score / PointsPerLevel) + 1;
    if (newLevel != m_level) {
        m_level = newLevel;
        emit levelChanged(m_level);
        triggerScreenShake(7.0f);
        addFloatingText("¡NIVEL " + QString::number(m_level) + "!", width() / 2.0f, height() / 3.0f, QColor(150, 220, 175), 26);
        if (m_isStarted && !m_isPaused && !m_isGameOver && !m_isCountingDown) {
            m_dropTimer.start(getDropInterval(), this);
        }
    }
}

void SandTetrisBoard::start() {
    m_isStarted = true;
    m_isPaused = false;
    m_isGameOver = false;
    m_score = 0;
    m_clearsCount = 0;
    m_level = 1;
    m_canHold = true;
    m_holdPiece = Piece(TetrominoType::None);
    m_particles.clear();
    m_floatingTexts.clear();

    for (auto &row : m_sand) {
        row.fill(0);
    }

    emit scoreChanged(m_score);
    emit highScoreChanged(m_highScore);
    emit linesChanged(m_clearsCount);
    emit levelChanged(m_level);
    emit levelProgressChanged(0, PointsPerLevel);
    emit holdPieceChanged(m_holdPiece);

    refillBag(true);
    m_nextPiece = Piece(takeFromBag());
    spawnPiece();

    m_isCountingDown = true;
    m_countdownMs = 0.0f;
    m_countdownStep = 3;
    addFloatingText("3", width() / 2.0f, height() / 2.0f - 30, QColor(100, 215, 230), 32);
}

void SandTetrisBoard::pause() {
    if (!m_isStarted || m_isGameOver || m_isCountingDown) return;

    m_isPaused = !m_isPaused;
    if (m_isPaused) {
        m_dropTimer.stop();
    } else {
        m_dropTimer.start(getDropInterval(), this);
    }
}

void SandTetrisBoard::restart() {
    start();
}

int SandTetrisBoard::getDropInterval() const {
    int interval = 500 - (m_level - 1) * 45;
    return std::max(interval, 75);
}

void SandTetrisBoard::holdCurrentPiece() {
    if (!m_canHold || !m_isStarted || m_isPaused || m_isGameOver || m_isCountingDown) {
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

        if (!tryMovePiece(3, 0, m_currentPiece.getRotation())) {
            m_isGameOver = true;
            m_dropTimer.stop();
            emit gameOverSignal();
        }
    }

    m_canHold = false;
    emit holdPieceChanged(m_holdPiece);
    triggerScreenShake(2.0f);
}

void SandTetrisBoard::spawnPiece() {
    m_currentPiece = m_nextPiece;
    m_nextPiece = Piece(takeFromBag());
    emit nextPieceChanged(m_nextPiece);

    m_currentPiece.setPosition(3, 0);

    if (!tryMovePiece(3, 0, m_currentPiece.getRotation())) {
        m_isGameOver = true;
        m_dropTimer.stop();
        triggerScreenShake(10.0f);
        addFloatingText("GAME OVER", width() / 2.0f, height() / 2.0f - 20, QColor(250, 140, 160), 26);
        emit gameOverSignal();
    }
}

bool SandTetrisBoard::tryMovePiece(int newBlockX, int newBlockY, int newRotation) {
    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 4; ++c) {
            if (m_currentPiece.isBlockAt(r, c, newRotation)) {
                int bx = newBlockX + c;
                int by = newBlockY + r;

                // Límites de bloques
                if (bx < 0 || bx >= 10 || by >= 20) {
                    return false;
                }

                int grainX = bx * BlockGrainScale;
                int grainY = by * BlockGrainScale;

                // Comprobar colisión con los granos de arena
                for (int gy = 0; gy < BlockGrainScale; ++gy) {
                    int py = grainY + gy;
                    if (py < 0) continue;
                    if (py >= SandHeight) return false;

                    for (int gx = 0; gx < BlockGrainScale; ++gx) {
                        int px = grainX + gx;
                        if (px < 0 || px >= SandWidth) return false;

                        if (m_sand[py][px] != 0) {
                            return false; // Colisionó con arena existente
                        }
                    }
                }
            }
        }
    }
    return true;
}

void SandTetrisBoard::meltPieceIntoSand() {
    uint8_t colorId = static_cast<uint8_t>(m_currentPiece.getType());

    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 4; ++c) {
            if (m_currentPiece.isBlockAt(r, c)) {
                int bx = m_currentPiece.getX() + c;
                int by = m_currentPiece.getY() + r;

                int grainX = bx * BlockGrainScale;
                int grainY = by * BlockGrainScale;

                for (int gy = 0; gy < BlockGrainScale; ++gy) {
                    int py = grainY + gy;
                    if (py < 0 || py >= SandHeight) continue;

                    for (int gx = 0; gx < BlockGrainScale; ++gx) {
                        int px = grainX + gx;
                        if (px < 0 || px >= SandWidth) continue;

                        m_sand[py][px] = colorId;
                    }
                }
            }
        }
    }

    // Pequeñas chispas del impacto de la arena
    QColor col = Piece::getColorForType(m_currentPiece.getType());
    float impactX = (m_currentPiece.getX() + 1.5f) * 32.0f;
    float impactY = (m_currentPiece.getY() + 3.0f) * 32.0f;
    spawnParticlesAt(impactX, impactY, col, 10, 2.5f);

    m_canHold = true;
    checkSandClears();
    spawnPiece();
}

void SandTetrisBoard::dropStep() {
    if (m_isGameOver || m_isPaused || m_isCountingDown) return;

    if (tryMovePiece(m_currentPiece.getX(), m_currentPiece.getY() + 1, m_currentPiece.getRotation())) {
        m_currentPiece.move(0, 1);
    } else {
        meltPieceIntoSand();
    }
}

void SandTetrisBoard::hardDrop() {
    if (m_isGameOver || m_isPaused || m_isCountingDown) return;

    int dropDistance = 0;
    while (tryMovePiece(m_currentPiece.getX(), m_currentPiece.getY() + 1, m_currentPiece.getRotation())) {
        m_currentPiece.move(0, 1);
        dropDistance++;
    }

    if (dropDistance > 0) {
        addScore(dropDistance * 2);
        triggerScreenShake(std::min(2.0f + dropDistance * 0.3f, 6.0f));
    }

    meltPieceIntoSand();
}

void SandTetrisBoard::updateSandPhysics() {
    // 2 subpasos de simulación de arena por frame para que fluya natural
    for (int step = 0; step < 2; ++step) {
        bool leftToRight = (QRandomGenerator::global()->generateDouble() > 0.5);
        int startCol = leftToRight ? 0 : (SandWidth - 1);
        int endCol = leftToRight ? SandWidth : -1;
        int stepCol = leftToRight ? 1 : -1;

        for (int y = SandHeight - 2; y >= 0; --y) {
            for (int x = startCol; x != endCol; x += stepCol) {
                uint8_t val = m_sand[y][x];
                if (val == 0) continue;

                // 1. Caída recta hacia abajo
                if (m_sand[y + 1][x] == 0) {
                    m_sand[y + 1][x] = val;
                    m_sand[y][x] = 0;
                } else {
                    // 2. Deslizamiento diagonal (ángulo de reposo de la arena)
                    bool canLeft = (x > 0 && m_sand[y + 1][x - 1] == 0);
                    bool canRight = (x < SandWidth - 1 && m_sand[y + 1][x + 1] == 0);

                    if (canLeft && canRight) {
                        int dir = (QRandomGenerator::global()->generateDouble() > 0.5) ? -1 : 1;
                        m_sand[y + 1][x + dir] = val;
                        m_sand[y][x] = 0;
                    } else if (canLeft) {
                        m_sand[y + 1][x - 1] = val;
                        m_sand[y][x] = 0;
                    } else if (canRight) {
                        m_sand[y + 1][x + 1] = val;
                        m_sand[y][x] = 0;
                    }
                }
            }
        }
    }
}

void SandTetrisBoard::checkSandClears() {
    // Regla de Sandtrix: si la arena del mismo color conecta el borde izquierdo con el derecho, se disuelve!
    for (uint8_t colId = 1; colId <= 7; ++colId) {
        std::vector<std::pair<int, int>> startNodes;
        for (int y = 0; y < SandHeight; ++y) {
            if (m_sand[y][0] == colId) {
                startNodes.push_back({0, y});
            }
        }
        if (startNodes.empty()) continue;

        std::vector<std::vector<bool>> visited(SandHeight, std::vector<bool>(SandWidth, false));
        std::vector<std::pair<int, int>> toClear;

        for (auto &start : startNodes) {
            if (visited[start.second][start.first]) continue;

            std::queue<std::pair<int, int>> q;
            std::vector<std::pair<int, int>> currentComp;
            bool reachedRight = false;

            q.push(start);
            visited[start.second][start.first] = true;

            while (!q.empty()) {
                auto [cx, cy] = q.front();
                q.pop();
                currentComp.push_back({cx, cy});

                if (cx == SandWidth - 1) {
                    reachedRight = true;
                }

                // 8 direcciones vecinas (los granos de arena se tocan diagonal y ortogonalmente)
                static const int dx[] = {-1, 0, 1, -1, 1, -1, 0, 1};
                static const int dy[] = {-1, -1, -1, 0, 0, 1, 1, 1};

                for (int i = 0; i < 8; ++i) {
                    int nx = cx + dx[i];
                    int ny = cy + dy[i];

                    if (nx >= 0 && nx < SandWidth && ny >= 0 && ny < SandHeight) {
                        if (!visited[ny][nx] && m_sand[ny][nx] == colId) {
                            visited[ny][nx] = true;
                            q.push({nx, ny});
                        }
                    }
                }
            }

            if (reachedRight) {
                toClear.insert(toClear.end(), currentComp.begin(), currentComp.end());
            }
        }

        if (!toClear.empty()) {
            int grainsCount = static_cast<int>(toClear.size());
            QColor grainColor = Piece::getColorForType(static_cast<TetrominoType>(colId));

            for (auto &[gx, gy] : toClear) {
                m_sand[gy][gx] = 0; // Se disuelve el grano
                if (QRandomGenerator::global()->bounded(12) == 0) {
                    spawnParticlesAt(gx * GrainSize + GrainSize / 2.0f,
                                     gy * GrainSize + GrainSize / 2.0f,
                                     grainColor, 1, 3.0f);
                }
            }

            m_clearsCount++;
            emit linesChanged(m_clearsCount);

            int points = (grainsCount * 2) * m_level;
            addScore(points);
            triggerScreenShake(std::min(5.0f + grainsCount * 0.02f, 14.0f));
            addFloatingText(QString("¡ARENA DISUELTA! +%1").arg(points),
                            width() / 2.0f, height() / 2.0f, grainColor, 18);
        }
    }
}

void SandTetrisBoard::triggerScreenShake(float intensity) {
    m_shakeIntensity = std::max(m_shakeIntensity, intensity);
}

void SandTetrisBoard::spawnParticlesAt(float x, float y, const QColor &color, int count, float speed) {
    for (int i = 0; i < count; ++i) {
        float angle = QRandomGenerator::global()->generateDouble() * 2.0 * M_PI;
        float spd = (0.4f + QRandomGenerator::global()->generateDouble()) * speed;
        float life = 300.0f + QRandomGenerator::global()->generateDouble() * 250.0f;
        float size = 2.0f + QRandomGenerator::global()->generateDouble() * 2.5f;

        m_particles.push_back({
            x, y,
            std::cos(angle) * spd,
            std::sin(angle) * spd,
            1.0f, life, size, color
        });
    }
}

void SandTetrisBoard::addFloatingText(const QString &text, float x, float y, const QColor &color, int size) {
    m_floatingTexts.push_back({text, x, y, -1.1f, 1.0f, color, size});
}

void SandTetrisBoard::onAnimationFrame() {
    if (m_shakeIntensity > 0.05f) {
        m_shakeIntensity *= 0.86f;
    } else {
        m_shakeIntensity = 0.0f;
    }

    if (m_isCountingDown) {
        m_countdownMs += 16.0f;
        if (m_countdownMs >= 550.0f) {
            m_countdownMs = 0.0f;
            m_countdownStep--;

            if (m_countdownStep == 2) {
                addFloatingText("2", width() / 2.0f, height() / 2.0f - 30, QColor(135, 175, 245), 32);
            } else if (m_countdownStep == 1) {
                addFloatingText("1", width() / 2.0f, height() / 2.0f - 30, QColor(255, 225, 130), 32);
            } else if (m_countdownStep == 0) {
                addFloatingText("¡ARENA!", width() / 2.0f, height() / 2.0f - 30, QColor(255, 225, 130), 32);
            } else if (m_countdownStep < 0) {
                m_isCountingDown = false;
                m_dropTimer.start(getDropInterval(), this);
            }
        }
    }

    // Actualizar física de arena continuamente
    if (m_isStarted && !m_isPaused && !m_isGameOver) {
        updateSandPhysics();
        // Verificar periódicamente si se formaron conexiones completas al caer la arena
        checkSandClears();
    }

    for (auto it = m_particles.begin(); it != m_particles.end();) {
        it->x += it->vx;
        it->y += it->vy;
        it->vy += 0.12f;
        it->vx *= 0.98f;
        it->life -= 16.0f / it->maxLife;

        if (it->life <= 0.0f) {
            it = m_particles.erase(it);
        } else {
            ++it;
        }
    }

    for (auto it = m_floatingTexts.begin(); it != m_floatingTexts.end();) {
        it->y += it->vy;
        it->life -= 16.0f / 900.0f;

        if (it->life <= 0.0f) {
            it = m_floatingTexts.erase(it);
        } else {
            ++it;
        }
    }

    update();
}

void SandTetrisBoard::timerEvent(QTimerEvent *event) {
    if (event->timerId() == m_dropTimer.timerId()) {
        dropStep();
    } else {
        QWidget::timerEvent(event);
    }
}

void SandTetrisBoard::keyPressEvent(QKeyEvent *event) {
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

    if (m_isPaused || m_isGameOver || m_isCountingDown) {
        return;
    }

    switch (event->key()) {
    case Qt::Key_Left:
    case Qt::Key_A:
        if (tryMovePiece(m_currentPiece.getX() - 1, m_currentPiece.getY(), m_currentPiece.getRotation())) {
            m_currentPiece.move(-1, 0);
        }
        break;

    case Qt::Key_Right:
    case Qt::Key_D:
        if (tryMovePiece(m_currentPiece.getX() + 1, m_currentPiece.getY(), m_currentPiece.getRotation())) {
            m_currentPiece.move(1, 0);
        }
        break;

    case Qt::Key_Down:
    case Qt::Key_S:
        if (tryMovePiece(m_currentPiece.getX(), m_currentPiece.getY() + 1, m_currentPiece.getRotation())) {
            m_currentPiece.move(0, 1);
            addScore(1);
        }
        break;

    case Qt::Key_Up:
    case Qt::Key_W: {
        int nextRot = (m_currentPiece.getRotation() + 1) % 4;
        if (tryMovePiece(m_currentPiece.getX(), m_currentPiece.getY(), nextRot)) {
            m_currentPiece.rotateClockwise();
        } else if (tryMovePiece(m_currentPiece.getX() - 1, m_currentPiece.getY(), nextRot)) {
            m_currentPiece.move(-1, 0);
            m_currentPiece.rotateClockwise();
        } else if (tryMovePiece(m_currentPiece.getX() + 1, m_currentPiece.getY(), nextRot)) {
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

void SandTetrisBoard::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, false);

    painter.fillRect(rect(), QColor(20, 22, 32));

    painter.save();
    if (m_shakeIntensity > 0.0f) {
        float shakeX = (QRandomGenerator::global()->generateDouble() * 2.0f - 1.0f) * m_shakeIntensity;
        float shakeY = (QRandomGenerator::global()->generateDouble() * 2.0f - 1.0f) * m_shakeIntensity;
        painter.translate(shakeX, shakeY);
    }

    // 1. Dibujar los granos de arena acumulados
    // Tabla de colores pastel
    static const QColor sandColors[8] = {
        QColor(0, 0, 0, 0),
        Piece::getColorForType(TetrominoType::I),
        Piece::getColorForType(TetrominoType::J),
        Piece::getColorForType(TetrominoType::L),
        Piece::getColorForType(TetrominoType::O),
        Piece::getColorForType(TetrominoType::S),
        Piece::getColorForType(TetrominoType::T),
        Piece::getColorForType(TetrominoType::Z)
    };

    for (int y = 0; y < SandHeight; ++y) {
        for (int x = 0; x < SandWidth; ++x) {
            uint8_t val = m_sand[y][x];
            if (val > 0 && val <= 7) {
                painter.fillRect(x * GrainSize, y * GrainSize, GrainSize, GrainSize, sandColors[val]);
            }
        }
    }

    // 2. Dibujar la pieza activa cayendo
    if (m_isStarted && !m_isGameOver) {
        // Encontrar posición fantasma de impacto
        int ghostY = m_currentPiece.getY();
        while (tryMovePiece(m_currentPiece.getX(), ghostY + 1, m_currentPiece.getRotation())) {
            ghostY++;
        }

        QColor pieceColor = Piece::getColorForType(m_currentPiece.getType());

        // Pieza fantasma (outline punteado)
        painter.setRenderHint(QPainter::Antialiasing, true);
        for (int r = 0; r < 4; ++r) {
            for (int c = 0; c < 4; ++c) {
                if (m_currentPiece.isBlockAt(r, c)) {
                    int bx = m_currentPiece.getX() + c;
                    int by = ghostY + r;
                    QRectF bRect(bx * 32 + 2, by * 32 + 2, 28, 28);

                    QColor gc = pieceColor;
                    gc.setAlphaF(0.5f);
                    painter.setPen(QPen(gc, 1.5, Qt::DashLine));
                    painter.setBrush(QColor(pieceColor.red(), pieceColor.green(), pieceColor.blue(), 25));
                    painter.drawRoundedRect(bRect, 4, 4);
                }
            }
        }

        // Pieza activa
        for (int r = 0; r < 4; ++r) {
            for (int c = 0; c < 4; ++c) {
                if (m_currentPiece.isBlockAt(r, c)) {
                    int bx = m_currentPiece.getX() + c;
                    int by = m_currentPiece.getY() + r;
                    QRectF bRect(bx * 32 + 2, by * 32 + 2, 28, 28);

                    painter.setPen(QPen(pieceColor.darker(118), 1.2));
                    painter.setBrush(pieceColor);
                    painter.drawRoundedRect(bRect, 4, 4);
                }
            }
        }
    }

    // 3. Partículas
    painter.setRenderHint(QPainter::Antialiasing, true);
    for (const auto &p : m_particles) {
        painter.setPen(Qt::NoPen);
        QColor pc = p.color;
        pc.setAlphaF(std::clamp(p.life, 0.0f, 1.0f));
        painter.setBrush(pc);
        painter.drawEllipse(QPointF(p.x, p.y), p.size * p.life, p.size * p.life);
    }

    // 4. Textos flotantes
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

    // 5. Overlay Pausa / Game Over / Inicio
    if (!m_isStarted || m_isPaused || m_isGameOver) {
        painter.setRenderHint(QPainter::Antialiasing, true);
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
            mainText = "TETRIS ARENA";
            subText = "Física de arena activa\nESPACIO para jugar";
            mainColor = QColor(255, 225, 130);
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
