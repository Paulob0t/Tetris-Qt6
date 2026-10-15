# Tetris-Qt6 🎮

Una implementación moderna, fluida y multiplataforma del clásico **Tetris** desarrollada en **C++20** y **Qt 6**, con una estética visual pastel, animaciones suaves a 60 FPS, sistema de partículas y mecánicas competitivas modernas.

---

## ✨ Características

* **Modos de Juego (Clásico & Arena):**
  * **Modo Clásico:** El Tetris tradicional competitivo con 7-Bag y Lock Delay.
  * **Modo Arena (Sand Tetris):** Las piezas se derriten en miles de granos de arena con física de autómatas celulares. Si una cadena continua del mismo color conecta el borde izquierdo con el derecho, ¡toda la arena conectada se disuelve!
* **Paleta Pastel Minimalista:** Diseño limpio y moderno con tonos pastel suaves que no cansan la vista.
* **Fluidez a 60 FPS:** Motor de renderizado desacoplado de la velocidad de caída para animaciones ultrasuaves.
* **Subida de Nivel por Puntuación:** Barra de progreso interactiva en tiempo real; cada 1,000 puntos subes de nivel.
* **Algoritmo 7-Bag Randomizer:** Generación oficial mediante bolsas de 7 piezas barajadas con regla de inicio limpio.
* **Lock Delay (500 ms):** Tiempo de gracia al tocar el suelo para poder deslizar y rotar la pieza antes de fijarse.
* **Mecánica HOLD (Guardar pieza):** Guarda la pieza actual o intercámbiala en cualquier momento con `C` o `Shift`.
* **Proyección Fantasma (*Ghost Piece*):** Guía visual punteada que indica la posición exacta de impacto.
* **Efectos de Impacto (*Juice*):**
  * **Screen Shake:** Sacudida física de pantalla en caídas instantáneas y eliminaciones de líneas.
  * **Sistema de Partículas:** Explosión de chispas pastel al limpiar filas y en impactos fuertes.
  * **Animación de limpieza:** Destello y disolución suave de las filas completadas.
  * **Textos Flotantes:** Marcadores emergentes animados (`+100`, `+300 DOBLE`, `¡TETRIS! +800`, `¡NIVEL X!`).
* **Puntuación y Récord Persistente:** Guarda automáticamente tu puntuación más alta en el sistema mediante `QSettings`.
* **Multiplataforma:** Compila de forma nativa en **Linux** y **Windows** a través de CMake.

---

## 🕹️ Controles

| Tecla | Acción |
| :--- | :--- |
| **← / →** o **A / D** | Mover pieza a la izquierda / derecha |
| **↑** o **W** | Rotar pieza (con wall-kicks) |
| **↓** o **S** | Caída suave (*Soft drop*) |
| **Espacio** | Caída instantánea (*Hard drop*) / Iniciar |
| **C** o **Shift** | Guardar / Intercambiar pieza (*HOLD*) |
| **P** | Pausar / Continuar |
| **R** | Reiniciar partida |

---

## 🛠️ Requisitos

* Compilador compatible con **C++20** (GCC 11+, Clang 13+, o MSVC 2019+).
* **CMake 3.20** o superior.
* **Qt 6** (módulos `QtCore`, `QtGui`, `QtWidgets`).

---

## 🚀 Compilación y Ejecución

### En Linux:

```bash
# 1. Configurar y generar archivos de construcción
cmake -B build

# 2. Compilar
cmake --build build

# 3. Ejecutar
./build/TetrisQt6
```

### En Windows (PowerShell / CMD):

```cmd
# 1. Configurar con CMake
cmake -B build

# 2. Compilar en modo Release
cmake --build build --config Release

# 3. Ejecutar
.\build\Release\TetrisQt6.exe
```

---

## 📁 Estructura del Proyecto

```text
├── CMakeLists.txt              # Configuración de CMake y enlace con Qt6
├── .gitignore                  # Exclusión de binarios y temporales
├── README.md                   # Documentación del proyecto
└── src/
    ├── main.cpp                # Punto de entrada de la aplicación Qt
    ├── Piece.h / .cpp          # Definición de Tetrominós, matrices 4x4 y rotaciones
    ├── TetrisBoard.h / .cpp    # Lógica de juego, colisiones, timers, partículas y renderizado
    ├── PiecePreviewWidget.h    # Widget para previsualizar piezas (Hold y Next)
    └── MainWindow.h / .cpp     # Ventana principal, layouts de 3 columnas y panel de control
```

---

## 📄 Licencia

Este proyecto está bajo la licencia MIT. Siéntete libre de usarlo, modificarlo y compartirlo.
