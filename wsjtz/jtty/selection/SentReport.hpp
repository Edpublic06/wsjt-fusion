// JTTY manual-log metadata. Does not trigger logging. GPL-3.0-or-later.
#ifndef FUSION_JTTY_SENT_REPORT_HPP
#define FUSION_JTTY_SENT_REPORT_HPP
#include <QString>
#include <QStringList>
#include <QRegularExpression>
namespace wsjtz {
inline QString jttySentReport(QString const& text,QString const& hisCall,QString const& myCall) {
 if(hisCall.trimmed().isEmpty() || myCall.trimmed().isEmpty())return {};
 static QRegularExpression const report {"^(R?[+-][0-9]{2}|[1-5][1-9][1-9])$"};
 QString result;
 for(auto const& line:text.toUpper().split('\n')) {
  auto words=line.simplified().split(' ',Qt::SkipEmptyParts);
  if(words.size()<2 || words[0]!=hisCall.trimmed().toUpper())continue;
  int index=words[1]==myCall.trimmed().toUpper()?2:1;
  if(index>=words.size() || !report.match(words[index]).hasMatch())continue;
  result=words[index];if(result.startsWith('R'))result.remove(0,1);
 }
 return result;
}
}
#endif
