#include "ColorSwatchPicker.h"

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QProcessEnvironment>

ColorSwatchPicker::ColorSwatchPicker(QWidget *parent) : QWidget(parent) {
  setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  setMinimumHeight(SWATCH_SIZE + RING_WIDTH * 2 + 4);
  setCursor(Qt::PointingHandCursor);
}

// ---------------------------------------------------------------------------
// Static helpers
// ---------------------------------------------------------------------------

QString ColorSwatchPicker::daemonJsonPath() {
  // Build: /tmp/kde-material-you-colors-<username>.json
  // Use $USER env var, fall back to the homedir basename.
  QString user = QProcessEnvironment::systemEnvironment().value(
      QStringLiteral("USER"),
      QDir::home().dirName() // e.g. "ammar" from /home/ammar
  );
  return QString("/tmp/kde-material-you-colors-%1.json").arg(user);
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

void ColorSwatchPicker::setColors(const QList<QColor> &colors) {
  m_colors = colors;
  if (m_selectedIndex >= m_colors.size())
    m_selectedIndex = 0;
  updateGeometry();
  update();
}

void ColorSwatchPicker::setSelectedIndex(int index) {
  if (index < 0 || index >= m_colors.size())
    index = 0;
  m_selectedIndex = index;
  update();
}

void ColorSwatchPicker::refreshFromDaemon() {
  QString path = daemonJsonPath();
  QFile f(path);
  if (!f.open(QIODevice::ReadOnly)) {
    // Daemon not running — keep a single placeholder gray swatch
    if (m_colors.isEmpty()) {
      m_colors = {QColor(0x80, 0x80, 0x80)};
      m_selectedIndex = 0;
      updateGeometry();
      update();
    }
    return;
  }

  QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
  f.close();

  if (!doc.isObject())
    return;

  QJsonArray best = doc.object().value(QStringLiteral("best")).toArray();
  if (best.isEmpty())
    return;

  QList<QColor> colors;
  for (const QJsonValue &v : best) {
    QColor c(v.toString());
    if (c.isValid())
      colors.append(c);
  }

  if (!colors.isEmpty()) {
    // Keep the selected index if still in range
    int prev = m_selectedIndex;
    m_colors = colors;
    m_selectedIndex = (prev < colors.size()) ? prev : 0;
    updateGeometry();
    update();
  }
}

// ---------------------------------------------------------------------------
// Geometry
// ---------------------------------------------------------------------------

QSize ColorSwatchPicker::sizeHint() const {
  int count = qMax(1, m_colors.size());
  int w = count * SWATCH_SIZE + (count - 1) * SWATCH_GAP + RING_WIDTH * 2;
  int h = SWATCH_SIZE + RING_WIDTH * 2 + 4;
  return QSize(w, h);
}

QRect ColorSwatchPicker::swatchRect(int index) const {
  int x = RING_WIDTH + index * (SWATCH_SIZE + SWATCH_GAP);
  int y = RING_WIDTH + 2;
  return QRect(x, y, SWATCH_SIZE, SWATCH_SIZE);
}

// ---------------------------------------------------------------------------
// Paint
// ---------------------------------------------------------------------------

void ColorSwatchPicker::paintEvent(QPaintEvent *) {
  QPainter p(this);
  p.setRenderHint(QPainter::Antialiasing, true);

  // Palette colors for ring
  QColor ringColor = palette().color(QPalette::Highlight);
  QColor ringBg    = palette().color(QPalette::Window);

  for (int i = 0; i < m_colors.size(); ++i) {
    QRect r = swatchRect(i);

    if (i == m_selectedIndex) {
      // Outer ring: window background gap then highlight ring
      p.setPen(Qt::NoPen);
      p.setBrush(ringBg);
      p.drawEllipse(r.adjusted(-RING_WIDTH, -RING_WIDTH, RING_WIDTH, RING_WIDTH));

      p.setBrush(ringColor);
      p.drawEllipse(r.adjusted(-(RING_WIDTH - 1), -(RING_WIDTH - 1),
                               RING_WIDTH - 1, RING_WIDTH - 1));
    }

    // Fill the circle with the swatch color
    p.setPen(Qt::NoPen);
    p.setBrush(m_colors[i]);
    p.drawEllipse(r);
  }
}

// ---------------------------------------------------------------------------
// Mouse
// ---------------------------------------------------------------------------

void ColorSwatchPicker::mousePressEvent(QMouseEvent *event) {
  if (event->button() != Qt::LeftButton)
    return;

  for (int i = 0; i < m_colors.size(); ++i) {
    QRect r = swatchRect(i).adjusted(-4, -4, 4, 4); // slightly larger hit area
    if (r.contains(event->pos())) {
      if (m_selectedIndex != i) {
        m_selectedIndex = i;
        update();
        Q_EMIT colorSelected(i);
      }
      return;
    }
  }
}
