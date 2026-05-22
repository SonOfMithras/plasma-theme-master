#ifndef TEMPLATEENGINE_H
#define TEMPLATEENGINE_H

#include "UniversalThemeExporter.h"
#include <QMap>
#include <QString>

/**
 * TemplateEngine - renders .tpl files using {{VARIABLE}} substitution.
 *
 * Variable names match the palette fields exactly. Additional key/value
 * pairs can be passed via `extra` to inject app-specific tokens.
 */
class TemplateEngine {
public:
    /**
     * Render a template file to a string.
     * @param templatePath  Absolute path to the .tpl file.
     * @param palette       Color palette to source variables from.
     * @param extra         Optional extra variables (override or extend palette vars).
     * @return              Rendered string, or empty if templatePath is invalid.
     */
    static QString render(const QString &templatePath,
                          const UniversalPalette &palette,
                          const QMap<QString, QString> &extra = {});

    /**
     * Build the full variable map from a palette.
     * Useful externally (e.g. for helpers that need the same var set).
     */
    static QMap<QString, QString> buildVars(const UniversalPalette &palette);

private:
    static QString substitute(const QString &tmpl,
                              const QMap<QString, QString> &vars);
    static QString colorHex(const QColor &c);
    static QString colorRgb(const QColor &c);
    static QString colorRgbSpace(const QColor &c);
};

#endif // TEMPLATEENGINE_H
