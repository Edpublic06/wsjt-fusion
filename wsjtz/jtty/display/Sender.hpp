// Fusion JTTY sender identification; deliberately conservative for free text.
// GPL-3.0-or-later.
#ifndef FUSION_JTTY_SENDER_HPP
#define FUSION_JTTY_SENDER_HPP
#include <QString>
#include <QStringList>
#include <QRegularExpression>
namespace wsjtz {
template<class IsCall> QString jttySender(QString const& message,IsCall isCall) {
  static QRegularExpression const token {"[A-Z0-9]+(?:/[A-Z0-9]+)*"};
  QStringList words;
  auto matches=token.globalMatch(message.toUpper());
  while(matches.hasNext())words.append(matches.next().captured());
  // Explicit identification takes precedence over destination and repeated calls.
  for(int i=0;i+1<words.size();++i)
    if(words[i]=="DE" && isCall(words[i+1]))return words[i+1];
  if(!words.isEmpty() && words[0]=="CQ") {
    int i=1;
    if(i<words.size() && !isCall(words[i]))++i; // CQ DX / region
    if(i<words.size() && isCall(words[i]))return words[i];
  }
  if(words.size()>=2 && isCall(words[0]) && isCall(words[1]))return words[1];
  return {}; // Reports, fragments and arbitrary mentions cannot identify a sender.
}
}
#endif
