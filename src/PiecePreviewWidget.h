#pragma once

#include "Piece.h"
#include <QWidget>
#include <QPainter>

class PiecePreviewWidget : public QWidget {
    Q_OBJECT

public:
    explicit PiecePreviewWidget(QWidget *parent = nullptr) : QWidget(parent) {
        setFixedSize(130, 110);
    }

public slots:
    void setPiece(const Piece &piece) {
        m_piece = piece;
        update();
    }

    void clear() {
        m_piece = Piece(TetrominoType::None);
        update();
    }

protected:
    void paintEvent(QPaintEvent *) override {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);

        // Fondo plano suave
        painter.fillRect(rect(), QColor(26, 28, 38));

        // Borde plano limpio
        painter.setPen(QPen(QColor(48, 52, 70), 1.5));
        painter.drawRoundedRect(rect().adjusted(1, 1, -1, -1), 8, 8);

        if (m_piece.getType() == TetrominoType::None) return;

        const int blockSize = 20;
        int offsetX = (width() - 4 * blockSize) / 2;
        int offsetY = (height() - 4 * blockSize) / 2;

        QColor color = Piece::getColorForType(m_piece.getType());

        // Centrado específico para O e I
        if (m_piece.getType() == TetrominoType::O) {
            offsetX += blockSize / 2;
        } else if (m_piece.getType() == TetrominoType::I) {
            offsetY -= blockSize / 4;
        }

        for (int r = 0; r < 4; ++r) {
            for (int c = 0; c < 4; ++c) {
                if (m_piece.isBlockAt(r, c, 0)) {
                    int px = offsetX + c * blockSize;
                    int py = offsetY + r * blockSize;

                    QRectF blockRect(px + 1.5, py + 1.5, blockSize - 3, blockSize - 3);

                    // Color pastel plano sin gradientes
                    painter.setPen(QPen(color.darker(115), 1.2));
                    painter.setBrush(color);
                    painter.drawRoundedRect(blockRect, 4, 4);
                }
            }
        }
    }

private:
    Piece m_piece;
};
