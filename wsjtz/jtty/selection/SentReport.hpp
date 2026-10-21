// JTTY manual-log metadata. Does not trigger logging. GPL-3.0-or-later.
#ifndef FUSION_JTTY_SENT_REPORT_HPP
#define FUSION_JTTY_SENT_REPORT_HPP
#include <QString>
#include <QStringList>
#include <QRegularExpression>
namespace wsjtz {
// Accept only an explicit message addressed to us by the selected peer.
inline QString jttyReceivedReport(QString const& text,QString const& hisCall,QString const& myCall) {
 if(hisCall.trimmed().isEmpty() || myCall.trimmed().isEmpty())return {};
 auto words=text.toUpper().simplified().split(' ',Qt::SkipEmptyParts);
 if(words.size()<3 || words[0]!=myCall.trimmed().toUpper())return {};
 int sender=words[1]=="DE"?2:1;
 if(words.size()<=sender+1 || words[sender]!=hisCall.trimmed().toUpper())return {};
 static QRegularExpression const report {"^R?([+-][0-9]{2}|[1-5][1-9][1-9])$"};
 auto match=report.match(words[sender+1]);
 return match.hasMatch()?match.captured(1):QString {};
}
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
