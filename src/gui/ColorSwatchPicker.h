#ifndef COLORSWATCHPICKER_H
#define COLORSWATCHPICKER_H

#include <QColor>
#include <QList>
#include <QWidget>

/**
 * A widget that displays a row of colored circular swatches.
 * Each swatch corresponds to one of the dominant colors extracted
 * from the current wallpaper by kde-material-you-colors (the "best"
 * array in its temp JSON file). Clicking a swatch selects it and
 * emits colorSelected(index).
 */
class ColorSwatchPicker : public QWidget {
  Q_OBJECT
public:
  explicit ColorSwatchPicker(QWidget *parent = nullptr);

  /** Replace the displayed swatches with new colors and reset to index 0. */
  void setColors(const QList<QColor> &colors);

  /** Set the currently highlighted swatch without emitting a signal. */
  void setSelectedIndex(int index);

  int selectedIndex() const { return m_selectedIndex; }

  /** Read /tmp/kde-material-you-colors-<user>.json and refresh the swatches. */
  void refreshFromDaemon();

  QSize sizeHint() const override;

Q_SIGNALS:
  void colorSelected(int index);

protected:
  void paintEvent(QPaintEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;

private:
  QList<QColor> m_colors;
  int m_selectedIndex = 0;

  static constexpr int SWATCH_SIZE = 36;
  static constexpr int SWATCH_GAP = 8;
  static constexpr int RING_WIDTH = 3;

  /** Returns the center rect for swatch at the given index. */
  QRect swatchRect(int index) const;

  /** Returns the path to the daemon JSON for the current user. */
  static QString daemonJsonPath();
};

#endif // COLORSWATCHPICKER_H
