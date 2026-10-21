// Fusion text-only JTTY coloring. GPL-3.0-or-later.
#ifndef FUSION_JTTY_TEXT_COLORS_HPP
#define FUSION_JTTY_TEXT_COLORS_HPP
#include <QTextEdit>
#include <QTextBlock>
namespace wsjtz {
inline void resetJttyText(QTextEdit& view,QString const& text,QFont const& font) {
 view.document()->setDefaultFont(font);
 view.setCurrentCharFormat(QTextCharFormat{});view.setPlainText(text);
 QTextCursor cursor(view.document());cursor.select(QTextCursor::Document);
 QTextCharFormat format;format.setFont(font);cursor.setCharFormat(format);
 cursor.setBlockFormat(QTextBlockFormat{});
}
inline void colorJttyLine(QTextDocument* document,QTextBlock const& block,QColor bg,QColor fg) {
 QTextCursor row(document);row.setPosition(block.position());
 row.setPosition(block.position()+block.text().size(),QTextCursor::KeepAnchor);
 QTextCharFormat format;
 if(bg.isValid())format.setBackground(bg);
 if(fg.isValid())format.setForeground(fg);
 row.mergeCharFormat(format);
}
}
#endif
